#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>        
#include <stdlib.h> 
#define BAUDRATE B115200 
#define MODEMDEVICE "/dev/ttyS1"/*UART NAME IN PROCESSOR*/
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int command_fd=0, n;

struct termios SerialPortSettings;

void A7_command_readport(void)
{
  unsigned char buff;

  while (1) { 
  n = read(command_fd, &buff, 1);
   //	fcntl(command_fd,F_SETFL,0);
  if (n == -1) switch(errno) {
         case EAGAIN: /* sleep() */ 
            continue;
          
         default: break;
         }
  if (n ==0) break;
  printf("%d %c\n", n,buff);
  }
}


void A7_command_writeport(unsigned char * buff)
{
  int buff_len;
  
  buff_len = strlen(buff);

  n = write(command_fd, buff, buff_len);

                if (n < 0)
                {
                        printf("write() of bytes failed!\n");
                }
                else
                {
                        //printf("Write successfully %d\n",n);
                }
                
  
}

void A7_command_closeport(void)
{
	close(command_fd);
}
void A7_command_openport(void)
{
	 command_fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
	 
         if (command_fd < 0)
         {
			 printf("\A7 Command NOT port open :%d\n",command_fd);
         	perror(MODEMDEVICE);
         }
		 else
			 printf("\A7 Command port open :%d\n",command_fd);

		tcgetattr(command_fd, &SerialPortSettings);		 

		cfsetispeed(&SerialPortSettings,B9600);
        cfsetospeed(&SerialPortSettings,B9600);
     
}

