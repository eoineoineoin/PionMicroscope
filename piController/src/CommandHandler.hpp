#pragma once
#include <cstdint>

class CommandHandler
{
public:
	virtual void setResolution(uint16_t resolutionX, uint16_t resolutionY) = 0;

	enum class Axis { X, Y };

	virtual void lock(Axis axis, float fraction) = 0;
	virtual void free(Axis axis) = 0;
};

