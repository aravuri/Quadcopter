#ifndef CONTROL_MODULE
#define CONTROL_MODULE

enum ErrorMode {
    linear, angular
};

class PIDController {
    public:
    const double kp;
    const double ki;
    const double kd;
    const double minOutput;
    const double maxOutput;
    double integral;
    double prevError;
    ErrorMode mode;

    PIDController(double _kp, double _ki, double _kd, double _minOutput, double _maxOutput, ErrorMode _mode);

    double update(double targetValue, double measuredValue, double dt);

};

double linearError(double targetValue, double measuredValue);
double angularError(double targetValue, double measuredValue);

#endif