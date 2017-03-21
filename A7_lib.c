#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gps_init.h>
#include <A7_command_serial.h>
#include <A7_gps_data_serial.h>
#include <A7_http_data_server.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <gpio_lib.h>
#include "A7_lib.h"
#include "rs232.h"
#define true 1
#define false 0


extern struct gpsStruct gps;
extern pthread_mutex_t lock;
extern sem_t done_filling_list;
extern sem_t filling_list;


int A7_GPSPowerON = false;
int A7_httpInitialize = false;
int A7_dataConnected = false;


int A7_commond_cport_nr=1,    /* /dev/ttyS1 */
    A7_commond_bdrate=115200; /* 115200 baud */
char A7_commond_mode[]={'8','N','1',0},str[512];

int A7_data_cport_nr=2,    /* /dev/ttyS2 */
    A7_data_bdrate=9600; /* 9600 baud */
char A7_data_mode[]={'8','N','1',0},str[512];


char A7_device_id_str[1024];

const unsigned char A7_OKToken[]={"OK"};
const unsigned char A7_Token[]={">"};

unsigned char A7_buf[6000];
int A7_buf_SIZE=sizeof(A7_buf);

char  A7_updated_time_str[7];
char  A7_updated_date_str[7];

static unsigned char * MapForward (unsigned char *pMapData, unsigned short   MapDataLength,
unsigned char *pMapPoints, unsigned short   MapPointsLength )
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


char *dtostrf (double val, signed char width, unsigned char prec, char *sout) { 
   char fmt[20]; 
   sprintf(fmt, "%%%d.%df", width, prec); 
   sprintf(sout, fmt, val); 
   return sout; 
} 

void Resetbufer(unsigned char *buf,int size)
{
        int i;
        for(i=0;i<size;i++)
        {
                buf[i] = '0';
        }
}


int resetSoftA7GSMModule(){
    char gsm_power_soft_reset[]= "AT+RST=1\r\n";

    printf("going to soft reset the A7 GSM Module... ");
	RS232_cputs(A7_commond_cport_nr, gsm_power_soft_reset);
	Resetbufer(A7_buf,sizeof(A7_buf));
	ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
}

int resetHardA7GSMModule() {

    char gsm_power_soft_reset[]= "AT+RST=1\r\n";
	
	A7_GPSPowerON = false;
	A7_httpInitialize = false;
	A7_dataConnected = false;

restart:
    printf("going to reset the A7 GSM Module... ");
	RS232_cputs(A7_commond_cport_nr, gsm_power_soft_reset);
	Resetbufer(A7_buf,sizeof(A7_buf));
	ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	sleep(35);


retry2:
	if(!GPSA7Power(1))
	{
		printf("\n GPS is power ON failed !!!");
		//goto retry2;

	}
	else
		{
		printf("\n GPS is enabled!!!");
		
		GPSA7NIMEAData(1);
		sleep(1);
		return 1;
		}
	return 0;

	
}


int powerONA7GSMModule() {
     printf("Power ON the A7 Module : ");
	 A7_GPS_GSM_Module_Power();
     sleep(4);
}



int resetHardA7GPSModule(int n) {
	
	char gps_power_off[]= "AT+GPS=0\r\n";
	char gps_power_on[]= "AT+GPS=1\r\n";
	char gps_power_warm_reset1[]= "AT\r\n";

	restart:
			RS232_cputs(A7_commond_cport_nr, gps_power_off);
			sleep(1);
			Resetbufer(A7_buf,sizeof(A7_buf));
			ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
			//if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
				//goto exit;


			RS232_cputs(A7_commond_cport_nr, gps_power_on);
			Resetbufer(A7_buf,sizeof(A7_buf));
			ReadComport(A7_commond_cport_nr,A7_buf,6000,5000000);
			//if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
				//goto exit;
			sleep(30);
			GPSA7NIMEAData(1);
			sleep(1);

	SUCCESS: printf("\nGPS RESET SUCCESS\n");
	return(1);
	exit: printf("\nGPS RESET FAILED Try Again\n");
	goto restart;



}



