#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>

// This file contains the definition of MAX_ITERATIONS, among other things.
#include "samples.h"


// Delay in nanoseconds (1 millisecond)
#define DELAY 1000000

// Number of samples that do_work() handles
#define PROCESSING_INTERVAL  256 

// Could be a local variable, but you may need it as a static variable
// here when you modify this file according to the lab instructions.
static int sample_buffer[PROCESSING_INTERVAL];
static int work_buffer[PROCESSING_INTERVAL];

struct sched_param p0;
struct sched_param p1;
sem_t sem;

void do_work(int *samples)
{
  int i;

  //  A busy loop. (In a real system this would do something
  //  more interesting such as an FFT or a particle filter,
  //  etc...)
  volatile int dummy; // A compiler warning is ok here
  for(i=0; i < 20000000;i++){
    dummy=i;
  }

  // Write out the samples.
  for(i=0; i < PROCESSING_INTERVAL; i++){
    write_sample(0,samples[i]);
  }
}

void* work_thread(void* arg)
{
  if(pthread_setschedparam(pthread_self(), SCHED_RR, &p1) != 0){
    perror("Could not set the thread priority");
  }
  int i;
  for(i=0; i < MAX_ITERATIONS/PROCESSING_INTERVAL; i++)
    {
      // Wait until 256 samples have been processed
      sem_wait(&sem);
      do_work(work_buffer);
    }
  return NULL;
}

struct timespec firsttime;
void *maintask(void *arg)
{
  if(pthread_setschedparam(pthread_self(), SCHED_RR, &p0) != 0){
    perror("Could not set the thread priority");
  }
  int channel = 0;
  struct timespec current;
  int i;
  int samplectr = 0;
  current = firsttime;

  for(i=0; i < MAX_ITERATIONS; i++){
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &current, NULL);

    sample_buffer[samplectr] = read_sample(channel);
    samplectr++;
    if(samplectr == PROCESSING_INTERVAL){
      int j;
      for(j=0; j<PROCESSING_INTERVAL; j++)
	{
	  work_buffer[j] = sample_buffer[j];
	}
      samplectr = 0;
      sem_post(&sem);
    }

    // Increment current time point
    current.tv_nsec +=  DELAY;
    if(current.tv_nsec >= 1000000000){
      current.tv_nsec -= 1000000000;
      current.tv_sec++;
    }
  }
  return NULL;
}

int main(int argc,char **argv)
{
  if(mlockall(MCL_CURRENT | MCL_FUTURE) != 0)
   {
     printf("Error when trying mlockall\n");
   }
  
  pthread_t thread0;
  pthread_t thread1;
  pthread_attr_t attr;

  sem_init(&sem,0,0);

  clock_gettime(CLOCK_MONOTONIC, &firsttime);

  // Start the sampling at an even multiple of a second (to make
  // the sample times easy to analyze by hand if necessary)
  firsttime.tv_sec+=2;
  firsttime.tv_nsec = 0;
  printf("Starting sampling at about t+2 seconds\n");
        
  samples_init(&firsttime);

  if(pthread_attr_init(&attr)){
    perror("pthread_attr_init");
  }
  // Set default stacksize to 64 KiB (should be plenty)
  if(pthread_attr_setstacksize(&attr, 65536)){
    perror("pthread_attr_setstacksize()");
  }
  p1.sched_priority = 4;
  p0.sched_priority = 5;

  pthread_create(&thread0, &attr, maintask, NULL);
  pthread_create(&thread1, &attr, work_thread, NULL);

  pthread_join(thread0, NULL);
  pthread_join(thread1, NULL);
 
  // Dump output data which will be used by the analyze.m script
  dump_outdata();
  dump_sample_times();

  munlockall();
  return 0;
}
