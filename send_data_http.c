#include <stdio.h>
#include <string.h>


#define SIZE 1024
char send_string[ SIZE ];

char http_header_str[] = "http_header_str ?";
char latitude_str[] = "latitude_str";
char longitude_str[] = "longitude_str";
char device_id_str[] = "device_id_str";
char updated_time_str[] = "updated_time_str";

void sendToWebserver();

int main() 
{
	
	sendToWebserver();
	
}
void sendToWebserver()
{

snprintf( send_string, sizeof( send_string ), "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", http_header_str,"latitude=",latitude_str,"&" \
          "latitude=",latitude_str,"&" , "longitude=",longitude_str,"&","device_id=",device_id_str,"&","updated_time=",updated_time_str,"\r\n");
		  
printf("\n Final string |%s| \n", send_string );

}