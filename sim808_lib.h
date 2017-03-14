#ifndef LIB808_INCLUDED
#define LIB808_INCLUDED

extern int cport_nr;


extern int openSIM808Port() ;
extern int sendDataToServer();

extern int getSim808DeviceInfo();

extern int Sim808DataConnect();
extern int GPSSim808NIMEAData(int ON) ;
extern int GPSSim808Power(int ON);

#endif

