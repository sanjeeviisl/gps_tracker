#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


int main()
{
char send_string[ 1024 ];

char A7_device_id_str[]="1234457777";
char A7_updated_time_str[]="123455";
char A7_updated_date_str[]="20022017";
char A7_latitude_str[]="0.333333333";
char A7_longitude_str[]="9.000000000";

char tcp_string1[]= "at+cipstatus\r\n";
char tcp_string2[]= "AT+CIPSTART=\"TCP\",\"www.iisl.co.in\",80\r\n";
char tcp_string3[]= "at+cipstatus\r\n";
char tcp_string4[]= "AT+CIPSEND\r\n";	

char tcp_header_str[] = "GET http://iisl.co.in/gps_control_panel/gps_mapview/adddevicelocation.php?";	

char tcp_body_str[] = " HTTP/1.0\r\n";

char tcp_footer_str[] = "Host: www.iisl.co.in:8080\r\n";


char end_of_file_byte = (char)26;

char tcp_string_end[1];
char tcp_string_end1[]= "\r\n";

char tcp_string20[]= "AT+CIPCLOSE\r\n";
char tcp_string21[]= "at+cifsr\r\n";
tcp_string_end[0] = end_of_file_byte;
strcpy(A7_device_id_str,"1234567890");

snprintf( send_string, sizeof( send_string ), "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", tcp_header_str,"device_id=",A7_device_id_str,"&", \
					  "latitude=",A7_latitude_str,"&" , "longitude=",A7_longitude_str,"&","utcdate_stamp=",A7_updated_date_str,"&","utctime_stamp=",\
					  A7_updated_time_str,tcp_body_str,tcp_footer_str);

printf("send string |%s|", send_string);



}
