#pragma once
#include <cstdint>

class CommandHandler
{
public:
	virtual void setResolution(uint16_t resolutionX, uint16_t resolutionY) = 0;
	virtual void lockX(uint16_t xImagespace) = 0;
	virtual void freeX() = 0;
};

