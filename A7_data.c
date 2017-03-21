


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
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>        
#include <stdlib.h> 
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include "A7_lib.h"
#include "rs232.h"

#define true 1
#define false 0

extern pthread_mutex_t lock;
extern sem_t done_filling_list;        /* barrier to sync fill_list threads and empty_list threads */
extern sem_t filling_list;             /* to protect threads_fill_done */

void parseDataA7GPS(void);
void parseA7GPSNIMEADATA(char c);
void parseNimeaData(unsigned char * string, int size);

struct gpsStruct gps;
char * temp;
char t_buffer[10];
char t_buffer1[10];
char t_buffer2[10];
char t_buffer3[10];
char t_buffer4[10];
char t_buffer5[10];

int n,write_position,readComplete;
unsigned char gps_data_buffer[22400];



int charToInt(char c){ return (c - 48);}
double trunc(double d){ return (d>0) ? floor(d) : ceil(d) ; }

int receiveA7GPSData() {
int count = 0 ;
unsigned char buff;

n = 0;
write_position = 0;
readComplete = true;

while (true) { 
	  n = RS232_PollComport(A7_data_cport_nr,&buff,1 );
	  if (n == -1) switch(errno) {
	         case EAGAIN:  sleep(1) ;
	            continue;
	         default: {sleep(1) ;continue;}
	         }
	  if (n == 0) {sleep(1); continue;}
	  
	  if(count > 120) {	  
	  		gps_data_buffer[write_position] = 0;   
			parseNimeaData(gps_data_buffer, write_position);
			
	  	}

	  if(buff == '$') count++;
	  gps_data_buffer[write_position++] = buff;
		//printf("%c", buff);
	}

   return 1;
}


int getDataStatus()
{
	return gps.flagDataReady;
}

void parseNimeaData(unsigned char * string, int size) {
	int i;
	char ch;
	pthread_mutex_lock(&lock);
	for(i= 0 ; i < size ;i++)
	{
		  ch = string[i];
		  parseA7GPSNIMEADATA(ch);
	}
	pthread_mutex_unlock(&lock);
	printf(".");
	sleep(1);
}




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


		gps.UTCMin = gps.UTCMin+30;
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


//        A7_longitude_str=dtostrf(gps.longitude,0,6,t_buffer1);
//        A7_latitude_str=dtostrf(gps.latitude,0,6,t_buffer2);


		strncpy(gps.time,gps.words[1],6);
		gps.time[7]=0;
		
//		A7_updated_time_str[7]= 0;

        // parse number of satellites
        gps.satellitesUsed = (int)strtof(gps.words[7], NULL);
        
        // parse horizontal dilution of precision
        gps.dilution = strtof(gps.words[8], NULL);

        // parse altitude
        gps.altitude = strtof(gps.words[9], NULL);

        //printf("\n Lattitude %f Longitude %f Sattellite used %d ",gps.latitude,gps.longitude,gps.satellitesUsed);
        //printf("\n Time: %d.%d.%d ",gps.UTCHour,gps.UTCMin,gps.UTCSec);
       // sleep(1); 
        // data ready
//        A7_count++;
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
            gps.flagDateReady = false;
    //        printf("\nNo Valid signal");
            return;
        }
        gps.dataValid = true;

        gps.speed = strtof(gps.words[7], NULL);
//        gps.speed *= 1.15078; // convert to mph
        // parse bearing
        gps.bearing = strtof(gps.words[8], NULL);

//		strncpy(A7_updated_date_str,gps.words[9],6);
//		A7_updated_date_str[7]=0;
		
        // parse UTC date
        gps.UTCDay = charToInt(gps.words[9][0]) * 10 + charToInt(gps.words[9][1]);
        gps.UTCMonth = charToInt(gps.words[9][2]) * 10 + charToInt(gps.words[9][3]);
        gps.UTCYear = charToInt(gps.words[9][4]) * 10 + charToInt(gps.words[9][5]);

		strncpy(gps.date,gps.words[9],6);
		gps.date[7]=0;
		
        //printf("\n Speed %f Bearing %f ",gps.speed,gps.bearing);
        //printf("\n Date %d-%d-%d ",gps.UTCDay,gps.UTCMonth,gps.UTCYear);
        //sleep(1); 
        // data ready
        gps.flagDateReady = true;
    }//$GPRMC
    return;
}//parseDataSIM808GPS

void parseA7GPSNIMEADATA(char c)
	{
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





