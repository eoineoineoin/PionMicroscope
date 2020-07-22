#pragma once
#include <cstdint>

struct DACBoard
{
	enum ReferenceVoltage
	{
		REF_3V3,
		REF_5V
	};

	enum class Channel { A, B };

	DACBoard(ReferenceVoltage ref, int chipSelectPin = 23);
	void writeVoltage(Channel channel, float out);

protected:
	uint8_t m_chipSelectPin;
	float m_refVoltage;
};

namespace A2D
{
// Perform calibration and update internal state.
// Should be called before reading any value.
void calibrate();

float getChannelValue(uint8_t channelIdx);
}
