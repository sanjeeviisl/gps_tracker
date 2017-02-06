#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>        
#include <stdlib.h> 
#include <A7_gps_data_serial.h>

#define BAUDRATE B9600 
#define MODEMDEVICE "/dev/ttyS2"/*UART NAME IN PROCESSOR*/
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int gps_data_fd=0, n;

struct termios SerialPortSettings;

void A7_gps_data_readport(void)
{
  char read_buffer[32];                
  int  bytes_read = 0;                 
                             
  bytes_read = read(gps_data_fd,&read_buffer,32);
  
  printf("\nData Read %s",read_buffer);


}


void A7_gps_data_closeport(void)
{
	close(gps_data_fd);
}


void A7_gps_data_openport(void)
{
	 gps_data_fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
	 
         if (gps_data_fd < 0)
         {
			 printf("\A7 GPS Data port NOT port open :%d\n",gps_data_fd);
         	perror(MODEMDEVICE);
         }
		 else
			 printf("\A7 GPS Data  port open :%d\n",gps_data_fd);

		tcgetattr(gps_data_fd, &SerialPortSettings);		 
		
		SerialPortSettings.c_cflag |= CREAD | CLOCAL;//enable receiver
                                                              
		/* Setting Time outs */                                       
		SerialPortSettings.c_cc[VMIN]  = 32; /* Read 10 characters */  
		SerialPortSettings.c_cc[VTIME] = 0;  /* Wait indefinitely   */ 
		
		tcsetattr(gps_data_fd,TCSANOW,&SerialPortSettings);   

		//cfsetispeed(&SerialPortSettings,B9600);
        //cfsetospeed(&SerialPortSettings,B9600);
     
}

