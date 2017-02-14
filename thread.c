#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include <pthread.h>
#include <semaphore.h>
 
pthread_t tid[2];
pthread_mutex_t lock;
sem_t done_filling_list;        /* barrier to sync fill_list threads and empty_list threads */
sem_t filling_list;             /* to protect threads_fill_done */


extern int sendGPSData() ; 
extern int receiveGPSData() ; 
void* gpsDataReceiverTask(void *arg)
{
    unsigned long i = 0;
    pthread_t id = pthread_self();

    while(1)
    {
		
	/* entering critical section with semaphore (could use mutex too) */
    sem_wait(&filling_list); // blocks is semaphore 0. If semaphore nonzero,
                             // it decrements semaphore and proceeds
    pthread_mutex_lock(&lock);    
    if(pthread_equal(id,tid[0]))
    {
        printf("\n Receiver  thread processing\n");
		sleep(5);
    receiveGPSData();
    }

    pthread_mutex_unlock(&lock);
   sem_post(&done_filling_list);

    }

    return NULL;
}

void* gpsDataSenderTask(void *arg)
{
    unsigned long i = 0;
    pthread_t id = pthread_self();

    while(1)
    {
    
	sem_wait(&done_filling_list);
	
    pthread_mutex_lock(&lock);    
    if(pthread_equal(id,tid[1]))
    {
		printf("\n Sender  thread processing\n");
                sendGPSData();
		sleep(13);
    }
	
    pthread_mutex_unlock(&lock);
	sem_post(&filling_list);
	
    }

    return NULL;
}






int main(void)
{
    int i = 0;
    int err;

    int res = sem_init(&done_filling_list,  /* pointer to semaphore */
                       0 ,                  /* 0 if shared between threads, 1 if shared between processes */
                       0);                  /* initial value for semaphore (0 is locked) */
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

    err = pthread_create(&(tid[0]), NULL, &gpsDataReceiverTask, NULL);
    if (err != 0)
       printf("\ncan't create thread :[%s]", strerror(err));
    else
       printf("\n Receiver Thread created successfully\n");

    err = pthread_create(&(tid[1]), NULL, &gpsDataSenderTask, NULL);
    if (err != 0)
       printf("\ncan't create thread :[%s]", strerror(err));
    else
       printf("\n Sender Thread created successfully\n");

    while(1)
        sleep(500000);
    return 0;
}

