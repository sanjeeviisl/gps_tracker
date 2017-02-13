
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
#include "rs232.h"

#define true 1
#define false 0

void UART1SendNak(void);     // Send Nak sequence for the here defined simple protocol
void UART1SendAck(void);     // Send Ack sequence for the here defined simple protocol
void checkForSerialCommand(void);   // Function used to check the serial command and perform related action
void gpsUartInit(void);
void parseDataSIM808GPS(void);
void fuseDataGPS(char c);
int charToInt(char c);
double trunc(double d);
typedef int bool ;
int count =0;
#define SIZE 1024
char send_string[ SIZE ];



struct gpsStruct {//GPRMC
    int     timeZone;

    bool    flagRead;        // flag used by the parser, when a valid sentence has begun
    bool    flagDataReady;   // valid GPS fix and data available, user can call reader functions
    char    words[20][15];  // hold parsed words for one given NMEA sentence
    char    szChecksum[15];	// hold the received checksum for one given NMEA sentence

    // will be set to true for characters between $ and * only
    bool    flagComputedCks;     // used to compute checksum and indicate valid checksum interval (between $ and * in a given sentence)
    int     checksum;            // numeric checksum, computed for a given sentence
    bool    flagReceivedCks;     // after getting  * we start cuttings the received checksum
    int     index_received_checksum;// used to parse received checksum

    // word cutting variables
    int     wordIdx;         // the current word in a sentence
    int     prevIdx;		// last character index where we did a cut
    int     nowIdx ;		// current character index

    // globals to store parser results
    bool    positionFixIndicator;   //GPGGA
    bool    dataValid;              //GPRMC
    float   longitude;     // GPRMC and GPGGA
    float   latitude;	// GPRMC and GPGGA
    unsigned char   UTCHour, UTCMin, UTCSec,		// GPRMC and GPGGA
                                    UTCDay, UTCMonth, UTCYear;	// GPRMC
    int     satellitesUsed;// GPGGA
    float   dilution;      //GPGGA
    float   altitude;	// GPGGA
    float   speed;		// GPRMC
    float   bearing;	// GPRMC


};

static struct gpsStruct gps;
char * temp;
char t_buffer[10];
char t_buffer1[10];
char t_buffer2[10];
char t_buffer3[10];
char t_buffer4[10];
char t_buffer5[10];
char * http_header_str;
char * latitude_str;
char * longitude_str;
char device_id_str[10];
char updated_time_str[6];

int cport_nr=1,    /* /dev/ttyS0 */
    bdrate=115200; /* 115200 baud */
const unsigned char OKToken[]={"OK"};
unsigned char buf[6000];
int buf_SIZE=sizeof(buf);

char *dtostrf (double val, signed char width, unsigned char prec, char *sout) { 
   char fmt[20]; 
   sprintf(fmt, "%%%d.%df", width, prec); 
   sprintf(sout, fmt, val); 
   return sout; 
} 

/*******************************************************************************
* Funtion name : MapForward                                                    *
*                                                                              *
* Description  : This function performs forward mapping and returns pointer    *
*                to the start of the found data or else returns NULL pointer   *
*                                                                              *
* Arguments    : 1) Poshorter to the data in which mapping is to be made       *
*                2) Length of the data in which mapping is to be made          *
*                3) Poshorter to the data points to be mapped                  *
*                4) Length of the data points to be mapped                     *
*                                                                              *
* Returns      : Pointer in which result is returned                           *
*******************************************************************************/
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
        int n;
        n=0;
        usleep(count);
  while(1)
  {

    n = RS232_PollComport(cport_nr, buf, size);

    if(n > 0)
    {
      buf[n] = 0;   /* always put a "null" at the end of a string! */

      //for(i=0; i < n; i++)
      //{
        //if(buf[i] < 32)  /* replace unreadable control-codes by dots */
        //{
        //  buf[i] = '.';
        //}
      //}

      printf("%s\n", (char *)buf);
      break;
    }
}

        return n;
}

static void Resetbufer(unsigned char *buf,int size)
{
        int i;
        for(i=0;i<size;i++)
        {
                buf[i] = '0';
        }
}



