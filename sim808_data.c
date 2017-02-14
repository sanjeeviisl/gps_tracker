


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


int receiveGPSData() {
unsigned char buff;
int count = 0 ;
int  n = 0;

if(!GPSSim808Power(1))
	return 0;
sleep(10);
if(!GPSSim808NIMEAData(1))
	return 0;

file = fopen( "gpslog1.txt", "w+" );

while (true) { 
  n = RS232_PollComport(cport_nr,&buff,1 );
  if (n == -1) switch(errno) {
         case EAGAIN: /* sleep() */ 
            continue;
         default: goto quit;
         }
  if (n ==0) break;
  fputc(buff, file);
  printf("%c", buff);
  count++;
  if(count > 1024 *1024 * 1024) break;
  }

quit:
   fclose (file);
   GPSSim808NIMEAData(0);
   sleep(1); 
   GPSSim808Power(0);
   return n;

   
}