int openA7Port() {
	if(RS232_OpenComport(A7_commond_cport_nr, A7_commond_bdrate, A7_commond_mode))
		{
	    printf("Can not open comport A7_commond_mode\n");
	    return(0);
		}
	if(RS232_OpenComport(A7_data_cport_nr, A7_data_bdrate, A7_data_mode))
		{
		printf("Can not open A7_data_cport_nr\n");
		return(0);
		}
    return(1);
}

void A7_GPS_GSM_Module_Power()
{
sunxi_gpio_init();
sunxi_gpio_set_cfgpin(SUNXI_GPA(7), SUNXI_GPIO_OUTPUT);
sunxi_gpio_output(SUNXI_GPA(7), 1);
sleep(2);
sunxi_gpio_output(SUNXI_GPA(7), 0);
printf("A7 POWR ON OFF\n");
A7_GPSPowerON = false;
A7_httpInitialize = false;
A7_dataConnected = false;
}

int getA7DeviceInfo() {

char device_string1[]= "AT\r\n";
char device_string2[]= "AT+CGMM\r\n";
char device_string3[]= "AT+CGMI\r\n";
char device_string4[]= "ATE0\r\n";



restart:

    RS232_cputs(A7_commond_cport_nr, device_string1);
	sleep(1);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
        goto exit;


    RS232_cputs(A7_commond_cport_nr, device_string2);
	sleep(1);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
        goto exit;


    RS232_cputs(A7_commond_cport_nr, device_string3);
	sleep(1);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
        goto exit;

	//strcpy(A7_device_id_str,A7_buf);

    RS232_cputs(A7_commond_cport_nr, device_string4);
	sleep(1);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
        goto exit;


SUCCESS: printf("DEVICE INFO SUCCESS\n");
return(1);
exit: printf("\nDEVICE INFO FAILED , MAY BE DEVICE IS POWER OFF !\n");
	powerONA7GSMModule();
	goto restart;

	
}


int GPSA7Power(int ON) {
char gps_power_string1[]= "AT+GPS=1\r\n";
char gps_power_string2[]= "AT+GPS=0\r\n";
if(ON)
{

	if(!A7_GPSPowerON){
	    RS232_cputs(A7_commond_cport_nr, gps_power_string1);
   		sleep(2);
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;
   		sleep(40);
		printf("GPS POWER ON SUCCESS\n");
		A7_GPSPowerON = true;
		}
	else{
		printf("ALREADY GPS POWER ON SUCCESS\n");
		A7_GPSPowerON = true;
		}
}
else
{

	A7_GPSPowerON = false;
    RS232_cputs(A7_commond_cport_nr, gps_power_string2);
	sleep(2);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
        goto exit;
	printf("GPS POWER OFF SUCCESS\n");
}


SUCCESS: printf("GPS POWER SUCCESS\n");
return(1);
exit: printf("\nGPS POWER FAILED\n");
	A7_GPSPowerON = false;
return(0);

	
}


int ClearCOMPortData() {
	
	char data_string1[]= "AT\r\n"; 
	RS232_cputs(A7_commond_cport_nr, data_string1);
	sleep(2);
	Resetbufer(A7_buf,sizeof(A7_buf));
	ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);

}

int GPSA7NIMEAData(int ON) {


char nimea_data_string1[]= "AT+GPSRD=2\r\n"; //NIMEA DATA ON
char nimea_data_string2[]= "AT+GPSRD=0\r\n";  //NIMEA DATA OFF


if(ON)
{
    RS232_cputs(A7_commond_cport_nr, nimea_data_string1);
	sleep(1);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
   // if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
     //   goto exit;
	printf("NIMEA DATA STARTED\n");
}
else
{

    RS232_cputs(A7_commond_cport_nr, nimea_data_string2);
	sleep(1);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
        goto exit;

	printf("NIMEA DATA STOPPED\n");

}

SUCCESS: //printf("NIMEA DATA SUCCESS\n");
return(1);
exit: printf("\nNIMEA DATA FAILED");

return(0);

	
}


