#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gps_init.h>
#include <A7_command_serial.h>
#include <A7_gps_data_serial.h>
#include <A7_http_data_server.h>


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