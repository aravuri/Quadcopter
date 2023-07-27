#include "sensor.h"
#include <pigpio.h>
#include <vector>

void Reciever::poll(int pin, int level, uint32_t tick) {
    if (level == 1) {
        riseTick[pin] = tick;
    } else if (level == 0) {
        pulseWidth[pin] = tick - riseTick[pin];
    }
}

Reciever::Reciever(std::vector<int> _pins) : pins(_pins)  {
    for (auto pin : pins) {
        gpioSetMode(pin, PI_INPUT);
        gpioSetAlertFunc(pin, poll);
    }
}

int Reciever::getPulseWidth(int pin) {
    return pulseWidth[pin];
}

double Reciever::getValue(int pin) {
    return (pulseWidth[pin] - MIN_PWM) / (double) (MAX_PWM - MIN_PWM);
}