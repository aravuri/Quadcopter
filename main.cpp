#include <iostream>
#include <pigpio.h>
#include <omp.h>
#include <unistd.h>
#include <RTIMULib.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include "esc.h"
#include "control.h"
#include "sensor.h"

#define RAD_TO_DEGREE 57.2957795131

#define CW_MOTOR_F_PIN 18
#define CCW_MOTOR_F_PIN 21
#define CW_MOTOR_B_PIN 26
#define CCW_MOTOR_B_PIN 4

#define THROTTLE_PIN 6
#define YAW_PIN 5
#define PITCH_PIN 13
#define ROLL_PIN 19

#define MAX_TILT 30

void initGPIO() {
    if (gpioInitialise() < 0) {
        std::cerr << "Failed to initialize pigpio\n";
        exit(1);
    }
}

std::vector<ESC> initMotors() {

    ESC CW_MOTOR_F(CW_MOTOR_F_PIN);
    ESC CCW_MOTOR_F(CCW_MOTOR_F_PIN);
    ESC CW_MOTOR_B(CW_MOTOR_B_PIN);
    ESC CCW_MOTOR_B(CCW_MOTOR_B_PIN);

    std::vector<ESC> motors{CW_MOTOR_F, CCW_MOTOR_F, CW_MOTOR_B, CCW_MOTOR_B};
    armAll(motors);

    return motors;
}

RTIMU* initIMU() {
    RTIMUSettings *settings = new RTIMUSettings("RTIMULib");
    RTIMU *imu = RTIMU::createIMU(settings);

    if ((imu == NULL) || (imu->IMUType() == RTIMU_TYPE_NULL)) {
        printf("No IMU found\n");
        exit(1);
    }

    //  Initialization
    if (!imu->IMUInit())
    {
        printf("Failed to Init IMU\n");
        exit(1);
    }

    //  This is an opportunity to manually override any settings before the call IMUInit

    //  Set up the rates
    imu->setSlerpPower(0.02);
    imu->setGyroEnable(true);
    imu->setAccelEnable(true);
    imu->setCompassEnable(true);

    return imu;
}

void testIMU() {
   RTIMU* imu = initIMU();

    //initialize the yaw, pitch, and roll
    double initRoll = 0.0, initPitch = 0.0, initYaw = 0.0;
    int count = 0;
    while (count < 1000) {
        usleep(imu->IMUGetPollInterval() * 1000);
        while (imu->IMURead()) {
            RTIMU_DATA imuData = imu->getIMUData();
            initRoll = imuData.fusionPose.x()*RAD_TO_DEGREE;
            initPitch = imuData.fusionPose.y()*RAD_TO_DEGREE;
            initYaw = imuData.fusionPose.z()*RAD_TO_DEGREE;
            std::cout << initRoll << " " << initPitch << " " << initYaw << std::endl;
        }
        if (initRoll != 0.0) count++;
    }

    while (1) {
        usleep(imu->IMUGetPollInterval() * 1000);
        while (imu->IMURead()) {
            RTIMU_DATA imuData = imu->getIMUData();
            double globalRoll = imuData.fusionPose.x() * RAD_TO_DEGREE;
            double globalPitch = imuData.fusionPose.y() * RAD_TO_DEGREE;
            double globalYaw = imuData.fusionPose.z() * RAD_TO_DEGREE;

            double roll = angularError(globalRoll, initRoll);
            double pitch = angularError(globalPitch, initPitch);
            double yaw = angularError(globalYaw, initYaw);
            printf("global roll:%f pitch:%f yaw:%f\n", globalRoll, globalPitch, globalYaw);
            printf("roll:%f pitch:%f yaw:%f\n", roll, pitch, yaw);
        }
    }
}

Reciever* initReciever() {
    initGPIO();

    std::vector<int> recieverPins = {THROTTLE_PIN, YAW_PIN, PITCH_PIN, ROLL_PIN};
    Reciever* reciever = new Reciever(recieverPins);

    return reciever;

}

int normalizeThrust(double thrustValue) {
    return (int) std::round(std::min(std::max(thrustValue, 0.0), (double) MAX_SPEED));
}

double normalizeRecieverAngle(double recieverValue) {
    return (recieverValue - 0.5) * 2 * MAX_TILT;
}

double normalizeRecieverThrottle(double recieverValue) {
    return recieverValue * MAX_SPEED;
}


