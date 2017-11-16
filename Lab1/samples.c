// This is a file which emulates I/O by using a file (data.raw) which
// contains fixed data. To emulate the effects of jitter on the
// sampling times, calls to clock_gettime() is used to read data from
// the sample buffer at the specified time.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <sched.h>
#include <pthread.h>

// This file contains the definition of MAX_SAMPLES and MAX_ITERATIONS, among other things.
#include "samples.h"


static int8_t raw_data[MAX_SAMPLES];
static unsigned int channel_iterations[MAX_CHANNELS];
static struct timespec sample_times[MAX_CHANNELS][MAX_ITERATIONS];
static uint64_t first_sample_time_in_usecs;

int8_t outdata[MAX_CHANNELS][MAX_ITERATIONS];
unsigned int output_idx[MAX_CHANNELS];

void samples_init(struct timespec *first_sampletime)
{
	FILE *fp = fopen("data.raw","r");
	fread(&raw_data[0], MAX_SAMPLES, 1, fp);
	fclose(fp);

	// The nanoseconds are rounded to the nearest microsecond
	first_sample_time_in_usecs = first_sampletime->tv_sec*1000000 + (first_sampletime->tv_nsec+500)/1000;
	printf("first_sample_time: %" PRIu64 "\n", first_sample_time_in_usecs);
}

int8_t read_sample(unsigned int channel)
{
	unsigned int current_sample;
	uint64_t time_in_usecs;
	struct timespec ts;
	if(channel >= MAX_CHANNELS){
		fprintf(stderr,"Too high channel number in read_sample(%d)\n", channel);
	}
	clock_gettime(CLOCK_MONOTONIC, &ts);
	time_in_usecs = ts.tv_sec*1000000 + (ts.tv_nsec+500)/1000;

	// We want fairly deterministic results. Hence we should make
	// sure that we always start on sample 0.

	time_in_usecs -= first_sample_time_in_usecs;
	current_sample = channel_iterations[channel];
	sample_times[channel][current_sample++] = ts;
	channel_iterations[channel] = current_sample;
	return raw_data[time_in_usecs & (MAX_SAMPLES-1)];
}


void write_sample(unsigned int channel, int8_t data)
{
	if(channel >= MAX_CHANNELS){
		fprintf(stderr,"Too high channel number in write_sample(%d, ...)\n", channel);
	}
	outdata[channel][output_idx[channel]] = data;
	output_idx[channel]++;
}


void dump_outdata(void)
{
	FILE *fp = fopen("output.txt","w");
	int i, j;
	printf("Dumping outdata\n");
	
	for(i=0; i < MAX_ITERATIONS; i++){
		for(j=0; j < MAX_CHANNELS; j++){
			fprintf(fp, "%d ", outdata[j][i]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

void dump_sample_times(void)
{
	FILE *fp = fopen("sample_times.txt","w");
	int i,j;
	printf("Dumping sample timing information\n");
	for(i=0; i < MAX_ITERATIONS; i++){
		for(j=0; j < MAX_CHANNELS; j++){
		  fprintf(fp, "%d.%09d ", (int) sample_times[j][i].tv_sec,(int) sample_times[j][i].tv_nsec);
		}
		fprintf(fp, "\n");
        }
        fclose(fp);
	return;
}

