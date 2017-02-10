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

unsigned char * MapForward

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

 

 

int ReadFromComport(int cport_nr,unsigned char *buf,int size,useconds_t count) //size=6000
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
      printf("%s\n", (char *)buf);
      break;
    }
}
return n;
}

 

void Resetbufer(unsigned char *buf,int size)

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

int http_post_main()

{

int      cport_nr=0,        /* /dev/ttyUSB5 Look above to get the port code. () */

bdrate=9600;       /* 9600 baud */

 

const unsigned char OKToken[]={"OK"};

 

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

    RS232_cputs(cport_nr, "AAAAAAAAAAAAAT\r\n");    

    //usleep(5000000);  /* sleep for 100 milliSeconds */  

    Resetbufer(buf,sizeof(buf));

    ReadComport(cport_nr,buf,6000,500000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)

goto exit;

 

    RS232_cputs(cport_nr, "AT+SAPBR=3,1,\"ConType\",\"GPRS\"\r\n");    

    //usleep(5000000);  /* sleep for 100 milliSeconds */

    Resetbufer(buf,sizeof(buf));

    ReadComport(cport_nr,buf,6000,500000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)

goto exit;

 

    RS232_cputs(cport_nr, "AT+SAPBR=3,1,\"APN\",\"www\"\r\n");    

    //usleep(5000000);  /* sleep for 100 milliSeconds */

    Resetbufer(buf,sizeof(buf));

    ReadComport(cport_nr,buf,6000,500000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)

goto exit;

 

    RS232_cputs(cport_nr, "AT+SAPBR=3,1,\"USER\",\"\"\r\n");    

    //usleep(5000000);  /* sleep for 100 milliSeconds */

    Resetbufer(buf,sizeof(buf));

    ReadComport(cport_nr,buf,6000,500000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)

goto exit;

 

    RS232_cputs(cport_nr, "AT+SAPBR=3,1,\"PWD\",\"\"\r\n");    

    //usleep(5000000);  /* sleep for 100 milliSeconds */

    Resetbufer(buf,sizeof(buf));

    ReadComport(cport_nr,buf,6000,500000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)

goto exit;

 

    RS232_cputs(cport_nr, "AT+SAPBR=1,1\r\n");    

    usleep(5000000);  /* sleep for 100 milliSeconds */

    Resetbufer(buf,sizeof(buf));

    ReadComport(cport_nr,buf,6000,1000000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)"ERROR",5) == NULL)

    {

        RS232_cputs(cport_nr, "AT+SAPBR=1,0\r\n");    

        //usleep(5000000);  /* sleep for 100 milliSeconds */

        Resetbufer(buf,sizeof(buf));

        ReadComport(cport_nr,buf,6000,500000);    

        if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)

   goto exit;

        goto restart;

    }

 

    RS232_cputs(cport_nr, "AT+SAPBR=2,1\r\n");    

    

    Resetbufer(buf,sizeof(buf));

    ReadComport(cport_nr,buf,6000,500000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)

goto exit;

        

    RS232_cputs(cport_nr, "AT+HTTPINIT\r\n");    

    

    Resetbufer(buf,sizeof(buf));

    ReadComport(cport_nr,buf,6000,500000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)"ERROR",5) != NULL)

    {

        RS232_cputs(cport_nr, "AT+HTTPTERM\r\n");    

        //usleep(5000000);  /* sleep for 100 milliSeconds */

        Resetbufer(buf,sizeof(buf));

        ReadComport(cport_nr,buf,6000,500000);    

        if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)

   goto exit;

        goto restart;

    }

 

        

    RS232_cputs(cport_nr, "AT+HTTPPARA=\"CID\",1\r\n");    

    

    Resetbufer(buf,sizeof(buf));

    ReadComport(cport_nr,buf,6000,500000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)

goto exit;

 

    RS232_cputs(cport_nr, "AT+HTTPPARA=\"URL\",\"");    

 

    RS232_cputs(cport_nr, (const char*)URL);

    RS232_cputs(cport_nr, "\"\r\n");    

    

    Resetbufer(buf,sizeof(buf));

    ReadComport(cport_nr,buf,6000,500000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)

goto exit;

        

    RS232_cputs(cport_nr, "AT+HTTPPARA=\"CONTENT\",\"multipart/form-data; boundary=----WebKitFormBoundaryvZ0ZHShNAcBABWFy\"\r\n");    

    

    Resetbufer(buf,sizeof(buf));

    ReadComport(cport_nr,buf,6000,500000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)

goto exit;

 

    RS232_cputs(cport_nr, "AT+HTTPDATA=");    

    sprintf(str,"%d,10000\r\n",fsize+182);

    RS232_cputs(cport_nr, str);    

    

    Resetbufer(buf,sizeof(buf));

    ReadComport(cport_nr,buf,6000,500000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)"DOWNLOAD",8) == NULL)

goto exit;

    

  //printf("%s",string);

  RS232_cputs(cport_nr, "------WebKitFormBoundaryvZ0ZHShNAcBABWFy\n");

RS232_cputs(cport_nr, "Content-Disposition: form-data; name=\"fileToUpload\"; filename=\"");

RS232_cputs(cport_nr, (const char*)FILENAME);

RS232_cputs(cport_nr, "\"\n");

RS232_cputs(cport_nr, "Content-Type: text/plain\n\n");

  RS232_cputs(cport_nr, string);    

RS232_cputs(cport_nr, "\n------WebKitFormBoundaryvZ0ZHShNAcBABWFy\n");

  //RS232_SendByte(cport_nr,0x1A);

  //RS232_SendByte(cport_nr,0x1A);

  //RS232_SendByte(cport_nr,0x1A);

 

    

    Resetbufer(buf,sizeof(buf));

usleep(5000000);

    ReadComport(cport_nr,buf,6000,12000000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)

goto exit;

 

    RS232_cputs(cport_nr, "AT+HTTPACTION=1\r\n");    

    

    Resetbufer(buf,sizeof(buf));

usleep(5000000);

usleep(5000000);

usleep(5000000);

usleep(5000000);

usleep(5000000);

usleep(5000000);

usleep(5000000);

usleep(5000000);

    ReadComport(cport_nr,buf,6000,8000000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)"ACTION:",7) == NULL)

goto exit;

 

    RS232_cputs(cport_nr, "AT+HTTPREAD\r\n");    

    

    Resetbufer(buf,sizeof(buf));

usleep(5000000);

    ReadComport(cport_nr,buf,6000,6000000);    

    if(MapForward(buf,buf_SIZE,(unsigned char*)"READ:",5) == NULL)

goto exit;

 

    goto SUCCESS;

    //free(string);

    SUCCESS: printf("SUCCESS");

    return(0);

    exit: printf("FAILED");

    return(0);

}

 