int A7DataDisconnect() {
	
	int  n =0;	

	char data_disconnect_string1[]= "AT+CGACT=0,1\r\n";
	A7_dataConnected = false;
	
	restart:
		n++;
		RS232_cputs(A7_commond_cport_nr, data_disconnect_string1);
		sleep(1);
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;

	
	SUCCESS: printf("DATA DISCONNECT SUCCESS \n");
	return(1);
	exit: printf("DATA DISCONNECT FAILED \n ");
	if(n < 3)
		goto restart;
	else
		return(0);
	
	}
	

int A7DataConnect() {
	
	int  n =0;	
	char data_connect_string1[]= "AT+CREG?\r\n";
	char data_connect_string2[]= "AT+CGACT?\r\n";
	char data_connect_string3[]= "AT+CMEE=1\r\n";
	char data_connect_string4[]= "AT+CGATT=1\r\n";
	char data_connect_string5[]= "AT+CGACT=1,1\r\n";
	char data_connect_string6[]= "AT+CGPADDR=1\r\n";

	restart:

		if(!A7_dataConnected) {
		n++;
		
		RS232_cputs(A7_commond_cport_nr, data_connect_string1);
		sleep(1);
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;

		RS232_cputs(A7_commond_cport_nr, data_connect_string2);
		sleep(1);
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;
		sleep(1);
	
		RS232_cputs(A7_commond_cport_nr, data_connect_string3);
		sleep(1);
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;
	
		RS232_cputs(A7_commond_cport_nr, data_connect_string4);
		sleep(4);
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;

	
		RS232_cputs(A7_commond_cport_nr, data_connect_string5);
		sleep(8);
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;

		A7_dataConnected = true;

		}
	
	
		RS232_cputs(A7_commond_cport_nr, data_connect_string6);
		sleep(1);
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;
	
	SUCCESS: printf("DATA CONNECT SUCCESS \n");
	return(1);
	exit: printf("DATA CONNECT FAILED \n ");
		A7_dataConnected = false;
	if(n < 2)
		goto restart;
	else
		return(0);
	
	}
	

char t_buffer11[10];
char t_buffer22[10];

