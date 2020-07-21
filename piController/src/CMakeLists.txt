# Path to the rasperry pi toolchain
set(PICHAIN /home/eoin/code/rpicc/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/)

# Third party files which are necessary for controlling the DAC;
set(DAC_CONTROL ADS1256.c DAC8532.c DEV_Config.c)

cmake_minimum_required(VERSION 2.4)
project(piController)
set (CMAKE_CXX_STANDARD 14)
include_directories(.)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")

SET(CMAKE_C_COMPILER ${PICHAIN}bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER ${PICHAIN}bin/arm-linux-gnueabihf-g++)

add_executable(piController ${DAC_CONTROL} BeamController.cpp ControlServer.cpp main.cpp)
target_link_directories(piController PUBLIC ../lib)
target_link_libraries(piController bcm2835 m)

add_executable(dacTest ${DAC_CONTROL} dacTest.cpp DACBoard.cpp)
target_link_directories(dacTest PUBLIC ../lib)
target_link_libraries(dacTest bcm2835 m)

add_executable(readA0Test ${DAC_CONTROL} readA0Test.cpp DACBoard.cpp)
target_link_directories(readA0Test PUBLIC ../lib)
target_link_libraries(readA0Test bcm2835 m)