int main() {
    //initialize stuff
    initGPIO();
    std::vector<ESC> motors = initMotors();

    // for (int speed = 0; speed < 1000; speed+=100) {
    //     motors[0].run(speed);
    //     motors[1].run(speed);
    //     motors[2].run(speed);
    //     motors[3].run(speed);
    //     sleep(1);
    // }

    // std::vector<ESC> motors;

    RTIMU* imu = initIMU();
    Reciever* reciever = initReciever();

    //kp, ki, kd, min output, max output
    PIDController rollController(15, 0, 0, -100, 100, angular);
    PIDController pitchController(15, 0, 0, -100, 100, angular);
    PIDController yawController(0.2, 0, 0, -100, 100, angular);

    // initialize the yaw, pitch, and roll
    double initRoll = 0.0, initPitch = 0.0, initYaw = 0.0;
    int count = 0;
    while (count < 1000) {
        usleep(imu->IMUGetPollInterval() * 1000);
        while (imu->IMURead()) {
            RTIMU_DATA imuData = imu->getIMUData();
            initRoll = imuData.fusionPose.x()*RAD_TO_DEGREE;
            initPitch = imuData.fusionPose.y()*RAD_TO_DEGREE;
            initYaw = imuData.fusionPose.z()*RAD_TO_DEGREE;
            std::cout << initRoll << " " << initPitch << " " << initYaw << std::endl;
        }
        if (initRoll != 0.0) count++;
    }

    //control loop
    while (1) {
        int dt = imu->IMUGetPollInterval();
        usleep(dt * 1000);
        while (imu->IMURead()) {
            RTIMU_DATA imuData = imu->getIMUData();
            double globalRoll = imuData.fusionPose.x() * RAD_TO_DEGREE;
            double globalPitch = imuData.fusionPose.y() * RAD_TO_DEGREE;
            double globalYaw = imuData.fusionPose.z() * RAD_TO_DEGREE;

            double roll = angularError(globalRoll, initRoll);
            double pitch = angularError(globalPitch, initPitch);
            double yaw = angularError(globalYaw, initYaw);

            double desiredRoll = -normalizeRecieverAngle(reciever->getValue(ROLL_PIN));
            double desiredPitch = normalizeRecieverAngle(reciever->getValue(PITCH_PIN));
            double throttle = normalizeRecieverThrottle(reciever->getValue(THROTTLE_PIN));

            printf("global roll:%f pitch:%f yaw:%f\n", globalRoll, globalPitch, globalYaw);
            printf("roll:%f pitch:%f yaw:%f\n", roll, pitch, yaw);
            printf("desired roll:%f pitch:%f throttle:%f\n", desiredRoll, desiredPitch, throttle);

            double rollCorrection = rollController.update(desiredRoll, roll, dt);
            double pitchCorrection = pitchController.update(desiredPitch, pitch, dt);
            double yawCorrection = yawController.update(0, yaw, dt);
            printf("correct factors for roll:%f pitch:%f yaw:%f\n", rollCorrection, pitchCorrection, yawCorrection);

            int CW_MOTOR_F_THRUST = normalizeThrust(throttle + rollCorrection - pitchCorrection + yawCorrection);
            int CCW_MOTOR_F_THRUST = normalizeThrust(throttle - rollCorrection - pitchCorrection - yawCorrection);
            int CW_MOTOR_B_THRUST = normalizeThrust(throttle - rollCorrection + pitchCorrection + yawCorrection);
            int CCW_MOTOR_B_THRUST = normalizeThrust(throttle + rollCorrection + pitchCorrection - yawCorrection);
            printf("CW_MOTOR_F_THRUST:%d\n", CW_MOTOR_F_THRUST);
            printf("CCW_MOTOR_F_THRUST:%d\n", CCW_MOTOR_F_THRUST);
            printf("CW_MOTOR_B_THRUST:%d\n", CW_MOTOR_B_THRUST);
            printf("CCW_MOTOR_B_THRUST:%d\n", CCW_MOTOR_B_THRUST);

            motors[0].run(CW_MOTOR_F_THRUST);
            motors[1].run(CCW_MOTOR_F_THRUST);
            motors[2].run(CW_MOTOR_B_THRUST);
            motors[3].run(CCW_MOTOR_B_THRUST);
        }
    }
    gpioTerminate();

}
