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



extern char * latitude_str;
extern char * longitude_str;
extern char updated_time_str[6];
extern char updated_date_str[8];


int A7_commond_cport_nr=1,    /* /dev/ttyS1 */
    A7_commond_bdrate=115200; /* 115200 baud */
char A7_commond_mode[]={'8','N','1',0},str[512];

int A7_data_cport_nr=2,    /* /dev/ttyS2 */
    A7_data_bdrate=9600; /* 9600 baud */
char A7_data_mode[]={'8','N','1',0},str[512];


char A7_device_id_str[10];

const unsigned char A7_OKToken[]={"OK"};
unsigned char A7_buf[6000];
int A7_buf_SIZE=sizeof(A7_buf);


int resetHardA7GSMModule() {
     printf("going to reset the GSM Module... ");
	Sim808_GPS_GSM_Module_Power();
    sleep(2);
    Sim808_GPS_GSM_Module_Power();
	getSim808DeviceInfo();
}


int powerOFFA7GSMModule() {
     printf("Power ON the Sim808 Module : ");
	 Sim808_GPS_GSM_Module_Power();
     sleep(4);

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
	sleep(20);
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




//SendTextMessage()
///this function is to send a sms message
void SendTextMessage()
{
A7_command_writeport("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
sleep(1);
A7_command_writeportln("AT + CMGS = \"+86138xxxxx615\"");//send sms message, be careful need to add a country code before the cellphone number
sleep(1);
A7_command_writeportln("A test message!");//the content of the message
sleep(1);
A7_command_writeportln((char)26);//the ASCII code of the ctrl+z is 26
sleep(1);
A7_command_writeportln("AT");
}

///DialVoiceCall
///this function is to dial a voice call
void DialVoiceCall()
{
A7_command_writeportln("ATD + +86138xxxxx615;");//dial the number
sleep(1);
A7_command_writeportln("AT");
}

///SubmitHttpRequest()
///this function is submit a http request
///attention:the time of sleep is very important, it must be set enough 
void SubmitHttpRequest()
{
A7_command_writeportln("AT+CSQ");
sleep(1);

ShowSerialData();// this code is to show the data from gprs shield, in order to easily see the process of how the gprs shield submit a http request, and the following is for this purpose too.

A7_command_writeportln("AT+CGATT?");
sleep(1);

ShowSerialData();

A7_command_writeportln("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");//setting the SAPBR, the connection type is using gprs
sleep(1);

ShowSerialData();

A7_command_writeportln("AT+SAPBR=3,1,\"APN\",\"CMNET\"");//setting the APN, the second need you fill in your local apn server
sleep(4);

ShowSerialData();

A7_command_writeportln("AT+SAPBR=1,1");//setting the SAPBR, for detail you can refer to the AT command mamual
sleep(2);

ShowSerialData();

A7_command_writeportln("AT+HTTPINIT"); //init the HTTP request

sleep(2); 
ShowSerialData();

A7_command_writeportln("AT+HTTPPARA=\"URL\",\"www.google.com.hk\"");// setting the httppara, the second parameter is the website you want to access
sleep(1);

ShowSerialData();

A7_command_writeportln("AT+HTTPACTION=0");//submit the request 
sleep(10);//the sleep is very important, the sleep time is base on the return from the website, if the return datas are very large, the time required longer.
//while(!mySerial.available());

ShowSerialData();

A7_command_writeportln("AT+HTTPREAD");// read the data from the website you access
sleep(1);

ShowSerialData();

A7_command_writeportln("AT");
sleep(1);
}

///send2Pachube()///
///this function is to send the sensor data to the pachube, you can see the new value in the pachube after execute this function///
void Send2Pachube()
{



char * humidity = "1031";//these 4 line code are imitate the real sensor data, because the demo did't add other sensor, so using 4 string variable to replace.
char * moisture = "1242";//you can replace these four variable to the real sensor data in your project
char * temperature = "30";//
char * barometer = "60.56";//

A7_command_writeportln("AT+CGATT?");
sleep(1);

ShowSerialData();

A7_command_writeportln("AT+CSTT=\"CMNET\"");//start task and setting the APN,
sleep(1);

ShowSerialData();

A7_command_writeportln("AT+CIICR");//bring up wireless connection
sleep(1);

ShowSerialData();

A7_command_writeportln("AT+CIFSR");//get local IP adress
sleep(2);

ShowSerialData();

A7_command_writeportln("AT+CIPSPRT=0");
sleep(3);

ShowSerialData();

A7_command_writeportln("AT+CIPSTART=\"tcp\",\"www.iisl.co.in\",\"8081\"");//start up the connection
sleep(2);

ShowSerialData();

A7_command_writeportln("AT+CIPSEND");//begin send data to remote server
sleep(4);
ShowSerialData();
sleep(1);
ShowSerialData();
A7_command_writeportln(" \"version\": \"1.0.0\",\"datastreams\": ");
sleep(1);
ShowSerialData();
A7_command_writeportln("[\"id\": \"01\",\"current_value\": \"" );
A7_command_writeportln(barometer);
A7_command_writeportln("\",");
sleep(1);
ShowSerialData();
A7_command_writeportln("\"id\": \"02\",\"current_value\": \"" );
A7_command_writeportln(humidity );
A7_command_writeportln( "\",");
sleep(1);
ShowSerialData();
A7_command_writeportln("\"id\": \"03\",\"current_value\": \"" );
A7_command_writeportln(moisture );
A7_command_writeportln("\",");
sleep(1);
ShowSerialData();
A7_command_writeportln("\"id\": \"04\",\"current_value\": \"" );
A7_command_writeportln(temperature );
A7_command_writeportln("\"],\"token\": \"lee\"");

sleep(1);
ShowSerialData();

A7_command_writeportln((char)26);//sending
sleep(1);//waitting for reply, important! the time is base on the condition of internet 
A7_command_writeportln("AT");

ShowSerialData();

A7_command_writeportln("AT+CIPCLOSE");//close the connection
sleep(1);
ShowSerialData();

}
void ShowSerialData()
{
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


int powerONA7GSMModule() {
     printf("Power ON the A7 Module : ");
	 A7_GPS_GSM_Module_Power();
     sleep(1);

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
//char gps_power_string3[]= "AT+CGPSSTATUS?\r\n";
//char gps_power_string4[]= "AT+CGPSINF=0\r\n";


if(ON)
{
//printf("%s",gps_power_string1);

	if(!A7_GPSPowerON){
	    RS232_cputs(A7_commond_cport_nr, gps_power_string1);
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;
		sleep(40);
		A7_GPSPowerON = true;

		}

/*
//printf("%s",gps_power_string3);

    RS232_cputs(A7_commond_cport_nr, gps_power_string3);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
        goto exit;

//printf("%s",gps_power_string4);

    RS232_cputs(A7_commond_cport_nr, gps_power_string4);
    Resetbufer(A7_buf,sizeof(A7_buf));
    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(A7_buf,A7_buf_SIZE,(unsigned char*)A7_OKToken,2) == NULL)
        goto exit;
*/
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
	
	return(0);
	
	}
	


int sendA7DataToTCPServer()
{
	
char latitude_string[ 112 ];
char longitude_string[ 112 ];
char updated_date_string[ 112 ];
char updated_time_string[ 112 ];

char tcp_string1[]= "AT+CGATT?\r\n";
char tcp_string2[]= "AT+CSTT=\"CMNET\"\r\n";
char tcp_string3[]= "AT+CIICR\r\n";
char tcp_string4[]= "AT+CIFSR\r\n";
char tcp_string5[]= "AT+CIPSPRT=0\r\n";
char tcp_header_str[] = "AT+CIPSTART=\"tcp\",\"www.iisl.co.in\",\"8081\"\r\n";	
char tcp_string6[]= "AT+CIPSEND\r\n";	
char tcp_string7[]= "{\"method\": \"put\",\"resource\": \"/feeds/42742/\",\"params\"";
char tcp_string8[]= ": {},\"body\":";
char tcp_string9[]= " {\"version\": \"1.0.0\",\"datastreams\": ";

char tcp_string10[]= "[{\"latitude\": \"" ;
char tcp_string11[]= "70.000" ;
char tcp_string12[]= "\"},";

char tcp_string13[]= "{\"longitude\": \"" ;
char tcp_string14[]= "20.0000" ;
char tcp_string15[]= "\"},";

char tcp_string16[]= "{\"utcdate_stamp\": \"" ;
char tcp_string17[]= "20.0000" ;
char tcp_string18[]= "\"},";


char tcp_string19[]= "{\"utctime_stamp\": \"" ;
char tcp_string20[]=  "124050 ";
char tcp_string21[] = "\"}]}}\r\n";


char end_of_file_byte = (char)26;


char tcp_string22[]= "AT+CIPCLOSE\r\n";




strcpy(A7_device_id_str,"1234567890");

//printf("%s",http_string);
restart:

	if(!A7_httpInitialize) {
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


		//printf("%s",http_string3);

		    RS232_cputs(A7_commond_cport_nr, tcp_string3);
		    Resetbufer(A7_buf,sizeof(A7_buf));
		    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		    // Check if "OK" string is present in the received data 
		    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
		        goto exit;

		sleep(1);
		//printf("%s",http_string4);
			
			RS232_cputs(A7_commond_cport_nr, tcp_string4);
			Resetbufer(A7_buf,sizeof(A7_buf));
			ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
			// Check if "OK" string is present in the received data 
			if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
				goto exit;
			sleep(4);

		
		A7_httpInitialize = true;
		}

	//printf("%s",http_string5);
		
		RS232_cputs(A7_commond_cport_nr, tcp_string5);
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		// Check if "OK" string is present in the received data 
		if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;
		sleep(2);
	

	//printf("%s",http_string6);

	    RS232_cputs(A7_commond_cport_nr, tcp_string6);
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;

		sleep(3);

		//printf("%s",http_string7);

	    RS232_cputs(A7_commond_cport_nr, tcp_string7);
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;
		sleep(10);


		//printf("%s",http_string8);

	    RS232_cputs(A7_commond_cport_nr, tcp_string8);
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;
		sleep(4);


		//printf("%s",http_string9);

	    RS232_cputs(A7_commond_cport_nr, tcp_string9);
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;
		sleep(1);

		snprintf( latitude_string, sizeof( latitude_string ), "%s%s%s", tcp_string10,latitude_str,tcp_string12 );
		snprintf( longitude_string, sizeof( longitude_string ), "%s%s%s", tcp_string13,longitude_str,tcp_string15 );
		snprintf( updated_date_string, sizeof( latitude_string ), "%s%s%", tcp_string16,updated_date_str,tcp_string18 );
		snprintf( updated_time_string, sizeof( latitude_string ), "%s%s%s", tcp_string19,updated_time_str,tcp_string21 );

		//printf("%s",latitude_string);
		
		RS232_cputs(A7_commond_cport_nr, latitude_string);
		RS232_cputs(A7_commond_cport_nr, longitude_string);
		RS232_cputs(A7_commond_cport_nr, updated_date_string);
		RS232_cputs(A7_commond_cport_nr, updated_time_string);		


		
		Resetbufer(A7_buf,sizeof(A7_buf));
		ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		// Check if "OK" string is present in the received data 
		if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
			goto exit;


	
		//printf("%s",http_string22);

	    RS232_cputs(A7_commond_cport_nr, tcp_string22);
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;
		sleep(4);
		


	SUCCESS: printf("\n SEND DATA SUCCESS \n");
	return(1);
	exit: printf("\n SEND DATA FAILED\ n");
	return(0);





}




int sendA7DataToServer()
{
	
char send_string[ 1024 ];
char http_string1[]= "AT+CSQ\r\n";
char http_string2[]= "AT+CGATT?\r\n";
char http_string3[]= "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n";
char http_string4[]= "AT+SAPBR=3,1,\"APN\",\"CMNET\"\r\n";
char http_string5[]= "AT+SAPBR=1,1\r\n";
char http_string6[]= "AT+HTTPINIT\r\n";
char http_header_str[] = "AT+HTTPPARA=\"URL\",\"http://speedfleet.in/speedfleet_control_panel/mapview/addexecutivelocation.php?";	
char http_string7[]= "AT+HTTPACTION=0\r\n";
char http_string8[]= "AT+HTTPREAD\r\n";
char http_string9[]= "AT+HTTPTERM\r\n";

strcpy(A7_device_id_str,"1234567890");

//printf("%s",http_string);
restart:

	if(!A7_httpInitialize) {
		//printf("%s",http_string1);
		    RS232_cputs(A7_commond_cport_nr, http_string1);
		    Resetbufer(A7_buf,sizeof(A7_buf));
		    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		    // Check if "OK" string is present in the received data 
		    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
		        goto exit;

		//printf("%s",http_string2);
		    RS232_cputs(A7_commond_cport_nr, http_string2);
		    Resetbufer(A7_buf,sizeof(A7_buf));
		    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		    // Check if "OK" string is present in the received data 
		    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
		        goto exit;


		//printf("%s",http_string3);

		    RS232_cputs(A7_commond_cport_nr, http_string3);
		    Resetbufer(A7_buf,sizeof(A7_buf));
		    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
		    // Check if "OK" string is present in the received data 
		    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
		        goto exit;

		sleep(1);
		//printf("%s",http_string4);
			
			RS232_cputs(A7_commond_cport_nr, http_string4);
			Resetbufer(A7_buf,sizeof(A7_buf));
			ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
			// Check if "OK" string is present in the received data 
			if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
				goto exit;
			sleep(4);

		//printf("%s",http_string5);
			
			RS232_cputs(A7_commond_cport_nr, http_string5);
			Resetbufer(A7_buf,sizeof(A7_buf));
			ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
			// Check if "OK" string is present in the received data 
			if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
				goto exit;
			sleep(2);

		
		A7_httpInitialize = true;
		}

	//printf("%s",http_string6);

	    RS232_cputs(A7_commond_cport_nr, http_string6);
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;


		sleep(3);
		snprintf( send_string, sizeof( send_string ), "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", http_header_str,"device_id=",A7_device_id_str,"&" \
		          "latitude=",latitude_str,"&" , "longitude=",longitude_str,"&","utcdate_stamp=",updated_date_str,"&","utctime_stamp=",updated_time_str,"\"\r\n");

		//printf("%s",send_string);

	    RS232_cputs(A7_commond_cport_nr, send_string);
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;



		//printf("%s",http_string7);

	    RS232_cputs(A7_commond_cport_nr, http_string7);
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;
		sleep(10);


		//printf("%s",http_string8);

	    RS232_cputs(A7_commond_cport_nr, http_string8);
	    Resetbufer(A7_buf,sizeof(A7_buf));
	    ReadComport(A7_commond_cport_nr,A7_buf,6000,500000);
	    // Check if "OK" string is present in the received data 
	    if(MapForward(A7_buf,A7_OKToken,(unsigned char*)A7_OKToken,2) == NULL)
	        goto exit;
		sleep(4);


		//printf("%s",http_string9);

	    RS232_cputs(A7_commond_cport_nr, http_string9);
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









