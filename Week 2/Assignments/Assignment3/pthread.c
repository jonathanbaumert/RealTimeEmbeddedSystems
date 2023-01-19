#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sched.h>
#include <syslog.h>

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
pthread_t mainthread;
pthread_t startthread;
threadParams_t threadParams[NUM_THREADS];
pthread_attr_t fifo_sched_attr;
struct sched_param fifo_param;

#define SCHED_POLICY SCHED_FIFO

void set_scheduler(void)
{
    int max_prio, scope, rc, cpuidx;
    cpu_set_t cpuset;

    pthread_attr_init(&fifo_sched_attr);
    pthread_attr_setinheritsched(&fifo_sched_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&fifo_sched_attr, SCHED_POLICY);

    CPU_ZERO(&cpuset);
    cpuidx=(3);
    CPU_SET(cpuidx, &cpuset);

    pthread_attr_setaffinity_np(&fifo_sched_attr, sizeof(cpu_set_t), &cpuset);

    max_prio = sched_get_priority_max(SCHED_POLICY);
    fifo_param.sched_priority = max_prio;

    sched_setscheduler(getpid(), SCHED_POLICY, &fifo_param);

    pthread_attr_setschedparam(&fifo_sched_attr, &fifo_param);
}

void *sumUpThread(void *threadp)
{
    int sum = 0; // initial sum
    int i; // index variable

    threadParams_t *threadParams = (threadParams_t *) threadp;

    for(i=1; i<(threadParams->threadIdx); i++)
    {
        sum = sum + i;
    }    
    
    syslog(LOG_INFO, "[COURSE 1][ASSIGNMENT 3]: Thread idx=%d ,sum[1...%d]= Running on core : %d",
        threadParams->threadIdx, threadParams->threadIdx, sum, sched_getcpu());
}

void * starterThread(void *threadp){
    int i, rc;

    for(i=0; i < NUM_THREADS; i++)
    {
        threadParams[i].threadIdx = i + 1;

        pthread_create(&threads[i],
                        &fifo_sched_attr,
                        sumUpThread,
                        (void *) &(threadParams[i]));
    }

    for( i = 0; i < NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }
}

int main (int argc, char *argv[])
{
   
}