void parseDataSIM808GPS(void){
//    int received_cks = 16*digit2dec(gps.tmp_szChecksum[0]) + charToInt(gps.tmp_szChecksum[1]);
    int received_cks = 16*charToInt(gps.szChecksum[0]) + charToInt(gps.szChecksum[1]);
    //uart1.Send("seq: [cc:%X][words:%d][rc:%s:%d]\r\n", m_nChecksum,m_nWordIdx, tmp_szChecksum, received_cks);
    // check checksum, and return if invalid!
    if (gps.checksum != received_cks) {
        //m_bFlagDataReady = false;
        return;
    }
    /* $GPGGA
     * $GPGGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
     * WORDS:
     *  0    = $GPGGA - Gloabl Positioning System Fixed Data
     *  1    = UTC of Position
     *  2    = Latitude
     *  3    = N or S
     *  4    = Longitude
     *  5    = E or W
     *  6    = GPS quality indicator (0=invalid; 1=GPS fix; 2=Diff. GPS fix)
     *  7    = Number of satellites in use [not those in view]
     *  8    = Horizontal dilution of position
     *  9    = Antenna altitude above/below mean sea level (geoid)
     *  10   = Meters  (Antenna height unit)
     *  11   = Geoidal separation (Diff. between WGS-84 earth ellipsoid and mean sea level.
     *      -geoid is below WGS-84 ellipsoid)
     *  12   = Meters  (Units of geoidal separation)
     *  13   = Age in seconds since last update from diff. reference station
     *  14   = Diff. reference station ID#
     *  15   = Checksum
     */
    if (strcmp(gps.words[0], "$GPGGA") == 0) {

        // Check GPS Fix: 0=no fix, 1=GPS fix, 2=Dif. GPS fix
        if (gps.words[6][0] == '0') {
            gps.positionFixIndicator = false;
            // clear data
//            gps.res_fLatitude = 0;
//            gps.res_fLongitude = 0;
            gps.flagDataReady = false;
            printf("\nNo Valid Data");
            return;
        }
        
        gps.positionFixIndicator = true;
        // parse time
        gps.UTCHour = charToInt(gps.words[1][0]) * 10 + charToInt(gps.words[1][1]);
        gps.UTCMin = charToInt(gps.words[1][2]) * 10 + charToInt(gps.words[1][3]);
        gps.UTCSec = charToInt(gps.words[1][4]) * 10 + charToInt(gps.words[1][5]);
        // parse latitude and longitude in NMEA format
        gps.latitude = strtof(gps.words[2], NULL);
        gps.longitude = strtof(gps.words[4], NULL);

        // get decimal format
        if (gps.words[3][0] == 'S') gps.latitude  *= -1.0;
        if (gps.words[5][0] == 'W') gps.longitude *= -1.0;
        float degrees = trunc(gps.latitude / 100.0f);
        float minutes = gps.latitude - (degrees * 100.0f);
        gps.latitude = degrees + minutes / 60.0f;

        degrees = trunc(gps.longitude / 100.0f);
        minutes = gps.longitude - (degrees * 100.0f);
        gps.longitude = degrees + minutes / 60.0f;


        longitude_str=dtostrf(gps.longitude,0,6,t_buffer1);
        latitude_str=dtostrf(gps.latitude,0,6,t_buffer2);


	strncpy(updated_time_str,gps.words[1],6);
        // parse number of satellites
        gps.satellitesUsed = (int)strtof(gps.words[7], NULL);
        
        // parse horizontal dilution of precision
        gps.dilution = strtof(gps.words[8], NULL);

        // parse altitude
        gps.altitude = strtof(gps.words[9], NULL);

        printf("\n Lattitude %f Longitude %f Sattellite used %d ",gps.latitude,gps.longitude,gps.satellitesUsed);
        printf("\n Time: %d.%d.%d ",gps.UTCHour,gps.UTCMin,gps.UTCSec);
        count++;
        sleep(1); 
        // data ready
        gps.flagDataReady = true;
    }//$GPGGA

    /* $GPRMC
     * note: a siRF chipset will not support magnetic headers.
     * $GPRMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a*hh
     * ex: $GPRMC,230558.501,A,4543.8901,N,02112.7219,E,1.50,181.47,230213,,,A*66,
     *
     * WORDS:
     *  0    = $GPRMC - Recommended Minimum Specific GNSS Data
     *  1    = UTC of position fix
     *  2    = Data status (V=navigation receiver warning)
     *  3    = Latitude of fix
     *  4    = N or S
     *  5    = Longitude of fix
     *  6    = E or W
     *  7    = Speed over ground in knots
     *  8    = Track made good in degrees True, Bearing This indicates the direction that the device is currently moving in,
     *       from 0 to 360, measured in “azimuth”.
     *  9    = UT date
     *  10   = Magnetic variation degrees (Easterly var. subtracts from true course)
     *  11   = E or W
     *  12   = Checksum
     */
    if (strcmp(gps.words[0], "$GPRMC") == 0) {
        // Check data status: A-ok, V-invalid
        if (gps.words[2][0] == 'V') {
            gps.dataValid = false;
            // clear data
//            gps.res_fLatitude = 0;
//            gps.res_fLongitude = 0;
            gps.flagDataReady = false;
            printf("\nNo Valid signal");
            return;
        }
        gps.dataValid = true;

        gps.speed = strtof(gps.words[7], NULL);
        gps.speed *= 1.15078; // convert to mph
        // parse bearing
        gps.bearing = strtof(gps.words[8], NULL);
        // parse UTC date
        gps.UTCDay = charToInt(gps.words[9][0]) * 10 + charToInt(gps.words[9][1]);
        gps.UTCMonth = charToInt(gps.words[9][2]) * 10 + charToInt(gps.words[9][3]);
        gps.UTCYear = charToInt(gps.words[9][4]) * 10 + charToInt(gps.words[9][5]);
        printf("\n Speed %f Bearing %f ",gps.speed,gps.bearing);
        printf("\n Date %d-%d-%d ",gps.UTCDay,gps.UTCMonth,gps.UTCYear);
        sleep(1); 
        // data ready
        gps.flagDataReady = true;
    }//$GPRMC
    return;
}//parseDataSIM808GPS

