/*
 * state_machine.h
 *
 * Created: 17.08.2022 13:19:48
 *  Author: domis
 */ 


#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	s_error,
	s_init,
	s_idle_prologue,
	s_idle,
	s_time_in,
	s_export_log_prologue,
	s_export_log,
	s_value_in,
	s_work_watch,
	s_work_check,
	s_fini
} _state;

typedef enum
{
	e_error,
	e_nothing,
	e_timeout,
	e_lcd_refresh,
	e_button_sta,
	e_button_stb,
	e_button_encb,
	e_button_encb_long
} _event;

typedef struct
{
	_state state;
	void * data_ptr;
} _state_flow;

typedef _state_flow (*f_sm_handler)(void *data_ptr);

typedef struct
{
	_state state;
	f_sm_handler handler;
} _state_machine;

extern _state_flow g_Next_state;
extern volatile bool g_event_flag;

_state f_sm_get_current_state();
_event f_sm_read_event();

inline void f_sm_report_event()
{
	g_event_flag = true;
}





#endif /* STATE_MACHINE_H_ */