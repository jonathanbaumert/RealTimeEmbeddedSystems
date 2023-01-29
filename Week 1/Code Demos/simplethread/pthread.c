#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <syslog.h>
#define NUM_THREADS 10

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];


void *helloWorldThread(void *threadp)
{
    syslog(LOG_INFO, "[COURSE:1][ASSIGNMENT:1] 2. Hello World from Thread!\n");
}


int main (int argc, char *argv[])
{
   int rc;
   int i;

   char * command = "uname -a";
   FILE * fp;
   char path[1000];
   fp = popen(command, "r");
   while (fgets(path, 1000, fp)!=NULL)
        syslog(LOG_INFO,"%s",path);
   //system(command);
   syslog(LOG_INFO, "[COURSE:1][ASSIGNMENT:1] 1. Hello World from Main!\n");
   
   for(i=0; i < NUM_THREADS; i++)
   {
       threadParams[i].threadIdx=i;

       pthread_create(&threads[i],   // pointer to thread descriptor
                      (void *)0,     // use default attributes
                      helloWorldThread, // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                     );
   }

   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);
   
   
}
