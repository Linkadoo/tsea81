#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/time.h>
#include "lift.h"
#include "si_ui.h"


// Unfortunately the rand() function is not thread-safe. However, the
// rand_r() function is thread-safe, but need a pointer to an int to
// store the current state of the pseudo-random generator.  This
// pointer needs to be unique for every thread that might call
// rand_r() simultaneously. The functions below are wrappers around
// rand_r() which should work in the environment encountered in
// assignment 3.
//

static unsigned int rand_r_state[MAX_N_PERSONS];
// Get a random value between 0 and maximum_value. The passenger_id
// parameter is used to ensure that the rand_r() function is called in
// a thread-safe manner.

sem_t init_passenger;

static int get_random_value(int passenger_id, int maximum_value)
{
	return rand_r(&rand_r_state[passenger_id]) % (maximum_value + 1);
}

static lift_type Lift;

// Initialize the random seeds used by the get_random_value() function
// above.
static void init_random(void)
{
	int i;
	for(i=0; i < MAX_N_PERSONS; i++){
		// Use this statement if the same random number sequence
		// shall be used for every execution of the program.
		rand_r_state[i] = i;

		// Use this statement if the random number sequence
		// shall differ between different runs of the
		// program. (As the current time (as measured in
		// seconds) is used as a seed.)
		rand_r_state[i] = i+time(NULL);
	}
}



static void *lift_thread(void *unused)
{
	int floor_init = 0;
	int direction_init = 0;
	int* next_floor = &floor_init;
	int* change_direction = &direction_init;
	while(1){
		lift_next_floor(Lift, next_floor, change_direction);
		lift_move(Lift, *next_floor, *change_direction);
		lift_has_arrived(Lift);
	}
	return NULL;
}

static void *passenger_thread(void *idptr)
{
	
	// Code that reads the passenger ID from the idptr pointer
	// (due to the way pthread_create works we need to first cast
	// the void pointer to an int pointer).
	

	int *tmp = (int *) idptr;
	int id = *tmp;
	int nr_of_rides = 0;
	sem_post(&init_passenger);
	int to_floor=0, from_floor=0; 
	
	struct timeval starttime;
	struct timeval endtime;
	long long int timediff;
	gettimeofday(&starttime,NULL);
	
	while(1){
	  if(nr_of_rides == 10000)
	    {
	      gettimeofday(&endtime,NULL);
	      timediff = (endtime.tv_sec *1000000ULL + endtime.tv_usec)-(starttime.tv_sec *1000000ULL + starttime.tv_usec);
	      printf(" time difference: %lld\n", timediff);
	      return NULL;
	    }
		// * Select random floors
		 to_floor = get_random_value(id, N_FLOORS-1);
		 from_floor = get_random_value(id, N_FLOORS-1);
		if(to_floor != from_floor)
		{
			if(enter_floor(Lift,id,from_floor)){
			// * Travel between these floors
				lift_travel(Lift,id,from_floor,to_floor);
				nr_of_rides++;
			}
			// * Wait a little while
			//usleep(5000000);
		}
	}
	return NULL;
}

static void *user_thread(void *unused)
{
        
	int init_id = MAX_N_PERSONS;
	int* current_passenger_id = &init_id;
	char message[SI_UI_MAX_MESSAGE_SIZE]; 
	pthread_t thread_handle[MAX_N_PERSONS];
	si_ui_set_size(670, 700); 
	while(1){
		// Read a message from the GUI
		si_ui_receive(message);
		if(!strcmp(message, "new")){
			// create a new passenger if possible, else
			// use si_ui_show_error() to show an error
			// message if too many passengers have been
			// created. Make sure that each passenger gets
			// a unique ID between 0 and MAX_N_PERSONS-1.
			if(*current_passenger_id != MAX_N_PERSONS-1)
			{
				
			pthread_create(&thread_handle[*current_passenger_id], NULL, passenger_thread,(void*)current_passenger_id);
		       	sem_wait(&init_passenger);	
			(*current_passenger_id)++;
				
			}
			else
			{
			 si_ui_show_error("To many persons!");
			}
	
		}else if(!strcmp(message, "exit")){
			lift_delete(Lift);
			exit(0);
		}
	}
	return NULL;
}

int main(int argc, char **argv)
{
	si_ui_init();
	init_random();
	Lift = lift_create();
	
	pthread_t lift_thread_handle;
	pthread_t user_thread_handle;

	int init_id = 0;
	int* current_passenger_id = &init_id;
	pthread_t thread_handle[MAX_N_PERSONS];
	sem_init(&init_passenger,0,0);

	pthread_create(&lift_thread_handle, NULL, lift_thread, 0);
	pthread_create(&user_thread_handle, NULL, user_thread, 0);

	int i;
	for(i = 0; i<MAX_N_PERSONS; i++)
	  {
	    pthread_create(&thread_handle[*current_passenger_id], NULL, passenger_thread,(void*)current_passenger_id);
	    sem_wait(&init_passenger);
	    (*current_passenger_id)++;
	  }
	pthread_join(lift_thread_handle, NULL);
	pthread_join(user_thread_handle, NULL);

	//for(i=0; i<MAX_N_PERSONS; i++)
	  // {
	    // pthread_join(thread_handle[i], NULL);
	    //}
	return 0;
}
