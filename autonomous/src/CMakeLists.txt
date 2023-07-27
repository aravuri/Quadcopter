cmake_minimum_required(VERSION 3.12)
project(sensor)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

#set(CMAKE_PREFIX_PATH ~/git/general/libtorch)
#find_package(Torch REQUIRED)

find_package(Boost COMPONENTS system filesystem thread chrono REQUIRED)
find_library(PIGPIO_LIB pigpio)
find_library(RT_LIB rt)

message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")

include_directories("${PROJECT_SOURCE_DIR}/include")

#add_executable(example sample/src/example.cpp)
#target_link_libraries(example ${TORCH_LIBRARIES})

add_executable(gps gps/src/gps.cpp)
target_link_libraries(gps ${Boost_LIBRARIES})

add_executable(imu imu/src/imu.cpp)
target_link_libraries(imu ${Boost_LIBRARIES})

add_executable(quadcopter quadcopter/src/quadcopter.cpp)
target_link_libraries(quadcopter ${Boost_LIBRARIES} ${PIGPIO_LIB} ${RT_LIB})
