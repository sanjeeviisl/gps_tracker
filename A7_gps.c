


#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include "A7_lib.h"
#include "rs232.h"

#define true 1
#define false 0

int send_count = 0;
<<<<<<< HEAD
int sent = false;

static int  charToInt(char c);
double trunc(double d);


int sendA7GPSData() ;
void parseDataA7GPS(void);


typedef int bool ;

int A7_count =0;
char * A7_latitude_str;
char * A7_longitude_str;
char A7_updated_time_str[7];
char A7_updated_date_str[9]="01012017";
char A7_newFileName[26];
char move_file_name[112];
extern char A7_logFileName[];




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




void parseDataA7GPS(void){
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
           // printf("\nNo Valid Data");
            return;
        }
        
        gps.positionFixIndicator = true;
        // parse time
        gps.UTCHour = charToInt(gps.words[1][0]) * 10 + charToInt(gps.words[1][1]);
        gps.UTCMin = charToInt(gps.words[1][2]) * 10 + charToInt(gps.words[1][3]);
        gps.UTCSec = charToInt(gps.words[1][4]) * 10 + charToInt(gps.words[1][5]);


		gps.UTCMin = gps.UTCMin+32;
		if(gps.UTCMin > 59)
			{
			gps.UTCMin = gps.UTCMin-60;
			gps.UTCHour = gps.UTCHour+5+1;
			}
		else
			{
			gps.UTCHour = gps.UTCHour+5;
			}

		if(gps.UTCHour > 23)
			gps.UTCHour = 0;
			


		gps.words[1][0] =(int) gps.UTCHour /10 + '0';
		gps.words[1][1] =(int) gps.UTCHour % 10 + '0';

		gps.words[1][2] =(int) gps.UTCMin /10 + '0';
		gps.words[1][3] =(int) gps.UTCMin % 10 + '0';

		gps.words[1][4] =(int) gps.UTCSec /10 + '0';
		gps.words[1][5] =(int) gps.UTCSec % 10 + '0';
		
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


        A7_longitude_str=dtostrf(gps.longitude,0,6,t_buffer1);
        A7_latitude_str=dtostrf(gps.latitude,0,6,t_buffer2);


		strncpy(A7_updated_time_str,gps.words[1],6);
		
		A7_updated_time_str[7]= 0;

        // parse number of satellites
        gps.satellitesUsed = (int)strtof(gps.words[7], NULL);
        
        // parse horizontal dilution of precision
        gps.dilution = strtof(gps.words[8], NULL);

        // parse altitude
        gps.altitude = strtof(gps.words[9], NULL);

        //nprintf("\n Lattitude %f Longitude %f Sattellite used %d ",gps.latitude,gps.longitude,gps.satellitesUsed);
        //printf("\n Time: %d.%d.%d ",gps.UTCHour,gps.UTCMin,gps.UTCSec);
		if(!sent){
			sendA7DataToTCPServer("A7_device1",A7_longitude_str,A7_latitude_str,A7_updated_time_str,A7_updated_date_str);
			sent = true;
			}
       // sleep(1); 
        // data ready
        A7_count++;
        gps.flagDataReady = true;
    }//$GPGGA
=======
int data_found;
extern pthread_mutex_t lock;
extern sem_t done_filling_list;
extern sem_t filling_list;
>>>>>>> 5a07b59be794440e2665fb998bff1d3b09fe6186


<<<<<<< HEAD
        gps.speed = strtof(gps.words[7], NULL);
        gps.speed *= 1.15078; // convert to mph
        // parse bearing
        gps.bearing = strtof(gps.words[8], NULL);

		strncpy(A7_updated_date_str,gps.words[9],8);
		A7_updated_date_str[9]=0;
		
        // parse UTC date
        gps.UTCDay = charToInt(gps.words[9][0]) * 10 + charToInt(gps.words[9][1]);
        gps.UTCMonth = charToInt(gps.words[9][2]) * 10 + charToInt(gps.words[9][3]);
        gps.UTCYear = charToInt(gps.words[9][4]) * 10 + charToInt(gps.words[9][5]);
		
        //printf("\n Speed %f Bearing %f ",gps.speed,gps.bearing);
        //printf("\n Date %d-%d-%d ",gps.UTCDay,gps.UTCMonth,gps.UTCYear);
        //sleep(1); 
        // data ready
        gps.flagDataReady = true;
    }//$GPRMC
    return;
}//parseDataSIM808GPS

void parseA7GPSNIMEADATA(char c){
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
			parseDataA7GPS();
		} else {
			// computed m_nChecksum logic: A7_count all chars between $ and * exclusively
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




static char* read_file (const char* filename, size_t* length)
{
  int fd;
  struct stat file_info;
  char* buffer;

  /* Open the file.  */
  fd = open (filename, O_RDONLY);

  if (fd == NULL) {
    *length = 0;
    return NULL;
  }


  /* Get information about the file.  */
  fstat (fd, &file_info);
  *length = file_info.st_size;

  /* Allocate a buffer large enough to hold the file's contents.  */
  buffer = (char*) malloc (*length);
  /* Read the file into the buffer.  */
  read (fd, buffer, *length);

  /* Finish up.  */
  close (fd);
  return buffer;
}



static char* read_from_buffer (size_t* length)
{
  *length = write_position;
  return gps_data_buffer;
}



static void release_file (char* buffer)
{
free(buffer);
//snprintf(move_file_name,sizeof(move_file_name),"%s%s%s", "rm ", " ",A7_logFileName);
system(move_file_name);
}
=======
int sendA7GPSData() {
>>>>>>> 5a07b59be794440e2665fb998bff1d3b09fe6186


data_found =getDataStatus();

send_count++;


if(A7DataConnect())
	{
	printf("sending data to web server from buffer\n");
	//
	if(data_found)
		sendA7StatusToTCPServer(1);

	A7DataDisconnect();
	
	SUCCESS: printf("sendGPSData SUCCESS \n");
	
	return(1);
	}

	else
		{
		exit: printf("\n sendGPSData FAILED\ n");
		return(0);
		}

}