int sendA7DataToTCPServer(char * device_id,char * longitude,char * latitude,char * updated_time,char * updated_date)
{

char send_string[ 1024 ];

char tcp_string1[]= "at+cipstatus\r\n";
char tcp_string2[]= "AT+CIPSTART=\"TCP\",\"www.iisl.co.in\",80\r\n";
char tcp_string3[]= "at+cipstatus\r\n";
char tcp_string4[]= "AT+CIPSEND\r\n";	

char tcp_header_str1[] = "GET /adddevicelocation.php?";	
char tcp_header_str[] = "GET /gps_control_panel/gps_mapview/adddevicelocation.php?";	
char send_string1[] = "GET /adddevicelocation.php HTTP/1.1\r\n";	

char tcp_body_str[] = " HTTP/1.1\r\n";
char tcp_footer_str[] = "Host: www.iisl.co.in:80\r\n\r\n";

char end_of_file_byte = (char)26;

char tcp_string_end[1];
char tcp_string_end1[]= "\r\n";

char tcp_string20[]= "AT+CIPCLOSE\r\n";
char tcp_string21[]= "at+cifsr\r\n";


tcp_string_end[0] = end_of_file_byte;

Resetbufer(send_string,1024);

restart:

		    RS232_cputs(A7_commond_cport_nr, tcp_string1);
			sleep(1);
		    Resetbufer(A7_buf,sizeof(A7_buf));
		    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		    // Check if "OK" string is present in the received data 
		    //if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
		      //  goto exit;

		    RS232_cputs(A7_commond_cport_nr, tcp_string2);
			sleep(10);		
		    Resetbufer(A7_buf,sizeof(A7_buf));
		    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		    // Check if "OK" string is present in the received data 
		    //if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
		      //  goto exit;


		    RS232_cputs(A7_commond_cport_nr, tcp_string3);
		    Resetbufer(A7_buf,sizeof(A7_buf));
		    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		    // Check if "OK" string is present in the received data 
		    //if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
		      //  goto exit;
			
			RS232_cputs(A7_commond_cport_nr, tcp_string4);
			sleep(4);		
			Resetbufer(A7_buf,sizeof(A7_buf));
			ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
			// Check if ">" string is present in the received data 
			//if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_Token,2) == NULL)
				//goto exit;

			snprintf(send_string,sizeof(send_string),"%s%s%s%s%s%s%s%s%s%s%s%s", tcp_header_str,"device_id=",device_id,"&latitude=",latitude,"&longitude=",longitude,"&utcdate_stamp=",updated_date,"&utctime_stamp=",updated_time,tcp_body_str);
	
			RS232_cputs(A7_commond_cport_nr, send_string);

			RS232_cputs(A7_commond_cport_nr, tcp_footer_str);

		    RS232_cputs(A7_commond_cport_nr, tcp_string_end);

			RS232_cputs(A7_commond_cport_nr, tcp_string_end1);

			sleep(5);			


		    Resetbufer(A7_buf,sizeof(A7_buf));
		    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		    // Check if "OK" string is present in the received data 
		    //if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
		      //  goto exit;


		    RS232_cputs(A7_commond_cport_nr, tcp_string20);
		    Resetbufer(A7_buf,sizeof(A7_buf));
		    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		    // Check if "OK" string is present in the received data 
		  //  if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
		    //    goto exit;
			sleep(1);
			
		    RS232_cputs(A7_commond_cport_nr, tcp_string21);
		    Resetbufer(A7_buf,sizeof(A7_buf));
		    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		    // Check if "OK" string is present in the received data 
			//if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
			//     goto exit;
			sleep(1);

	SUCCESS: printf("SEND DATA SUCCESS \n");
	return(1);
	exit: printf("\n SEND DATA FAILED\ n");
	return(0);

}




