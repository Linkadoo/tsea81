/* A clock program 

Two tasks are used, clock_task for updating a clock, and 
set_task for setting the clock 

*/ 

/* standard library includes */ 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "display.h"

/* clock variables */ 
sem_t start_alarm;

/* mutex for protecting the current time */ 
pthread_mutex_t Mutex;

/* the current time */ 
int Hours, Minutes, Seconds; 
int AHours, AMinutes, ASeconds; 
volatile int Active;
/* functions operating on the clock */ 

/* init_clock: initialises the clock variables */ 
void init_clock(void)
{
    /* set starting time */ 
    Hours = 12;
    Minutes = 59; 
    Seconds = 30; 

    /* initialise mutex */ 
    pthread_mutex_init(&Mutex, NULL);
    sem_init(&start_alarm,0,0);
}

/* increment_time: increments the current time with 
   one second */ 
void increment_time(void)
{
    /* reserve clock variables */ 
    pthread_mutex_lock(&Mutex);

    /* increment time */ 
    Seconds++; 
    if (Seconds > 59)
    {
        Seconds = 0; 
        Minutes++; 
        if (Minutes > 59)
        {
            Minutes = 0; 
            Hours++; 
            if (Hours > 12)
            {
                Hours = 1; 
            }
        }
    }

    /* release clock variables */ 
    pthread_mutex_unlock(&Mutex);
}

/* set_time: set time to hours, minutes and seconds */ 
void set_time(int hours, int minutes, int seconds)
{
    /* reserve clock variables */ 
    pthread_mutex_lock(&Mutex);

    /* assign values */
    Hours = hours; 
    Minutes = minutes;
    Seconds = seconds; 
    
    /* release clock variables */ 
    pthread_mutex_unlock(&Mutex);
}

/* set_alarm: set alarm to hours, minutes and seconds */ 
void set_alarm(int hours, int minutes, int seconds)
{
    /* reserve clock variables */ 
    pthread_mutex_lock(&Mutex);

    /* assign values */
    AHours = hours; 
    AMinutes = minutes;
    ASeconds = seconds; 
    
    /* release clock variables */ 
    pthread_mutex_unlock(&Mutex);
}

/* get_time: read time from common clock variables */ 
void get_time(int *hours, int *minutes, int *seconds)
{
    /* reserve clock variables */ 
    pthread_mutex_lock(&Mutex);

    /* read values */ 
    *hours = Hours; 
    *minutes = Minutes; 
    *seconds = Seconds;
        
    /* release clock variables */ 
    pthread_mutex_unlock(&Mutex);
}


/* time_from_set_message: extract time from set message from user interface */ 
void time_from_set_message(char message[], int *hours, int *minutes, int *seconds)
{
    sscanf(message,"set:%d:%d:%d",hours, minutes, seconds); 
}

/* time_from_set_alarm: extract alarm from set message from user interface */ 
void time_from_set_alarm(char message[], int *hours, int *minutes, int *seconds)
{
    sscanf(message,"alarm:%d:%d:%d",hours, minutes, seconds); 
}

/* time_ok: returns nonzero if hours, minutes and seconds represents a valid time */ 
int time_ok(int hours, int minutes, int seconds)
{
    return hours >= 1 && hours <= 12 && minutes >= 0 && minutes <= 59 && 
        seconds >= 0 && seconds <= 59; 
}


/* tasks */ 

/* clock_task: clock task */ 
void *clock_thread(void *unused) 
{
    /* local copies of the current time */ 
    int hours, minutes, seconds; 

    /* infinite loop */ 
    while (1)
    {
        /* read and display current time */ 
        get_time(&hours, &minutes, &seconds); 
        display_time(hours, minutes, seconds); 

	if(Hours == AHours && Minutes == AMinutes && Seconds == ASeconds)
	{
	    Active = 1;
	    sem_post(&start_alarm);	
	}
        /* increment time */ 
        increment_time();
        /* wait ½ second */ 
        usleep(500000);
    }
}

/* set_task: reads messages from the user interface, and 
   sets the clock, or exits the program */ 
void * set_thread(void *unused)
{
    /* message array */ 
    char message[SI_UI_MAX_MESSAGE_SIZE]; 

    /* time read from user interface */ 
    int hours, minutes, seconds, ahours, aminutes, aseconds; 

    /* set GUI size */ 
    si_ui_set_size(400, 200); 

    while(1)
    {
        /* read a message */ 
        si_ui_receive(message); 
        /* check if it is a set message */ 
        if (strncmp(message, "set", 3) == 0)
        {
            time_from_set_message(message, &hours, &minutes, &seconds); 
            if (time_ok(hours, minutes, seconds))
            {
                set_time(hours, minutes, seconds); 
            }
            else
            {
                si_ui_show_error("Illegal value for hours, minutes or seconds"); 
            }
        }
	else if (strncmp(message, "alarm" , 5) == 0)
	{
	    time_from_set_alarm(message, &ahours, &aminutes, &aseconds); 
            if (time_ok(ahours, aminutes, aseconds))
            {
                set_alarm(ahours, aminutes, aseconds); 
		display_alarm_time(AHours, AMinutes, ASeconds);
            }
            else
            {
                si_ui_show_error("Illegal value for hours, minutes or seconds"); 
            }
	}
	else if (strncmp(message, "reset" , 5) == 0)
	{
	    Active = 0;
	    erase_alarm_time();
	    erase_alarm_text();
	    AHours = 0;
	    AMinutes = 0;
	    ASeconds = 0;
	}
        /* check if it is an exit message */ 
        else if (strcmp(message, "exit") == 0)
        {
            exit(0); 
        }
        /* not a legal message */ 
        else 
        {
            si_ui_show_error("unexpected message type"); 
        }
    }
}

/* clock_task: clock task */ 
void *alarm_thread(void *unused) 
{
    /* infinite loop */ 
    while (1)
    {
	sem_wait(&start_alarm);
	while(Active)
	{
      	display_alarm_text();
	usleep(1500000);
	}
    }
}

/* main */ 
int main(void)
{
    /* initialise UI channel */ 
    si_ui_init(); 

    /* initialise variables */         
    init_clock(); 
    display_init();

    /* create tasks */ 
    pthread_t clock_thread_handle;
    pthread_t set_thread_handle;
    pthread_t alarm_thread_handle;

    pthread_create(&clock_thread_handle, NULL, clock_thread, 0);
    pthread_create(&set_thread_handle, NULL, set_thread, 0);
    pthread_create(&alarm_thread_handle, NULL, alarm_thread, 0);

    pthread_join(clock_thread_handle, NULL);
    pthread_join(set_thread_handle, NULL);
    pthread_join(alarm_thread_handle, NULL);
    /* will never be here! */ 
    return 0; 
}

