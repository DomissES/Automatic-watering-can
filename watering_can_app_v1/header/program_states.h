/*
 * program_states.h
 *
 * Created: 17.08.2022 14:14:29
 *  Author: domis
 */ 


#ifndef PROGRAM_STATES_H_
#define PROGRAM_STATES_H_

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "state_machine.h"

extern const __flash _state_machine g_Program_states[];

typedef struct
{
	uint8_t hg_set;
	uint8_t time_on_set;
	uint8_t time_check_set;
	uint8_t timer_page_counter;
	uint8_t timer_lock_counter;
}_ps_work_data;


_state_flow ps_error();
_state_flow ps_init();
_state_flow ps_idle_prologue();
_state_flow ps_idle();
_state_flow ps_time_in();
_state_flow ps_export_log_prologue();
_state_flow ps_export_log();
_state_flow ps_value_in();
_state_flow ps_work_watch(_ps_work_data *Parameter);
_state_flow ps_work_check(_ps_work_data *Parameter);
_state_flow ps_fini();



#endif /* PROGRAM_STATES_H_ */