/*
 * sensorTask.hpp
 *
 *  Created on: Jun 15, 2018
 *      Author: mravuri
 */

#ifndef DEVICE_TASK_HPP_
#define DEVICE_TASK_HPP_

#include <iostream>
#include <boost/thread.hpp>
#include <core/deviceData.hpp>
#include <utils/misc.hpp>

using namespace std;

template<class T>
class DeviceTask {
protected:
    boost::mutex mtx;
    const int samplingFrequency; // in Hz
    bool isShutdown;

    DeviceData<T> *result;

public:
    explicit DeviceTask(const int &samplingFrequency, const unsigned int k) :
        samplingFrequency(samplingFrequency), isShutdown(false) {
        result = new DeviceData<T>(k);
    }

    virtual ~DeviceTask() {
        delete result;
    }

    /**
     * Run the sensor sampling task at a given sampling frequency.
     */
    virtual void run() {
        int microSeconds = 1000000 / samplingFrequency;
        while (!isShutdown) {
            {
                boost::lock_guard<boost::mutex> lk(mtx);
                fetch();
            }
            boost::this_thread::sleep_for(boost::chrono::microseconds(microSeconds));
        }
    }

    /**
     * Get the latest result from the sensor.
     */
    virtual string get() {
        boost::lock_guard<boost::mutex> lk(mtx);
        return result->toString();
    }

    virtual T getData() {
        boost::lock_guard<boost::mutex> lk(mtx);
        return *result->getCurrentValue();
    }

    virtual void shutdown() {
        cout << "Shutting down the sensor task..." << endl;
        isShutdown = true;
    }

protected:
    /**
     * Extend this class and implement this method to fetch the sensory data.
     * Returns currentIndex of the most recent value stored
     */
    virtual void fetch() {
        result->currentIndex = (result->currentIndex + 1) % result->k;
    }

};

#endif /* DEVICE_TASK_HPP_ */
