//
// Created by Muralidhar Ravuri on 10/26/18.
//

#ifndef MPU9250_HPP_
#define MPU9250_HPP_

#include <math.h>
#include <sensor/imu.hpp>
#include <utils/misc.hpp>
#include <utils/math.hpp>

#define MPU9250_FIFO_CHUNK_SIZE     12      // gyro and accels take 12 bytes

template<class T>
class MPU9250 : public IMU<T> {
private:
    Vector3 gyroBias{0.000928911, -0.00910854, -0.00138218};                // gyro bias
    Vector3 gyroNoiseVariance{4.23832e-07, 5.52286e-07, 5.5688e-07};        // gyro noise variance
    Vector3 accelBias{0.0160525, 0.0497671, -0.00852};                      // accelerometer bias
    Vector3 accelNoiseVariance{1.19585e-06, 1.3439e-06, 3.41964e-06};       // accelerometer noise variance

public:
    MPU9250() : IMU<T>() {
        setDefaults();
    }

    bool imuInit() override {
        if (!configureParameters()) {
            return false;
        }

        unsigned char result;
        //  enable the bus
        if (!this->i2CDevice.deviceOpen()) {
            return false;
        }
        //  reset the MPU9250
        if (!this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_PWR_MGMT_1, 0x80,
                                         "Failed to initiate MPU9250 reset")) {
            return false;
        }
        this->i2CDevice.delayMs(100);
        if (!this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_PWR_MGMT_1, 0x00,
                                         "Failed to stop MPU9250 reset")) {
            return false;
        }
        if (!this->i2CDevice.deviceRead(this->i2CSlaveAddress, MPU9250_WHO_AM_I, &result,
                                        "Failed to read MPU9250 id", 1)) {
            return false;
        }
        if (result != MPU9250_ID) {
            cerr << "Incorrect MPU9250 id " << result << endl;
            return false;
        }
        //  now configure the various components
        if (!setGyroConfig() || !setAccelConfig() || !compassInit()) {
            return false;
        }
        if (!setGyroAccelSampleRate() || !setCompassSampleRate()) {
            return false;
        }
        //  enable the sensors
        if (!this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_PWR_MGMT_1, 1, "Failed to set pwr_mgmt_1") ||
            !this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_PWR_MGMT_2, 0, "Failed to set pwr_mgmt_2")) {
            return false;
        }
        //  select the data to go into the FIFO and enable
        if (!resetFifo()) {
            return false;
        }

        cout << "MPU9250 init complete" << endl;

        if (gyroBias.isZero()) {
            this->calibrate(gyroBias, gyroNoiseVariance, accelBias, accelNoiseVariance);
        }
        return true;
    }

    int getPollInterval() override {
        if (this->gyroAccelSampleRate > 400) {
            return 1;
        } else {
            return (400 / this->gyroAccelSampleRate);
        }
    }

    bool read(double &delta_t, T *imuData) override {
        unsigned char fifoCount[2];
        unsigned int count;
        unsigned char fifoData[12];
        unsigned char compassData[8];
        bool slowSampling = false;

        if (!this->i2CDevice.deviceRead(this->i2CSlaveAddress, MPU9250_FIFO_COUNT_H, fifoCount,
                                        "Failed to read fifo count", 2)) {
            return false;
        }

        count = ((unsigned int) fifoCount[0] << 8) + fifoCount[1];

        if (count == 512) {
            cout << "MPU-9250 fifo has overflowed" << endl;
            resetFifo();
            imuData->timestamp += this->gyroAccelSampleInterval * (512 / MPU9250_FIFO_CHUNK_SIZE + 1);
            return false;
        }

        if (count > MPU9250_FIFO_CHUNK_SIZE * 40) {
            // more than 40 samples behind - going too slowly so discard some samples but maintain timestamp correctly
            while (count >= MPU9250_FIFO_CHUNK_SIZE * 2) {
                if (!this->i2CDevice.deviceRead(this->i2CSlaveAddress, MPU9250_FIFO_R_W, fifoData,
                                                "Failed to read fifo data", MPU9250_FIFO_CHUNK_SIZE)) {
                    return false;
                }
                count -= MPU9250_FIFO_CHUNK_SIZE;
                imuData->timestamp += this->gyroAccelSampleInterval;
            }
            cout << "MPU-9250 discarding samples... " << count << endl;
            slowSampling = true;
        }

        if (count < MPU9250_FIFO_CHUNK_SIZE) {
            return false;
        }

        if (!this->i2CDevice.deviceRead(this->i2CSlaveAddress, MPU9250_FIFO_R_W, fifoData,
                                        "Failed to read fifo data", MPU9250_FIFO_CHUNK_SIZE) ||
            !this->i2CDevice.deviceRead(this->i2CSlaveAddress, MPU9250_EXT_SENS_DATA_00, compassData,
                                        "Failed to read compass data", 8)) {
            return false;
        }
        long long previousTimestamp = imuData->timestamp;
        if (!slowSampling) {
            imuData->timestamp = currentMicroSecondsSinceEpoch();
        }
        delta_t = (imuData->timestamp - previousTimestamp) / 1000000.0;

        convertToVector(fifoData, imuData->accelRaw, this->accelScale, true);
        convertToVector(fifoData + 6, imuData->gyroRaw, this->gyroScale, true);
        convertToVector(compassData + 1, imuData->compassRaw, 0.6f, false);

        //  sort out gyro axes
        imuData->gyroRaw.setY(-imuData->gyroRaw.y());
        imuData->gyroRaw.setZ(-imuData->gyroRaw.z());

        //  sort out accel data;
        imuData->accelRaw.setX(-imuData->accelRaw.x());
        imuData->accelRaw.setZ(-imuData->accelRaw.z());

        //  use the compass fuse data adjustments
        imuData->compassRaw.setX(imuData->compassRaw.x() * this->compassAdjust[0]);
        imuData->compassRaw.setY(imuData->compassRaw.y() * this->compassAdjust[1]);
        imuData->compassRaw.setZ(imuData->compassRaw.z() * this->compassAdjust[2]);

        //  sort out compass axes
        double temp = imuData->compassRaw.x();
        imuData->compassRaw.setX(imuData->compassRaw.y());
        imuData->compassRaw.setY(-temp);

        // remove bias
        imuData->gyroRaw -= gyroBias;
        imuData->accelRaw -= accelBias;

        return true;
    }

