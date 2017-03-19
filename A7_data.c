


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
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>        
#include <stdlib.h> 
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include "A7_lib.h"
#include "rs232.h"

#define true 1
#define false 0

extern pthread_mutex_t lock;
extern sem_t done_filling_list;        /* barrier to sync fill_list threads and empty_list threads */
extern sem_t filling_list;             /* to protect threads_fill_done */


int n,write_position,readComplete;
unsigned char gps_data_buffer[22400];

int receiveA7GPSData() ;

int receiveA7GPSData() {
int count = 0 ;
unsigned char buff;
unsigned char buffer[10250];

n = 0;
write_position = 0;
readComplete = true;


retry2:
	if(!GPSA7Power(1))	
	{
		printf("\n GPS is power ON failed !!!");
		goto retry2;
	}
     sleep(10);

//sleep(10);
if(!GPSA7NIMEAData(1))
	{
		printf("\n GPS is NIMEA DATA failed !!!");
		return 0;
	}

while (true) { 
  n = RS232_PollComport(A7_data_cport_nr,&buff,1 );
  if (n == -1) switch(errno) {
         case EAGAIN:  sleep(1) ;
            continue;
         default: goto quit;
         }
  if (n == 0) {sleep(1); continue;}
  
  if(count > 60) {	  
	  pthread_mutex_unlock(&lock);
	  sem_post(&done_filling_list);
	  count = 0;
	  readComplete = false;
  	}
  if(readComplete)
  	{
	  if(buff == '$') count++;
	  gps_data_buffer[write_position++] = buff;
  	}
//printf("%c", buff);
}


quit:

//  GPSA7NIMEAData(0);
//  RS232_PollComport(A7_data_cport_nr,buffer,10240 );
//  ClearCOMPortData();
   printf("receiveA7GPSData SUCCESS \n");
 //  sleep(1);
   return 1;

   
}


