/****************************************************************************/
/* Function: nanosleep and POSIX 1003.1b RT clock demonstration             */
/*                                                                          */
/* Sam Siewert      - 02/05/2011                                            */
/* Jonathan Baumert - 01/29/2023                                            */
/****************************************************************************/

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define NSEC_PER_SEC (1000000000)                               //number of nanoseconds per second
#define NSEC_PER_MSEC (1000000)                                 //number of nanoseconds per millisecond
#define NSEC_PER_USEC (1000)                                    //number of nanoseconds per microsecond
#define ERROR (-1)                                              // define error code numbering
#define OK (0)                                                  // define success code numbering
#define TEST_SECONDS (0)
#define TEST_NANOSECONDS (NSEC_PER_MSEC * 10)                   // set each test to ten nanoseconds

void end_delay_test(void);        

static struct timespec sleep_time = {0, 0};                     // struct to store seconds and nanoseconds of the current sleep duration
static struct timespec sleep_requested = {0, 0};                // struct to store seconds and nanoseconds of the requested sleep duration
static struct timespec remaining_time = {0, 0};                 // struct to store the difference between current and requested sleep durations

static unsigned int sleep_count = 0;                            // number of sleep iterations

pthread_t main_thread;                                          // global main thread
pthread_attr_t main_sched_attr;                                 // global value to be used to set the attributes of the main thread
int rt_max_prio, rt_min_prio, min;                              // priority variables
struct sched_param main_param;                                  // global value for scheduling parameters to be set


void print_scheduler(void)
{
  // display the type of scheduling policy that is in use
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
     case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;
     case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
           printf("Pthread Policy is SCHED_RR\n");
           break;
     default:
       printf("Pthread Policy is UNKNOWN\n");
   }
}


double d_ftime(struct timespec *fstart, struct timespec *fstop)
{
  // compute the difference in time inputs
  // Inputs:
  //  - fstart - struct timespec * that recorded the time at the start of the period
  //  - fstop  - struct timespec * that recorded the time at the end of the period
  // Output: the difference between fstart and fstop
  double dfstart = ((double)(fstart->tv_sec) 
                 + ((double)(fstart->tv_nsec) / 1000000000.0)); 
  double dfstop = ((double)(fstop->tv_sec) 
                + ((double)(fstop->tv_nsec) / 1000000000.0));

  return(dfstop - dfstart); 
}


int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
  // delta_t compute the difference in time inputs to evaluate potential scenarios
  // Inputs:
  //  - fstart - struct timespec * that recorded the time at the start of the period
  //  - fstop  - struct timespec * that recorded the time at the end of the period
  // Output: the code indicating scenario (OK or ERROR)

  // record the difference between start and stop for seconds and nanoseconds
  int dt_sec=stop->tv_sec - start->tv_sec;
  int dt_nsec=stop->tv_nsec - start->tv_nsec;

  // case 1 - less than a second of change
  if(dt_sec == 0)
  {	  
	  if(dt_nsec >= 0 && dt_nsec < NSEC_PER_SEC) //nanosec greater at stop than start
	  {
	          
		  delta_t->tv_sec = 0;
		  delta_t->tv_nsec = dt_nsec;
	  }
	  else if(dt_nsec > NSEC_PER_SEC) //nanosec overflow
	  {
	          
		  delta_t->tv_sec = 1;
		  delta_t->tv_nsec = dt_nsec-NSEC_PER_SEC;
	  }
	  else // ERROR condition: dt_nsec < 0 means stop is earlier than start
	  {
	         printf("stop is earlier than start\n");
		 return(ERROR);  
	  }
  }

  // case 2 - more than a second of change, check for roll-over
  else if(dt_sec > 0)
  {	  
	  if(dt_nsec >= 0 && dt_nsec < NSEC_PER_SEC) //nanosec greater at stop than start
	  {	          
		  delta_t->tv_sec = dt_sec;
		  delta_t->tv_nsec = dt_nsec;
	  }

	  else if(dt_nsec > NSEC_PER_SEC) //nanosec overflow
	  {	          
		  delta_t->tv_sec = delta_t->tv_sec + 1;
		  delta_t->tv_nsec = dt_nsec-NSEC_PER_SEC;
	  }

	  else // dt_nsec < 0 means roll over
	  {
	    delta_t->tv_sec = dt_sec-1;
		  delta_t->tv_nsec = NSEC_PER_SEC + dt_nsec;
	  }
  }

  return(OK);
}

static struct timespec rtclk_dt = {0, 0};                       // real time clock delta 
static struct timespec rtclk_start_time = {0, 0};               // real time clock value at the start of process
static struct timespec rtclk_stop_time = {0, 0};                // real time clock value at the end of process
static struct timespec delay_error = {0, 0};                    // error in time due to delays

//#define MY_CLOCK CLOCK_REALTIME
//#define MY_CLOCK CLOCK_MONOTONIC
#define MY_CLOCK CLOCK_MONOTONIC_RAW                            // set the clock to use the monotonic raw clock
//#define MY_CLOCK CLOCK_REALTIME_COARSE
//#define MY_CLOCK CLOCK_MONOTONIC_COARSE

#define TEST_ITERATIONS (100)                                   // define the number of iterations to test

