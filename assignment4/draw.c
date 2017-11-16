#include "draw.h"

#include "lift.h"

#include <pthread.h>

#include "si_ui.h"

#include <stdio.h>

#define LEVEL_OFFSET 75

static char message[SI_UI_MAX_MESSAGE_SIZE]; 

static int xBuilding = 50; 
static int yBuilding = 100; 

static int xLift = 355; 
static int yLift[N_FLOORS] = {180, 268, 356, 444, 532}; 

static int cxGubbe = 41; 
static int cyGubbe = 73; 
static int delta_y_Gubbe = 0; 

static int info_x = 160;
static int info_y = 20;

static int id_x_offset = 7; 
static int id_y_offset = -10; 

void draw_lift(lift_type lift)
{
    int floor, i; 

    si_ui_draw_begin(); 

    si_ui_draw_string(
        "Lift Simulation (send \"new\" to create new passengers)",
        info_x, info_y); 

    si_ui_draw_image("building", xBuilding, yBuilding); 

    si_ui_draw_image("lift", xLift, yLift[lift->floor]); 

    /* draw lift passengers */ 
    for (i = 0; i < MAX_N_PASSENGERS; i++)
    {
        if (lift->passengers_in_lift[i].id != NO_ID)
        {
            si_ui_draw_image("gubbe", 
                           xLift + i*cxGubbe, yLift[lift->floor] - cyGubbe + delta_y_Gubbe); 

            sprintf(message, "%d   %d", 
                    lift->passengers_in_lift[i].id, 
                    lift->passengers_in_lift[i].to_floor); 
            si_ui_draw_string(message, xLift + i*cxGubbe + id_x_offset, 
                            yLift[lift->floor] - cyGubbe + delta_y_Gubbe + id_y_offset);
        }
    }

    /* draw waiting persons */ 
    for (floor = 0; floor < N_FLOORS; floor++)
    {
        sprintf(message, " %d ", floor); 
        si_ui_draw_string(message, xLift - LEVEL_OFFSET/2 + 5, yLift[floor] - 3); 

        for (i = 0; i < MAX_N_PERSONS; i++)
        {
            if (lift->persons_to_enter[floor][i].id != NO_ID)
            {
                si_ui_draw_image("gubbe", xLift - LEVEL_OFFSET - i*(cxGubbe + 2), 
                               yLift[floor] - cyGubbe); 
                sprintf(message, "%d", lift->persons_to_enter[floor][i].id); 
                si_ui_draw_string(message, xLift  - LEVEL_OFFSET - i*(cxGubbe + 2) - 3, 
                                yLift[floor] - cyGubbe + 5);

            }
        }
    }

    si_ui_draw_end(); 
}

