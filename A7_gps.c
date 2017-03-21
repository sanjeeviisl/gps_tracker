


#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include "A7_lib.h"
#include "rs232.h"

#define true 1
#define false 0

int send_count = 0;
int data_found;
extern pthread_mutex_t lock;
extern sem_t done_filling_list;
extern sem_t filling_list;


int sendA7GPSData() {


data_found =getDataStatus();

send_count++;


if(A7DataConnect())
	{
	printf("sending data to web server from buffer\n");
	//
	if(data_found)
		sendA7StatusToTCPServer(1);

	A7DataDisconnect();
	
	SUCCESS: printf("sendGPSData SUCCESS \n");
	
	return(1);
	}

	else
		{
		exit: printf("\n sendGPSData FAILED\ n");
		return(0);
		}

}

