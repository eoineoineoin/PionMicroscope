#include <stdlib.h>     //exit()
#include <signal.h>     //signal()
#include <time.h>
#include <stdio.h>

#include <string.h>
#include <time.h>
#include <math.h>
 
#include <unistd.h>

#include <DAC8532.h>
#include <ADS1256.h>
#include <ControlServer.hpp>

void  Handler(int signo)  //This block of code allows exit of the program via "ctrl+c" when in the command Terminal, DO NOT EDIT
{
    //System Exit
    printf("\r\nEND                  \r\n");
    DEV_ModuleExit();

    exit(0);
}

// Output voltage
inline float vToOutVoltage(float vIn)
{
	return vIn / 1.55f;
}

int main(void)
{
    DEV_ModuleInit();

    // Exception handling:ctrl + c (This allows exit of program via "ctrl+c" when in command Terminal, DO NOT EDIT)
    signal(SIGINT, Handler);

	if(ADS1256_init() == 1)
	{
		printf("Failed to initialize ADS\n");
	}

	ControlServer controlServer;

//#define OUTPUT_TEST
#ifdef OUTPUT_TEST
	const float maxV = 3.3f;
	while(1)
	{
		static float v = 0.0f;
		v += 0.001f;
		if(v > maxV) v = 0.0f;

		DAC8532_Out_Voltage(channel_A, v);
	}
#endif

#define INPUT_TEST
#ifdef INPUT_TEST

	while(1)
	{
		float input = ADS1256_GetChannalValue(1);
		static int _rep = 0;
		if((_rep++ % 50) == 0)
		{
			printf("%f\n", input);
		}
		ControlState curState = { input };
		controlServer.step(curState);

		usleep(2 * 1e4);
	}
#endif
    
    DAC8532_Out_Voltage(channel_A,0);
    DAC8532_Out_Voltage(channel_B,0);
    
    return 0;
}
