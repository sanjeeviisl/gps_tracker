#ifndef LIBA7_INCLUDED
#define LIBA7_INCLUDED

extern int A7_commond_cport_nr;

extern int A7_data_cport_nr;

extern int openA7Port() ;
extern int sendA7DataToServer();

extern int getA7DeviceInfo();

extern int A7DataConnect();
extern int GPSA7NIMEAData(int ON) ;
extern int GPSA7Power(int ON);
extern resetHardA7GSMModule();
extern int sendA7DataToTCPServer(char * device_id,char * longitude,char * latitude,char * updated_time,char * updated_date);



#endif

