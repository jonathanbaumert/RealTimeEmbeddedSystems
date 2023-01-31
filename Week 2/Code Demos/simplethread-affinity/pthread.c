#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>

#define NUM_THREADS 64
#define NUM_CPUS 4

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
pthread_t mainthread;
pthread_t startthread;
threadParams_t threadParams[NUM_THREADS];

pthread_attr_t fifo_sched_attr;
pthread_attr_t orig_sched_attr;
struct sched_param fifo_param;

#define SCHED_POLICY SCHED_FIFO
#define MAX_ITERATIONS (1000000)


void print_scheduler(void)
{
    // dprint_scheduler: display the type of scheduling policy that is in use
    int schedType = sched_getscheduler(getpid());

    switch(schedType)
    {
        case SCHED_FIFO:
            printf("Pthread policy is SCHED_FIFO\n");
            break;
        case SCHED_OTHER:
            printf("Pthread policy is SCHED_OTHER\n");
            break;
        case SCHED_RR:
            printf("Pthread policy is SCHED_RR\n");
            break;
        default:
            printf("Pthread policy is UNKNOWN\n");
    }
}


void set_scheduler(void)
{
    //set_scheduler: set the desired scheduler attributes and scheduling policy setting
    int max_prio, scope, rc, cpuidx;
    cpu_set_t cpuset;

    printf("INITIAL "); print_scheduler();                                          // print the initial scheduling information

    pthread_attr_init(&fifo_sched_attr);                                            // initialize the scheduling attributes
    pthread_attr_setinheritsched(&fifo_sched_attr, PTHREAD_EXPLICIT_SCHED);         // set the inhereted scheduler to explicit
    pthread_attr_setschedpolicy(&fifo_sched_attr, SCHED_POLICY);                    // set the scheduling policy to FIFO
    CPU_ZERO(&cpuset);                                                              // zero out the cpu set
    cpuidx=(3);                                                                     // set the id of the cpu to use (CPU 4)
    CPU_SET(cpuidx, &cpuset);                                                       // set the cpu to cpu id of the 4th CPU
    pthread_attr_setaffinity_np(&fifo_sched_attr, sizeof(cpu_set_t), &cpuset);      // set the attribute that will limit the thread to running on the specified thread

    max_prio=sched_get_priority_max(SCHED_POLICY);                                  // get the maximum priority for the requested SCHED_POLICY
    fifo_param.sched_priority=max_prio;                                             // set the priority of the fifo param to the maximum priority of the SCHED_POLICY

    if((rc=sched_setscheduler(getpid(), SCHED_POLICY, &fifo_param)) < 0)            // set the scheduler with the given policy, and parameters
        perror("sched_setscheduler");                                               

    pthread_attr_setschedparam(&fifo_sched_attr, &fifo_param);                      // set the thread attributes to the set FIFO sheduling attributes

    printf("ADJUSTED "); print_scheduler();                                         // print the final scheduler information
}




void *counterThread(void *threadp)
{
    // counterThread: sum up to the id of the given thread, and do so MAX_ITERATION times
    // Inputs:
    //  - threadp: void * to a struct containing threadp information
    
    int sum=0, i, rc, iterations;
    threadParams_t *threadParams = (threadParams_t *)threadp;
    pthread_t mythread;
    double start=0.0, stop=0.0;
    struct timeval startTime, stopTime;

    gettimeofday(&startTime, 0);
    start = ((startTime.tv_sec * 1000000.0) + startTime.tv_usec)/1000000.0;


    for(iterations=0; iterations < MAX_ITERATIONS; iterations++)
    {
        sum=0;
        for(i=1; i < (threadParams->threadIdx)+1; i++)
            sum=sum+i;
    }


    gettimeofday(&stopTime, 0);
    stop = ((stopTime.tv_sec * 1000000.0) + stopTime.tv_usec)/1000000.0;

    printf("\nThread idx=%d, sum[0...%d]=%d, running on CPU=%d, start=%lf, stop=%lf", 
           threadParams->threadIdx,
           threadParams->threadIdx, sum, sched_getcpu(),
           start, stop);
}


void *starterThread(void *threadp)
{
    // starterThread: function to as an entry point to initialize the requested number of threads
    // Input:
    //  - threadp: void * to a struct containing threadp information
    int i, rc;

    printf("starter thread running on CPU=%d\n", sched_getcpu());

    for(i=0; i < NUM_THREADS; i++)
    {
        threadParams[i].threadIdx=i;

        pthread_create(&threads[i],   // pointer to thread descriptor
                        &fifo_sched_attr,     // use FIFO RT max priority attributes
                        counterThread, // thread function entry point
                        (void *)&(threadParams[i]) // parameters to pass in
                        );

    }

    for(i=0;i<NUM_THREADS;i++)
        pthread_join(threads[i], NULL);

}


int main (int argc, char *argv[])
{
   int rc;
   int i, j;
   cpu_set_t cpuset;

   set_scheduler();

   CPU_ZERO(&cpuset);

   // get affinity set for main thread
   mainthread = pthread_self();

   // Check the affinity mask assigned to the thread 
   rc = pthread_getaffinity_np(mainthread, sizeof(cpu_set_t), &cpuset);
   if (rc != 0)
       perror("pthread_getaffinity_np");
   else
   {
       printf("main thread running on CPU=%d, CPUs =", sched_getcpu());

       for (j = 0; j < CPU_SETSIZE; j++)
           if (CPU_ISSET(j, &cpuset))
               printf(" %d", j);

       printf("\n");
   }

   pthread_create(&startthread,   // pointer to thread descriptor
                  &fifo_sched_attr,     // use FIFO RT max priority attributes
                  starterThread, // thread function entry point
                  (void *)0 // parameters to pass in
                 );

   pthread_join(startthread, NULL);

   printf("\nTEST COMPLETE\n");
}
