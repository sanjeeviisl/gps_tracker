/*
A7HTTP.cpp - HTTP Post library for Orange PI One
2017/02/06
Made by sanjeev from Intime Information System
Based on code from Andreas Spiess
License: Opensource

http://gitlab.koukei.de/Tobias/A7HTTPLibrary.git
*/

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <A7_http_data_server.h>
#include <A7_command_serial.h>

    unsigned char OK;
 	unsigned char NOTOK;
 	unsigned char TIMEOUT;
 	unsigned char RST;
 	unsigned char RX;
 	unsigned char TX;
 	int A7baud;
 	int SERIALTIMEOUT;
 	char end_c[2];

    char * _APN;
	char * _host;
	char * _path;
	int _port;
	char * _Content_Type;

	unsigned long millis (void);
	unsigned char A6waitFor(char * response1,char * response2, int timeOut);
    unsigned char A6command(char * command, char * response1, char * response2, int timeOut, int repetitions);
    void A7input();
    int A7begin();
	short A7board_available();
    void A7HTTP_ShowSerialData();

	char * A7read();
    short  A7HTTP_A7command(const char * command, const char * response1, const  char * response2, int timeOut, int repetitions);
	short  A7HTTP_A7waitFor(char * response1, char * response2, int timeOut);
		
	
void A7HTTP_post_setup(char * APN, char * host, char * path, int port, char * Content_Type)
{
 OK = 1;
 NOTOK = 2;
 TIMEOUT = 3;
 RST = 10; //This is your Resetpin
 RX = 4; //This is your RX-Pin on orange_pi_one
 TX = 3; //This is your TX-Pin on orange_pi_one
 A7baud = 9600; //Baudrate between orange_pi_one and A7
 SERIALTIMEOUT = 3000;

 A7_command_openport();
 
 _APN = APN;
_host = host;
 _path = path;
 _port = port;
_Content_Type = Content_Type;
}

short A7board_available() {
	return 1;
}


void A7HTTP_begin()
{
A7_command_baudrate(A7baud);
printf("Start setup");
Init_GPS_GSM_Module();
printf("Setup done");
}

int A7_Show_GSM_Siganl_Qauality()
{
  printf("Start A7_Show_GSM_Siganl_Qauality function");
 
  A7HTTP_A7command("AT+CGATT?", "OK", "yy", 20000, 2);
  A7HTTP_A7command("AT+CGATT=1", "OK", "yy", 20000, 2);
  A7board_println("AT+CSQ"); //Signal Quality
  sleep(1);
  A7HTTP_ShowSerialData();
  sleep(1);
}

int A7HTTP_HttpPost(char * body)
{
  int bodyLength ;
  unsigned long   entry;
  printf("Start post function");
  bodyLength = strlen(body);
  int aInt = 368;
  char bodyLength_str[15];

  sprintf(bodyLength_str, "%d", bodyLength);
  
  printf(bodyLength);
  A7HTTP_A7command("AT+CGATT?", "OK", "yy", 20000, 2);
  A7HTTP_A7command("AT+CGATT=1", "OK", "yy", 20000, 2);
  A7board_println("AT+CSQ"); //Signal Quality
  sleep(1);
  A7HTTP_ShowSerialData();
    A7board_print("AT+CGDCONT=1,\"IP\",\""); //bring up wireless connection
	  A7board_print( _APN ); //bring up wireless connection
	    A7HTTP_A7command("\"", "OK", "yy", 20000, 2); //bring up wireless connection
  
  A7HTTP_A7command("AT+CGACT=1,1", "OK", "yy", 10000, 2);
 // A7HTTP_A7command("AT+CIFSR", "OK", "yy", 20000, 2);  
  sleep(1);
  A7board_print("AT+CIPSTART=\"TCP\",\"" ); //start up the connection
  A7board_print( _host ); //start up the connection
  A7board_print("\","); //start up the connection
  A7HTTP_A7command(_port, "CONNECT OK", "yy", 25000, 2); //start up the connection
  
  A7HTTP_A7command("AT+CIPSTATUS", "OK", "yy", 10000, 2);
  A7HTTP_A7command("AT+CIPSEND", ">", "yy", 10000, 1); //begin send data to remote server
  sleep(1);
  A7board_print("POST ");
  A7board_print( _path);
  A7board_print(" HTTP/1.1");
  A7board_print("\r\n");
  A7board_print("HOST: ");
  A7board_print(_host);
  A7board_print("\r\n");
  A7board_print("Content-Length: ");
  A7board_print(bodyLength_str);
  A7board_print("\r\n");
  A7board_print("Content-Type: " );
  A7board_print(_Content_Type);
  A7board_print("\r\n");
  A7board_print("\r\n");
  A7board_print(body);
  A7HTTP_A7command(end_c, "OK", "yy", 30000, 1); //begin send data to remote server
  entry = millis();
  A7HTTP_A7command("AT+CIPSTATUS", "OK", "yy", 10000, 2);
  sleep(1);
  A7HTTP_A7command("AT+CIPCLOSE", "OK", "yy", 15000, 1); 
  sleep(1);
}

