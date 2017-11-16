#ifndef _MESSAGES_H
#define _MESSAGES_H
void message_send(char *msg, unsigned int len, unsigned int queueid, unsigned int priority);
ssize_t message_receive(char *msg, unsigned int max_len, unsigned int queueid);
void message_init(void);
#endif