void parseGPSNIMEADATA(char c){
	if (c == '$') {
		gps.flagRead = true;
		// init parser vars
		gps.flagComputedCks = false;
		gps.checksum = 0;
		// after getting  * we start cuttings the received m_nChecksum
		gps.flagReceivedCks = false;
                gps.index_received_checksum = 0;
		// word cutting variables
		gps.wordIdx = 0; gps.prevIdx = 0; gps.nowIdx = 0;
	}

	if (gps.flagRead) {
		// check ending
		if (c == '\r' || c== '\n') {
			// catch last ending item too
			gps.words[gps.wordIdx][gps.nowIdx - gps.prevIdx] = 0;
			gps.wordIdx++;
			// cut received m_nChecksum
			gps.szChecksum[gps.index_received_checksum] = 0;
			// sentence complete, read done
			gps.flagRead = false;
			// parse
			parseDataSIM808GPS();
		} else {
			// computed m_nChecksum logic: count all chars between $ and * exclusively
			if (gps.flagComputedCks && c == '*') gps.flagComputedCks = false;
			if (gps.flagComputedCks) gps.checksum ^= c;
			if (c == '$') gps.flagComputedCks = true;
			// received m_nChecksum
			if (gps.flagReceivedCks)  {
				gps.szChecksum[gps.index_received_checksum] = c;
				gps.index_received_checksum++;
			}
			if (c == '*') gps.flagReceivedCks = true;
			// build a word
			gps.words[gps.wordIdx][gps.nowIdx - gps.prevIdx] = c;
			if (c == ',') {
				gps.words[gps.wordIdx][gps.nowIdx - gps.prevIdx] = 0;
				gps.wordIdx++;
				gps.prevIdx = gps.nowIdx;
			}
			else gps.nowIdx++;
		}
	}

    return;
}//fuseDataGPS




