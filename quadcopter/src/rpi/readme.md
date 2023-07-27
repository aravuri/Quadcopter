# Initial Setup - Raspberry Pi
```bash
sudo apt update
sudo apt install dist-upgrade

sudo apt install tmux vim libxml2-dev libxslt1-dev

# setup RSA keys
ssh-keygen

# setup andromeda codebase
mkdir -p ~/git/general
git clone git@ravurigpu:mravuri/andromeda.git

# install miniconda
wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-armv7l.sh
/bin/bash Miniconda3-latest-Linux-armv7l.sh

# instead of using conda below, use pip
conda config --add channels rpi
conda create -n andromeda-quadcopter-rpi python=3.6
pip install -r rpi/requirements.txt

# install cmake
wget https://cmake.org/files/v3.12/cmake-3.12.3.tar.gz
./bootstrap
make -j4
sudo make install

# install boost
wget https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.bz2
tar xvfo boost_1_68_0.tar.bz2
./bootstrap.sh
sudo ./b2 install
```

In ~/.bashrc, add
```bash
alias bt='cd ~/git/general/andromeda'
alias rm='rm -i'
alias cp='cp -i'
```

In /etc/hosts, add
```bash
192.168.0.195    ravurigpu
```

To run cmake for gps and for pytorch
```bash
cmake .
cmake -DCMAKE_PREFIX_PATH=/home/mravuri/git/general/libtorch .
```

To launch the quadcopter
```bash
cd ~/git/general/andromeda/src/quadcopter
source activate andromeda-quadcopter-rpi
export PYTHONPATH=~/git/general/andromeda/src/quadcopter

# sometimes we may have to kill pigpiod and relaunching it before running the cli.py command
sudo killall pigpiod
sudo pigpiod
python rpi/cli.py manual-quadcopter
```

# PyTorch on Raspberry Pi

First of all, we need to set up SWAP. Edit file /etc/dphys-swapfile, find the constant CONF_SWAPSIZE and 
change its value to at least 2048, which means sparing 2G for swap file system. I set it to 4096 (4G), 
because I use a 32G card. To activate the swap file system, do the following commands:

```bash
# increase swap-size only to build opencv and pytorch (only temporarily - otherwise, you will burn your micro-sd card)
sudo vim /etc/dphys-swapfile
CONF_SWAPSIZE=4096
sudo /etc/init.d/dphys-swapfile stop
sudo /etc/init.d/dphys-swapfile start

# install dependencies
sudo apt-get install libopenblas-dev cython3 libatlas-dev m4 libblas-dev
conda activate andromeda-quadcopter-rpi
pip install pyyaml numpy

wget http://ftp.us.debian.org/debian/pool/main/n/numactl/libnuma1_2.0.12-1_armhf.deb
sudo dpkg -i libnuma1_2.0.12-1_armhf.deb
wget http://ftp.us.debian.org/debian/pool/main/n/numactl/libnuma-dev_2.0.12-1_armhf.deb
sudo dpkg -i libnuma-dev_2.0.12-1_armhf.deb

sudo apt install libgoogle-glog-dev

# compile and install pytorch from source
git clone --recursive https://github.com/pytorch/pytorch
cd pytorch

export NO_CUDA=1
export NO_DISTRIBUTED=1
export CMAKE_PREFIX_PATH="/home/pi/miniconda3"

# Remove all occurrences of -mavx2 since gcc on Raspberry Pi does not support this option

# build pytorch
python setup.py build

# create wheel
export NO_CUDA=1
export NO_DISTRIBUTED=1
conda activate andromeda-quadcopter-arm
pip install wheel
python setup.py bdist_wheel
cd dist

# Reset swapsize back to 100 after done with compilation
sudo vim /etc/dphys-swapfile
CONF_SWAPSIZE=100
sudo /etc/init.d/dphys-swapfile stop
sudo /etc/init.d/dphys-swapfile start

```

# GPS Module

- Tutorial (use below instrutions to make it work for Rasberry Pi 3B+)

http://osoyoo.com/2016/10/25/use-rapsberry-pi-to-drive-u-blox-neo-6m-gps-module/

https://ava.upuaut.net/?p=726

- Pin connections

| Raspberry Pi            | GPS Module |
|:-----------------------:|:----------:|
| 3.3 V (Left 1)          | VCC        |
| GND (Right 3)           | GND        |
| GPIO 15 (RxD) (Right 5) | TxD        |
| GPIO 14 (TxD) (Right 4) | RxD        |

- Software installation
```bash
sudo apt-get -y install minicom
```

- Block boot messages
```bash
sudo vim /boot/cmdline.txt
# original: dwc_otg.lpm_enable=0 console=serial0,115200 console=tty1 root=PARTUUID=ecf8b749-02 rootfstype=ext4 elevator=deadline fsck.repair=yes rootwait quiet splash plymouth.ignore-serial-consoles
remove console=serial0,115200
reboot
```

- Get the GPS data
```bash
# for Raspberry Pi 3+
sudo minicom -b 9600 -o -D /dev/serial0
```

- Typical NMEA sentences 

#### GPS Fix data
$GPGGA,050105.00,3720.61633,N,12200.67448,W,2,11,0.86,48.8,M,-30.0,M,,0000*57
utc:hhmmss.ss,latitude:dddmm.mm,N/S,longitude:dddmm.mm,E/W,quality,numSatellites,horizontalDilutionPrecision,altitude,altitudeUnits,referenceFrequencyOffset,geoidAltitudeUnits,ageDifferentialGPS,differentialStationID,checkSum

