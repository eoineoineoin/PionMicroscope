#include <DACBoard.h>
#include <bcm2835.h>
#include <algorithm>


DACBoard::DACBoard(ReferenceVoltage ref, int chipSelectPin)
	: m_chipSelectPin(chipSelectPin)
{
	if(ref == REF_3V3)
		m_refVoltage = 3.3f;
	else
		m_refVoltage = 5.0f;

    bcm2835_gpio_fsel(m_chipSelectPin, BCM2835_GPIO_FSEL_OUTP);
}


void DACBoard::writeVoltage(Channel channel, float out)
{
	uint8_t channelId = channel == Channel::A ? 0x30 : 0x34;
	uint32_t quantized = out * UINT16_MAX / m_refVoltage;
	uint32_t maxV = UINT16_MAX;
	uint16_t voltage = (uint16_t)std::min(quantized, maxV);

	bcm2835_gpio_write(m_chipSelectPin, 1);
	bcm2835_gpio_write(m_chipSelectPin, 0);
	bcm2835_spi_transfer(channelId);
	bcm2835_spi_transfer(voltage >> 8);
	bcm2835_spi_transfer(voltage & 0xff);
	bcm2835_gpio_write(m_chipSelectPin, 1);
}

