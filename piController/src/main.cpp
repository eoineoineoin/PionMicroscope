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
	(void)signo;
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
