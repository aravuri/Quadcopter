//
// Created by Muralidhar Ravuri on 2018-11-09.
//

#ifndef SENSOR_QUADCONTROLTASK_HPP
#define SENSOR_QUADCONTROLTASK_HPP

#include <core/deviceTask.hpp>
#include <utils/math.hpp>
#include <sensor/imuTask.hpp>
#include <control/pid.hpp>

struct ControlValue {
    long long timestamp;
    Vector3 attitudeControl;
    double altitudeControl;
    Vector3 referenceAttitude;
    double referenceAltitude;

public:
    ControlValue() : timestamp(currentMicroSecondsSinceEpoch()) {
    }

    json toJson() {
        json j;
        j["timestamp"] = timestamp;
        j["attitudeControl"] = {{"yaw",   attitudeControl.x()},
                                {"pitch", attitudeControl.y()},
                                {"roll",  attitudeControl.z()}};
        j["altitudeControl"] = altitudeControl;
        j["referenceAttitude"] = {{"yaw",   referenceAttitude.x()},
                                  {"pitch", referenceAttitude.y()},
                                  {"roll",  referenceAttitude.z()}};
        j["referenceAltitude"] = referenceAltitude;
        return j;
    }
};

class QuadControlTask : public DeviceTask<ControlValue> {
private:
    IMUSensorTask &imuSensorTask;

    Vector3 referenceAttitude{};
    double referenceAltitude = 0.0;

    PID yawController{1.0, 1.0, 1.0};
    PID pitchController{1.0, 1.0, 1.0};
    PID rollController{1.0, 1.0, 1.0};
    PID altitudeController{1.0, 1.0, 1.0};

public:
    QuadControlTask(const int &samplingFrequency, const unsigned int k, IMUSensorTask &imuSensorTask)
        : DeviceTask(samplingFrequency, k), imuSensorTask(imuSensorTask) {
    }

    void setReference(double yaw, double pitch, double roll, double altitude) {
        boost::lock_guard<boost::mutex> lk(mtx);
        referenceAttitude.setX(yaw);
        referenceAttitude.setY(pitch);
        referenceAttitude.setZ(roll);
        referenceAltitude = altitude;

        auto *controlData = result->getCurrentValue();
        controlData->referenceAttitude = referenceAttitude;
        controlData->referenceAltitude = altitude;
    }

protected:
    void fetch() override {
        DeviceTask::fetch();
        control();
    }

    void control() {
        auto *controlData = result->getCurrentValue();

        IMUValue imuData = imuSensorTask.getData();
        Vector3 eulerAngles;
        imuData.thetaCompFilter.toEuler(eulerAngles);

        controlData->attitudeControl.setX(yawController.control(referenceAttitude.x(), eulerAngles.x()));
        controlData->attitudeControl.setY(pitchController.control(referenceAttitude.y(), eulerAngles.y()));
        controlData->attitudeControl.setZ(rollController.control(referenceAttitude.z(), eulerAngles.z()));
        controlData->altitudeControl = altitudeController.control(referenceAltitude, 0.0);
    }

};

#endif //SENSOR_QUADCONTROLTASK_HPP
