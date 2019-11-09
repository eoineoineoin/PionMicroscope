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
#include <BeamController.hpp>

void  Handler(int signo)  //This block of code allows exit of the program via "ctrl+c" when in the command Terminal, DO NOT EDIT
{
    //System Exit
    printf("\r\nEND                  \r\n");
    DEV_ModuleExit();

    exit(0);
}

int main(void)
{
	//<TODO.eoin Move initialization inside beam controller
    DEV_ModuleInit();

    // Exception handling:ctrl + c (This allows exit of program via "ctrl+c" when in command Terminal, DO NOT EDIT)
    signal(SIGINT, Handler);

	if(ADS1256_init() == 1)
	{
		printf("Failed to initialize ADS\n");
	}

	ControlServer controlServer;

//#define OUTPUT_LINEARITY_TEST
#ifdef OUTPUT_LINEARITY_TEST
	while(1)
	{
		printf("Input: ");
		int v;
		scanf("%i", &v);
		uint16_t i16 = (uint16_t)(v);
		Write_DAC8532(channel_A, i16);
	}
#endif

//#define OUTPUT_TEST
#ifdef OUTPUT_TEST
	uint16_t vi = 0;
	while(1)
	{
		Write_DAC8532(channel_A, vi);
		vi += 1;
	}
#endif
//#define OUTPUT_TEST2
#ifdef OUTPUT_TEST2
	const float maxV = 3.3f;
	while(1)
	{
		static float v = 0.0f;
		v += 0.001f;
		if(v > maxV) v = 0.0f;

		DAC8532_Out_Voltage(channel_A, v);
	}
#endif

//#define INPUT_TEST
#ifdef INPUT_TEST

	while(1)
	{
		float input = ADS1256_GetChannalValue(1);
		static int _rep = 0;
		if((_rep++ % 50) == 0)
		{
			printf("%f\n", input);
		}
		usleep(2 * 1e4);
	}
#endif

	BeamController beamController;
	CommandHandler* commandHandler = &beamController; //<TODO.eoin Probably split this out a different way
	while(1)
	{
		Packets::BeamState beamState = beamController.step();

		ControlServer::OnConnectInfo connectionInfo;
		connectionInfo.m_resolutionX = beamController.getResolutionX();
		connectionInfo.m_resolutionY = beamController.getResolutionY();

		controlServer.step(commandHandler, connectionInfo, beamState);
	}
	printf("Finished\n");
    
    DAC8532_Out_Voltage(channel_A,0);
    DAC8532_Out_Voltage(channel_B,0);
    
    return 0;
}
