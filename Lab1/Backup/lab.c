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

struct timespec firsttime;
void *maintask(void *arg)
{
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
                        samplectr = 0;
                        do_work(sample_buffer);
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
        pthread_t thread0;
        pthread_attr_t attr;

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
        
        pthread_create(&thread0, &attr, maintask, NULL);
        pthread_join(thread0, NULL);

        // Dump output data which will be used by the analyze.m script
        dump_outdata();
        dump_sample_times();
        return 0;
}
