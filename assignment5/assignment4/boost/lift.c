#include "lift.h"

/* Simple_OS include */ 
#include <pthread.h>

/* drawing module */ 
#include "draw.h"

/* standard includes */ 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* --- monitor data type for lift and operations for create and delete START --- */

/* lift_create: creates and initialises a variable of type lift_type */
lift_type lift_create(void) 
{
    /* the lift to be initialised */
    lift_type lift;

    /* floor counter */ 
    int floor; 

    /* loop counter */
    int i;
    
    /* allocate memory */
    lift = (lift_type) malloc(sizeof(lift_data_type));

    /* initialise variables */

    /* initialise floor */
    lift->floor = 0; 
    
    /* set direction of lift travel to up */
    lift->up = 1;

    /* the lift is not moving */ 
    lift->moving = 0; 

    lift->leaving = 0;
    lift->entering=0;
    /* initialise person information */
    for (floor = 0; floor < N_FLOORS; floor++)
    {
        for (i = 0; i < MAX_N_PERSONS; i++)
        {
            lift->persons_to_enter[floor][i].id = NO_ID; 
            lift->persons_to_enter[floor][i].to_floor = NO_FLOOR; 
        }
    }

    /* initialise passenger information */
    for (i = 0; i < MAX_N_PASSENGERS; i++) 
    {
        lift->passengers_in_lift[i].id = NO_ID; 
        lift->passengers_in_lift[i].to_floor = NO_FLOOR;
    }

   /* initialise travelcount information */
    for (i = 0; i < MAX_N_PERSONS; i++) 
    {
      lift->travel_count[i] = 0;
    }

    return lift;
}

/* lift_delete: deallocates memory for lift */
void lift_delete(lift_type lift)
{
    free(lift);
}


/* --- monitor data type for lift and operations for create and delete END --- */


/* --- functions related to lift task START --- */

/* MONITOR function lift_next_floor: computes the floor to which the lift 
   shall travel. The parameter *change_direction indicates if the direction 
   shall be changed */
void lift_next_floor(lift_type lift, int *next_floor, int *change_direction)
{
  if(lift->up != 0){
    (*next_floor)++;
  }else{
   (*next_floor)--;
  }
  if(*next_floor  == N_FLOORS-1 || *next_floor  == 0){
    *change_direction = 1;
  }else{
	*change_direction = 0;
  }
}

void lift_move(lift_type lift, int next_floor, int change_direction)
{

    /* the lift has arrived at next_floor */ 
    lift->floor = next_floor; 

    /* check if direction shall be changed */ 
    if (change_direction)
    {
        lift->up = !lift->up; 
    }
}

/* --- functions related to person task END --- */
