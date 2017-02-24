#ifndef LIB808_INCLUDED
#define LIB808_INCLUDED

extern int A7_commond_cport_nr;

extern int A7_data_cport_nr;

extern int openA7Port() ;
extern int sendA7DataToServer();

extern int getA7DeviceInfo();

extern int A7DataConnect();
extern int GPSA7NIMEAData(int ON) ;
extern int GPSA7Power(int ON);

#endif

