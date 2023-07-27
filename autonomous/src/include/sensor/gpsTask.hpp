//
// Created by Muralidhar Ravuri on 10/30/18.
//

#ifndef SENSOR_GPSTASK_HPP
#define SENSOR_GPSTASK_HPP

#include <device/uart.hpp>
#include <core/deviceTask.hpp>
#include <utils/misc.hpp>
#include <utils/json.hpp>

using nlohmann::json;

struct GPSValue {
    long long timestamp = currentMicroSecondsSinceEpoch();          // from epoch
    double latitude = 0.0;                                          // degrees
    char latitudeHemisphere = 'N';                                  // N/S
    double longitude = 0.0;                                         // degrees
    char longitudeHemisphere = 'W';                                 // E/W
    int numSatellites = 0;
    double altitude = 0;                                            // in meters

public:
    virtual ~GPSValue() = default;

    /**
     * Convert degrees and minutes to degrees.
     */
    double toLocation(const string &data) {
        double value = stod(data);
        value = value / 100;
        double decimal = 100 * (value - int(value)) / 60;
        return int(value) + decimal;
    }

    json toJson() {
        json j;
        j["timestamp"] = timestamp;
        j["latitude"] = to_string(latitude) + latitudeHemisphere;
        j["longitude"] = to_string(longitude) + longitudeHemisphere;
        j["altitude"] = altitude;
        j["numSatellites"] = numSatellites;
        return j;
    }

    static GPSValue fromJson(string &value) {
        json j = json::parse(value);

        string s;
        GPSValue data;
        data.timestamp = j["timestamp"];

        string lat = j["latitude"];
        data.latitude = stod(lat.substr(0, lat.size() - 1));
        data.latitudeHemisphere = lat[lat.size() - 1];

        string lon = j["longitude"];
        data.longitude = stod(lon.substr(0, lon.size() - 1));
        data.longitudeHemisphere = lon[lon.size() - 1];

        data.numSatellites = j["numSatellites"];
        data.altitude = j["altitude"];
        return data;
    }

};

class GPSSensorTask : public DeviceTask<GPSValue> {
private:
    UART uart;
    string lastState;

public:
    GPSSensorTask(string deviceName, const int &samplingFrequency, const unsigned int k = 1)
        : DeviceTask(samplingFrequency, k), uart(std::move(deviceName)), lastState("") {
    }

protected:
    void fetch() override {
        DeviceTask::fetch();
        receive();
    }

    void receive() {
        if (!uart.isDeviceOpen()) {
            return;
        }
        auto *gpsData = result->getCurrentValue();
        string block(lastState);
        string key("$GPGLL");
        while (true) {
            char buffer[128];
            ssize_t length = uart.receive(buffer, 127);
            if (length > 0) {
                string row(buffer, static_cast<unsigned long>(length));
                block += row;
                size_t found = block.find(key, 1); // search after the first character to find second occurrence
                if (found != string::npos) {
                    lastState = block.substr(found);
                    block = block.substr(0, found);
                    getGPSData(block, gpsData);
                    return;
                }
                if (block.size() >= 1024) {
                    cout << "Too large GPS block" << endl;
                    break;
                }
            }
        }
    }

private:
    void getGPSData(string &block, GPSValue *data) {
        string gpsMinSpecRow = getGPSRow(block, "$GPRMC");
        vector<string> gpsMinSpecRowParts = split(gpsMinSpecRow, ',');
        if (gpsMinSpecRowParts.size() < 10) {
            return;
        }
        time_t timestamp = timeSinceEpoch(gpsMinSpecRowParts[9], gpsMinSpecRowParts[1]);

        string gpsFixRow = getGPSRow(block, "$GPGGA");

        // $GPGGA,050105.00,3720.61633,N,12200.67448,W,2,11,0.86,48.8,M,-30.0,M,,0000*57
        //      label,utc:hhmmss.ss,latitude:dddmm.mm,N/S,longitude:dddmm.mm,E/W,quality,numSatellites,
        //      horizontalDilutionPrecision,altitude,altitudeUnits,referenceFrequencyOffset,geoidAltitudeUnits,
        //      ageDifferentialGPS,differentialStationID,checkSum
        vector<string> gpsFixRowParts = split(gpsFixRow, ',');
        if (gpsFixRowParts.size() < 10) {
            return;
        }

        data->timestamp = timestamp;
        data->latitude = data->toLocation(gpsFixRowParts[2]);
        data->latitudeHemisphere = gpsFixRowParts[3][0];
        data->longitude = data->toLocation(gpsFixRowParts[4]);
        data->longitudeHemisphere = gpsFixRowParts[5][0];
        data->numSatellites = stoi(gpsFixRowParts[7]);
        data->altitude = stoi(gpsFixRowParts[9]);
    }

    string getGPSRow(string &block, const char *gpsKey) {
        size_t found = block.find(gpsKey);
        if (found != string::npos) {
            size_t nextline_begin = block.find("$GP", found + 1);
            if (nextline_begin != string::npos) {
                return block.substr(found, nextline_begin);
            } else {
                return block.substr(found);
            }
        } else {
            return "";
        }
    }

    time_t timeSinceEpoch(string &gpsDate, string &gpsTime) {
        int gpsDateValue = stoi(gpsDate); // ddmmyy
        double gpsTimeValue = stod(gpsTime); // hhmmss.ss
        double decimal = gpsTimeValue - int(gpsTimeValue);
        if (decimal > 0) {
            cout << "GPS time in milliseconds = " << decimal * 1000 << endl;
        }

        struct tm t{};
        t.tm_year = 100 + gpsDateValue % 100;
        t.tm_mon = ((gpsDateValue / 100) % 100) - 1;
        t.tm_mday = gpsDateValue / 10000;
        t.tm_hour = int(gpsTimeValue) / 10000;
        t.tm_min = (int(gpsTimeValue) / 100) % 100;
        t.tm_sec = int(gpsTimeValue) % 100;
        t.tm_wday = t.tm_yday = t.tm_isdst = 0;

        setenv("TZ", "UTC", 1);
        return mktime(&t);
    }

};

#endif // SENSOR_GPSTASK_HPP
