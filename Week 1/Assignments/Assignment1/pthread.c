#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <syslog.h>
#define NUM_THREADS 1

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
    // print Hello World from the thread to the syslog
    syslog(LOG_INFO, "[COURSE:1][ASSIGNMENT:1] 2. Hello World from Thread!\n");
}


int main (int argc, char *argv[])
{
   int i; // create in for loop counter

   char * command = "uname -a";     // record the command to generate information "uname -a"
   FILE * fp;                       // create a file pointer to be used to record the output of the command
   char path[1000];                 // char pointer to record the output in pf
   fp = popen(command, "r");        // use popen with the command in read mode "-r"
   while (fgets(path, 1000, fp)!=NULL)
        syslog(LOG_INFO,"[COURSE:1][ASSIGNMENT:1]: %s",path); // print the output of "uname -a" to the syslog
   pclose(fp);                      // close the file 
   
   // print hello world from Main to the syslog
   syslog(LOG_INFO, "[COURSE:1][ASSIGNMENT:1] 1. Hello World from Main!\n");
   
   for(i=0; i < NUM_THREADS; i++) // loop through the number of threads creating the requested number of threads
   {
       threadParams[i].threadIdx=i; // set the thread id

       // Create a thread with the following descriptor, default info, function entry point and parameters 
       pthread_create(&threads[i],   // pointer to thread descriptor
                      (void *)0,     // use default attributes
                      helloWorldThread, // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                     );
   }

   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL); // join all created threads
   
   
}
