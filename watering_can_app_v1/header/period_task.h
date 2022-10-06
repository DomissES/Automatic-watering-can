/*
 * period_task.h
 *
 * Created: 17.08.2022 13:42:15
 *  Author: domis
 */ 


#ifndef PERIOD_TASK_H_
#define PERIOD_TASK_H_

#include <stdbool.h>
#include "circ_buffer.h"

#define TASK_TIMER_ENABLE TCCR2 |= _BV(CS22)
#define TASK_TIMER_DISABLE TCCR2 &= ~_BV(CS22)

typedef bool (*_task_handler)();

extern _circ_buffer Task_buffer;

#define f_task_request(task) f_cb_enqueue(&Task_buffer, task)

void f_init_task_timer();

#endif /* PERIOD_TASK_H_ */