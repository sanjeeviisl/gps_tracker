


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
#include "A7_lib.h"
#include "rs232.h"

#define true 1
#define false 0

int receiveA7GPSData() ;

FILE *file;
char A7_logFileName[]="A7_gpslog.txt";

int receiveA7GPSData() {
int count = 0 ;
unsigned char buff;
unsigned char buffer[1025];
int  n = 0;

//sleep(10);
if(!GPSA7NIMEAData(1))
	{
		printf("\n GPS is NIMEA DATA failed !!!");
		return 0;
	}

file = fopen(A7_logFileName , O_WRONLY );
if(file == NULL)
    return 0;

while (true) { 
  n = RS232_PollComport(A7_data_cport_nr,&buff,1 );
  if (n == -1) switch(errno) {
         case EAGAIN:  sleep(1) ;
            continue;
         default: goto quit;
         }
  if (n == 0) {sleep(1); continue;}
  
  if(buff == '$') count++;
  if(count > 30) break;
  fputc(buff, file);
  //printf("%c", buff);
  }

quit:
   fclose (file);
   GPSA7NIMEAData(0);
   RS232_PollComport(A7_data_cport_nr,buffer,1024 );
   ClearCOMPortData();
   printf("receiveA7GPSData SUCCESS \n");
   sleep(1);
   return 1;

   
}


