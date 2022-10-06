/*
 * uart_service.c
 *
 * Created: 30.08.2022 22:39:56
 *  Author: domis
 */ 

#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <util/atomic.h>
#include <avr/pgmspace.h>
#include "uart_service.h"
#include "circ_buffer.h"
#include "system.h"
#include "memory.h"

#define MAX_RX_BYTES 16

typedef struct
{
	uint8_t length;
	char data[];
} _uart_data;

_circ_buffer Uart_TX_buffer, Uart_RX_buffer;
volatile bool tx_busy;

static inline void f_uart_transmit_data_interrupt() __attribute__((always_inline));
static inline bool f_uart_receive_data_interrupt() __attribute__((always_inline));


ISR(USART0_TX_vect)
{
	f_uart_transmit_data_interrupt();
}

ISR(USART0_RX_vect)
{
	f_uart_receive_data_interrupt();
}


static void f_uart_baud_19200()
{
	#define BAUD 19200
	#include <util/setbaud.h>
	
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	
	#if USE_2X
	UCSR0A |= _BV(U2X0);
	#else
	UCSR0A &= ~_BV(U2X0);
	#endif
}

void f_init_uart()
{
	f_uart_baud_19200();
	
	UCSR0B |= _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0) | _BV(TXCIE0);
	UCSR0C |= _BV(UCSZ00) | _BV(UCSZ01);
	
}

bool f_uart_send_byte(const uint8_t byte)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(tx_busy) return false;
		else
		{
			while (!(UCSR0A & _BV(UDRE0))) ;
			UDR0 = byte;
		}
	}
	return true;
}

static inline _uart_data* f_uart_prepare_data(const __memx uint8_t* data, uint8_t length)
{
	_uart_data *element;
	element = malloc_mine(sizeof(_uart_data) + length);
	
	if(__builtin_avr_flash_segment(data)==255) memcpy((char*)&element->data, data, length);
	else memcpy_P((char*)&element->data, data, length);
	
	element->length = length;
	return element;
}

bool f_uart_send_data(const __memx void* data, uint8_t length)
{
	_uart_data *element = f_uart_prepare_data(data, length);
	
	bool isOk = f_cb_try_enqueue(&Uart_TX_buffer, element);
	
	if(isOk)
	{
		if(!tx_busy)
		{
			tx_busy = true;
			USART0_TX_vect();
		}
	}
	else f_sys_report_error(err_tx_ovf, 1);
	
	return isOk;
}

uint8_t *f_uart_get_data()
{
	_uart_data *element = f_cb_dequeue(&Uart_RX_buffer);
	uint8_t *data;
	
	if(element != NULL)
	{
		data = malloc_mine(element->length);
		
		memcpy(data, element->data, element->length);
		free_mine(element);
	}
	else data = NULL;
	
	return data;
}

static inline void f_uart_transmit_data_interrupt()
{
	static _uart_data *transmit;
	static uint8_t position;
	
	if(transmit == NULL)
	{
		transmit = f_cb_dequeue(&Uart_TX_buffer);
		position = 0;
		
		if(transmit == NULL) tx_busy = false;
	}
	
	if(transmit)
	{
		UDR0 = transmit->data[position];
		position++;
		
		if(position >= transmit->length)
		{
			free_mine(transmit);
			transmit = NULL;
		}
	}
}

static inline bool f_uart_receive_data_interrupt()
{
	bool isOk;
	
	static _uart_data *receive;
	static uint8_t position;
	
	uint8_t data = UDR0;
	
	if(receive == NULL)
	{
		receive = malloc_mine(sizeof(_uart_data) + MAX_RX_BYTES + 1);
	}
	
	if(data == 0x0D) //carriage return
	{
		realloc(receive, sizeof(_uart_data) + position + 1);
		receive->data[position] = 0x00; // "\0"
		
		position = 0;
		receive->length = position;
		
		isOk = f_cb_enqueue(&Uart_RX_buffer, receive);
		if(isOk) receive = NULL;
	}
	else
	{
		receive->data[position] = data;
		position++;
		
		if(position >= MAX_RX_BYTES)
		{
			isOk = false;
			f_sys_report_error(err_rx_ovf, 1);
		}
	}
	return isOk;
}
