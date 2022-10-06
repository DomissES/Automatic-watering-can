/*
 * watering_can_app_v1.c
 *
 * Created: 17.08.2022 13:16:20
 * Author : domis
 */ 

#include <stdlib.h>
#include "state_machine.h"
#include "program_states.h"
#include "uart_service.h"


int main(void)
{
    
	g_Next_state.state = s_init;
	
	
    while (1) 
    {
		if (g_Program_states[g_Next_state.state].handler != NULL)
			g_Next_state = (*g_Program_states[g_Next_state.state].handler)(g_Next_state.data_ptr);
		
	}
}

