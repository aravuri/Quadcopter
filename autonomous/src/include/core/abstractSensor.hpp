//
// Created by Muralidhar Ravuri on 10/10/18.
//

#ifndef ABSTRACTSENSOR_HPP_
#define ABSTRACTSENSOR_HPP_

#include <iostream>
#include <sstream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using namespace std;
using boost::asio::ip::tcp;

class AbstractSensor {
protected:
    const char delimiter = '\n';

public:
    string read(tcp::socket& socket) {
        stringstream ss;
        for (;;) {
            boost::array<char, 1024> buf{};
            boost::system::error_code ec;

            size_t len = socket.read_some(boost::asio::buffer(buf), ec);

            if (ec == boost::asio::error::eof) {
                cout << "EOF reached..." << endl;
                break;
            } else if (ec) {
                throw boost::system::system_error(ec);
            }
            if (buf.data()[len - 1] == delimiter) {
                ss.write(buf.data(), len - 1);
                break;
            }

            ss.write(buf.data(), len);
        }
        return ss.str();
    }

    void write(tcp::socket& socket, const string& s) {
        boost::system::error_code ec;
        boost::asio::write(socket, boost::asio::buffer(s), ec);
    }

    void writeDelimiter(tcp::socket& socket) {
        boost::system::error_code ec;
        boost::asio::write(socket, boost::asio::buffer(string(1, delimiter)), ec);
    }

};

#endif /* ABSTRACTSENSOR_HPP_ */
