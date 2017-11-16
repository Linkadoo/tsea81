#ifndef SI_COMM_H
#define SI_COMM_H

#define SI_COMM_OK 0

/* si_comm_open: opens the communication */ 
void si_comm_open(void); 

/* si_comm_read: reads a message, and stores it in 
   message_data as a null-terminated string. 
   Returns SI_COMM_OK if reading was ok. */  
int si_comm_read(char message_data[], int message_data_size); 

/* si_comm_write: writes a message, defined as a 
   null-terminated string, stored in message_data. 
   Returns SI_COMM_OK if writing was ok. */ 
int si_comm_write(const char message_data[]); 

/* si_comm_close: closes the communication */ 
void si_comm_close(void); 

#endif 
