/*
 * circ_buffer.h
 *
 * Created: 17.08.2022 18:32:13
 *  Author: domis
 */ 


#ifndef CIRC_BUFFER_H_
#define CIRC_BUFFER_H_

#include <stdbool.h>
#include <stdint.h>

#define ENQEUEUE_TRIES 3
#define MAX_BUFFER_SIZE 16

typedef volatile struct
{
	uint8_t head;
	uint8_t tail;
	void *elements[MAX_BUFFER_SIZE];
} _circ_buffer;

uint8_t f_cb_check_size(_circ_buffer *cb);
bool f_cb_enqueue(_circ_buffer *cb, void *element);
void *f_cb_dequeue(_circ_buffer *cb);
bool f_cb_try_enqueue(_circ_buffer *cb, void *element);


#endif /* CIRC_BUFFER_H_ */