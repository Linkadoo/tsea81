
#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdlib.h>
#include <unistd.h>


#define MAX_ID 400
static mqd_t queue_map[MAX_ID];

// Ensure all message queues are removed before we start so that we don't get any stale messages.
void message_init(void)
{
	int i;
	for(i=0; i < MAX_ID; i++){
		char *name;
		if(asprintf(&name, "/liftqueue-%u-%u", getuid(), i) == -1){
			fprintf(stderr, "Could not format queue name\n");
			exit(1);
		}
		mq_unlink(name);
		free(name);
	}
}

static mqd_t open_or_create_queue(unsigned int id)
{
	char *name;
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = 1024;
	attr.mq_curmsgs = 0;
	// Include the user id in the name so we are guaranteed to be able to
	// open the queue again and avoid a situation where a queue is opened by
	// another student and not deleted before logging off.
	if(asprintf(&name, "/liftqueue-%u-%u", getuid(), id) == -1){
		fprintf(stderr,"Could not format queue name\n");
		exit(1);
	}
	mqd_t queue = mq_open(name, O_RDWR | O_CREAT, 0600, &attr);
	free(name);
	if(queue == -1){
		perror("mq_open");
		exit(1);
	}
	return queue;
}

static void check_queue(unsigned int queueid)
{
	if(queueid >= MAX_ID){
		fprintf(stderr,"Fatal Internal error: queueid %u too large\n", queueid);
		exit(1);
	}
	if(queue_map[queueid] == 0){
		queue_map[queueid] = open_or_create_queue(queueid);
	}
}

void message_send(char *msg, unsigned int len, unsigned int queueid, unsigned int priority)
{
	check_queue(queueid);
	mq_send(queue_map[queueid], msg, len, priority);
}


ssize_t message_receive(char *msg, unsigned int max_len, unsigned int queueid)
{
	check_queue(queueid);
	ssize_t length = mq_receive(queue_map[queueid], msg, max_len, NULL);
	if(length == -1){
		perror("mq_receive");
		fprintf(stderr,"    pid for mq_receive: %d, max_len is %d, queueid is %d\n", getpid(), max_len, queueid);
	}
	return length;
}
