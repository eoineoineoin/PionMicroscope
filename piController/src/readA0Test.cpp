#include <ADS1256.h>
#include <DACBoard.h>
#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

void moduleExit(int)
{
	DEV_ModuleExit();
	exit(0);
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

	A2D::calibrate();

	timeval programStart;
	gettimeofday(&programStart, nullptr);
	uint32_t start_us = programStart.tv_sec * 1'000'000 + programStart.tv_usec;

	printf("Time_us, Voltage_channel0, Voltage_channel1\n");

	while(true)
	{
#if 0
		uint32_t inputVoltage = ADS1256_GetChannalValue(0);
		//float v = (float(inputVoltage) / 5243115.0f) * 5.0f;
		float v = (float(inputVoltage) / float(0x7fffff)) * 5.0f;
		printf("%u (%f)\n", inputVoltage, v);
#endif

		timeval now;
		gettimeofday(&now, nullptr);
		uint32_t now_us = now.tv_sec * 1'000'000 + now.tv_usec;
		
		float voltageC0 = A2D::getChannelValue(0);
		float voltageC1 = A2D::getChannelValue(1);
		
		printf("%d, %f, %f\n", now_us - start_us, voltageC0, voltageC1);
		usleep(10'000);
	}
}
