//
// Created by Muralidhar Ravuri on 10/04/18.
//

#include <core/baseClient.hpp>
#include <core/baseServer.hpp>
#include <sensor/gpsTask.hpp>

#define CLIENT_FREQUENCY            2               // frequency in Hz
#define SERVER_FREQUENCY            10              // frequency in Hz

// seeing weird oscillations when NUM_SAMPLES > 1 - possibly because of Nyquist theorem
#define NUM_SAMPLES                 1               // number of samples in circular buffer to store

#define THREAD_POOL_COUNT           2
#define HOSTNAME                    "localhost"

boost::asio::thread_pool threadPool(THREAD_POOL_COUNT);

const unsigned short port = 5000;

void launchClient() {
    boost::asio::io_context io;
    BaseClient client(io, HOSTNAME, port);
    tcp::socket *socket = client.connect(CLIENT_FREQUENCY);
    if (socket) {
        auto data = client.read(*socket);
        while (!data.empty()) {
            cout << data << endl;
            data = client.read(*socket);
        }
    }
}

void launchServer(const string& deviceName) {
    GPSSensorTask sensorTask(deviceName, SERVER_FREQUENCY, NUM_SAMPLES);
    boost::asio::post(threadPool, boost::bind(&GPSSensorTask::run, &sensorTask));

    BaseServer<GPSValue> server(HOSTNAME, port, sensorTask);
    boost::asio::io_context io;
    server.launch(io);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        if (string(argv[1]) == "--client") {
            launchClient();
        } else {
            // pass deviceName: /dev/serial0
            launchServer(string(argv[1]));
        }
    }

    return 0;
}
