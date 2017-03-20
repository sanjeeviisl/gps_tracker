


#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "A7_lib.h"
#include "rs232.h"

#define true 1
#define false 0

int send_count = 0;


//int sent = false;

//static int  charToInt(char c);
//double trunc(double d);


int sendA7GPSData() ;

int sendA7GPSData() {

int no_data_found;

no_data_found =getDataStatus();


//Test
no_data_found = 1;



if(A7DataConnect())
	{
	printf("sending data to web server from buffer\n");
	//
	if(no_data_found)
		sendA7StatusToTCPServer(1);
	else
		sendA7DataToTCPServer("A7_device1",get_A7_longitude_str(),get_A7_latitude_str(),get_A7_updated_time_str(),get_A7_updated_date_str());

	A7DataDisconnect();

	
	SUCCESS: printf("sendGPSData SUCCESS \n");
			send_count++;
	
	return(1);
	}

	else
		{
		exit: printf("\n sendGPSData FAILED\ n");
		send_count++;
		return(0);
		}

}