int sendA7StatusToTCPServer(int testData)
	{
	
	
	char * A7_latitude_str;
	char * A7_longitude_str;
	
	char send_string[ 1024 ];
	
	
	char tcp_string1[]= "at+cipstatus\r\n";
	char tcp_string2[]= "AT+CIPSTART=\"TCP\",\"www.iisl.co.in\",80\r\n";
	char tcp_string3[]= "at+cipstatus\r\n";
	char tcp_string4[]= "AT+CIPSEND\r\n";	
	
	char tcp_header_str1[] = "GET /adddevicelocation.php?"; 
	char tcp_header_str[] = "GET /gps_control_panel/gps_mapview/adddevicelocation.php?";	
	char send_string1[] = "GET /adddevicelocation.php HTTP/1.1\r\n";	
	
	char tcp_body_str[] = " HTTP/1.1\r\n";
	char tcp_footer_str[] = "Host: www.iisl.co.in:80\r\n\r\n";
	
	char end_of_file_byte = (char)26;
	
	char tcp_string_end[1];
	char tcp_string_end1[]= "\r\n";
	
	char tcp_string20[]= "AT+CIPCLOSE\r\n";
	char tcp_string21[]= "at+cifsr\r\n";
	
	
	tcp_string_end[0] = end_of_file_byte;
	Resetbufer(send_string,1024);	
	strcpy(A7_device_id_str,"1234567890");
	if(testData)
	{
	//test data should be comment after real data
	A7_longitude_str=dtostrf(88.8888888,0,6,t_buffer11);
	A7_latitude_str=dtostrf(88.8888888,0,6,t_buffer22);
	strncpy(A7_updated_date_str,"310117",6);
	strncpy(A7_updated_time_str,"101010",6);
	A7_updated_time_str[7]= 0;
	A7_updated_date_str[7] =0 ;
	}

	pthread_mutex_lock(&lock);

	if(gps.flagDataReady)
		{
		//test data should be comment after real data
		A7_longitude_str=dtostrf(gps.longitude,0,6,t_buffer11);
		A7_latitude_str=dtostrf(gps.latitude,0,6,t_buffer22);
		//strncpy(A7_updated_time_str,gps.time,6);
		//A7_updated_time_str[7] =0 ;
		}
	
	if(gps.flagDateReady)
		{
		//strncpy(A7_updated_date_str,gps.date,6);
		//A7_updated_date_str[7] =0 ;
		}

	pthread_mutex_unlock(&lock);
		
	printf("\n Lat %s Lang %s Time %s Date %s ", A7_longitude_str,A7_latitude_str,gps.time,gps.date);
	//sleep(10);
	
	
				Resetbufer(send_string,1024);
	
				RS232_cputs(A7_commond_cport_nr, tcp_string1);
				sleep(1);
				Resetbufer(A7_buf,sizeof(A7_buf));
				ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
				// Check if "OK" string is present in the received data 
				//if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
				  //  goto exit;
	
				RS232_cputs(A7_commond_cport_nr, tcp_string2);
				sleep(10);		
				Resetbufer(A7_buf,sizeof(A7_buf));
				ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
				// Check if "OK" string is present in the received data 
				//if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
				  //  goto exit;
	
	
				//RS232_cputs(A7_commond_cport_nr, tcp_string3);
				//Resetbufer(A7_buf,sizeof(A7_buf));
				//ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
				// Check if "OK" string is present in the received data 
				//if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
				  //  goto exit;
				
				RS232_cputs(A7_commond_cport_nr, tcp_string4);
				sleep(4);		
				Resetbufer(A7_buf,sizeof(A7_buf));
				ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
				// Check if ">" string is present in the received data 
				//if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_Token,2) == NULL)
					//goto exit;
				
				snprintf(send_string,sizeof(send_string),"%s%s%s%s%s%s%s%s%s%s%s%s", tcp_header_str,"device_id=",A7_device_id_str,"&latitude=",A7_latitude_str,"&longitude=",A7_longitude_str,"&utcdate_stamp=",A7_updated_time_str,"&utctime_stamp=",A7_updated_time_str,tcp_body_str);
	
				RS232_cputs(A7_commond_cport_nr, send_string);
				RS232_cputs(A7_commond_cport_nr, tcp_footer_str);
				RS232_cputs(A7_commond_cport_nr, tcp_string_end);
				RS232_cputs(A7_commond_cport_nr, tcp_string_end1);
				sleep(5);		
				
				Resetbufer(A7_buf,sizeof(A7_buf));
				ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
				// Check if "OK" string is present in the received data 
				//if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
				  //  goto exit;
	
	
				RS232_cputs(A7_commond_cport_nr, tcp_string20);
				Resetbufer(A7_buf,sizeof(A7_buf));
				ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
				// Check if "OK" string is present in the received data 
			  //  if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
				//	  goto exit;
				sleep(1);
				
				RS232_cputs(A7_commond_cport_nr, tcp_string21);
				Resetbufer(A7_buf,sizeof(A7_buf));
				ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
				// Check if "OK" string is present in the received data 
				//if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
				//	   goto exit;
				sleep(1);
	
		SUCCESS: printf("SEND STATUS SUCCESS \n");
		return(1);
		exit: printf("\n SEND STATUS FAILED\ n");
		return(0);
		
	
	}



void startRecoveryForA7DataConnectFailed(int n){
   	resetHardA7GSMModule();

}

void startRecoveryForA7SendDataFailed(int n){
   	resetHardA7GSMModule();
	


}


void startRecoveryForA7GPSNimeaDataFailed(int n){
   	resetHardA7GPSModule(2);
	
}


void startRecoveryForA7GPSPowerFailed(int n){
   	resetHardA7GPSModule(n);
	

}


void startRecoveryForA7GPSPowerResetFailed(int n){
   	resetHardA7GSMModule();
	


}


void startRecoveryForA7ReceiveDataFailed(int n){
   	resetHardA7GPSModule(n);
}









