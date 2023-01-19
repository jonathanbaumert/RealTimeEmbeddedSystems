#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

#define COUNT  1000
#define NUM_THREADS 128
#define NUM_CPUS 8

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];
pthread_attr_t fifo_sched_attr;
struct sched_param fifo_param;

// Unsafe global
int gsum=0;

void *sumUpThread(void *threadp)
{
    int sum = 0; // initial sum
    int i; // index variable
}

void *incThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        gsum=gsum+i;
        printf("Increment thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
    }
}


void *decThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        gsum=gsum-i;
        printf("Decrement thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
    }
}




int main (int argc, char *argv[])
{
   int rc;
   int i=0;

   threadParams[i].threadIdx=i;
   pthread_create(&threads[i],   // pointer to thread descriptor
                  (void *)0,     // use default attributes
                  incThread, // thread function entry point
                  (void *)&(threadParams[i]) // parameters to pass in
                 );
   i++;

   threadParams[i].threadIdx=i;
   pthread_create(&threads[i], (void *)0, decThread, (void *)&(threadParams[i]));

   for(i=0; i<2; i++)
     pthread_join(threads[i], NULL);

   printf("TEST COMPLETE\n");
}
