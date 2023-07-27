//
// Created by Muralidhar Ravuri on 2018-11-06.
//

#ifndef SENSOR_PID_HPP
#define SENSOR_PID_HPP

#include <utils/misc.hpp>

class Controller {
public:
    virtual double control(double reference, double sensor) = 0;
};

/**
 * K(t) = Kp * error + Ki * integral(error * dt) + Kd * d(error)/dt
 */
class PID : public Controller {
private:
    const float kp;
    const float ki;
    const float kd;

    uint64_t lastTimestamp = currentMicroSecondsSinceEpoch();
    double lastError = 0.0;
    double accumulatedError = 0.0;

public:
    PID(float kp, float ki, float kd) : kp(kp), ki(ki), kd(kd) {
    }

    double control(double reference, double sensor) override {
        uint64_t currentTimestamp = currentMicroSecondsSinceEpoch();
        float delta_t = (currentTimestamp - lastTimestamp) / 1000000.0f;

        double output;
        double error = reference - sensor;
        accumulatedError += error * delta_t;

        output = kp * error;
        output += ki * accumulatedError;
        output += kd * (error - lastError) / delta_t;

        lastTimestamp = currentTimestamp;
        lastError = error;
        return output;
    }
};

#endif // SENSOR_PID_HPP
