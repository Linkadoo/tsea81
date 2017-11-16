CFLAGS=-O2 -g -Wall
LIBS=-lrt -lpthread
CFILES=lift.c main.c si_ui.c si_comm.c draw.c debug.c
HEADERS=lift.h si_ui.h si_comm.h draw.h debug.h
lift_pthreads: $(CFILES) $(HEADERS)
	gcc $(CFLAGS) $(CFILES) -o lift_pthreads $(LIBS)

clean:
	rm -f lift_pthreads *~ *.o
