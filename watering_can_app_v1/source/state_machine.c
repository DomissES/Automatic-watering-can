/*
 * state_machine.c
 *
 * Created: 17.08.2022 13:54:18
 *  Author: domis
 */ 


#include "state_machine.h"
#include "period_task.h"

#include "hmi.h"
#include "system.h"

#include "uart_service.h"

_state_flow g_Next_state;

volatile bool g_event_flag;


_state f_sm_get_current_state()
{
	return g_Next_state.state;
}

_event f_sm_read_event()
{
	_event what_happened;
	g_event_flag = false;
	_error error = f_sys_check_last_error();
	
	if(error.status) what_happened = e_error;
	else if(f_hmi_read_button(b_sta)) what_happened = e_button_sta;
	else if(f_hmi_read_button(b_stb)) what_happened = e_button_stb;
	else if(f_hmi_read_button(b_encb))
	{
		if(f_hmi_get_pressed_seconds(b_encb) >= 3) what_happened = e_button_encb_long;
		else what_happened = e_button_encb;
	}
	else what_happened = e_nothing;
	
	return what_happened;
}
