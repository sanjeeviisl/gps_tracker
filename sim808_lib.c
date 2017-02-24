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
#include <gpio_lib.h>
#include "sim808_lib.h"
#include "rs232.h"
#define true 1
#define false 0

#define GPS_POWER_SUCCESS 1
#define NIMEA_DATA_SUCCESS 2
#define DATA_CONNECT_SUCCESS 3
#define SEND_DATA_SUCCESS 4

#define GPS_POWER_FAILED 5
#define NIMEA_DATA_FAILED 6
#define DATA_CONNECT_FAILED 7
#define SEND_DATA_FAILED 8




int gpsPowerON = false;
int httpInitialize = false;
int dataConnected = false;
extern char * latitude_str;
extern char * longitude_str;
extern char updated_time_str[6];
extern char updated_date_str[8];


int cport_nr=1,    /* /dev/ttyS0 */
    bdrate=115200; /* 115200 baud */
char mode[]={'8','N','1',0},str[512];



char device_id_str[10];

const unsigned char OKToken[]={"OK"};
unsigned char buf[6000];
int buf_SIZE=sizeof(buf);


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
static unsigned char * MapForwardReturn
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

static int ReadComport(int cport_nr,unsigned char *buf,int size,useconds_t count) //size=6000
{
  int n=0;
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

static void Resetbufer(unsigned char *buf,int size)
{
        int i;
        for(i=0;i<size;i++)
        {
                buf[i] = '0';
        }
}


int openSIM808Port() {

if(RS232_OpenComport(cport_nr, bdrate, mode))
	{
    printf("Can not open comport\n");
    return(0);
	}
    return(1);
	
}

void Sim808_GPS_GSM_Module_Power()
{
sunxi_gpio_init();
sunxi_gpio_set_cfgpin(SUNXI_GPA(7), SUNXI_GPIO_OUTPUT);
sunxi_gpio_output(SUNXI_GPA(7), 0);
sleep(2);
sunxi_gpio_output(SUNXI_GPA(7), 1);
printf("SIM808 POWR ON OFF\n");
gpsPowerON = false;
httpInitialize = false;
dataConnected = false;
}

void Sim808_Status_ON_LED(int n)
{

sunxi_gpio_init();
sunxi_gpio_set_cfgpin(SUNXI_GPA(n), SUNXI_GPIO_OUTPUT);

switch (n){
	case GPS_POWER_SUCCESS : 
		sunxi_gpio_output(SUNXI_GPA(n), 1);		
		break;

	case NIMEA_DATA_SUCCESS : 
		sunxi_gpio_output(SUNXI_GPA(n), 1);		
		break;

	case DATA_CONNECT_SUCCESS : 
		sunxi_gpio_output(SUNXI_GPA(n), 1);		
		break;

	case SEND_DATA_SUCCESS: 
		sunxi_gpio_output(SUNXI_GPA(n), 1);		
		break;

	default : ;
	}
	
}

void Sim808_Status_OFF_LED(int n)
{

sunxi_gpio_init();
sunxi_gpio_set_cfgpin(SUNXI_GPA(n), SUNXI_GPIO_OUTPUT);

switch (n){
	case GPS_POWER_SUCCESS : 
		sunxi_gpio_output(SUNXI_GPA(n), 0);		
		break;

	default : ;
	}
	
}

int resetHardSim808GSMModule() {
     printf("going to reset the GSM Module... ");
	Sim808_GPS_GSM_Module_Power();
    sleep(2);
    Sim808_GPS_GSM_Module_Power();
	getSim808DeviceInfo();
}


int powerONSim808GSMModule() {
     printf("Power ON the Sim808 Module : ");
	 Sim808_GPS_GSM_Module_Power();
     sleep(4);

}

int powerOFFSim808GSMModule() {
     printf("Power ON the Sim808 Module : ");
	 Sim808_GPS_GSM_Module_Power();
     sleep(4);

}

int resetHardSim808GPSModule(int n) {
	
	char gps_power_cold_reset[]= "AT+CGPSRST=0\r\n";
	char gps_power_hot_reset[]= "AT+CGPSRST=1\r\n";
	char gps_power_warm_reset1[]= "AT+CGPSRST=2\r\n";

	restart:
	if(n == 0)
	{
			RS232_cputs(cport_nr, gps_power_cold_reset);
			Resetbufer(buf,sizeof(buf));
			ReadComport(cport_nr,buf,6000,500000);
			// Check if "OK" string is present in the received data 
			if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
				goto exit;
	sleep(20);
	}
	
	if(n == 1)
	{
			RS232_cputs(cport_nr, gps_power_hot_reset);
			Resetbufer(buf,sizeof(buf));
			ReadComport(cport_nr,buf,6000,500000);
			// Check if "OK" string is present in the received data 
			if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
				goto exit;
	sleep(15);
	}

	if(n == 2)
	{
			RS232_cputs(cport_nr, gps_power_warm_reset1);
			Resetbufer(buf,sizeof(buf));
			ReadComport(cport_nr,buf,6000,500000);
			// Check if "OK" string is present in the received data 
			if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
				goto exit;
	sleep(10);
	}

	SUCCESS: printf("\nGPS RESET SUCCESS\n");
	return(1);
	exit: printf("\nGPS RESET FAILED Try Again\n");
	goto restart;



}



int getSim808DeviceInfo() {

char device_string1[]= "AT\r\n";
char device_string2[]= "AT+CGMM\r\n";
char device_string3[]= "AT+CGMI\r\n";

restart:

//printf("%s",device_string1);

    RS232_cputs(cport_nr, device_string1);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;


//printf("%s",device_string2);

    RS232_cputs(cport_nr, device_string2);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;

//printf("%s",device_string3);

    RS232_cputs(cport_nr, device_string3);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;


SUCCESS: printf("\nDEVICE INFO SUCCESS\n");
return(1);
exit: printf("\nDEVICE INFO FAILED, MAY BE IT is power Off !\n");
	powerONSim808GSMModule();
        goto restart;

return(1);

	
}


int GPSSim808Power(int ON) {


int n;
char gps_power_string1[]= "AT+CGPSPWR=1\r\n";
char gps_power_string2[]= "AT+CGPSPWR=0\r\n";
char gps_power_string3[]= "AT+CGPSSTATUS?\r\n";
char gps_power_string4[]= "AT+CGPSINF=0\r\n";


if(ON)
{
//printf("%s",gps_power_string1);

	if(!gpsPowerON){
		n =0;
	    RS232_cputs(cport_nr, gps_power_string1);
	    Resetbufer(buf,sizeof(buf));
	    ReadComport(cport_nr,buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
	        goto exit;
		sleep(30);
		gpsPowerON = true;

		}


//printf("%s",gps_power_string3);
	n =1;

    RS232_cputs(cport_nr, gps_power_string3);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;

//printf("%s",gps_power_string4);
	n =2;

    RS232_cputs(cport_nr, gps_power_string4);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;

	gpsPowerON = true;

}
else
{

//printf("%s",gps_power_string2);
	n =4;

	gpsPowerON = false;
    RS232_cputs(cport_nr, gps_power_string2);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;
}


SUCCESS: printf("\nGPS POWER SUCCESS\n");
return(1);
exit: printf("\nGPS POWER FAILED\n");
	startRecoveryForGPSPowerFailed(0);
return(0);

	
}



int GPSSim808NIMEAData(int ON) {


char nimea_data_string1[]= "AT+CGPSOUT=255\r\n"; //NIMEA DATA ON
char nimea_data_string2[]= "AT+CGPSOUT=0\r\n";  //NIMEA DATA OFF

restart:

if(ON)
{
//printf("%s",nimea_data_string1);

    RS232_cputs(cport_nr, nimea_data_string1);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;
	sleep(1);
}
else
{

//printf("%s",nimea_data_string2);

    RS232_cputs(cport_nr, nimea_data_string2);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;

}

SUCCESS:// printf("\nNIMEA DATA SUCCESS\n");
return(1);
exit: 	printf("\nNIMEA DATA FAILED");
		startRecoveryForGPSNimeaDataFailed();
		goto restart;
	
}




int sendDataToServer()
{
	

char send_string[ 1024 ];

char http_string[]= "AT+CREG?\r\n";
char http_string0[]= "AT+SAPBR=2,1\r\n";
char http_string1[]= "AT+SAPBR=1,1\r\n";
char http_string2[]= "AT+HTTPINIT\r\n";
char http_header_str[] = "AT+HTTPPARA=\"URL\",\"http://iisl.co.in/gps_control_panel/gps_mapview/adddevicelocation.php?";	
char http_string4[]= "AT+HTTPPARA=\"CID\",1\r\n";
char http_string5[]= "AT+HTTPACTION=0\r\n";
char http_string6[]= "AT+HTTPREAD\r\n";
char http_string7[]= "AT+HTTPTERM\r\n";

if(longitude_str == NULL || longitude_str == NULL){
	printf("\n NO DATA TO SEND \n");
	return 1;
}
strcpy(device_id_str,"1234567890");

//printf("%s",http_string);
restart:

	if(!httpInitialize) {
	    RS232_cputs(cport_nr, http_string);
	    Resetbufer(buf,sizeof(buf));
	    ReadComport(cport_nr,buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
	        goto exit;

sleep(4);
	//printf("%s",http_string0);
	    RS232_cputs(cport_nr, http_string0);
	    Resetbufer(buf,sizeof(buf));
	    ReadComport(cport_nr,buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
	        goto exit;
sleep(4);



	//printf("%s",http_string1);

	    RS232_cputs(cport_nr, http_string1);
	    Resetbufer(buf,sizeof(buf));
	    ReadComport(cport_nr,buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
	        goto exit;
sleep(4);
	//printf("%s",http_string2);

	    RS232_cputs(cport_nr, http_string2);
	    Resetbufer(buf,sizeof(buf));
	    ReadComport(cport_nr,buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
	        goto exit;

sleep(8);

		httpInitialize = true;
		}

snprintf( send_string, sizeof( send_string ), "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", http_header_str,"device_id=",device_id_str,"&" \
          "latitude=",latitude_str,"&" , "longitude=",longitude_str,"&","utcdate_stamp=",updated_date_str,"&","utctime_stamp=",updated_time_str,"\"\r\n");

//printf("%s",send_string);

    RS232_cputs(cport_nr, send_string);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;



//printf("%s",http_string4);

    RS232_cputs(cport_nr, http_string4);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;


//printf("%s",http_string5);

    RS232_cputs(cport_nr, http_string5);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;

sleep(6);

//printf("%s",http_string6);

    RS232_cputs(cport_nr, http_string6);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;


sleep(4);
//printf("%s",http_string7);
/*
    RS232_cputs(cport_nr, http_string7);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;

sleep(4);
*/


SUCCESS: printf("\n SEND DATA SUCCESS \n");
return(1);
exit: printf("\n SEND DATA FAILED\ n");
return(0);

}

int Sim808DataConnect() {
	
int  n =0;	
char data_connect_string1[]= "AT+CREG?\r\n";
char data_connect_string2[]= "AT+CGACT?\r\n";
char data_connect_string3[]= "AT+CMEE=1\r\n";
char data_connect_string4[]= "AT+CGATT=1\r\n";
char data_connect_string5[]= "AT+CGACT=1,1\r\n";
char data_connect_string6[]= "AT+CGPADDR=1\r\n";


restart:

//printf("%s",data_connect_string1);

	if(!dataConnected) {
		n++;
    RS232_cputs(cport_nr, data_connect_string1);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;
	sleep(2);

//printf("%s",data_connect_string2);
	n++;
    RS232_cputs(cport_nr, data_connect_string2);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;
	sleep(3);

//printf("%s",data_connect_string3);
	n++;
    RS232_cputs(cport_nr, data_connect_string3);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;

//printf("%s",data_connect_string4);
	n++;
    RS232_cputs(cport_nr, data_connect_string4);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;
	sleep(10);


//printf("%s",data_connect_string5);
	n++;
    RS232_cputs(cport_nr, data_connect_string5);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;
	sleep(5);

//printf("%s",data_connect_string6);
    dataConnected =true;
    }

	n++;
    RS232_cputs(cport_nr, data_connect_string6);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;

SUCCESS: printf("\nDATA CONNECT SUCCESS \n");
return(1);
exit: printf("DATA CONNECT FAILED \n ");

return(0);

}

void startRecoveryForDataConnectFailed(int n){
   	resetHardSim808GSMModule();

}

void startRecoveryForSendDataFailed(int n){
   	resetHardSim808GSMModule();
	


}


void startRecoveryForGPSNimeaDataFailed(int n){
   	resetHardSim808GPSModule(2);
	
}


void startRecoveryForGPSPowerFailed(int n){
   	resetHardSim808GPSModule(n);
	

}


void startRecoveryForGPSPowerResetFailed(int n){
   	resetHardSim808GSMModule();
	


}


void startRecoveryForReceiveDataFailed(int n){
   	resetHardSim808GPSModule(n);
}




