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
#include <unistd.h>
#include <gpio_lib.h>
#include "A7_lib.h"
#include "rs232.h"
#define true 1
#define false 0

int A7_GPSPowerON = false;
int A7_httpInitialize = false;
int A7_dataConnected = false;

extern char * A7_latitude_str;
extern char * A7_longitude_str;
extern char A7_updated_time_str[6];
extern char A7_updated_date_str[8];


int A7_commond_cport_nr=1,    /* /dev/ttyS1 */
    A7_commond_bdrate=115200; /* 115200 baud */
char A7_commond_mode[]={'8','N','1',0},str[512];

int A7_data_cport_nr=2,    /* /dev/ttyS2 */
    A7_data_bdrate=9600; /* 9600 baud */
char A7_data_mode[]={'8','N','1',0},str[512];


char A7_device_id_str[10];

const unsigned char A7_OKToken[]={"OK"};
const unsigned char A7_Token[]={">"};

unsigned char A7_buf[6000];
int A7_buf_SIZE=sizeof(A7_buf);


int resetHardA7GSMModule() {

    char gsm_power_soft_reset[]= "AT+RST=1\r\n";

restart:
    printf("going to reset the A7 GSM Module... ");
	RS232_cputs(A7_commond_cport_nr, gsm_power_soft_reset);
	Resetbufer(A7_buf,sizeof(A7_buf));
	ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	sleep(10);
	// Check if "OK" string is present in the received data 
	if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
		goto exit;

retry1:
	if(!A7DataConnect())
	 {
	   
	   printf("\n GPSRS Data is not connected !!!");
	   	resetHardA7GSMModule();
	   goto retry1;
    }
	else
		printf("\n GPSRS Data is connected !!!");

retry2:
	if(!GPSA7Power(1))
	{
		printf("\n GPS is power ON failed !!!");
		goto retry2;

	}
	else
		printf("\n GPS is enabled!!!");


exit: goto restart;
	
}


int powerONA7GSMModule() {
     printf("Power ON the A7 Module : ");
	 A7_GPS_GSM_Module_Power();
     sleep(4);
	 getA7DeviceInfo();

}

int resetHardA7GPSModule(int n) {
	
	char gps_power_cold_reset[]= "AT+GPS=1\r\n";
	char gps_power_hot_reset[]= "AT+GPS=1\r\n";
	char gps_power_warm_reset1[]= "AT+GPS=1\r\n";

	restart:
	if(n == 0)
	{
			RS232_cputs(A7_commond_cport_nr, gps_power_cold_reset);
			Resetbufer(A7_buf,sizeof(A7_buf));
			ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
			// Check if "OK" string is present in the received data 
			if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
				goto exit;
	sleep(10);
	}
	
	if(n == 1)
	{
			RS232_cputs(A7_commond_cport_nr, gps_power_hot_reset);
			Resetbufer(A7_buf,sizeof(A7_buf));
			ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
			// Check if "OK" string is present in the received data 
			if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
				goto exit;
	sleep(15);
	}

	if(n == 2)
	{
			RS232_cputs(A7_commond_cport_nr, gps_power_warm_reset1);
			Resetbufer(A7_buf,sizeof(A7_buf));
			ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
			// Check if "OK" string is present in the received data 
			if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
				goto exit;
	sleep(10);
	}

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

restart:

//printf("%s",device_string1);

    RS232_cputs(A7_commond_cport_nr, device_string1);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
        goto exit;


//printf("%s",device_string2);

    RS232_cputs(A7_commond_cport_nr, device_string2);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
        goto exit;

//printf("%s",device_string3);

    RS232_cputs(A7_commond_cport_nr, device_string3);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
        goto exit;


SUCCESS: printf("\nDEVICE INFO SUCCESS\n");
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
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;
		sleep(30);
		A7_GPSPowerON = true;

		}

	A7_GPSPowerON = true;

}
else
{

//printf("%s",gps_power_string2);
	A7_GPSPowerON = false;
    RS232_cputs(A7_commond_cport_nr, gps_power_string2);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
        goto exit;
}


