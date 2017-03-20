#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

char logFileName[]= "A7_gpslog.txt";

char A7_newFileName[26];
char A7_updated_time_str[6];
char A7_updated_date_str[6];


int main()
{
strncpy(A7_updated_date_str,"31012017",8);
strncpy(A7_updated_time_str,"101010",6);

strcpy(A7_newFileName,A7_updated_time_str);
strcat(A7_newFileName,A7_updated_date_str);
strcat(A7_newFileName,logFileName);

printf("final NAme %s\n",A7_newFileName);



}