#### GNSS Satellites in view
$GPGSV,4,1,14,03,08,174,18,04,22,063,31,05,05,325,,07,60,302,28*74
numMessages = 4
messageId = 1
numSatellitesInView = 14
satelliteId = 03
elevation = 08
azimuth = 174

#### Recommended minimum specific GPS data
$GPRMC,050105.00,A,3720.61633,N,12200.67448,W,0.073,,171018,,,D*63
utc:hhmmss.ss,validity:A-good:V-notgood,latitude:dddmm.mm,N/S,longitude:dddmm.mm,E/W,speedKnots,trackDegrees,utc:ddmmyy,magneticVariation,variationDirectionE/W,checkSum

```text
$GPGSA,A,3,23,03,09,16,51,07,48,30,27,08,28,,1.49,0.86,1.21*05
$GPGSV,4,1,14,03,08,174,18,04,22,063,31,05,05,325,,07,60,302,28*74
$GPGSV,4,2,14,08,25,117,26,09,81,084,43,16,20,041,31,23,55,120,29*76
$GPGSV,4,3,14,27,25,082,36,28,13,209,32,30,32,273,23,46,46,192,33*7B
$GPGSV,4,4,14,48,45,198,34,51,44,157,32*70
$GPGLL,3720.61631,N,12200.67448,W,050104.00,A,D*73
$GPRMC,050105.00,A,3720.61633,N,12200.67448,W,0.073,,171018,,,D*63
$GPVTG,,T,,M,0.073,N,0.136,K,D*26
$GPGGA,050105.00,3720.61633,N,12200.67448,W,2,11,0.86,48.8,M,-30.0,M,,0000*57
```

# ESC

- We typically use ESC_PIN as 17, 18, 23 and 24
- Calibrate using:
```bash
cd rpi
python cli.py calibrate-esc 17
```

- For basic testing: connect the battery to the power distribution board and run the above command for each of the
four pins for the four ESC's and choose 'control'.

# IMU setup

- Datasheet for MPU-9250 is at: https://www.invensense.com/wp-content/uploads/2015/02/PS-MPU-9250A-01-v1.1.pdf
- When using RTIMULib, change the MPU-9250 id from 0x71 to 0x73 and recompile the code. This change is in
`RTIMULib/IMUDrivers/RTIMUDefs.h`.
- More details at:
    - https://github.com/RPi-Distro/RTIMULib
    - https://github.com/RPi-Distro/RTIMULib/tree/master/Linux

# Cross compile on Mac OSX for Raspberry Pi

https://www.jaredwolff.com/blog/cross-compiling-on-mac-osx-for-raspberry-pi/

https://medium.com/coinmonks/setup-gcc-8-1-cross-compiler-toolchain-for-raspberry-pi-3-on-macos-high-sierra-cb3fc8b6443e

```bash
brew install crosstool-ng
brew install grep gnu-sed
brew install gettext
chmod +x /usr/local/Cellar/crosstool-ng/1.23.0_2/lib/crosstool-ng-1.23.0/scripts/crosstool-NG.sh
brew install help2man bison
brew install libtool automake
brew upgrade cmake

# this linkage can be later reverted by running:
#   brew unlink bison
# after the linking we should restart the console or run:
#   source ~/.bash_profile
# to correctly apply the linkage. by running:
#   which bison
# the console should return something other than /usr/bin/bison
brew link --force bison
echo 'export PATH="/usr/local/opt/bison/bin:$PATH"' >> ~/.bash_profile
source ~/.bash_profile
export LDFLAGS="-L/usr/local/opt/bison/lib"

# create a disk using disk utility of type APFS Case-sensitive with 10GB and name xtool-build-env
cp loops/cross_compile/.config /Volumes/xtool-build-env/

# update volume names in .config as necessary
CT_WORK_DIR="/Volumes/xtool-build-env/.build"
CT_PREFIX_DIR="/Volumes/xtool-build-env/${CT_TARGET}"
CT_LOCAL_PATCH_DIR="/Volumes/xtool-build-env/packages"

# copy packages from crosstool-ng
git clone https://github.com/crosstool-ng/crosstool-ng
cp -rf crosstool-ng/packages /Volumes/xtool-build-env/

# check ulimit and increase it to at least 1024 if smaller (ulimit -n 1024)
ulimit -n

# upgrade perl to be at least 5.28.0
brew upgrade perl

# initial setup to copy files and patch them
ct-ng list-steps
ct-ng +companion_tools_for_build

# Update XCFLAGS
vim /Volumes/xtool-build-env/.build/src/gcc-8.2.0/libatomic/configure
# change XCFLAGS="$XCFLAGS -Wall -Werror" to below
XCFLAGS="$XCFLAGS -Wall"

# Now kickoff the build
ct-ng companion_tools_for_build+

# Test on a sample project
git clone https://github.com/yc2986/CMakeRPiExample.git
cd build
cmake .. && make

```

# Cross compile on Ubuntu for Raspberry Pi

https://medium.com/@au42/the-useful-raspberrypi-cross-compile-guide-ea56054de187

```bash
git clone https://github.com/raspberrypi/tools.git raspberrypi-tools

# sample library to compile
git clone git://git.drogon.net/wiringPi
cd wiringPi
cp /home/mravuri/git/general/loops/cross_compile/ubuntu/wiringPi/CMakeLists.txt wiringPi/

cmake . -DCMAKE_TOOLCHAIN_FILE=/home/mravuri/git/general/loops/cross_compile/ubuntu/Toolchain-rpi.cmake
make

# compile blink sample source code
cmake .
make
```

# Install GLM

```bash
https://github.com/g-truc/glm.git
cd glm
mkdir build
cd build
cmake ..
make -j4
sudo make install
```
