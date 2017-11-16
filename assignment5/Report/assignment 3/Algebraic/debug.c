#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/prctl.h>
#include "lift.h"

struct debug_info {
	pthread_mutex_t mutex;
	pthread_cond_t cv;
	int paused;
	int override[MAX_N_PERSONS];
	int to[MAX_N_PERSONS];
	int from[MAX_N_PERSONS];
};


static struct debug_info dbg;
void debug_init(void)
{
	pthread_mutex_init(&dbg.mutex,NULL);
	pthread_cond_init(&dbg.cv,NULL);
}

void debug_pause(void)
{
	pthread_mutex_lock(&dbg.mutex);
	dbg.paused = 1;
	pthread_cond_broadcast(&dbg.cv);
	pthread_mutex_unlock(&dbg.mutex);
}

void debug_unpause(void)
{
	pthread_mutex_lock(&dbg.mutex);
	dbg.paused = 0;
	pthread_cond_broadcast(&dbg.cv);
	pthread_mutex_unlock(&dbg.mutex);
}

void debug_check_override(int id, int *from, int *to)
{
	pthread_mutex_lock(&dbg.mutex);
	while(dbg.paused){
		pthread_cond_wait(&dbg.cv, &dbg.mutex);
	}
	if(dbg.override[id]) {
		dbg.override[id] = 0;
		*from = dbg.from[id];
		*to = dbg.to[id];
	}
	pthread_mutex_unlock(&dbg.mutex);
}

void debug_override(int id, int from, int to)
{
	pthread_mutex_lock(&dbg.mutex);
	dbg.override[id] = 1;
	dbg.from[id] = from;
	dbg.to[to] = to;
	pthread_mutex_unlock(&dbg.mutex);
}