protected:
    void setDefaults() override {
        IMU<T>::setDefaults();

        this->gyroLowPassFilter = MPU9250_GYRO_LPF_41;
        this->accelLowPassFilter = MPU9250_ACCEL_LPF_41;

        this->gyroFullScaleRange = MPU9250_GYROFSR_1000;
        this->accelFullScaleRange = MPU9250_ACCELFSR_8;
        this->compassFullScaleRange = AK8963_FSR_16BITS;
    }

    bool configureParameters() {
        //  configure IMU parameters
        if (!setGyroAccelSampleRate(this->gyroAccelSampleRate, this->gyroAccelSampleInterval,
                                    this->gyroAccelSampleRate) ||
            !setCompassSampleRate(this->compassSampleRate)) {
            return false;
        }

        return setGyroLowPassFilter(this->gyroLowPassFilter) &&
               setAccelLowPassFilter(this->accelLowPassFilter) &&
               setGyroFullScaleRange(this->gyroFullScaleRange) &&
               setAccelFullScaleRange(this->accelFullScaleRange) &&
               setCompassFullScaleRange(this->compassFullScaleRange);
    }

private:
    bool setGyroAccelSampleRate(int &sampleRate, uint64_t &sampleInterval, int rate) {
        if ((rate < MPU9250_SAMPLERATE_MIN) || (rate > MPU9250_SAMPLERATE_MAX)) {
            cerr << "Illegal sample rate " << rate << endl;
            return false;
        }

        // Note: rates interact with the low pass filter settings
        if ((rate < MPU9250_SAMPLERATE_MAX) && (rate >= 8000)) {
            rate = 8000;
        }

        if ((rate < 8000) && (rate >= 1000)) {
            rate = 1000;
        }

        if (rate < 1000) {
            int sampleDiv = (1000 / sampleRate) - 1;
            sampleRate = 1000 / (1 + sampleDiv);
        } else {
            sampleRate = rate;
        }
        sampleInterval = (uint64_t) 1000000 / sampleRate; // microseconds
        return true;
    }

    bool setGyroAccelSampleRate() {
        // SMPRT not used above 1000Hz
        if (this->gyroAccelSampleRate > 1000) {
            return true;
        }
        return this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_SMPRT_DIV,
                                           (unsigned char) (1000 / this->gyroAccelSampleRate - 1),
                                           "Failed to set sample rate");
    }

    bool setGyroLowPassFilter(unsigned char lpf) {
        switch (lpf) {
            case MPU9250_GYRO_LPF_8800:
            case MPU9250_GYRO_LPF_3600:
            case MPU9250_GYRO_LPF_250:
            case MPU9250_GYRO_LPF_184:
            case MPU9250_GYRO_LPF_92:
            case MPU9250_GYRO_LPF_41:
            case MPU9250_GYRO_LPF_20:
            case MPU9250_GYRO_LPF_10:
            case MPU9250_GYRO_LPF_5:
                this->gyroLowPassFilter = lpf;
                return true;
            default:
                cerr << "Illegal MPU9250 gyro low pass filter " << lpf << endl;
                return false;
        }
    }

    bool setGyroFullScaleRange(unsigned char fsr) {
        switch (fsr) {
            case MPU9250_GYROFSR_250:
                this->gyroFullScaleRange = fsr;
                this->gyroScale = M_PI * 250.0 / (32768.0 * 180.0);
                return true;
            case MPU9250_GYROFSR_500:
                this->gyroFullScaleRange = fsr;
                this->gyroScale = M_PI * 500.0 / (32768.0 * 180.0);
                return true;
            case MPU9250_GYROFSR_1000:
                this->gyroFullScaleRange = fsr;
                this->gyroScale = M_PI * 1000.0 / (32768.0 * 180.0);
                return true;
            case MPU9250_GYROFSR_2000:
                this->gyroFullScaleRange = fsr;
                this->gyroScale = M_PI * 2000.0 / (32768.0 * 180.0);
                return true;
            default:
                cerr << "Illegal MPU9250 gyro fsr " << fsr << endl;
                return false;
        }
    }

    bool setGyroConfig() {
        auto gyroConfig = static_cast<unsigned char>(this->gyroFullScaleRange + ((this->gyroLowPassFilter >> 3) & 3));
        auto gyroLpf = static_cast<unsigned char>(this->gyroLowPassFilter & 7);

        return this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_GYRO_CONFIG, gyroConfig,
                                           "Failed to write gyro config") &&
               this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_GYRO_LPF, gyroLpf,
                                           "Failed to write gyro lpf");
    }

    bool setAccelLowPassFilter(unsigned char lpf) {
        switch (lpf) {
            case MPU9250_ACCEL_LPF_1130:
            case MPU9250_ACCEL_LPF_460:
            case MPU9250_ACCEL_LPF_184:
            case MPU9250_ACCEL_LPF_92:
            case MPU9250_ACCEL_LPF_41:
            case MPU9250_ACCEL_LPF_20:
            case MPU9250_ACCEL_LPF_10:
            case MPU9250_ACCEL_LPF_5:
                this->accelLowPassFilter = lpf;
                return true;
            default:
                cerr << "Illegal MPU9250 accel low pass filter " << lpf << endl;
                return false;
        }
    }

    bool setAccelFullScaleRange(unsigned char fsr) {
        switch (fsr) {
            case MPU9250_ACCELFSR_2:
                this->accelFullScaleRange = fsr;
                this->accelScale = 2.0 / 32768.0;
                return true;
            case MPU9250_ACCELFSR_4:
                this->accelFullScaleRange = fsr;
                this->accelScale = 4.0 / 32768.0;
                return true;
            case MPU9250_ACCELFSR_8:
                this->accelFullScaleRange = fsr;
                this->accelScale = 8.0 / 32768.0;
                return true;
            case MPU9250_ACCELFSR_16:
                this->accelFullScaleRange = fsr;
                this->accelScale = 16.0 / 32768.0;
                return true;
            default:
                cerr << "Illegal MPU9250 accel fsr " << fsr << endl;
                return false;
        }
    }

    bool setAccelConfig() {
        return this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_ACCEL_CONFIG, this->accelFullScaleRange,
                                           "Failed to write accel config") &&
               this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_ACCEL_LPF, this->accelLowPassFilter,
                                           "Failed to write accel lpf");
    }

    bool setCompassSampleRate(int rate) {
        if ((rate < MPU9250_COMPASSRATE_MIN) || (rate > MPU9250_COMPASSRATE_MAX)) {
            cerr << "Illegal compass rate " << rate << endl;
            return false;
        }
        this->compassSampleRate = rate;
        return true;
    }

    bool setCompassSampleRate() {
        auto rate = static_cast<unsigned char>(this->gyroAccelSampleRate / this->compassSampleRate - 1);
        if (rate > 31) {
            rate = 31;
        }
        return this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_I2C_SLV4_CTRL, rate,
                                           "Failed to set slave ctrl 4");
    }

    bool setCompassFullScaleRange(unsigned char fsr) {
        switch (fsr) {
            case AK8963_FSR_14BITS:
                this->compassScale = 10.0 * 4912.0 / 8190.0; // Proper scale to return milliGauss
                return true;
            case AK8963_FSR_16BITS:
                this->compassScale = 10.0 * 4912.0 / 32760.0; // Proper scale to return milliGauss
                return true;
            default:
                cerr << "Illegal MPU9250 compass fsr " << fsr << endl;
                return false;
        }
    }

    bool compassInit() {
        unsigned char data[3];

        bypassOn();
        // get fuse ROM data
        if (!this->i2CDevice.deviceWrite(AK8963_ADDRESS, AK8963_CNTL, 0,
                                         "Failed to set compass in power down mode 1") ||
            !this->i2CDevice.deviceWrite(AK8963_ADDRESS, AK8963_CNTL, 0x0f,
                                         "Failed to set compass in fuse ROM mode") ||
            !this->i2CDevice.deviceRead(AK8963_ADDRESS, AK8963_ASAX, data, "Failed to read compass fuse ROM", 3) ||
            !this->i2CDevice.deviceWrite(AK8963_ADDRESS, AK8963_CNTL, 0,
                                         "Failed to set compass in power down mode 2")) {
            bypassOff();
            return false;
        }
        bypassOff();

        //  convert data to usable scale factor
        this->compassAdjust[0] = ((double) data[0] - 128.0) / 256.0 + 1.0;
        this->compassAdjust[1] = ((double) data[1] - 128.0) / 256.0 + 1.0;
        this->compassAdjust[2] = ((double) data[2] - 128.0) / 256.0 + 1.0;

        return this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_I2C_MST_CTRL, 0x40,
                                           "Failed to set I2C master mode") &&
               this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_I2C_SLV0_ADDR, 0x80 | AK8963_ADDRESS,
                                           "Failed to set slave 0 address") &&
               this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_I2C_SLV0_REG, AK8963_ST1,
                                           "Failed to set slave 0 reg") &&
               this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_I2C_SLV0_CTRL, 0x88,
                                           "Failed to set slave 0 ctrl") &&
               this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_I2C_SLV1_ADDR, AK8963_ADDRESS,
                                           "Failed to set slave 1 address") &&
               this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_I2C_SLV1_REG, AK8963_CNTL,
                                           "Failed to set slave 1 reg") &&
               this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_I2C_SLV1_CTRL, 0x81,
                                           "Failed to set slave 1 ctrl") &&
               this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_I2C_SLV1_DO, 0x1,
                                           "Failed to set slave 1 DO") &&
               this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_I2C_MST_DELAY_CTRL, 0x3,
                                           "Failed to set mst delay");
    }

    bool bypassOn() {
        unsigned char userControl;
        if (!this->i2CDevice.deviceRead(this->i2CSlaveAddress, MPU9250_USER_CTRL, &userControl,
                                        "Failed to read user_ctrl reg", 1)) {
            return false;
        }
        userControl &= ~0x20;
        if (!this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_USER_CTRL, &userControl,
                                         "Failed to write user_ctrl reg", 1)) {
            return false;
        }
        this->i2CDevice.delayMs(50);
        if (!this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_INT_PIN_CFG, 0x82,
                                         "Failed to write int_pin_cfg reg")) {
            return false;
        }
        this->i2CDevice.delayMs(50);
        return true;
    }


    bool bypassOff() {
        unsigned char userControl;
        if (!this->i2CDevice.deviceRead(this->i2CSlaveAddress, MPU9250_USER_CTRL, &userControl,
                                        "Failed to read user_ctrl reg", 1)) {
            return false;
        }
        userControl |= 0x20;
        if (!this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_USER_CTRL, &userControl,
                                         "Failed to write user_ctrl reg", 1)) {
            return false;
        }
        this->i2CDevice.delayMs(50);
        if (!this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_INT_PIN_CFG, 0x80,
                                         "Failed to write int_pin_cfg reg")) {
            return false;
        }
        this->i2CDevice.delayMs(50);
        return true;
    }

    bool resetFifo() {
        if (!this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_INT_ENABLE, 0, "Writing int enable") ||
            !this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_FIFO_EN, 0, "Writing fifo enable") ||
            !this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_USER_CTRL, 0, "Writing user control") ||
            !this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_USER_CTRL, 0x04, "Resetting fifo") ||
            !this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_USER_CTRL, 0x60, "Enabling the fifo")) {
            return false;
        }
        this->i2CDevice.delayMs(50);
        return this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_INT_ENABLE, 1, "Writing int enable") &&
               this->i2CDevice.deviceWrite(this->i2CSlaveAddress, MPU9250_FIFO_EN, 0x78, "Failed to set FIFO enables");
    }

};

#endif /* MPU9250_HPP_ */
