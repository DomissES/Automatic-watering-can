/*
 * twi_hw.c
 *
 * Created: 11.09.2022 13:12:25
 *  Author: domis
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/twi.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <util/delay.h>
#include "twi_hw.h"
#include "system.h"
#include "circ_buffer.h"
#include "memory.h"
#include "period_task.h"


#define SEND_START		TWCR = _BV(TWINT) | _BV(TWIE) | _BV(TWSTA) | _BV(TWEN)
#define SEND_STOP		TWCR = _BV(TWINT) | _BV(TWIE) | _BV(TWSTO) | _BV(TWEN)
#define SEND_TRANSMIT	TWCR = _BV(TWINT) | _BV(TWIE) | _BV(TWEN)
#define SEND_ACK		TWCR = _BV(TWINT) | _BV(TWIE) | _BV(TWEN) | _BV(TWEA)
#define SEND_NACK		TWCR = _BV(TWINT) | _BV(TWIE) | _BV(TWEN)

#define TWI_BUS_SPEED 100000UL
#define MAX_TWI_BUFFER 8

typedef enum {
	twi_idle,
	twi_initializing,
	twi_master_transmitter,
	twi_master_receiver,
	twi_transmitted
	} _twi_state;

volatile struct {
	_twi_state state;
	uint8_t status;
	bool repeat_start;
} Twi;

typedef struct
{
	volatile bool ready:1;
	bool self_delete:	1;
	bool rw:			1;
	bool rep_start:		1;
	uint8_t:			0;
	uint8_t address;
	uint8_t length;
	char data[];
} _twi_data;

_circ_buffer Twi_buffer;

static inline void f_twi_transmit_data_interrupt() __attribute__((always_inline));

ISR(TWI_vect)
{
	f_twi_transmit_data_interrupt();
}

static inline void f_twi_send_address(_twi_data *element)//tu se sprawdzam czy mi git dziala 
{
	TWDR = element->address | element->rw;
	SEND_TRANSMIT;
}

static inline _twi_data* f_twi_transmission_handler(_twi_data *element)
{
	static uint8_t position;
	static uint8_t sla_nack_tries;
	
	switch(TW_STATUS)
	{
		case TW_REP_START:
	
		case TW_START:
			position = 0;
			f_twi_send_address(element);
			break;
		
		case TW_MT_SLA_ACK:
			sla_nack_tries = 0;
			Twi.state = twi_master_transmitter;
	
		case TW_MT_DATA_ACK:
			TWDR = element->data[position];
			position++;
			
			if(position >= element->length)
			{
				element->ready = true;
				if(element->self_delete) free_mine(element);
				Twi.state = twi_transmitted;
			}
			SEND_TRANSMIT;
			break;
					
			case TW_MR_SLA_ACK:
				sla_nack_tries = 0;
				Twi.state = twi_master_receiver;
				if((position + 1) >= element->length) SEND_NACK;
				else SEND_ACK;
				break;
			
			case TW_MR_DATA_ACK:
				element->data[position] = TWDR;
				position++;
				
				if((position + 1) >= element->length) SEND_NACK;
				else SEND_ACK;
				
				break;
		
			case TW_MR_DATA_NACK:
				if(element)
				{
					element->data[position] = TWDR;
					element->ready = true;
					Twi.state = twi_transmitted;
					break;
				}
				
				
			case TW_MR_SLA_NACK:
			case TW_MT_SLA_NACK:	
				if(sla_nack_tries < 3)
				{
					sla_nack_tries++;
					_delay_ms(5);
					SEND_START;
					break;
				}
			
			case TW_MT_DATA_NACK:
			case TW_MT_ARB_LOST:
				Twi.state = twi_idle;
				f_sys_report_error(err_twi_gen, 2);
				SEND_STOP;
			//TODO::przypadki inne ni¿ powy¿sze
	}
	return element;
}

static inline void f_twi_transmit_data_interrupt()
{
	static _twi_data *element;
	
	switch(Twi.state)
	{
		case twi_transmitted:
			element = f_cb_dequeue(&Twi_buffer);
			
			if(element == NULL)
			{
				Twi.state = twi_idle;
				SEND_STOP;
			}
			else if(element->rep_start == false)
			{
				f_task_request(&TWI_vect);
				Twi.state = twi_idle;
				SEND_STOP;
				break;
			}
		
		case twi_idle:
		
			if(element == NULL) element = f_cb_dequeue(&Twi_buffer);
			if(element)
			{
				Twi.state = twi_initializing;
				SEND_START;
			}
			break;
			
		case twi_initializing: //element must not be NULL		
		case twi_master_receiver:
		case twi_master_transmitter:
			element = f_twi_transmission_handler(element);
			
	}
}

static inline void f_twi_set_bus_speed(uint32_t speed)
{
	speed = (F_CPU/speed-16)/2;
	uint8_t prescaler = 0;
	while (speed > 255)
	{
		prescaler++;
		speed /= 4;
	}
	TWSR = (TWSR & (_BV(TWPS0) | _BV(TWPS1))) | prescaler;
	TWBR = speed;
}

void f_init_twi()
{
	f_twi_set_bus_speed(TWI_BUS_SPEED);
	TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
	
}

static inline _twi_data* f_twi_prepare_data( uint8_t address, const __memx void* data, uint8_t length, bool rep_start)
{
	_twi_data *element = malloc_mine(sizeof(_twi_data) + length);
	element->address = address;
	element->length = length;
	element->ready = 0;
	element->rep_start = rep_start;
	
	return element;
}

bool f_twi_send_data(uint8_t address, const __memx void* data, uint8_t length, bool rep_start)
{
	_twi_data *element = f_twi_prepare_data(address, data, length, rep_start);
	
	if(__builtin_avr_flash_segment(data)==255) memcpy((uint8_t*)&element->data, data, length);
	else memcpy_P((uint8_t*)&element->data, data, length);
	
	element->rw = 0;
	element->self_delete = true;
	
	bool isOk = f_cb_try_enqueue(&Twi_buffer, element);
	if(isOk)
	{
		if(Twi.state == twi_idle) TWI_vect();
	}
	else f_sys_report_error(err_twi_ovf, 1);
	
	
	return isOk;
}

bool f_twi_read_data(uint8_t address, void* data, uint8_t length, bool rep_start)
{
	_twi_data *element = f_twi_prepare_data(address, data, length, rep_start);
	
	element->rw = 1;
	element->self_delete = 0;
	
	bool isOk = f_cb_try_enqueue(&Twi_buffer, element);
	
	if (isOk)
	{
		if(Twi.status == twi_idle) TWI_vect();
		
		while(!element->ready)
			;
		
		memcpy(data, element->data, length);
		free_mine(element);
	}
	else return false;
	
	return true;
}