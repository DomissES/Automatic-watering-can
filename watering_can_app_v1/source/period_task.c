/*
 * period_task.c
 *
 * Created: 17.08.2022 14:51:59
 *  Author: domis
 */ 

#include <stdint.h>
#include <stdlib.h>
#include <util/atomic.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "period_task.h"
#include "circ_buffer.h"
#include "defines.h"

_circ_buffer Task_buffer;

static inline bool f_task_execute() __attribute__((always_inline));

ISR(TIMER2_COMP_vect, ISR_NOBLOCK)
{
	f_task_execute();
}

void f_init_task_timer()
{
	OCR2 = 32;
	TCCR2 |= _BV(WGM21);
	TIMSK |= _BV(OCIE2);
}

static inline void f_task_slowdown_timer()
{
	OCR2 ++;
	if(OCR2 == 255) OCR2 = 245;
}

static inline void f_task_speedup_timer()
{
	OCR2 /= 2;
	if(OCR2 <= 3) OCR2 = 4;
}

static inline bool f_task_execute()
{
	bool isOk;
	_task_handler task = f_cb_dequeue(&Task_buffer);
	
	if(task == NULL)
	{
		f_task_slowdown_timer();
		SET(PORT, LED_RDM);
		isOk = false;
	}
	else
	{
		CLR(PORT, LED_RDM);
		isOk = (*task)();
		
		if(f_cb_check_size(&Task_buffer) >= (3*MAX_BUFFER_SIZE/4)) f_task_speedup_timer();
	}
	
	return isOk;
}