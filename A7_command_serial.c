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
unsigned char read_buffer[1024];     
unsigned char read_string[1024];     

unsigned char * A7_command_readport(void)
{
int  bytes_read = 0;                 
                             
  bytes_read = read(command_fd,&read_buffer,1024);
  
  read_buffer[bytes_read] = '\0';
  printf("\nA7_command Response Read %d : %s",bytes_read, read_buffer);

  if(bytes_read > 0)
      return read_buffer;
  else
     return NULL; 
}

unsigned char * A7_command_read_string(void)
{
unsigned char buff;
int i = 0;

  while (1) { 
  n = read(command_fd, &buff, 1);
   //	fcntl(command_fd,F_SETFL,0);
  if (n == -1) switch(errno) {
         case EAGAIN: /* sleep() */ 
            continue;
          
         default: break;
         }
  if(buff == ' ') break;
  read_string[i++] = buff;
  if (n ==0) break;
  }
  
  return   read_string;
}

int A7_command_writeportln(unsigned char * buff)
{
  int buff_len;
  buff_len = A7_command_writeport(buff);
  A7_command_writeport("\n");
  return buff_len+1;
}

int A7_command_writeport(unsigned char * buff)
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
    // printf("Write %s successfully %d\n",buff, n);
     }
   return n;	                 
}


void A7_command_closeport(void)
{
	close(command_fd);
}
void A7_command_openport(void)
{
	 command_fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY |  O_NDELAY | O_NONBLOCK );
	 
         if (command_fd < 0)
         {
			 printf("\A7 Command NOT port open :%d\n",command_fd);
         	perror(MODEMDEVICE);
         }
		 else
			 printf("\A7 Command port open :%d\n",command_fd);

		tcgetattr(command_fd, &SerialPortSettings);		 
		
		SerialPortSettings.c_cflag |= CREAD | CLOCAL;//enable receiver
                                                              
		/* Setting Time outs */                                       
		SerialPortSettings.c_cc[VMIN]  = 32; /* Read 10 characters */  
		SerialPortSettings.c_cc[VTIME] = 10;  /* Wait indefinitely   */ 
		
		tcsetattr(command_fd,TCSANOW,&SerialPortSettings);   
		 
		// A7_command_baudrate(B9600);
     
}

void A7_command_baudrate(char * baud_rate)
{
		tcgetattr(command_fd, &SerialPortSettings);		 

		cfsetispeed(&SerialPortSettings,B9600);
        cfsetospeed(&SerialPortSettings,B9600);

}