char* read_file (const char* filename, size_t* length)
{
  int fd;
  struct stat file_info;
  char* buffer;

  /* Open the file.  */
  fd = open (filename, O_RDONLY);

  /* Get information about the file.  */
  fstat (fd, &file_info);
  *length = file_info.st_size;
  /* Make sure the file is an ordinary file.  */
  if (!S_ISREG (file_info.st_mode)) {
    /* It's not, so give up.  */
    close (fd);
    return NULL;
  }

  /* Allocate a buffer large enough to hold the file's contents.  */
  buffer = (char*) malloc (*length);
  /* Read the file into the buffer.  */
  read (fd, buffer, *length);

  /* Finish up.  */
  close (fd);
  return buffer;
}

int sendDataToServer()
{
char http_string[]= "AT+CREG?\r\n";
char http_string0[]= "AT+SAPBR=2,1\r\n";
char http_string1[]= "AT+SAPBR=1,1\r\n";
char http_string2[]= "AT+HTTPINIT\r\n";
char http_string3[]= "AT+HTTPPARA=\"URL\",\"http://iisl.co.in/gps_control_panel/device_details/add_devices_updated.php?device_id=123456789\"";
char http_header_str[] = "AT+HTTPPARA=\"URL\",\"http://iisl.co.in/gps_control_panel/device_details/add_devices_updated.php?";
char http_string4[]= "AT+HTTPPARA=\"CID\",1\r\n";
char http_string5[]= "AT+HTTPACTION=0\r\n";
char http_string6[]= "AT+HTTPREAD\r\n";
char http_string7[]= "AT+HTTPTERM\r\n";


strcpy(device_id_str,"1234567890");

printf("%s",http_string);
restart:
    RS232_cputs(cport_nr, http_string);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;

printf("%s",http_string0);
    RS232_cputs(cport_nr, http_string);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;



printf("%s",http_string1);

    RS232_cputs(cport_nr, http_string);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;


printf("%s",http_string2);

    RS232_cputs(cport_nr, http_string);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;


sleep(1);
snprintf( send_string, sizeof( send_string ), "%s%s%s%s%s%s%s%s%s%s%s%s", http_header_str,"device_id=",device_id_str,"&" \
          "latitude=",latitude_str,"&" , "longitude=",longitude_str,"&","updated_time=",updated_time_str,"\"\r\n");

printf("%s",send_string);

    RS232_cputs(cport_nr, http_string);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;



printf("%s",http_string4);

    RS232_cputs(cport_nr, http_string);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;


printf("%s",http_string5);

    RS232_cputs(cport_nr, http_string);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;


printf("%s",http_string6);

    RS232_cputs(cport_nr, http_string);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;


sleep(6);
printf("%s",http_string7);

    RS232_cputs(cport_nr, http_string);
    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;




SUCCESS: printf("SUCCESS");
return(1);
exit: printf("FAILED");
return(0);

}



int sim808_gps_test()
{


char mode[]={'8','N','1',0},str[512];


int i;
size_t size;
char ch;
char *string;

if(RS232_OpenComport(cport_nr, bdrate, mode))
  {
    printf("Can not open comport\n");
    return(0);
  }

restart:
    RS232_cputs(cport_nr, "AT\r\n");

    Resetbufer(buf,sizeof(buf));
    ReadComport(cport_nr,buf,6000,500000);
    // Check if "OK" string is present in the received data 
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
        goto exit;

    // You can send individual bytes to the serial/COM port using this function
    //RS232_SendByte(cport_nr,0x1A); // 0x1A is a hex equivalent of Ctrl^Z key press



string = read_file("gpslog.txt",&size);
if( string != NULL)
for(i= 0 ; i < size ;i++)
  {
  ch = string[i];
  parseGPSNIMEADATA(ch);
  if(count>1)
    {
     printf("\nsending data to web server \n");
     sendDataToServer();
     count =0;
     sleep(1);
    }  
  }



SUCCESS: printf("SUCCESS");
return(0);
exit: printf("FAILED");
}

