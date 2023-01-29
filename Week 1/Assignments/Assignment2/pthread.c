#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <syslog.h>
#define NUM_THREADS 128

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];


void *sumThread(void *threadp)
{
    int sum = 0; // initialize a sum counter to zero for this thread
    threadParams_t * thisThread = (threadParams_t *) threadp; // record this thread information
    for (int i = 1; i <= (thisThread->threadIdx); i++){ // loop through from 1 to this thread ID and record sum
        sum = sum + i;
    }

    // Print the sum information in the format below to the syslog
    //<System Time>  <Host Name>  [COURSE:1][ASSIGNMENT:2]: Thread idx=10, sum[1...10]=55
    syslog(LOG_INFO, "[COURSE:1][ASSIGNMENT:2]: Thread idx=%d, sum[1...%d]=%d\n", 
        thisThread->threadIdx,
        thisThread->threadIdx,
        sum);
}


int main (int argc, char *argv[])
{
   int i; // create int for loop counter

   char * command = "uname -a";     // record the command to generate information "uname -a"
   FILE * fp;                       // create a file pointer to be used to record the output of the command
   char path[1000];                 // char pointer to record the output in pf
   fp = popen(command, "r");        // use popen with the command in read mode "-r"
   while (fgets(path, 1000, fp)!=NULL)
        syslog(LOG_INFO,"[COURSE:1][ASSIGNMENT:2]: %s",path); // print the output of "uname -a" to the syslog
   pclose(fp);                      // close the file 
   
   for(i=0; i < NUM_THREADS; i++) // loop through the number of threads creating the requested number of threads
   {
       threadParams[i].threadIdx=i+1; // set the id of the thread

       // Create a thread with the following descriptor, default info, function entry point and parameters 
       pthread_create(&threads[i],   // pointer to thread descriptor
                      (void *)0,     // use default attributes
                      sumThread, // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                     );
   }

   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL); // join all created threads
   
   
}
