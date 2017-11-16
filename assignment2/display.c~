#include "display.h"

#include <pthread.h>

#include <stdio.h>

#define TIME_X 140
#define TIME_Y 40

#define ALARM_TIME_X 140
#define ALARM_TIME_Y 60

#define ALARM_TEXT_X 140
#define ALARM_TEXT_Y 80

/* data structure for display information */ 
typedef struct 
{
    /* time */ 
    int hours, minutes, seconds; 
    /* alarm time */ 
    int alarm_hours, alarm_minutes, alarm_seconds; 
    /* flag to indicate that the alarm shall be displayed */ 
    int alarm_display_on; 
    /* current alarm text number */ 
    int alarm_text_number; 
    /* flag to indicate the the alarm text number shall be displayed */ 
    int alarm_text_display_on; 

} display_data_type; 

static display_data_type Display_Data; 

/* semaphore for mutual exclusion */ 
static pthread_mutex_t Display_Mutex; 

void display_init(void)
{
    pthread_mutex_init(&Display_Mutex, NULL); 

    Display_Data.hours = 0; 
    Display_Data.minutes = 0; 
    Display_Data.seconds = 0; 

    Display_Data.alarm_hours = 0; 
    Display_Data.alarm_minutes = 0; 
    Display_Data.alarm_seconds = 0; 

    Display_Data.alarm_display_on = 0; 

    Display_Data.alarm_text_number = 0; 

    Display_Data.alarm_text_display_on = 0; 
}

/* make_display_message: create string for printout, given a time expressed 
   in hours, minutes and seconds */ 
static void make_display_message(char display_message[], int hours, int minutes, int seconds) 
{
    display_message[0] = hours / 10 + '0'; 
    display_message[1] = hours % 10 + '0'; 
    display_message[2] = ':'; 
    display_message[3] = minutes / 10 + '0'; 
    display_message[4] = minutes % 10 + '0'; 
    display_message[5] = ':'; 
    display_message[6] = seconds / 10 + '0'; 
    display_message[7] = seconds % 10 + '0'; 
    display_message[8] = '\0'; 
}

/* draw: performs drawing, according to information in data
   structure d */ 
static void draw(display_data_type d)
{
    char display_message[20]; 

    char draw_message[100]; 

    /* start writing to user interface */ 
    si_ui_draw_begin(); 

    /* convert hours, minutes and seconds to display format */ 
    make_display_message(display_message, d.hours, d.minutes, d.seconds); 

    sprintf(draw_message, "Time: %s", display_message); 
    si_ui_draw_string(draw_message, TIME_X, TIME_Y); 

    if (d.alarm_display_on)
    {
        /* convert alarm hours, minutes and seconds to display format */ 
        make_display_message(display_message, d.alarm_hours, d.alarm_minutes, d.alarm_seconds); 

        sprintf(draw_message, "Alarm time: %s", display_message); 
        si_ui_draw_string(draw_message, ALARM_TIME_X, ALARM_TIME_Y); 
    }
        
    if (d.alarm_text_display_on)
    {
        sprintf(draw_message, "--- ALARM nr %d ---", d.alarm_text_number); 
        si_ui_draw_string(draw_message, ALARM_TEXT_X, ALARM_TEXT_Y); 
    }

    /* finish writing to user interface */ 
    si_ui_draw_end(); 
}

void display_time(int hours, int minutes, int seconds)
{
    display_data_type draw_display_data; 

    pthread_mutex_lock(&Display_Mutex); 

    Display_Data.hours = hours; 
    Display_Data.minutes = minutes; 
    Display_Data.seconds = seconds; 

    draw_display_data = Display_Data; 

    pthread_mutex_unlock(&Display_Mutex); 

    draw(draw_display_data); 
}

void display_alarm_time(int hours, int minutes, int seconds)
{
    display_data_type draw_display_data; 

    pthread_mutex_lock(&Display_Mutex); 

    Display_Data.alarm_hours = hours; 
    Display_Data.alarm_minutes = minutes; 
    Display_Data.alarm_seconds = seconds; 

    Display_Data.alarm_display_on = 1; 

    draw_display_data = Display_Data; 

    pthread_mutex_unlock(&Display_Mutex); 

    draw(draw_display_data); 
}

void erase_alarm_time(void) 
{
    display_data_type draw_display_data; 

    pthread_mutex_lock(&Display_Mutex); 

    Display_Data.alarm_display_on = 0; 

    draw_display_data = Display_Data; 

    pthread_mutex_unlock(&Display_Mutex); 

    draw(draw_display_data); 
}

void display_alarm_text(void)
{
    display_data_type draw_display_data; 

    pthread_mutex_lock(&Display_Mutex); 

    Display_Data.alarm_text_display_on = 1; 
    Display_Data.alarm_text_number++; 

    draw_display_data = Display_Data; 

    pthread_mutex_unlock(&Display_Mutex); 

    draw(draw_display_data); 
}

void erase_alarm_text(void)
{
    display_data_type draw_display_data; 

    pthread_mutex_lock(&Display_Mutex); 

    Display_Data.alarm_text_display_on = 0; 
    Display_Data.alarm_text_number = 0; 

    draw_display_data = Display_Data; 

    pthread_mutex_unlock(&Display_Mutex); 

    draw(draw_display_data); 
}

