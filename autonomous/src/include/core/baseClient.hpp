/*
 * baseClient.hpp
 *
 *  Created on: Jun 15, 2018
 *      Author: mravuri
 */

#ifndef BASECLIENT_HPP_
#define BASECLIENT_HPP_

#include <core/abstractSensor.hpp>
#include <iostream>

using namespace std;
using boost::asio::ip::tcp;

/**
 * Protocol is as follows:
 * - client connects to server on a specific port
 * - client writes an integer frequency followed by a delimiter (for example, "10\n")
 * - client starts reading the data in a loop until EOF
 */
class BaseClient : public AbstractSensor {
protected:
    const string hostname;
    const int port;
    tcp::socket socket;

public:
    BaseClient(boost::asio::io_context& io, string hostname, const int &port)
        : hostname(std::move(hostname)), port(port), socket(io) {
    }

    virtual ~BaseClient() = default;

    virtual tcp::socket* connect(int frequency = 0) {
        try {
            tcp::resolver resolver(socket.get_io_context());
            tcp::resolver::results_type endpoints = resolver.resolve(hostname, to_string(port));

            boost::asio::connect(socket, endpoints);

            write(socket, to_string(frequency));
            writeDelimiter(socket);

            return &socket;
        } catch (exception& e) {
            cerr << e.what() << endl;
            return nullptr;
        }
    }
};

#endif /* BASECLIENT_HPP_ */
