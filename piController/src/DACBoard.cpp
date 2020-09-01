#include <DACBoard.h>
#include <ADS1256.h>
#include <bcm2835.h>
#include <algorithm>
#include <cmath>

#if SIMULATOR == 1
extern "C"
{
UBYTE DEV_ModuleInit(void)
{
	return 0;
}

void DEV_ModuleExit(void)
{
}

UBYTE ADS1256_init(void)
{
	return 0;
}

}
#endif

DACBoard::DACBoard(ReferenceVoltage ref, int chipSelectPin)
	: m_chipSelectPin(chipSelectPin)
{
	if(ref == REF_3V3)
		m_refVoltage = 3.3f;
	else
		m_refVoltage = 5.0f;

#if !defined(SIMULATOR) || (SIMULATOR == 0)
    bcm2835_gpio_fsel(m_chipSelectPin, BCM2835_GPIO_FSEL_OUTP);
#endif
}

#if SIMULATOR == 1
// Remember the last value written out, for simulating input. Just used to
// procedurally generate a value used by the A2D simulator.
static float s_fractionA = 0.0f;
static float s_fractionB = 0.0f;
#endif

void DACBoard::writeVoltage(Channel channel, float out)
{
#if SIMULATOR == 1
	if(channel == Channel::A)
	{
		s_fractionA = out / m_refVoltage;
	}
	else
	{
		s_fractionB = out / m_refVoltage;
	}
	return;
#else
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
#endif
}

namespace A2D
{
#if SIMULATOR == 0
static void ADS1256_WriteReg(UBYTE Reg, UBYTE data)
{
    DEV_Digital_Write(DEV_CS_PIN, 0);
    DEV_SPI_WriteByte(CMD_WREG | Reg);
    DEV_SPI_WriteByte(0x00);
    DEV_SPI_WriteByte(data);
    DEV_Digital_Write(DEV_CS_PIN, 1);
}

static void ADS1256_WriteCmd(UBYTE Cmd)
{
    DEV_Digital_Write(DEV_CS_PIN, 0);
    DEV_SPI_WriteByte(Cmd);
    DEV_Digital_Write(DEV_CS_PIN, 1);
}

static void waitForReadyPin(void)
{   
	while(DEV_Digital_Read(DEV_DRDY_PIN) == 1) {}
}

static uint8_t ADS1256_Read_register(uint8_t Reg)
{
    UBYTE temp = 0;
    DEV_Digital_Write(DEV_CS_PIN, 0);
    DEV_SPI_WriteByte(CMD_RREG | Reg);
    DEV_SPI_WriteByte(0x00);
    DEV_Delay_ms(1);
    temp = DEV_SPI_ReadByte();
    DEV_Digital_Write(DEV_CS_PIN, 1);
    return temp;
}

// Sign-extend the 24-bit signed value 'in' to 32 bits 
int32_t signExtend24(uint32_t in)
{
	uint32_t twoTwentyFour = 1 << 23;
	uint32_t signExtended = (in ^ twoTwentyFour) - twoTwentyFour;
	return (int32_t)(signExtended);
}

// Convert three bytes, with MSB first to a native 32-bit integer
uint32_t bytesTo24(uint8_t bytes[3])
{
	uint32_t ret = bytes[0];
	ret |= uint32_t(bytes[1]) << 8;
	ret |= uint32_t(bytes[2]) << 16;
	return ret;
}

static int32_t ADS1256_Read_ADC_Data(void)
{
    uint8_t buf[3] = {0,0,0};
    
    waitForReadyPin();
    DEV_Delay_ms(1);
    DEV_Digital_Write(DEV_CS_PIN, 0);
    DEV_SPI_WriteByte(CMD_RDATA);
    DEV_Delay_ms(1);
	// RDATA sends the MSB first:
    buf[2] = DEV_SPI_ReadByte();
    buf[1] = DEV_SPI_ReadByte();
    buf[0] = DEV_SPI_ReadByte();
    DEV_Digital_Write(DEV_CS_PIN, 1);

    uint32_t value24 = bytesTo24(buf);
	return signExtend24(value24);
}
#endif

static struct CalData
{
	int32_t ofc;
	int32_t fsc;
	float vref = 5.0f;

	float pga = 1.0f; // TODO This can be taken from ADCON register
	// These are correct values for 30kSamples/s, from the datasheet.
	// Don't think they can be determined at runtime.
	int32_t alpha = 0x400000;
	float beta = 1.8639f;
} s_calData;

void calibrate()
{
#if SIMULATOR == 1
	return;
#else
	ADS1256_WriteCmd(CMD_SELFCAL);
	waitForReadyPin();

	uint8_t ofc[3];
	ofc[0] = ADS1256_Read_register(REG_OFC0);
	ofc[1] = ADS1256_Read_register(REG_OFC1);
	ofc[2] = ADS1256_Read_register(REG_OFC2);
	s_calData.ofc = signExtend24(bytesTo24(ofc));

	uint8_t fsc[3];
	fsc[0] = ADS1256_Read_register(REG_FSC0);
	fsc[1] = ADS1256_Read_register(REG_FSC1);
	fsc[2] = ADS1256_Read_register(REG_FSC2);
	s_calData.fsc = bytesTo24(fsc); // FSC is unsigned
#endif
}

float getChannelValue(uint8_t channelIdx)
{
#if SIMULATOR == 1
	(void)channelIdx;
	return 5.0f * sin(s_fractionA * M_PI * 2 * 7) *
		0.5f * (1.0f + sin(s_fractionB * M_PI * 2 * 3));
#else
	// From datasheet, in calibration section:
	// output = ((pga * vin / 2vref) - (ofc / alpha)) * fsc * beta
	// So
	// vin = (output / (fsc * beta) + ofc / alpha) * 2 * vref / pga
	ADS1256_WriteReg(REG_MUX, (channelIdx<<4) | (1<<3));
	ADS1256_WriteCmd(CMD_SYNC);
	ADS1256_WriteCmd(CMD_WAKEUP);
	int32_t output = ADS1256_Read_ADC_Data();

	float term1 = float(output) / (float(s_calData.fsc) * s_calData.beta);
	float term2 = float(s_calData.ofc) / float(s_calData.alpha);
	float term3 = 2 * s_calData.vref / s_calData.pga;
	
	return ((term1 + term2) * term3) / 2.0f;
#endif
}

}
