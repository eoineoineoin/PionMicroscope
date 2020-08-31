#pragma once
#include <cstdint>

// Stateful struct to help read from the DAC8532.
// One of these can be created and used to write to both output channels (i.e.
// both DAC0 and DAC1); however, if you wish to use a second Waveshare board, a
// second instance if this should be created, specifying a new chip-select.
struct DACBoard
{
	// Describes the board's jumper setting for the reference voltage
	enum ReferenceVoltage
	{
		REF_3V3,
		REF_5V
	};

	// All the possible channels on the chip.
	// The A channel is labelled DAC0 on the board
	// The B channel is labelled DAC1 on the board
	enum class Channel { A, B };

	// Construct an instance to control a single DAC8532.
	// Reference voltage should be specified based on the configuration
	// of a jumper setting on the board; the default value of chip-select
	// pin should not be changed, unless you wish to communicate with an
	// additional board -- in which case, it should be the GPIO pin which is
	// connected to the chip-select pin on this additional board.
	DACBoard(ReferenceVoltage ref, int chipSelectPin = 23);


	// Write the value \a out to either A (DAC0) or B (DAC1)
	void writeVoltage(Channel channel, float out);

protected:
	uint8_t m_chipSelectPin;
	float m_refVoltage;
};

namespace A2D
{
// Perform calibration and update internal state;
// Should be called before reading any value.
void calibrate();

// Return a value in the range +/- 2*V_REF (specified by a jumper on the board)
// though in practice the minimum value reported is clamped to ~-100mV, due to
// limimitations of the ADS1256 input circuitry.
float getChannelValue(uint8_t channelIdx);
}
