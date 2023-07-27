//
// Created by Muralidhar Ravuri on 10/30/18.
//

#ifndef IMU_TASK_HPP_
#define IMU_TASK_HPP_

#include <core/deviceTask.hpp>
#include <sensor/mpu9250.hpp>
#include <utils/json.hpp>
#include <stream/rateIntegral.hpp>
#include <stream/complementaryFilter.hpp>

struct IMUValue {
    long long timestamp;
    Vector3 gyroRaw;
    Vector3 accelRaw;
    Vector3 compassRaw;
    Quaternion gyroAngle;                       // integrate gyro
    Quaternion accelAngle;                      // correct tilt using accelerometer
    Quaternion thetaCompFilter{1, 0, 0, 0};     // complementary filter

    RateIntegral rateIntegral;

public:
    IMUValue() : timestamp(currentMicroSecondsSinceEpoch()), rateIntegral() {
    }

    json toJson() {
        json j;
        j["timestamp"] = timestamp;
        j["gyroRaw"] = {{"x", gyroRaw.x()},
                        {"y", gyroRaw.y()},
                        {"z", gyroRaw.z()}};
        j["accelRaw"] = {{"x", accelRaw.x()},
                         {"y", accelRaw.y()},
                         {"z", accelRaw.z()}};
        j["compassRaw"] = {{"x", compassRaw.x()},
                           {"y", compassRaw.y()},
                           {"z", compassRaw.z()}};
        j["gyroAngle"] = {{"scalar", gyroAngle.scalar()},
                          {"x",      gyroAngle.x()},
                          {"y",      gyroAngle.y()},
                          {"z",      gyroAngle.z()}};
        j["accelAngle"] = {{"scalar", accelAngle.scalar()},
                           {"x",      accelAngle.x()},
                           {"y",      accelAngle.y()},
                           {"z",      accelAngle.z()}};
        j["thetaCompFilter"] = {{"scalar", thetaCompFilter.scalar()},
                                {"x",      thetaCompFilter.x()},
                                {"y",      thetaCompFilter.y()},
                                {"z",      thetaCompFilter.z()}};
        return j;
    }
};

class IMUSensorTask : public DeviceTask<IMUValue> {
private:
    MPU9250<IMUValue> imu{};

public:
    IMUSensorTask(const int &samplingFrequency, const unsigned int k) : DeviceTask(samplingFrequency, k) {
        if (!imu.discover()) {
            cout << "No IMU found" << endl;
            exit(1);
        }
        imu.imuInit();
    }

protected:
    void fetch() override {
        DeviceTask::fetch();
        receive();
    }

    void receive() {
        auto *imuData = result->getCurrentValue();
        double delta_t = 0.0;
        // wait till you can read
        while (!imu.read(delta_t, imuData)) {
            usleep(static_cast<useconds_t>(imu.getPollInterval() * 1000));
        }
        imu.applyFilters(delta_t, imuData);
    }

};

#endif /* IMU_TASK_HPP_ */
