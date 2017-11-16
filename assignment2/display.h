#ifndef DISPLAY_H
#define DISPLAY_H
#include "si_ui.h"
/* display_init: initialises the display module */
void display_init(void); 

/* display_time: displays time, defined by 
   hours, minutes and seconds */ 
void display_time(int hours, int minutes, int seconds);

/* display_alarm_time: displays alarm time, defined by 
   hours, minutes and seconds */ 
void display_alarm_time(int hours, int minutes, int seconds); 

/* erase_alarm_time: erases the alarm time */ 
void erase_alarm_time(void); 

/* display_alarm_text: displays an alarm text, together with a 
   number, which is incremented each time display_alarm_text 
   is called. The number is reset to zero when erase_alarm_text 
   is called */ 
void display_alarm_text(void); 

/* erase_alarm_text: sets the number in the alarm text to zero */ 
void erase_alarm_text(void); 

#endif

