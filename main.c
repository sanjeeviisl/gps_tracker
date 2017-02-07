
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gps_init.h>
#include <A7_command_serial.h>
#include <A7_gps_data_serial.h>
#include <A7_http_data_server.h>

int main()
{
int i, count =0 ;
char ch;
int databyte=0;
char  * data_buffer; 

char string1[] = "$GPRMC,113821.000,V,,,,,,,270117,,,N*47\r";
char string2[] ="$GPGGA,113822.000,,,,,0,00,,,M,,M,,0000*73\r";

A7_command_openport();
A7_gps_data_openport();

HARD_RESET :
printf("\n Initializing GPS Module .....");
Init_GPS_GSM_Module();

printf("\nstarting gps module ...");

sleep(10);
A7_command_writeport("AT+GPS=0\r\n");
A7_command_writeport("AT+GPS=1\r\n");
sleep(60);
printf("\nGPS ON\n");
A7_command_writeport("AT+GPS=1\r\n");
A7_command_writeport("AT+AGPS=1\r\n");
A7_command_writeport("AT+GPSRD=2\r\n");

sleep(30);
while(1)
 {
sleep(1);
//printf("\nreading GPS DATA\n");
databyte =A7_gps_data_readport();
if(databyte == 0)
	{
	A7_command_writeport("AT+GPS=1\r\n");
	A7_command_writeport("AT+AGPS=1\r\n");
	printf("\ntrying to GPS ON");
	A7_command_writeport("AT+GPSRD=2\r\n");
	count++;
	if ( count > 7)
		goto HARD_RESET;
	}

else{
	A7_Show_GSM_Siganl_Qauality();
    data_buffer = A7_gps_data_buffer();
	for(i= 0 ; i < databyte ;i++)
		{
		ch = data_buffer[i];
		//fuseDataGPS(ch);
		}
    }

 }
}




