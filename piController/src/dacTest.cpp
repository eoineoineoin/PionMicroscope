#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>

#include <string.h>
#include <time.h>
#include <math.h>
 
#include <unistd.h>

#include <DAC8532.h>
#include <ADS1256.h>

#include <DACBoard.h>

void Handler(int)
{
    DEV_ModuleExit();
    exit(0);
}

int main(void)
{
    DEV_ModuleInit();
    signal(SIGINT, Handler);

	if(ADS1256_init() == 1)
	{
		printf("Failed to initialize ADS\n");
	}


	DACBoard dacInterface(DACBoard::REF_5V);
	DACBoard interface2(DACBoard::REF_5V, 26);

	while(1)
	{
		float v;
		printf("DAC0 (V): ");
		scanf("%f", &v);
		dacInterface.writeVoltage(DACBoard::Channel::A, v);
		DAC8532_Out_Voltage(channel_B, v);

		printf("DAC1 (V): ");
		scanf("%f", &v);
		dacInterface.writeVoltage(DACBoard::Channel::B, v);

		printf("DAC2 (V): ");
		scanf("%f", &v);
		interface2.writeVoltage(DACBoard::Channel::A, v);

		printf("DAC3 (V): ");
		scanf("%f", &v);
		interface2.writeVoltage(DACBoard::Channel::B, v);
	}
    
    DAC8532_Out_Voltage(channel_A,0);
    DAC8532_Out_Voltage(channel_B,0);
    DEV_ModuleExit();
    
    return 0;
}
