//
// Created by Muralidhar Ravuri on 10/26/18.
//

#ifndef IMU_HPP_
#define IMU_HPP_

#include <sstream>
#include <sensor/imuDefs.h>
#include <device/i2c.hpp>
#include <utils/math.hpp>

template<class T>
class IMU {

protected:
    I2CDevice i2CDevice;
    unsigned char i2CSlaveAddress;                  // I2C slave address of the imu

    int gyroAccelSampleRate;
    uint64_t gyroAccelSampleInterval;               // interval between samples in microseconds
    int compassSampleRate;

    double compassAdjust[3];                         // the compass fuse ROM values converted for use
    unsigned char gyroLowPassFilter;
    unsigned char accelLowPassFilter;

    unsigned char gyroFullScaleRange;
    unsigned char accelFullScaleRange;
    unsigned char compassFullScaleRange;

    double gyroScale;
    double accelScale;
    double compassScale;

public:
    IMU() {
        i2CDevice.i2CBus = 1;
    }

    bool discover() {
        unsigned char result;
        if (i2CDevice.deviceOpen()) {
            if (i2CDevice.deviceRead(MPU9250_ADDRESS0, MPU9250_WHO_AM_I, &result, "", 1)) {
                if (result == MPU9250_ID) {
                    i2CSlaveAddress = MPU9250_ADDRESS0;
                    cout << "Detected MPU9250 at " << hex << (int) i2CSlaveAddress << " address" << dec << endl;
                    return true;
                }
            }

            if (i2CDevice.deviceRead(MPU9250_ADDRESS1, MPU9250_WHO_AM_I, &result, "", 1)) {
                if (result == MPU9250_ID) {
                    i2CSlaveAddress = MPU9250_ADDRESS1;
                    cout << "Detected MPU9250 at " << hex << (int) i2CSlaveAddress << " address" << dec << endl;
                    return true;
                }
            }
        }
        return false;
    }

    virtual bool imuInit() = 0;                          // set up the IMU
    virtual int getPollInterval() = 0;                   // returns the recommended poll interval in mS
    virtual bool read(double &delta_t, T *imuData) = 0;  // get a sample

    virtual void applyFilters(double &delta_t, T *imuData) {
        // compute integral of gyro to get angle
        // q_omega(t + 1) = q_omega(t) * q(delta_t * || omega_t ||, omega_t / || omega_t ||)
        imuData->gyroAngle = imuData->rateIntegral.apply(delta_t, imuData->gyroRaw, &imuData->thetaCompFilter);

        // use accelerometer to correct for the tilt
        // q(t) = q(phi, n / || n ||)
        //      gravity = (0, 0, 1)
        //      qa_world = q_omega(t + 1) * accel_t * inverse(q_omega(t + 1))
        //      v = vector(normalize(qa_world))
        //      n = v x gravity: cross product
        //      phi = acos(v . gravity): dot product
        Vector3 gravity(0, 0, 1);
        Quaternion qaWorld = imuData->gyroAngle.rotate(imuData->accelRaw);
        qaWorld.normalize();
        Vector3 v = qaWorld.vector();
        Vector3 n;
        Vector3::crossProduct(v, gravity, n);
        n.normalize();
        double phi = acos(Vector3::dotProduct(v, gravity));
        imuData->accelAngle.fromAngleVector(phi, n);

        // complementary filter
        // q_c(t) = q((1 - alpha) * phi, n) * q_omega(t + 1)
        double alpha = 0.9;
        Quaternion q_alpha;
        q_alpha.fromAngleVector((1.0 - alpha) * phi, n);
        imuData->thetaCompFilter = q_alpha * imuData->gyroAngle;
    }

    void calibrate(Vector3 &gyroMean, Vector3 &gyroVariance, Vector3 &accelMean, Vector3 &accelVariance) {
        T imuData;
        int numSamples = 1000;
        Vector3 gyroSum{}, accelSum{};
        Vector3 gyroSum2{}, accelSum2{};
        double delta_t = 0.0;
        for (int i = 0; i < numSamples; ++i) {
            while (!read(delta_t, &imuData)) {
                usleep(static_cast<useconds_t>(getPollInterval() * 1000));
            }
            gyroSum += imuData.gyroRaw;
            gyroSum2 += imuData.gyroRaw * imuData.gyroRaw;
            accelSum += imuData.accelRaw;
            accelSum2 += imuData.accelRaw * imuData.accelRaw;
        }
        gyroMean = gyroSum * (1.0 / numSamples);
        gyroVariance = gyroSum2 * (1.0 / (numSamples - 1.0)) -
                       gyroMean * gyroMean * (numSamples / (numSamples - 1.0));
        accelMean = accelSum * (1.0 / numSamples);
        accelVariance = accelSum2 * (1.0 / (numSamples - 1.0)) -
                        accelMean * accelMean * (numSamples / (numSamples - 1.0));

        cout << "gyro mean = " << gyroMean.toString() << endl;
        cout << "gyro variance = " << gyroVariance.toString() << endl;
        cout << "accel mean = " << accelMean.toString() << endl;
        cout << "accel variance = " << accelVariance.toString() << endl;
    }

protected:
    virtual void setDefaults() {
        i2CSlaveAddress = 0;

        gyroAccelSampleRate = 80;
        compassSampleRate = 40;

        gyroScale = 0.0;
        accelScale = 0.0;
        compassScale = 0.0;
    }

};

#endif /* IMU_HPP_ */
