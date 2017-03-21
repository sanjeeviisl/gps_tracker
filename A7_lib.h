#ifndef LIBA7_INCLUDED
#define LIBA7_INCLUDED

typedef int bool ;


struct gpsStruct {//GPRMC
    int     timeZone;
    bool    flagRead;        // flag used by the parser, when a valid sentence has begun
    bool    flagDataReady;   // valid GPS fix and data available, user can call reader functions
    bool    flagDateReady;   // valid GPS fix and data available, user can call reader functions    
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
    char    time[7];
	char    date[7];


};


extern int A7_commond_cport_nr;

extern int A7_data_cport_nr;

extern unsigned char gps_data_buffer[];
extern int write_position,readComplete;

extern int openA7Port() ;
extern int sendA7DataToServer();

extern int getA7DeviceInfo();

extern int A7DataConnect();
extern int GPSA7NIMEAData(int ON) ;
extern int GPSA7Power(int ON);
extern resetHardA7GSMModule();
extern int sendA7DataToTCPServer(char * device_id,char * longitude,char * latitude,char * updated_time,char * updated_date);
extern void Resetbufer(unsigned char *buf,int size);
extern char *dtostrf (double val, signed char width, unsigned char prec, char *sout);





#endif