SUCCESS: printf("\nGPS POWER SUCCESS\n");
return(1);
exit: printf("\nGPS POWER FAILED\n");
return(0);

	
}



int GPSA7NIMEAData(int ON) {


char nimea_data_string1[]= "AT+GPSRD=2\r\n"; //NIMEA DATA ON
char nimea_data_string2[]= "AT+GPSRD=0\r\n";  //NIMEA DATA OFF

if(ON)
{
//printf("%s",nimea_data_string1);

    RS232_cputs(A7_commond_cport_nr, nimea_data_string1);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
        goto exit;
	sleep(1);
}
else
{

//printf("%s",nimea_data_string2);

    RS232_cputs(A7_commond_cport_nr, nimea_data_string2);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
        goto exit;

}

SUCCESS: printf("\nNIMEA DATA SUCCESS\n");
return(1);
exit: printf("\nNIMEA DATA FAILED");
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

	char data_disconnect_string1[]= "AT+CGACT=0,1\r\n";
	
	restart:
	
	//printf("%s",data_connect_string1);
	
		if(!A7_dataConnected) {
			n++;
		RS232_cputs(A7_commond_cport_nr, data_connect_string1);
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		// Check if "OK" string is present in the received data 
		if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;
		sleep(2);
	
	//printf("%s",data_connect_string2);
		n++;
		RS232_cputs(A7_commond_cport_nr, data_connect_string2);
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		// Check if "OK" string is present in the received data 
		if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;
		sleep(3);
	
	//printf("%s",data_connect_string3);
		n++;
		RS232_cputs(A7_commond_cport_nr, data_connect_string3);
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		// Check if "OK" string is present in the received data 
		if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;
	
	//printf("%s",data_connect_string4);
		n++;
		RS232_cputs(A7_commond_cport_nr, data_connect_string4);
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		// Check if "OK" string is present in the received data 
		if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;
		sleep(10);
	
	
	//printf("%s",data_connect_string5);
		n++;
		RS232_cputs(A7_commond_cport_nr, data_connect_string5);
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		// Check if "OK" string is present in the received data 
		if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;
		sleep(5);
	
	//printf("%s",data_connect_string6);
		A7_dataConnected =true;
		}
	
		n++;
		RS232_cputs(A7_commond_cport_nr, data_connect_string6);
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		// Check if "OK" string is present in the received data 
		if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;
	
	SUCCESS: printf("\nDATA CONNECT SUCCESS \n");
	return(1);
	exit: printf("DATA CONNECT FAILED \n ");
	if(n < 30)
		goto restart;
	else
		return(0);
	
	}
	


