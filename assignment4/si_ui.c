#include "si_ui.h"

#include <pthread.h>

/* unistd is needed for usleep and sleep */ 
#include <unistd.h>

#include "si_comm.h"
#define PTHREADS

#ifndef PTHREADS
#include "console.h"
#endif

#if defined BUILD_X86_HOST || defined BUILD_X86_64_HOST || defined PTHREADS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#endif

#define SI_UI_MESSAGE_BUFFER_SIZE 10000

/* buffer with messages to send */ 
static char Message_Buffer[SI_UI_MESSAGE_BUFFER_SIZE]; 

/* semaphore to ensure only one task accesses the communication link, 
   during sending and receiving */ 
static pthread_mutex_t Si_Ui_Mutex; 

/* position where to start writing the next message into the buffer */ 
static int Message_Pos; 

/* string where temporary strings are stored before sending */ 
static char Message_String[SI_UI_MAX_MESSAGE_SIZE]; 

/* command delimiter, which must be used also in the GUI client */ 
static char Command_Delim; 

/* si_ui_init: initialise communication */ 
void si_ui_init(void)
{
    Command_Delim = ';'; 
    /* open communication channel */ 
    si_comm_open(); 
    /* initialise message buffer semaphore */ 
    pthread_mutex_init(&Si_Ui_Mutex, NULL); 
    /* start writing at the beginning of the message buffer */
    Message_Pos = 0; 
}

static void remove_trailing_command_delim(char buffer [])
{
    int len; 
    len = strlen(buffer); 
    if (buffer[len-1] == Command_Delim)
    {
        buffer[len-1] = '\0'; 
    }
}

/* send_buffer: sends the contents of the message buffer 
   NOTE: it is assumed that the buffer is reserved when this function 
   is called */ 
static void send_buffer(void)
{
    int si_comm_return_value; 

    int n_tries; 
    const int max_n_tries = 10000; 
    const int delay_ms_between_tries = 100; 

    remove_trailing_command_delim(Message_Buffer); 

    n_tries = 0; 

    do
    {
        /* write message buffer contents to communication channel */ 
        // printf(Message_Buffer); 

        si_comm_return_value = si_comm_write(Message_Buffer); 

        n_tries++; 

        /* wait and let other tasks try if writing is not ok */ 
        if (si_comm_return_value != SI_COMM_OK)
        {
            /* release buffer, wait a while, and then reserve buffer again */ 
            pthread_mutex_unlock(&Si_Ui_Mutex); 
            usleep(delay_ms_between_tries * 1000); 
            pthread_mutex_lock(&Si_Ui_Mutex); 
        }
    } while (n_tries < max_n_tries && si_comm_return_value != SI_COMM_OK); 

    if (si_comm_return_value != SI_COMM_OK)
    {
        printf("si_ui: NOTE: communication problem - number of write tries: %d", n_tries); 
    }
}

/* append_to_buffer: appends message to the message buffer */ 
static void append_to_buffer(char message[])
{
#if defined BUILD_X86_HOST || defined BUILD_X86_64_HOST || defined PTHREADS
    size_t i; 
#endif
#ifdef BUILD_ARM_BB
    int i; 
#endif
    int buffer_full = 0; 

    for (i = 0; i < strlen(message) && !buffer_full; i++)
    {
        Message_Buffer[Message_Pos] = message[i]; 
        Message_Pos++; 
        /* check if we are overflowing the buffer */ 
        if (Message_Pos >= SI_UI_MESSAGE_BUFFER_SIZE - 1)
        {
            printf("NOTE: message buffer OVERFLOW\n"); 
            /* add string terminator */    
            Message_Buffer[Message_Pos] = '\0'; 
            buffer_full = 1; 
            /* reset buffer, so that next write is from the buffer start position */ 
            Message_Pos = 0; 
        }
    }

    if (!buffer_full)
    {
        /* add command delimiter */ 
        Message_Buffer[Message_Pos] = Command_Delim; 
        Message_Pos++; 
        /* add string terminator, in case no more strings are addded to the buffer */ 
        Message_Buffer[Message_Pos] = '\0'; 
        // printf("MB: %s\n", Message_Buffer); 
    }
}

void si_ui_draw_begin(void)
{
    pthread_mutex_lock(&Si_Ui_Mutex); 

    /* start from the beginning */ 
    Message_Pos = 0; 
        
    append_to_buffer("draw_begin"); 

    pthread_mutex_unlock(&Si_Ui_Mutex); 
}

void si_ui_draw_end(void)
{
    pthread_mutex_lock(&Si_Ui_Mutex); 

    append_to_buffer("draw_end"); 

    /* add a string containing only a newline, to keep the Java client happy, 
       NOTE: this may need to change, if problems occur e.g. when using a mix of 
       Windows and Linux */ 
    append_to_buffer("\n"); 

    /* send the buffer */ 
    send_buffer(); 

    pthread_mutex_unlock(&Si_Ui_Mutex); 
}

void si_ui_draw_string(char string[], int x_coord, int y_coord)
{
    pthread_mutex_lock(&Si_Ui_Mutex); 

    sprintf(Message_String, "draw_string:%08X:%08X:%s", x_coord, y_coord, string); 

    append_to_buffer(Message_String); 

    pthread_mutex_unlock(&Si_Ui_Mutex); 
}
     
void si_ui_draw_image(char image_name[], int x_coord, int y_coord)
{
    pthread_mutex_lock(&Si_Ui_Mutex); 

    sprintf(Message_String, "draw_image:%s:%08X:%08X", image_name, x_coord, y_coord); 

    append_to_buffer(Message_String); 

    pthread_mutex_unlock(&Si_Ui_Mutex); 
}


void si_ui_show_error(char message[])
{
    si_ui_draw_begin(); 

    pthread_mutex_lock(&Si_Ui_Mutex); 

    sprintf(Message_String, "show_error:%s", message); 

    append_to_buffer(Message_String); 

    pthread_mutex_unlock(&Si_Ui_Mutex); 

    si_ui_draw_end(); 
}

void si_ui_set_size(int x_size, int y_size)
{
    si_ui_draw_begin(); 

    pthread_mutex_lock(&Si_Ui_Mutex); 

    sprintf(Message_String, "set_size:%08X:%08X", x_size, y_size); 

    append_to_buffer(Message_String); 

    pthread_mutex_unlock(&Si_Ui_Mutex); 

    si_ui_draw_end(); 
}

void si_ui_receive(char message[])
{
    int si_comm_return_value; 

    int n_tries; 

    const int max_n_tries = 10000; 
    const int delay_ms_between_tries = 250; 

    n_tries = 0; 

    pthread_mutex_lock(&Si_Ui_Mutex); 

    do
    {
        si_comm_return_value = si_comm_read(message, SI_UI_MAX_MESSAGE_SIZE); 

        n_tries++; 

        /* wait and let other tasks try if reading is not ok */ 
        if (si_comm_return_value != SI_COMM_OK)
        {
            pthread_mutex_unlock(&Si_Ui_Mutex); 
            usleep(delay_ms_between_tries * 1000); 
            pthread_mutex_lock(&Si_Ui_Mutex); 
        }
    } while (n_tries < max_n_tries && si_comm_return_value != SI_COMM_OK); 

    if (si_comm_return_value != SI_COMM_OK)
    {
        printf("si_ui: NOTE: communication problem - number of read tries %d", n_tries); 
    }

    pthread_mutex_unlock(&Si_Ui_Mutex); 
}

void si_ui_close(void)
{
    si_comm_close(); 
}
