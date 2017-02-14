


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

#define BAUDRATE B115200 

#include "rs232.h"
#define true 1
#define false 0

#define MODEMDEVICE "/dev/ttyUSB0"/*UART NAME IN PROCESSOR*/

int receiveGPSData() ;

FILE *file;
int fd=0,port_number, n;

int receiveGPSData() {
unsigned char buff;

if(!GPSSim808Power(1))
	return 0;
sleep(10);
if(!GPSSim808NIMEAData(1))
	return 0;


file = fopen( "gpslog1.txt", "w+" );
port_number = 1;

while (1) { 
  n = RS232_PollComport(port_number,&buff,1 );
  if (n == -1) switch(errno) {
         case EAGAIN: /* sleep() */ 
            continue;
         default: goto quit;
         }
  if (n ==0) break;
  fputc(buff, file);
  printf("%c.", buff);
  }

quit:
   fclose (file);
   GPSSim808NIMEAData(0);
   sleep(1); 
   GPSSim808Power(0);
   return 1;

   
}


