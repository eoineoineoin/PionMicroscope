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

int main()
{
    DEV_ModuleInit();
    signal(SIGINT, moduleExit);

	if(ADS1256_init() == 1)
	{
		printf("ADS1256_init() failed\n");
		exit(1);
	}
	while(true)
	{
		uint32_t inputVoltage = ADS1256_GetChannalValue(0);
		float v = (float(inputVoltage) / 5243115.0f) * 5.0f;
		printf("%u (%f)\n", inputVoltage, v);
		usleep(100'000);
	}
}
