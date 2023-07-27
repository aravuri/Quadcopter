#ifndef ESC_MODULE
#define ESC_MODULE

#define ESC_MIN 1000
#define ESC_MAX 2000
#define SAFETY_LIMIT 1200

#define MAX_SPEED 1000

#include <vector>

class ESC { 
    public:
    const int pin;

    ESC(int _pin);

    void calibrate();

    void arm();

    //speed is an integer from 0 to MAX_SPEED
    void run(int speed);

};

struct Quadcopter {
    ESC CW_MOTOR_F, CCW_MOTOR_F, CW_MOTOR_B, CCW_MOTOR_B;

    
};

void armAll(std::vector<ESC> motors);
void calibrateAll(std::vector<ESC> motors);

#endif