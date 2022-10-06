/*
 * circ_buffer.c
 *
 * Created: 17.08.2022 18:32:25
 *  Author: domis
 */ 

#include <stdlib.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "circ_buffer.h"
#include "memory.h"
#include "system.h"


static inline bool f_cb_is_full(_circ_buffer *cb)
{
	return (cb->head == ((cb->tail + 1) % MAX_BUFFER_SIZE));
}
static inline bool f_cb_is_empty(_circ_buffer *cb)
{
	return (cb->head == cb->tail);
}

uint8_t f_cb_check_size(_circ_buffer *cb)
{
	uint8_t size;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(cb->tail >= cb->head) size = cb->tail - cb->head + 1;
		else size = MAX_BUFFER_SIZE - cb->head - cb->tail;
	}
	return size;
}

bool f_cb_enqueue(_circ_buffer *cb, void *element)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (f_cb_is_full(cb)) return false;
		cb->elements[cb->tail] = element;
		cb->tail = (cb->tail+1) % MAX_BUFFER_SIZE;
	}
	return true;
}

void *f_cb_dequeue(_circ_buffer *cb)
{
	void *element;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(f_cb_is_empty(cb)) return NULL;
		element = cb->elements[cb->head];
		cb->head = (cb->head+1) % MAX_BUFFER_SIZE;
	}
	return element;
}

bool f_cb_try_enqueue(_circ_buffer *cb, void *element)
{
	bool isOk;
	uint8_t tries=0;
	
	for(tries = 0; tries < ENQEUEUE_TRIES; tries++)
	{
		isOk = f_cb_enqueue(cb, element);
		if(isOk) break;
		_delay_ms(5);
	}
	
	if(tries == ENQEUEUE_TRIES)
	{
		free_mine(element);
		f_sys_report_error(err_buf_ovf, 1);
	}
	return isOk;
}