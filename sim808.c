
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "rs232.h"


/*******************************************************************************
* Funtion name : MapForward                                                    *
*                                                                              *
* Description  : This function performs forward mapping and returns pointer    *
*                to the start of the found data or else returns NULL pointer   *
*                                                                              *
* Arguments    : 1) Poshorter to the data in which mapping is to be made       *
*                2) Length of the data in which mapping is to be made          *
*                3) Poshorter to the data points to be mapped                  *
*                4) Length of the data points to be mapped                     *
*                                                                              *
* Returns      : Pointer in which result is returned                           *
*******************************************************************************/
unsigned char * MapForwardReturn
(
	unsigned char *pMapData,
	unsigned short   MapDataLength,
	unsigned char *pMapPoints,
	unsigned short   MapPointsLength	
)
{
    unsigned short DataIndex;
    unsigned short MapPointIndex;

    for(DataIndex = 0; DataIndex < MapDataLength - MapPointsLength + 1; DataIndex++)
    {
        
        for(MapPointIndex = 0; MapPointIndex < MapPointsLength; MapPointIndex++)
        {
            if( pMapData[DataIndex + MapPointIndex] != pMapPoints[MapPointIndex])
            {
                goto PICK_NEXT_FMAPDATA;
            }
        }

        return(& pMapData[DataIndex]);
        
    PICK_NEXT_FMAPDATA:;

    }
    return(NULL);
}


int ReadComport(int cport_nr,unsigned char *buf,int size,useconds_t count) //size=6000
{
	int n;
        n=0;
	usleep(count);
  while(1)
  {

    n = RS232_PollComport(cport_nr, buf, size);

    if(n > 0)
    {
      buf[n] = 0;   /* always put a "null" at the end of a string! */

      //for(i=0; i < n; i++)
      //{
        //if(buf[i] < 32)  /* replace unreadable control-codes by dots */
        //{
        //  buf[i] = '.';
        //}
      //}

      printf("%s\n", (char *)buf);
      break;
    }
}

	return n;
}

void Resetbufer1(unsigned char *buf,int size)
{
	int i;
	for(i=0;i<size;i++)
	{
		buf[i] = '0';
	}
}
/*
char comports[38][16]={"/dev/ttyS0","/dev/ttyS1=2","/dev/ttyS2","/dev/ttyS3","/dev/ttyS4","/dev/ttyS5",
                       "/dev/ttyS6","/dev/ttyS7","/dev/ttyS8","/dev/ttyS9","/dev/ttyS10","/dev/ttyS11",
                       "/dev/ttyS12","/dev/ttyS13","/dev/ttyS14","/dev/ttyS15","/dev/ttyUSB0=16",
                       "/dev/ttyUSB1=17","/dev/ttyUSB2=18","/dev/ttyUSB3=19","/dev/ttyUSB4=20","/dev/ttyUSB5=21",
                       "/dev/ttyAMA0","/dev/ttyAMA1","/dev/ttyACM0","/dev/ttyACM1",
                       "/dev/rfcomm0","/dev/rfcomm1","/dev/ircomm0","/dev/ircomm1",
                       "/dev/cuau0","/dev/cuau1","/dev/cuau2","/dev/cuau3",
                       "/dev/cuaU0","/dev/cuaU1","/dev/cuaU2","/dev/cuaU3"};


*/
int sim808_main()
{
	int      cport_nr=21,        /* /dev/ttyUSB5 Look above to get the port code. () */
	bdrate=9600;       /* 9600 baud */

	const unsigned char OKToken[]={"OK"};
	//unsigned char URL[]={"http://www.marsinnovations.in/upload.php"};
	unsigned char URL[]={"http://posttestserver.com/post.php"};

	unsigned char FILENAME[]={"data.txt"}; // File to be sent
	char mode[]={'8','N','1',0},
	str[512];

	unsigned char buf[6000];
	int buf_SIZE=sizeof(buf);

	int fsize;	
	char string[10000];


	FILE *f = fopen((const char*)FILENAME, "rb");
	if (f != NULL)
	{
		fseek(f, 0, SEEK_END);
		fsize = ftell(f);
		fseek(f, 0, SEEK_SET);

		//char *string = malloc(fsize + 1);
		fread(string, fsize, 1, f);
		fclose(f);
	
		string[fsize] = 0;
		printf("%s",string);
		
	}
	else
	{
		printf("Doesnt exist");
	}



  if(RS232_OpenComport(cport_nr, bdrate, mode))
  {
    printf("Can not open comport\n");

    return(0);
  }
restart:
    //In my case i am using a SIM900 GSM modem to communicate using serial port,
    // using Linux & a CP2102 USB-UART brudge
    // Send AT command to GSM modem. 
    RS232_cputs(cport_nr, "AT\r\n");    

    // Clear the buffer i am using for reading data into.
    Resetbufer1(buf,sizeof(buf));
    // Read the data received in the serial port buffer.
    ReadComport(cport_nr,buf,6000,500000);    
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
	goto exit;

    // You can send individual bytes to the serial/COM port using this function
    RS232_SendByte(cport_nr,0x1A); // 0x1A is a hex equivalent of Ctrl^Z key press


    
    SUCCESS: printf("SUCCESS");
    return(0);
    exit: printf("FAILED");
    return(0);
}


