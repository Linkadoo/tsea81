/* A program with three tasks
   One task is modifying common data, and 
   one task is reading and displaying data 
   A third task is used for closing the program 
*/ 

#include <pthread.h>

/* standard library includes */ 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "si_ui.h"

/* stack size */ 
#define STACK_SIZE 5000 

/* stack declarations */ 


/* common variables */

/* integer number to be displayed */
int Number;

/* variable indicating if Number is even */
int Even_Number;

pthread_mutex_t Mutex;
/* long_calculation: function simulating a general floating point
   calculation with significant processing time */
void long_calculation(void) 
{
    int i;
    double x = 0.0;

    for (i = 0; i < 1000; i++) 
    {
        x+= 0.0001;
    }
}

/* ---- functions with access to common variables Number and Even_Number ---- */ 

/* increment_number: increments Number by adding 
   inc_value to Number, and updates Even_Number */ 
void increment_number(int inc_value) 
{
    pthread_mutex_lock(&Mutex);
    Number += inc_value;
    long_calculation();
    if (Number % 2 == 0) 
    {
        Even_Number = 1;
    } 
    else 
    {
        Even_Number = 0;
    }
     pthread_mutex_unlock(&Mutex);
}

/* decrement_number: decrements Number with dec_value, 
   and updates Even_Number */
void decrement_number(int dec_value) 
{
    pthread_mutex_lock(&Mutex);
    Number -= dec_value;
    if (Number % 2 == 0) 
    {
        Even_Number = 1;
    } 
    else 
    {
        Even_Number = 0;
    }
    pthread_mutex_unlock(&Mutex);
}

/* set_number: sets Number and updates Even_Number */
void set_number(int new_number) 
{
    pthread_mutex_lock(&Mutex);
    Number = new_number;
    if (Number % 2 == 0) 
    {
        Even_Number = 1;
    } 
    else 
    {
        Even_Number = 0;
    }
    pthread_mutex_unlock(&Mutex);
}

/* get_number: gets the value of Number, together with the 
   value of Even_Number, which indicates if Number is even or odd */ 
void get_number(
    int *number_value, int *even_number_value) 
{
    pthread_mutex_lock(&Mutex);
    *number_value = Number;
    *even_number_value = Even_Number;
    pthread_mutex_unlock(&Mutex);
}


/* ---- auxiliary functions ---- */ 

/* print_number: prints an integer number, together with information
   whether the number is odd or even */
void print_number(int number, int even_number) 
{
    /* value which should have the same value as the common variable
       Even_Number if everything is Ok */
    int check_even_number;

    /* string for the text to be printed */
    char write_str[40];

    /* conversion of number from integer to string */
    sprintf(write_str,"The number is: %d ",number);

    /* display information on odd or even number */
    if (even_number) 
    {
        strcat(write_str,"The number is EVEN");
    } 
    else 
    {
        strcat(write_str,"The number is ODD");
    }

    /* start drawing, i.e. prepare for writing to user interface */ 
    si_ui_draw_begin(); 

    /* display number information */ 
    si_ui_draw_string(write_str, 70, 50);

    /* check if the number really is odd or even */
    if (number % 2 == 0) 
    {
        check_even_number = 1;
    } 
    else 
    {
        check_even_number = 0;
    }

    /* display error message if necessary */
    if (even_number != check_even_number) 
    {
        if (check_even_number) 
        {
            sprintf(write_str,"ERROR: The number is really EVEN");
        } 
        else 
        {
            sprintf(write_str,"ERROR: The number is really ODD");
        } 
    }
    else
    {
        sprintf(write_str, "odd/even check without errors"); 
    }

    /*  display odd/even information */ 
    si_ui_draw_string(write_str, 70, 70);

    /* finish drawing */ 
    si_ui_draw_end(); 

}


/* ---- tasks ---- */ 

/* display_thread: display task */ 
void * display_thread(void * arg) 
{
    /* current value of the integer number */
    int number;
    /* current noted information whether the integer number is
       odd or even (a copy of the common variable Even_Number) */
    int even_number;

    /* set GUI size */ 
    si_ui_set_size(400, 200); 

    /* set initial value for the integer number */
    set_number(100); 

    while (1) 
      {
       
        /* get the number and the information regarding odd or even */
	get_number(&number, &even_number);

	/* print the number */
	
        print_number(number, even_number);
        
        /* wait for a while */ 
	usleep((1000 + rand() % 1000)*1000);
      }
}

/* change_thread: modifies the  integer number, and notes if the modified number
   is odd or even by setting the value of the common variable Even_Number */
void *change_thread(void *arg) 
{
    /* increment value */
    const int inc_value = 1;

    int i; 

    while(1) 
    {

        /* change the number */
        for (i = 0; i < 1000; i++) 
        {
           increment_number(inc_value);
        }
        for (i = 0; i < 1000; i++) 
        {
           decrement_number(inc_value);
        }
    }
}

/* exit_thread: used for closing the program */ 
void * exit_thread(void *arg)
{
    /* message array */ 
    char message[SI_UI_MAX_MESSAGE_SIZE]; 

    while(1)
    {
        /* read a message */ 
        si_ui_receive(message); 
        /* check if it is an exit message */ 
        if (strcmp(message, "exit") == 0)
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

/* ---- main ---- */ 

int main(void)
{
    /* initialise UI channel */ 
    si_ui_init(); 

    /* create threads */ 

    pthread_t disp_thread_handle;
    pthread_t change_thread_handle;
    pthread_t exit_thread_handle;
    
    pthread_mutex_init(&Mutex, NULL);
    pthread_create(&disp_thread_handle, NULL, display_thread, 0);
    pthread_create(&change_thread_handle, NULL, change_thread, 0);
    pthread_create(&exit_thread_handle, NULL, exit_thread, 0);
    
    pthread_join(disp_thread_handle, NULL);
    pthread_join(change_thread_handle, NULL);
    pthread_join(exit_thread_handle, NULL);
    pthread_mutex_destroy(&Mutex);

    return 0; 
}


