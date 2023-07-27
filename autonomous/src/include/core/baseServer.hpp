/*
 * baseServer.hpp
 *
 *  Created on: Jun 15, 2018
 *      Author: mravuri
 */

#ifndef BASESERVER_HPP_
#define BASESERVER_HPP_

#include <iostream>
#include <exception>
#include <core/abstractSensor.hpp>
#include <core/deviceTask.hpp>

using namespace std;
using boost::asio::ip::tcp;

template <class T>
class BaseServer : public AbstractSensor {
protected:
    const string hostname;
    const unsigned short port;

    DeviceTask<T> &sensorTask;
    bool isShutdown = false;
    int frequency = 0;  // in Hz

public:
    BaseServer(string hostname, const unsigned short &port, DeviceTask<T> &deviceTask)
        : hostname(std::move(hostname)), port(port), sensorTask(deviceTask) {
    }

    virtual ~BaseServer() = default;

    virtual void launch(boost::asio::io_context &io) {
        try {
            tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), port));

            cout << "Server ready..." << endl;
            while (!isShutdown) {
                tcp::socket socket(io);
                acceptor.accept(socket);

                runConnection(&socket);
            }
        } catch (exception &e) {
            cerr << "Exception: " << e.what() << endl;
        }
    }

    virtual void runConnection(tcp::socket *socket) {
        string s = read(*socket);
        try {
            frequency = stoi(s);
        } catch (exception &e) {
            cerr << "Invalid frequency " << s << ". using 0 instead..." << endl;
            frequency = 0;
        }
        if (frequency <= 0) {
            runTask(*socket);
        } else {
            // TODO: break the while loop if the socket is closed on the client side
            while (frequency > 0) {
                int microSeconds = 1000000 / frequency;
                runTask(*socket);
                writeDelimiter(*socket);
                boost::this_thread::sleep_for(boost::chrono::microseconds(microSeconds));
            }
        }
    }

    virtual void runTask(tcp::socket &socket) {
        string result = sensorTask.get();
        boost::system::error_code ec;
        boost::asio::write(socket, boost::asio::buffer(result), ec);
    }

    void setFrequency(int frequency) {
        this->frequency = frequency;
    }

    virtual void shutdown() {
        cout << "Shutting down the server..." << endl;
        isShutdown = true;
    }
};

#endif /* BASESERVER_HPP_ */
