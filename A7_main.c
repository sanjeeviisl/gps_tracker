
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gps_init.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include "A7_lib.h"

pthread_t tid[2];
pthread_mutex_t lock;
sem_t done_filling_list;        /* barrier to sync fill_list threads and empty_list threads */
sem_t filling_list;             /* to protect threads_fill_done */


extern int sendA7GPSData() ;
extern int receiveA7GPSData() ;
extern int A7_GPSPowerON ;
extern int A7_httpInitialize;
extern int A7_dataConnected ;



void* gpsA7DataReceiverTask(void *arg)
{
    unsigned long i = 0;
    pthread_t id = pthread_self();

    while(1)
    {
    sem_wait(&filling_list); // blocks is semaphore 0. If semaphore nonzero,

    pthread_mutex_lock(&lock);  
    if(pthread_equal(id,tid[0]))
    {
	//	 printf("\n Receiver  thread processing\n");
		 restart:
		  if(!receiveA7GPSData())
			 {
			 i++;
			 printf("\n ReceiveData Not OK, so resetting the module");
			 startRecoveryForA7ReceiveDataFailed(0);
			 sleep(1*i);
			 goto restart;
			 }
			  
	         
    }

    pthread_mutex_unlock(&lock);
    sem_post(&done_filling_list);

    }

    return NULL;
}

void* gpsA7DataSenderTask(void *arg)
{
    unsigned long i = 0;
    pthread_t id = pthread_self();

    while(1)
    {

    sem_wait(&done_filling_list);    
    pthread_mutex_lock(&lock);  
	
    if(pthread_equal(id,tid[1]))
    {
     // printf("\n Sender  thread processing\n");
      if(!sendA7GPSData())
		   {
		   printf("\n Send Data Not OK");
		   startRecoveryForA7SendDataFailed(1);
//		   pthread_mutex_unlock(&lock);
//		   sem_post(&filling_list);
  	   		}
    }
        
    pthread_mutex_unlock(&lock);
    sem_post(&filling_list);
     
    }

}

int A7_main()
{
    int i,s = 0;
    int err,res;
    //sleep(10);
    if(!openA7Port())
	{
         printf("\n Comport is not initialized\n");

		sleep(600);
         return 0;
	}

    if(!getA7DeviceInfo())
	{
         printf("Device is not initialized\n");
         return 0;
	}

	
	A7_GPSPowerON = 0;
	A7_httpInitialize = 0;
	A7_dataConnected = 0;

    res = sem_init(&done_filling_list,  /* pointer to semaphore */
                       0 ,              /* 0 if shared between threads, 1 if shared between processes */
                       0);              /* initial value for semaphore (0 is locked) */
    if (res < 0)
    {
        perror("Semaphore initialization failed");
        exit(0);
    }
    if (sem_init(&filling_list, 0, 1)) /* initially unlocked */
    {
        perror("Semaphore initialization failed");
        exit(0);
    }

    if (pthread_mutex_init(&lock, NULL) != 0)
       {
        printf("\n mutex init failed\n");
        return 1;
        }

    err = pthread_create(&(tid[0]), NULL, &gpsA7DataReceiverTask, NULL);
    if (err != 0)
       printf("\ncan't create thread :[%s]", strerror(err));

    err = pthread_create(&(tid[1]), NULL, &gpsA7DataSenderTask, NULL);
    if (err != 0)
       printf("\ncan't create thread :[%s]", strerror(err));

    while(1)
    	{
        sleep(86400);  //sleep for 24 hours (24*60*60)

        sem_wait(&filling_list);
	    sem_wait(&done_filling_list);
		
        printf("Canceling thread\n");

        s = pthread_cancel(tid[0]);
        s = pthread_cancel(tid[1]);

		sleep(60);
        system("reboot");
    	}
    return 0;

}




