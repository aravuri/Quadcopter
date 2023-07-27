//
// Created by Muralidhar Ravuri on 10/25/18.
//

#ifndef I2C_HPP_
#define I2C_HPP_

#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/ioctl.h>
#include <cstring>

#ifdef __linux__
#include <linux/i2c-dev.h>
#else
#define I2C_SLAVE	        0x0703	/* Use this slave address - from linux/i2c-dev.h */
#endif

#define MAX_WRITE_LEN       255

using namespace std;

class I2CDevice {
private:
    int i2C;
    unsigned char currentSlaveAddr;

public:
    unsigned char i2CBus;   // I2C bus of the imu (eg 1 for Raspberry Pi usually: /dev/i2c-1)

public:
    I2CDevice() : i2C(-1), currentSlaveAddr(255), i2CBus(255) {
    }

    virtual ~I2CDevice() {
        deviceClose();
    }

    bool deviceOpen() {
        char buf[32];
        if (i2C >= 0) {
            return true;
        }
        if (i2CBus == 255) {
            cerr << "No I2C bus has been set" << endl;
            return false;
        }

        sprintf(buf, "/dev/i2c-%d", i2CBus);
        i2C = open(buf, O_RDWR);
        if (i2C < 0) {
            cerr << "Failed to open I2C bus " << i2CBus << endl;
            i2C = -1;
            return false;
        }
        return true;
    }

    void deviceClose() {
        if (i2C >= 0) {
            close(i2C);
            i2C = -1;
            currentSlaveAddr = 255;
        }
    }

    bool deviceWrite(unsigned char slaveAddr, unsigned char regAddr, unsigned char const data, const char *errorMsg) {
        return deviceWrite(slaveAddr, regAddr, &data, errorMsg, 1);
    }

    bool deviceWrite(unsigned char slaveAddr, unsigned char regAddr, unsigned char const *data, const char *errorMsg,
                     unsigned char length) {
        ssize_t result;
        unsigned char txBuff[MAX_WRITE_LEN + 1];

        if (!i2CSelectSlave(slaveAddr, errorMsg)) {
            return false;
        }

        if (length == 0) {
            result = write(i2C, &regAddr, 1);

            if (result < 0) {
                if (strlen(errorMsg) > 0) {
                    cerr << "I2C write of regAddr failed - " << errorMsg << endl;
                }
                return false;
            } else if (result != 1) {
                if (strlen(errorMsg) > 0) {
                    cerr << "I2C write of regAddr failed (nothing written) - " << errorMsg << endl;
                }
                return false;
            }
        } else {
            txBuff[0] = regAddr; // write the register first followed by the actual data
            memcpy(txBuff + 1, data, length);

            result = write(i2C, txBuff, length + 1);

            if (result < 0) {
                if (strlen(errorMsg) > 0) {
                    cerr << "I2C data write of " << length << " bytes failed - " << errorMsg << endl;
                }
                return false;
            } else if (result < (int) length) {
                if (strlen(errorMsg) > 0) {
                    cerr << "I2C data write of " << length << " bytes failed, only " << result << " written - "
                         << errorMsg << endl;
                }
                return false;
            }
        }
        return true;
    }

    bool deviceRead(unsigned char slaveAddr, unsigned char regAddr, unsigned char *data, const char *errorMsg,
                    unsigned char length) {
        if (!deviceWrite(slaveAddr, regAddr, nullptr, errorMsg, 0)) {
            return false;
        }
        return readData(slaveAddr, regAddr, data, errorMsg, length);
    }

    bool deviceRead(unsigned char slaveAddr, unsigned char *data, const char *errorMsg, unsigned char length) {
        if (!i2CSelectSlave(slaveAddr, errorMsg)) {
            return false;
        }
        return readData(slaveAddr, 0, data, errorMsg, length);
    }

    bool i2CSelectSlave(unsigned char slaveAddr, const char *errorMsg) {
        if (currentSlaveAddr == slaveAddr) {
            return true;
        }

        if (!deviceOpen()) {
            cerr << "Failed to open I2C port - " << errorMsg << endl;
            return false;
        }

        if (ioctl(i2C, I2C_SLAVE, slaveAddr) < 0) {
            cerr << "I2C slave select " << slaveAddr << " failed - " << errorMsg << endl;
            return false;
        }

        currentSlaveAddr = slaveAddr;
        return true;
    }

    void delayMs(unsigned int milliSeconds) {
        usleep(1000 * milliSeconds);
    }

private:
    bool readData(unsigned char slaveAddr, unsigned char regAddr, unsigned char *data, const char *errorMsg,
                  unsigned char length) {
        ssize_t result;
        size_t total = 0;
        int tries = 0;
        while ((total < length) && (tries < 5)) {
            result = read(i2C, data + total, length - total);

            if (result < 0) {
                if (strlen(errorMsg) > 0) {
                    cerr << "I2C read error from " << slaveAddr << ", " << regAddr << " - " << errorMsg << endl;
                }
                return false;
            }

            total += result;
            if (total == length) {
                break;
            }

            delayMs(10);
            tries++;
        }

        if (total < length) {
            if (strlen(errorMsg) > 0) {
                cerr << "I2C read from " << slaveAddr << ", " << regAddr << " failed - " << errorMsg << endl;
            }
            return false;
        }
        return true;
    }

};

#endif /* I2C_HPP_ */