short  A7HTTP_A7waitFor(char * response1, char * response2, int timeOut) {
  unsigned long entry;
  int count = 0;
  char * reply1 ;
  char * reply2 ;
  short retVal = 99;

  entry = millis();  
  do {
    reply1 = A7read();
    reply2 = A7read();
    if (reply1 != "") {
      printf(reply1);
    }
	if (reply2 != "") {
      printf(reply2);
    }
  } while (((strcmp(reply1,response1) + strcmp(reply2,response2)) == 0) && (millis() - entry < timeOut) );
  if ((millis() - entry) >= timeOut) {
    retVal = TIMEOUT;
  } else {
    if (((strcmp(reply1,response1) + strcmp(reply2,response2))) == 0) retVal = OK;
    else retVal = NOTOK;
  }
  //  Serial.print("retVal = ");
  //  printf(retVal);
  return retVal;
}

short  A7HTTP_A7command(const char * command, const char * response1, const  char * response2, int timeOut, int repetitions) {
  short returnValue = NOTOK;
  short count = 0;
  A7board_println(command);
/*  
  while (count < repetitions && returnValue != OK) {
    A7board_println(command);
    printf(command);
    if (A7HTTP_A7waitFor(response1, response2, timeOut) == OK) {
      //     printf("OK");
      returnValue = OK;
    } else returnValue = NOTOK;
    //printf("NOTOK" + count);
    count++;
  }
  */
  return returnValue;
}



int  A7HTTP_A7begin() {
  A7board_println("AT+CREG?");
  short hi;
  hi = A7HTTP_A7waitFor("1,", "5,", 1500);  // 1: registered, home network ; 5: registered, roaming
  while ( hi != OK) {
    A7board_println("AT+CREG?");
    hi = A7HTTP_A7waitFor("1,", "5,", 1500);
  }

  if (A7HTTP_A7command("AT&F0", "OK", "yy", 5000, 2) == OK) {   // Reset to factory settings
    if (A7HTTP_A7command("ATE0", "OK", "yy", 5000, 2) == OK) {  // disable Echo
      if (A7HTTP_A7command("AT+CMEE=2", "OK", "yy", 5000, 2) == OK) return OK;  // enable better error messages
      else return NOTOK;
    }
  }
}

void  A7HTTP_ShowSerialData()
{
    printf(A7_command_read_string());
	printf(A7_command_read_string());
	printf(A7_command_read_string());
}

char * A7HTTP_A7read() {
  char * reply = "";
  if (A7board_available())  {
    reply = A7_command_readport();
  }
  //  printf(reply);
  return reply;
}

char * A7read(){
   char * reply = "";
  if (A7board_available())  {
    reply = A7_command_read_string();
  }
  //  printf(reply);
  return reply;
	
}

void A7board_println(char * command) {
	
	A7_command_writeport(command);
	A7_command_writeport("\n");
}

void A7board_print(char * command) {
	
	A7_command_writeport(command);

}

unsigned long millis (void)
{
    unsigned long   ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds

    return ms;
}