int sendA7DataToTCPServer()
{
	
char latitude_string[ 112 ];
char longitude_string[ 112 ];
char updated_date_string[ 112 ];
char updated_time_string[ 112 ];

char tcp_string1[]= "at+cipstatus\r\n";
char tcp_string2[]= "AT+CIPSTART=\"TCP\",\"www.iisl.co.in\",80\r\n";
char tcp_string3[]= "at+cipstatus\r\n";
char tcp_string4[]= "AT+CIPSEND\r\n";	
char tcp_string5[]= "{\"method\": \"put\",\"resource\": \"/feeds/42742/\",\"params\"";
char tcp_string6[]= ": {},\"body\":\"gps_control_panel\gps_mapview\adddevicelocation.php\"";
char tcp_string7[]= " {\"version\": \"1.0.0\",\"datastreams\": ";

char tcp_string8[]= "[{\"latitude\": \"" ;
char tcp_string9[]= "70.000" ;
char tcp_string10[]= "\"},";

char tcp_string11[]= "{\"longitude\": \"" ;
char tcp_string12[]= "20.0000" ;
char tcp_string13[]= "\"},";

char tcp_string14[]= "{\"utcdate_stamp\": \"" ;
char tcp_string15[]= "20.0000" ;
char tcp_string16[]= "\"},";


char tcp_string17[]= "{\"utctime_stamp\": \"" ;
char tcp_string18[]=  "124050 ";
char tcp_string19[] = "\"}]}}\r\n";


char end_of_file_byte = (char)26;

char tcp_string_end[1];
char tcp_string_end1[1]= "\r\n";

char tcp_string20[]= "AT+CIPCLOSE\r\n";
char tcp_string21[]= "at+cifsr\r\n";


tcp_string_end[0] = end_of_file_byte;
strcpy(A7_device_id_str,"1234567890");

//printf("%s",http_string);
restart:

		//printf("%s",http_string1);
		    RS232_cputs(A7_commond_cport_nr, tcp_string1);
		    Resetbufer(A7_buf,sizeof(A7_buf));
		    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		    // Check if "OK" string is present in the received data 
		    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
		        goto exit;

		//printf("%s",http_string2);
		    RS232_cputs(A7_commond_cport_nr, tcp_string2);
		    Resetbufer(A7_buf,sizeof(A7_buf));
		    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		    // Check if "OK" string is present in the received data 
		    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
		        goto exit;

		sleep(1);

		//printf("%s",http_string3);

		    RS232_cputs(A7_commond_cport_nr, tcp_string3);
		    Resetbufer(A7_buf,sizeof(A7_buf));
		    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		    // Check if "OK" string is present in the received data 
		    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
		        goto exit;
/*
		//printf("%s",http_string4);
			
			RS232_cputs(A7_commond_cport_nr, tcp_string4);
			Resetbufer(A7_buf,sizeof(A7_buf));
			ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
			// Check if ">" string is present in the received data 
			if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_Token,2) == NULL)
				goto exit;

	//printf("%s",http_string5);
		RS232_cputs(A7_commond_cport_nr, tcp_string5);
	//printf("%s",http_string6);
	    RS232_cputs(A7_commond_cport_nr, tcp_string6);
	//printf("%s",http_string7);
	    RS232_cputs(A7_commond_cport_nr, tcp_string7);

		//snprintf( latitude_string, sizeof( latitude_string ), "%s%s%s", tcp_string8,A7_latitude_str,tcp_string10 );
		//snprintf( longitude_string, sizeof( longitude_string ), "%s%s%s", tcp_string11,A7_longitude_str,tcp_string13 );
		//snprintf( updated_date_string, sizeof( latitude_string ), "%s%s%", tcp_string14,A7_updated_date_str,tcp_string16 );
		//snprintf( updated_time_string, sizeof( latitude_string ), "%s%s%s", tcp_string17,A7_updated_time_str,tcp_string19 );

		snprintf( latitude_string, sizeof( latitude_string ), "%s%s%s", tcp_string8,tcp_string9,tcp_string10 );
		snprintf( longitude_string, sizeof( longitude_string ), "%s%s%s", tcp_string11,tcp_string10,tcp_string13 );
		snprintf( updated_date_string, sizeof( latitude_string ), "%s%s%", tcp_string14,tcp_string15,tcp_string16 );
		snprintf( updated_time_string, sizeof( latitude_string ), "%s%s%s", tcp_string17,tcp_string18,tcp_string19 );
		
		RS232_cputs(A7_commond_cport_nr, latitude_string);
		RS232_cputs(A7_commond_cport_nr, longitude_string);
		RS232_cputs(A7_commond_cport_nr, updated_date_string);
		RS232_cputs(A7_commond_cport_nr, updated_time_string);		

	    RS232_cputs(A7_commond_cport_nr, tcp_string_end);
		RS232_cputs(A7_commond_cport_nr, tcp_string_end1);
		
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;
		sleep(2);
*/	
		//printf("%s",http_string22);

	    RS232_cputs(A7_commond_cport_nr, tcp_string20);
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;
		sleep(1);
		
	    RS232_cputs(A7_commond_cport_nr, tcp_string21);
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;
		sleep(1);

	SUCCESS: printf("\n SEND DATA SUCCESS \n");
	return(1);
	exit: printf("\n SEND DATA FAILED\ n");
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









