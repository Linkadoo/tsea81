all: set_clock 

set_clock: set_clock.c si_comm.c si_ui.c si_ui.h display.h display.c si_comm.h
	gcc set_clock.c si_comm.c si_ui.c display.c -g  -o set_clock -Wall -lpthread  


clean:
	rm -f set_clock *~ *.o
