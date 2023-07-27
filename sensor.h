#ifndef SENSOR_MODULE
#define SENSOR_MODULE

#include <pigpio.h>
#include <vector>

#define NUM_GPIO_PINS 26

#define MIN_PWM 1000
#define MAX_PWM 2000

class Reciever {
    inline static uint32_t riseTick[NUM_GPIO_PINS] = {0};
    inline static uint32_t pulseWidth[NUM_GPIO_PINS] = {0};

    static void poll(int pin, int level, uint32_t tick);

    public:
    const std::vector<int> pins;

    Reciever(std::vector<int> _pins);

    int getPulseWidth(int pin);

    //returns a value between 0.0 and 1.0
    double getValue(int pin);

};

#endif