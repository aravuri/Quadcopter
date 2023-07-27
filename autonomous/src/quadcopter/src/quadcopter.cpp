//
// Created by Muralidhar Ravuri on 11/5/18.
//

#include <boost/asio.hpp>
#include <sensor/gpsTask.hpp>
#include <sensor/imuTask.hpp>
#include <device/pwm.hpp>
#include <control/pid.hpp>
#include <core/baseServer.hpp>
#include <control/quadControlTask.hpp>

#define MOTOR_FRONT                     19
#define MOTOR_LEFT                      26
#define MOTOR_BACK                      20
#define MOTOR_RIGHT                     16

#define THREAD_POOL_COUNT               7

#define GPS_DEVICE_NAME                 "/dev/serial0"
#define GPS_SERVER_FREQUENCY            10              // frequency in Hz
#define IMU_SERVER_FREQUENCY            100             // frequency in Hz
#define QUAD_CONTROL_FREQUENCY          20              // frequency in Hz
#define MOTOR_PWM_FREQUENCY             5000            // PWM frequency

// seeing weird oscillations when NUM_SAMPLES > 1 - possibly because of Nyquist theorem
#define NUM_SAMPLES                     1               // number of samples in circular buffer to store

#define HOSTNAME                        "localhost"

const unsigned short gpsPort = 5000;
const unsigned short imuPort = 5001;
const unsigned short controlPort = 5002;

boost::asio::thread_pool threadPool(THREAD_POOL_COUNT);

class Quadcopter {
public:
    bool isShutdown = false;

    GPSSensorTask gpsSensorTask;
    IMUSensorTask imuSensorTask;
    QuadControlTask quadControlTask;

    PWM motorFront;
    PWM motorLeft;
    PWM motorBack;
    PWM motorRight;

public:
    Quadcopter() : gpsSensorTask(GPS_DEVICE_NAME, GPS_SERVER_FREQUENCY, NUM_SAMPLES),
                   imuSensorTask(IMU_SERVER_FREQUENCY, NUM_SAMPLES),
                   quadControlTask(QUAD_CONTROL_FREQUENCY, NUM_SAMPLES, imuSensorTask),
                   motorFront(MOTOR_FRONT, MOTOR_PWM_FREQUENCY),
                   motorLeft(MOTOR_LEFT, MOTOR_PWM_FREQUENCY),
                   motorBack(MOTOR_BACK, MOTOR_PWM_FREQUENCY),
                   motorRight(MOTOR_RIGHT, MOTOR_PWM_FREQUENCY) {
        boost::asio::post(threadPool, boost::bind(&GPSSensorTask::run, &gpsSensorTask));
        boost::asio::post(threadPool, boost::bind(&IMUSensorTask::run, &imuSensorTask));
        boost::asio::post(threadPool, boost::bind(&QuadControlTask::run, &quadControlTask));
    }

    void setup() {
        boost::asio::post(threadPool, boost::bind(&Quadcopter::launchGPSServer, this));
        boost::asio::post(threadPool, boost::bind(&Quadcopter::launchIMUServer, this));
        boost::asio::post(threadPool, boost::bind(&Quadcopter::launchControlServer, this));
    }

    void control() {
        cout << "Starting controller..." << endl;
        unsigned int microSeconds = 1000000 / QUAD_CONTROL_FREQUENCY;
        while (!isShutdown) {
            ControlValue controlValue = quadControlTask.getData();

            motorFront.set(controlValue.attitudeControl.x());
            motorLeft.set(controlValue.attitudeControl.y());
            motorBack.set(controlValue.attitudeControl.z());
            motorRight.set(controlValue.altitudeControl);

            usleep(microSeconds);
        }
    }

    void shutdown() {
        isShutdown = true;
    }

private:
    void launchGPSServer() {
        BaseServer<GPSValue> server(HOSTNAME, gpsPort, gpsSensorTask);
        boost::asio::io_context io;
        cout << "Launching GPS Server on port " << gpsPort << endl;
        server.launch(io);
    }

    void launchIMUServer() {
        BaseServer <IMUValue> server(HOSTNAME, imuPort, imuSensorTask);
        boost::asio::io_context io;
        cout << "Launching IMU Server on port " << imuPort << endl;
        server.launch(io);
    }

    void launchControlServer() {
        BaseServer <ControlValue> server(HOSTNAME, controlPort, quadControlTask);
        boost::asio::io_context io;
        cout << "Launching Control Server on port " << controlPort << endl;
        server.launch(io);
    }

};

int main(int argc, char *argv[]) {
    Quadcopter quadcopter{};
    quadcopter.setup();
    quadcopter.control();
    return 0;
}
