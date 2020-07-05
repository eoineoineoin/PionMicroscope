#include <ADS1256.h>
#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>

void moduleExit(int)
{
	DEV_ModuleExit();
	exit(0);
}

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
constexpr int32_t signExtend24(uint32_t in)
{
	uint32_t twoTwentyFour = 1 << 23;
	uint32_t signExtended = (in ^ twoTwentyFour) - twoTwentyFour;
    //return *reinterpret_cast<int32_t*>(&signExtended);
	return (int32_t)(signExtended);
}

// Convert three bytes, with MSB first to a native 32-bit integer
constexpr uint32_t bytesTo24(uint8_t bytes[3])
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

struct CalData
{
	int32_t ofc;
	int32_t fsc;
	float vref = 5.0f;

	float pga = 1.0f; // TODO This can be taken from ADCON register
	// These are correct values for 30kSamples/s, from the datasheet.
	// Don't think they can be determined at runtime.
	int32_t alpha = 0x400000;
	float beta = 1.8639f;
};

float getChannelValue(uint8_t channelIdx, const CalData& cal)
{
	// From datasheet, in calibration section:
	// output = ((pga * vin / 2vref) - (ofc / alpha)) * fsc * beta
	// So
	// vin = (output / (fsc * beta) + ofc / alpha) * 2 * vref / pga
	ADS1256_WriteReg(REG_MUX, (channelIdx<<4) | (1<<3));
	ADS1256_WriteCmd(CMD_SYNC);
	ADS1256_WriteCmd(CMD_WAKEUP);
	int32_t output = ADS1256_Read_ADC_Data();

	float term1 = float(output) / (float(cal.fsc) * cal.beta);
	float term2 = float(cal.ofc) / float(cal.alpha);
	float term3 = 2 * cal.vref / cal.pga;
	
	return ((term1 + term2) * term3) / 2.0f;
}

void calibrate(CalData& calInOut)
{
	ADS1256_WriteCmd(CMD_SELFCAL);
	waitForReadyPin();

	uint8_t ofc[3];
	ofc[0] = ADS1256_Read_register(REG_OFC0);
	ofc[1] = ADS1256_Read_register(REG_OFC1);
	ofc[2] = ADS1256_Read_register(REG_OFC2);
	calInOut.ofc = signExtend24(bytesTo24(ofc));

	uint8_t fsc[3];
	fsc[0] = ADS1256_Read_register(REG_FSC0);
	fsc[1] = ADS1256_Read_register(REG_FSC1);
	fsc[2] = ADS1256_Read_register(REG_FSC2);
	calInOut.fsc = bytesTo24(fsc); // FSC is unsigned
	printf("FSC: 0x%08x %d\n", bytesTo24(fsc), calInOut.fsc);
}

int main()
{
    DEV_ModuleInit();
    signal(SIGINT, moduleExit);

	if(ADS1256_init() == 1)
	{
		printf("ADS1256_init() failed\n");
		exit(1);
	}

	CalData curState;
	calibrate(curState);
	calibrate(curState);
	calibrate(curState);
	calibrate(curState);
	calibrate(curState);
	calibrate(curState);

	printf("PGA: %f\nOFC: %d\nFSC: %d\n", curState.pga, curState.ofc, curState.fsc);
	printf("ALPHA: %d\nBETA: %f\nVREF: %f\n", curState.alpha, curState.beta, curState.vref);

	while(true)
	{
#if 0
		uint32_t inputVoltage = ADS1256_GetChannalValue(0);
		//float v = (float(inputVoltage) / 5243115.0f) * 5.0f;
		float v = (float(inputVoltage) / float(0x7fffff)) * 5.0f;
		printf("%u (%f)\n", inputVoltage, v);
#endif
		static int _i = 0;
		if(_i++ % 20 == 0)
		{
			calibrate(curState);
		}
		printf("%f\n", getChannelValue(0, curState));
		usleep(100'000);
	}
}
