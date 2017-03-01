#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gps_init.h>
#include <A7_command_serial.h>
#include <A7_gps_data_serial.h>
#include <A7_http_data_server.h>

#define SIM808 0
int main()
{
#if SIM808
SIM808_main();
#else
A7_main();
#endif

}