void *delay_test(void *threadID)
{
  // *delay_test 
  // Inputs:
  //  - threadID - void * that contains...c
  
  int idx, rc;                        //id and return code variables
  unsigned int max_sleep_calls=3;     //maximum number of sleep iterations
  int flags = 0;                      //variable to store flags
  struct timespec rtclk_resolution;   //struct to store the real time clock resolution

  sleep_count = 0;                    //initialize sleep counter to 0

  // get the resolution of the clock that has been specified
  // display an error if getting the resolution fails otherwise print the returned resulution
  if(clock_getres(MY_CLOCK, &rtclk_resolution) == ERROR)
  {
      perror("clock_getres");
      exit(-1);
  }
  else
  {
      printf("\n\nPOSIX Clock demo using system RT clock with resolution:\n\t%ld secs, %ld microsecs, %ld nanosecs\n",
              rtclk_resolution.tv_sec, (rtclk_resolution.tv_nsec/1000), rtclk_resolution.tv_nsec);
  }

  // loop through clock test code TEST_ITERATIONS number of times
  for(idx=0; idx < TEST_ITERATIONS; idx++)
  {
      printf("test %d\n", idx); // print the current test iteration number

      /* run test for defined seconds */
      sleep_time.tv_sec=TEST_SECONDS;
      sleep_time.tv_nsec=TEST_NANOSECONDS;
      sleep_requested.tv_sec=sleep_time.tv_sec;
      sleep_requested.tv_nsec=sleep_time.tv_nsec;

      /* start time stamp */ 
      clock_gettime(MY_CLOCK, &rtclk_start_time);

      /* request sleep time and repeat if time remains */
      do 
      {
          if(rc=nanosleep(&sleep_time, &remaining_time) == 0) break;
         
          sleep_time.tv_sec = remaining_time.tv_sec;
          sleep_time.tv_nsec = remaining_time.tv_nsec;
          sleep_count++;
      } 
      while (((remaining_time.tv_sec > 0) || (remaining_time.tv_nsec > 0))
		      && (sleep_count < max_sleep_calls));

      clock_gettime(MY_CLOCK, &rtclk_stop_time); // stop time stamp

      delta_t(&rtclk_stop_time, &rtclk_start_time, &rtclk_dt);
      delta_t(&rtclk_dt, &sleep_requested, &delay_error);

      // perform end of test actions
      end_delay_test();
  }

}

void end_delay_test(void)
{
  // end_delay_test displays the relevant information at the end of a timed test iteration

  double real_dt;

  real_dt=d_ftime(&rtclk_start_time, &rtclk_stop_time);
  // print the time difference
  printf("MY_CLOCK clock DT seconds = %ld, msec=%ld, usec=%ld, nsec=%ld, sec=%6.9lf\n", 
         rtclk_dt.tv_sec, rtclk_dt.tv_nsec/1000000, rtclk_dt.tv_nsec/1000, rtclk_dt.tv_nsec, real_dt);

  // print the delay error
  printf("MY_CLOCK delay error = %ld, nanoseconds = %ld\n", 
         delay_error.tv_sec, delay_error.tv_nsec);
}

#define RUN_RT_THREAD

void main(void)
{
   int rc, scope;

   printf("Before adjustments to scheduling policy:\n");
   print_scheduler();                                                             // print the initial scheduling policy

#ifdef RUN_RT_THREAD
   pthread_attr_init(&main_sched_attr);                                           // initialize the attributes of the scheduling policy
   pthread_attr_setinheritsched(&main_sched_attr, PTHREAD_EXPLICIT_SCHED);        // set the scheduling inheritence mode attribute to Explicit
   pthread_attr_setschedpolicy(&main_sched_attr, SCHED_FIFO);                     // set the schedululing policy attribute to FIFO

   rt_max_prio = sched_get_priority_max(SCHED_FIFO);                              // record the max priority for SCHED_FIFO
   rt_min_prio = sched_get_priority_min(SCHED_FIFO);                              // record the min priority for SCHED_FIFO                              

   main_param.sched_priority = rt_max_prio;
   rc=sched_setscheduler(getpid(), SCHED_FIFO, &main_param);


   if (rc)                                                                        // if there is an error setting the sheduler display the error and exit
   {
       printf("ERROR; sched_setscheduler rc is %d\n", rc);
       perror("sched_setschduler"); exit(-1);
   }

   printf("After adjustments to scheduling policy:\n");                           // print the updated scheduling policy
   print_scheduler();

   main_param.sched_priority = rt_max_prio;                                       // set the max priority of the main param to the maximum FIFO prioirty value
   pthread_attr_setschedparam(&main_sched_attr, &main_param);                     // set the scheduling attribute and parameters for the main pthread

   rc = pthread_create(&main_thread, &main_sched_attr, delay_test, (void *)0);    // initiate the first thread

   if (rc)                                                                        // check if there was an error in the thread, if so display the error and exit
   {
       printf("ERROR; pthread_create() rc is %d\n", rc);
       perror("pthread_create");
       exit(-1);
   }

   pthread_join(main_thread, NULL);                                               // join the main thread

   if(pthread_attr_destroy(&main_sched_attr) != 0)                                // destroy the thread attributes
     perror("attr destroy");
#else
   delay_test((void *)0);
#endif

   printf("TEST COMPLETE\n");
}

