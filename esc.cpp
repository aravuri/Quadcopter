#include <iostream>
#include <pigpio.h>
#include <unistd.h>
#include <omp.h>
#include <vector>
#include "esc.h"

ESC::ESC(int _pin) : pin(_pin) {
    gpioSetMode(pin, PI_OUTPUT);
}

void ESC::calibrate() {
    gpioServo(pin, 0);
    std::cout << "Disconnect the battery and press Enter" << std::endl;
    getchar();
    gpioServo(pin, ESC_MAX);
    std::cout << "Connect the battery NOW. You will hear two beeps. Wait for a gradual falling tone, then press Enter." << std::endl;
    getchar();
    gpioServo(pin, ESC_MIN);
    std::cout << "Special tone..." << std::endl;
    sleep(7);
    std::cout << "Wait for it..." << std::endl;
    sleep(5);
    std::cout << "Setting zero as value..." << std::endl;
    gpioServo(pin, 0);
    sleep(2);
    std::cout << "Arming ESC now..." << std::endl;
    gpioServo(pin, ESC_MIN);
    sleep(1);
    std::cout << "Done calibrating..." << std::endl;
}

void ESC::arm() {
    gpioServo(pin, 0);
    sleep(1);
    gpioServo(pin, ESC_MAX);
    sleep(1);
    gpioServo(pin, ESC_MIN);
    sleep(1);
}

void ESC::run(int speed) {
    if (speed < 0 || speed > MAX_SPEED) {
        throw std::runtime_error("invalid speed bounds: " + std::to_string(speed));
    }
    gpioServo(pin, ESC_MIN + (SAFETY_LIMIT-ESC_MIN)*speed/MAX_SPEED);
}

void armAll(std::vector<ESC> motors) {
    std::cout << "Starting arming sequence..." << std::endl;

    // #pragma omp parallel for
    for (int i = 0; i < 4; i++) {
        motors[i].arm();
        usleep(500000);
    }
    sleep(1);

    std::cout << "Done Arming... will start in 3 seconds" << std::endl;
    sleep(3);
}

void calibrateAll(std::vector<ESC> motors) {
    for (int i = 0; i < 4; i++) {
        gpioServo(motors[i].pin, 0);
    }
    std::cout << "Disconnect the battery and press Enter" << std::endl;
    getchar();
    for (int i = 0; i < 4; i++) {
        gpioServo(motors[i].pin, ESC_MAX);
    }
    std::cout << "Connect the battery NOW. You will hear two beeps. Wait for a gradual falling tone, then press Enter." << std::endl;
    getchar();
    for (int i = 0; i < 4; i++) {
        gpioServo(motors[i].pin, ESC_MIN);
    }
    std::cout << "Special tone..." << std::endl;
    sleep(7);
    std::cout << "Wait for it..." << std::endl;
    sleep(5);
    std::cout << "Setting zero as value..." << std::endl;
    for (int i = 0; i < 4; i++) {
        gpioServo(motors[i].pin, 0);
        sleep(2);
    }
    sleep(2);
    std::cout << "Arming ESC now..." << std::endl;
    for (int i = 0; i < 4; i++) {
        gpioServo(motors[i].pin, ESC_MIN);
        sleep(1);
    }
    sleep(1);
    std::cout << "Done calibrating... Will start in 3 seconds" << std::endl;
    sleep(3);
}