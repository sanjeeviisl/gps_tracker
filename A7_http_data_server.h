#ifndef A7HTTP_h
#define A7HTTP_h

extern int A7HTTP_HttpPost(char * body);
extern void A7HTTP_begin();
  
extern int A7_Show_GSM_Siganl_Qauality();
extern void A7HTTP_post_setup(char * APN, char * host, char * path, int port, char * Content_Type);

#endif
