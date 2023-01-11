#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <syslog.h>

#define COUNT  1000 // set count to 1000

typedef struct
{
    int threadIdx;
} threadParams_t; // create a struct to be used to pass in thread IDs to the threads


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[2];
threadParams_t threadParams[2];


// Unsafe global
int gsum=0;

void *incThread(void *threadp)
{
    int i; // create a counter
    threadParams_t *threadParams = (threadParams_t *)threadp; // record teh thread information passed to the function

    // increment gsum COUNT times, each time increasing gsum by that respective iteration number i and print the result
    for(i=0; i<COUNT; i++)
    {
        gsum=gsum+i;
        printf("Increment thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
    }
}


void *decThread(void *threadp)
{
    int i; // create a counter
    threadParams_t *threadParams = (threadParams_t *)threadp; // record the thread information passed to the function

    // decrement gsum COUNT times, each time decreasing gsum by that respective iteration number i and print the result
    for(i=0; i<COUNT; i++)
    {
        gsum=gsum-i;
        printf("Decrement thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
    }
}




int main (int argc, char *argv[])
{
   int i=0; // create a counter

   threadParams[i].threadIdx=i; // set the thread id

   // create the first thread with the incThread function as the entry point
   pthread_create(&threads[i],   // pointer to thread descriptor
                  (void *)0,     // use default attributes
                  incThread, // thread function entry point
                  (void *)&(threadParams[i]) // parameters to pass in
                 );
   i++;


   threadParams[i].threadIdx=i; // set the id of the second thread

   // create a second thread with the decrement thread as the entry point
   pthread_create(&threads[i], (void *)0, decThread, (void *)&(threadParams[i]));

   for(i=0; i<2; i++) // join both of the two created threads
     pthread_join(threads[i], NULL);

   printf("TEST COMPLETE\n"); // display that the test has completed
}
