CFLAGS=-O2 -g -Wall
LIBS=-lrt -lpthread
CFILES=lift.c main.c si_ui.c si_comm.c draw.c messages.c
HEADERS=lift.h si_ui.h si_comm.h draw.h messages.h
lift_messages: $(CFILES) $(HEADERS)
	gcc $(CFLAGS) $(CFILES) -o lift_messages $(LIBS) 

clean:
	rm -f lift_messages *~ *.o
