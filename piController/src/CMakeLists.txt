# To build for Pi:
#TODO This should be the default, and should be easier to specify
#cmake ../ -DCMAKE_TOOLCHAIN_FILE=../RaspberryPi-Toolchain.Cmake

if(NOT DEFINED SIMULATOR)
	# The pi toolchain file sets SIMULATOR to 0
	# But we need to specify it for the deafault case:
	set(SIMULATOR 1)
endif(NOT DEFINED SIMULATOR)
	
# Check if we are doing a build targetting the pi:
if(NOT SIMULATOR)
	SET(BCMLIB bcm2835)
	# Third party files which are necessary for controlling the DAC;
	# TODO: Should implement these ourselves, only a few pieces left:
	set(DAC_CONTROL ADS1256.c DAC8532.c DEV_Config.c)
	add_definitions(-DSIMULATOR=0)
else(NOT SIMULATOR)
	add_definitions(-DSIMULATOR=1)
endif(NOT SIMULATOR)


cmake_minimum_required(VERSION 2.4)
project(piController)
set (CMAKE_CXX_STANDARD 14)
include_directories(.)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_executable(piController ${DAC_CONTROL} BeamController.cpp DACBoard.cpp ControlServer.cpp main.cpp)
target_link_directories(piController PUBLIC ../lib)
target_link_libraries(piController ${BCMLIB} m)

add_executable(dacTest ${DAC_CONTROL} dacTest.cpp DACBoard.cpp)
target_link_directories(dacTest PUBLIC ../lib)
target_link_libraries(dacTest ${BCMLIB} m)

add_executable(readA0Test ${DAC_CONTROL} readA0Test.cpp DACBoard.cpp)
target_link_directories(readA0Test PUBLIC ../lib)
target_link_libraries(readA0Test ${BCMLIB} m)
