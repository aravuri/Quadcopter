//
// Created by Muralidhar Ravuri on 10/30/18.
//

#ifndef SENSOR_SENSORDATA_HPP
#define SENSOR_SENSORDATA_HPP

#include <stream/ewma.hpp>
#include <utils/misc.hpp>
#include <utils/json.hpp>

using nlohmann::json;
using namespace std;

struct DeviceValue {
private:
    long long timestamp;
    double rawValue = 0.0;
    EWMA ewma;

public:
    double get() {
        return ewma.get();
    }

    void set(double value) {
        rawValue = value;
        ewma.apply(value);
    }

    double getRawValue() {
        return rawValue;
    }

    void setRawValue(double value) {
        rawValue = value;
    }

    json toJson() {
        json j;
        j["timestamp"] = timestamp;
        j["rawValue"] = rawValue;
        j["ewmaValue"] = ewma.get();
        return j;
    }
};

template<class T>
class DeviceData {
public:
    const unsigned int k;           // number of values to store in a circular buffer
    int currentIndex = -1;          // index of most recent value: circular buffer in the reverse direction
    vector<T *> values;             // list of k sensor values

    explicit DeviceData(unsigned int k) : k(k), values() {
        for (int i = 0; i < k; ++i) {
            values.emplace_back(new T());
        }
    }

    virtual ~DeviceData() {
        for (int i = 0; i < k; ++i) {
            delete values[i];
        }
    }

    virtual T *getCurrentValue() {
        return values[currentIndex];
    }

    virtual string toString() {
        json j;
        j["currentIndex"] = currentIndex;
        for (int i = 0; i < values.size(); ++i) {
            j["values"].emplace_back(values[i]->toJson());
        }
        return j.dump();
    }
};

#endif // SENSOR_SENSORDATA_HPP
