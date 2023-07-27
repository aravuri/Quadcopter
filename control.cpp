#include "control.h"
#include <math.h>

PIDController::PIDController(double _kp, double _ki, double _kd, double _minOutput, double _maxOutput, ErrorMode _mode)
 : kp(_kp), ki(_ki), kd(_kd), minOutput(_minOutput), maxOutput(_maxOutput), mode(_mode) {
    integral = 0;
    prevError = 0;
}

double PIDController::update(double targetValue, double measuredValue, double dt) {
    double error;
    if (mode == linear) {
        error = linearError(targetValue, measuredValue);
    } else if (mode == angular) {
        error = angularError(targetValue, measuredValue);
    }
    integral += error*dt;
    double derivative = (error - prevError)/dt;
    
    prevError = error;
    return kp*error + ki*integral + kd*derivative;
}

double linearError(double targetValue, double measuredValue) {
    return targetValue - measuredValue;
}

double angularError(double targetValue, double measuredValue) {
    double nonWrapLength = std::abs(targetValue - measuredValue);
    if (nonWrapLength <= 180) {
        return targetValue - measuredValue;
    }
    return (360 - nonWrapLength) * (measuredValue > targetValue ? 1 : -1);
}

