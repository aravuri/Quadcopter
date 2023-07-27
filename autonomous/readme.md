## Install boost on Raspberry Pi

- Compile latest boost build
```bash
wget https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.bz2
tar xvfo boost_1_68_0.tar.bz2
./bootstrap.sh
sudo ./b2 install
```

To fix the linker errors with boost, we need to do two things in eclipse CDT:
- Go to project properties, build settings, MacOSX C++ Linker, Libraries.
- Specify the library name to include. If the file in, say, /usr/local/Cellar/boost/boost_<>/lib/libboost_thread-mt.*, 
then include -lboost_thread-mt, -lboost_system and -lboost_chrono-mt.
- Include the directory of the libraries -L /usr/local/Cellar/boost/boost_<>/include
- To compile from command line, use the makefile present in Debug directory.

- Compile project using makefile
```bash
mkdir -p build
g++ -std=c++11 -I"/home/pi/git/general/loops/sensor/include" -c src/gps.cpp -o build/gps.o
g++ -o build/gps build/gps.o -lboost_system -lboost_thread -lboost_chrono -lpthread
```

- Compile using cmake from command line
```bash
cmake -DCMAKE_BUILD_TYPE=Debug /home/mravuri/git/general/loops/sensor
cmake --build /home/mravuri/git/general/loops/sensor/cmake-build-debug --target CMakeFiles/gpsSensor.dir/gps/src/gps.cpp.o -- -j 4 -f /home/mravuri/git/general/loops/sensor/cmake-build-debug/CMakeFiles/gpsSensor.dir/build.make --always-make
cmake --build /home/mravuri/git/general/loops/sensor/cmake-build-debug --target gpsSensor -- -j 4
make -j 4
```

## Common issues

```bash
brew update
brew install boost --with-python
brew install boost-python
```
- right click Index -> rebuild to avoid the red underline warnings

## Boost thread pool job submission
- If we have a class that we want to submit to a thread pool, then one way to do it is to make the class callable
by defining an operator() method. You can then post this class by
```
boost::asio::post(pool, ref(job));
```
where 'job' is a callable class.
- Choose dialect for C++ compiler as C++11 on eclipse
- Add C++ compiler include path on eclipse to include the sensor/include path.

## Sensors
- Server
    - Listens on a port
    - When a client connects to the server, a job is created and submitted to the thread pool
    - The server continues to listen for new connection. 
    - The job fetches the latest sensor reading and writes it to the client socket
    - 
    - Has another task that runs like a timer. This task is submitted to the thread pool as well
    - This task does not end, but it keeps updating the state with the latest sensor reading
- Client
    - Connects to the server and gets the latest sensor reading

## Python client using socket
```python
import json
import socket

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(("10.0.0.2", 5001))
client.send(b'10\n')  # set frequency = 10Hz
raw_data = client.recv(1024)
while raw_data:
    raw_data = client.recv(1024)
    print(raw_data)

data = json.loads(raw_data)
```

## Install PiGPIO

```bash
git clone https://github.com/joan2937/pigpio
make -j4
sudo make install
```
