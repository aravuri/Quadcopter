//
// Created by Muralidhar Ravuri on 10/14/18.
//

#ifndef UART_HPP_
#define UART_HPP_

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string>

using namespace std;

class UART {

private:
    const string deviceName;
    int uart0Filestream = -1;
    bool isOpen = false;

    void setup() {
        // OPEN THE UART
        // The flags (defined in fcntl.h):
        //     Access modes (use one of these):
        //         O_RDONLY - Open for reading only.
        //         O_RDWR - Open for reading and writing.
        //         O_WRONLY - Open for writing only.
        //     O_NDELAY / O_NONBLOCK (same function)
        //         - Enables nonblocking mode. When set read requests on the file can return immediately with a failure
        //           status if there is no input immediately available (instead of blocking). Likewise, write requests
        //           can also return immediately with a failure status if the output can't be written immediately.
        //     O_NOCTTY
        //         - When set and path identifies a terminal device, open() shall not cause the terminal device to
        //           become the controlling terminal for the process.
        uart0Filestream = open(deviceName.data(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (uart0Filestream == -1) {
            cout << "Error - Unable to open UART. Ensure it is not in use by another application" << endl;
            isOpen = false;
            return;
        }

        // CONFIGURE THE UART
        // The flags defined in /usr/include/termios.h
        //      (see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
        //      Baud rate: B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, etc
        //      CSIZE - CS5, CS6, CS7, CS8
        //      CLOCAL - Ignore modem status lines
        //      CREAD - Enable receiver
        //      IGNPAR - Ignore characters with parity errors
        struct termios options{};
        tcgetattr(uart0Filestream, &options);
        options.c_cflag = B9600 | CS8 | CLOCAL | CREAD; // Set baud rate
        options.c_iflag = IGNPAR;
        options.c_oflag = 0;
        options.c_lflag = 0;
        tcflush(uart0Filestream, TCIFLUSH);
        tcsetattr(uart0Filestream, TCSANOW, &options);
        isOpen = true;
    }

public:
    explicit UART(string deviceName) : deviceName(std::move(deviceName)) {
        setup();
    }

    virtual ~UART() = default;

    bool isDeviceOpen() {
        return isOpen;
    }

    void transmit(const string &message) {
        if (uart0Filestream != -1) {
            ssize_t count = write(uart0Filestream, message.data(), message.length());
            if (count < 0) {
                cout << "UART Tx error" << endl;
            }
        }
    }

    ssize_t receive(char *buffer, size_t buffer_size) {
        if (uart0Filestream != -1) {
            return read(uart0Filestream, (void *) buffer, buffer_size);
        }
        return -1;
    }

};

#endif /* UART_HPP_ */
