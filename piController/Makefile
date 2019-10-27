#DIR_FONTS = ./Fonts
SRC_DIR = ./src
DIR_BIN = ./build

OBJ_C = $(wildcard ${SRC_DIR}/*.c) $(wildcard ${SRC_DIR}/*.cpp)
OBJ_O = $(patsubst %.cpp,${DIR_BIN}/%.o,$(patsubst %.c,${DIR_BIN}/%.o,$(notdir ${OBJ_C})))

TARGET = piController

CC = /home/eoin/code/rpicc/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
CXX = /home/eoin/code/rpicc/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++

DEBUG = -g -O0 -Wall -Werror
CFLAGS += $(DEBUG) 
CFLAGS += -Iinclude -Isrc

CXXFLAGS += -std=c++14

LIB = -Llib -lbcm2835 -lm

${TARGET}:${OBJ_O}
	$(CXX) $(OBJ_O) -o $@  $(LIB)

${DIR_BIN}/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@

${DIR_BIN}/%.o : $(SRC_DIR)/%.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c  $< -o $@

	
clean :
	rm $(DIR_BIN)/*.* 
	rm $(TARGET) 
