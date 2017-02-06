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
void A7_command_openport(void);
void A7_command_readport(void);

int fd=0, n;
struct termios oldtp, newtp;

void A7_command_readport(void)
{
  unsigned char buff;

  while (1) { 
  n = read(fd, &buff, 1);
   //	fcntl(fd,F_SETFL,0);
  if (n == -1) switch(errno) {
         case EAGAIN: /* sleep() */ 
            continue;
          
         default: break;
         }
  if (n ==0) break;
  printf("%d %c\n", n,buff);
  }
}

void A7_command_openport(void)
{
	 fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY |O_NDELAY );
	 
         if (fd < 0)
         {
			 printf("\A7 Command NOT port open :%d\n",fd);
         	perror(MODEMDEVICE);
         }
		 else
			 printf("\A7 Command port open :%d\n",fd);
                                                                                
         fcntl(fd,F_SETFL,0);
         tcgetattr(fd,&oldtp); /* save current serial port settings */
    //   tcgetattr(fd,&newtp); /* save current serial port settings */
         bzero(&newtp, sizeof(newtp));
    //   bzero(&oldtp, sizeof(oldtp));
                                                                                
         newtp.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
                                                                                
         newtp.c_iflag = IGNPAR | ICRNL;
                                                                                
         newtp.c_oflag = 0;                                                                        
         newtp.c_lflag = ICANON;                                                                    
         newtp.c_cc[VINTR]    = 0;     /* Ctrl-c */
         newtp.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
         newtp.c_cc[VERASE]   = 0;     /* del */
         newtp.c_cc[VKILL]    = 0;     /* @ */
        // newtp.c_cc[VEOF]     = 4;     /* Ctrl-d */
         newtp.c_cc[VTIME]    = 0;     /* inter-character timer unused */
         newtp.c_cc[VMIN]     = 0;     /* blocking read until 1 character arrives */
         newtp.c_cc[VSWTC]    = 0;     /* '\0' */
         newtp.c_cc[VSTART]   = 0;     /* Ctrl-q */
         newtp.c_cc[VSTOP]    = 0;     /* Ctrl-s */
         newtp.c_cc[VSUSP]    = 0;     /* Ctrl-z */
         newtp.c_cc[VEOL]     = 0;     /* '\0' */
         newtp.c_cc[VREPRINT] = 0;     /* Ctrl-r */
         newtp.c_cc[VDISCARD] = 0;     /* Ctrl-u */
         newtp.c_cc[VWERASE]  = 0;     /* Ctrl-w */
         newtp.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
         newtp.c_cc[VEOL2]    = 0;     /* '\0' */
   	                                                                                                                                             
//	  tcflush(fd, TCIFLUSH);
//	 tcsetattr(fd,TCSANOW,&newtp);

     
}

