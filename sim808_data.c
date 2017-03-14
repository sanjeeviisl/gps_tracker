


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
#include "sim808_lib.h"
#include "rs232.h"

#define true 1
#define false 0

int receiveGPSData() ;

FILE *file;
char logFileName[]="_gpslog.txt";

int receiveGPSData() {
unsigned char buff;
int count = 0 ;
int  n = 0;

if(!GPSSim808Power(1))
{
	printf("\n GPS is power ON failed !!!");
	return 0;
}
//sleep(10);
if(!GPSSim808NIMEAData(1))
	{
		printf("\n GPS is NIMEA DATA failed !!!");
		return 0;
	}

file = fopen(logFileName , "w+" );

while (true) { 
  n = RS232_PollComport(cport_nr,&buff,1 );
  if (n == -1) switch(errno) {
         case EAGAIN:  sleep(1) ;
            continue;
         default: goto quit;
         }
  if (n == 0) {sleep(1); continue;}
  
  if(buff == '$') count++;
  if(count > 10) break;
  fputc(buff, file);
  //printf("%c", buff);
  }

quit:
   fclose (file);
   GPSSim808NIMEAData(0);
   return 1;

   
}


