#ifndef _SAMPLES_H
#define _SAMPLES_H

void samples_init(struct timespec *first_sampletime);
int8_t read_sample(unsigned int channel);
void write_sample(unsigned int channel, int8_t data);
void dump_outdata(void);
void dump_sample_times(void);


#define MAX_SAMPLES (4*1024*1024)
#define MAX_CHANNELS 2

#define MAX_ITERATIONS (2048)
#endif
