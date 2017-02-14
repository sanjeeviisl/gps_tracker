#ifndef rs232_INCLUDED
#define rs232_INCLUDED

extern int cport_nr;
extern int openSIM808Port() ;
extern int sendDataToServer();

extern int getSim808DeviceInfo();

extern int Sim808DataConnect();
extern int GPSSim808NIMEAData(int ON) ;
extern int GPSSim808Power(int ON);
#endif

