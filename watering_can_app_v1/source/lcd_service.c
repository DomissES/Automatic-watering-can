/*
 * lcd_service.c
 *
 * Created: 24.08.2022 23:25:35
 *  Author: domis
 */ 

#include <string.h>
#include <util/atomic.h>
#include <avr/pgmspace.h>
#include "lcd_service.h"
#include "hd44780_hw.h"
#include "defines.h"
#include "circ_buffer.h"
#include "memory.h"
#include "period_task.h"

_circ_buffer Lcd_buffer;

typedef struct  
{
	uint8_t length;
	char data[];
} _lcd_data;

void f_init_lcd()
{
	hd44780_init();
	
	hd44780_wait_ready(true);
	hd44780_outcmd(HD44780_CLR);
	hd44780_wait_ready(true);
	hd44780_outcmd(HD44780_ENTMODE(1,0));
	hd44780_wait_ready(true);
	hd44780_outcmd(HD44780_DISPCTL(1,0,0));
		
}

static bool f_lcd_put_cmd(uint8_t cmd)
{
	_lcd_data *element;
	
	element = malloc_mine(sizeof(_lcd_data) + 2);
	element->length = 1;
	element->data[0] = cmd;
	
	bool isOk = f_cb_try_enqueue(&Lcd_buffer, element);
	return isOk;
};

static inline _lcd_data* f_lcd_prepare_data(const __memx char* str)
{
	_lcd_data *element;
	uint8_t length;
		
	if(__builtin_avr_flash_segment(str)==255)
	{
		length = strlen(str) + 1;
		element = malloc_mine(sizeof(_lcd_data) + length);
		strcpy((char*)&element->data[1], str);
		element->length = length;
	}
	else
	{
		length = strlen_P(str) + 1;
		element = malloc_mine(sizeof(_lcd_data) + length);
		strcpy_P((char*)&element->data[1], str);
		element->length = length;
	}
	return element;
}

bool f_lcd_puttext(uint8_t col, uint8_t line, const __memx char* txt)
{
	_lcd_data *element = f_lcd_prepare_data(txt);
	
	element->data[0] = HD44780_DDADDR(col+line*0x40);
	bool isOk = f_cb_try_enqueue(&Lcd_buffer, element);
	
	f_task_request(&f_lcd_transmit_data_task);
	
	return isOk;
}

void f_lcd_clear_all()
{
	for(uint8_t i = 0; i<4; i++)
	{
		f_lcd_clear_segment(i);
	}
}

void f_lcd_clear_segment(_lcd_segments segment)
{
	char data[9];
	memset((char*)data, 0x20, 8); //" "
	memset((char*)&data[8], 0x00, 1); //"\0"
	
	f_lcd_puttext((8*(0x01 & segment)), (segment >> 1), (char *)data);
}

void f_lcd_dispctl(bool disp, bool cursor, bool blink)
{
	uint8_t data = HD44780_DISPCTL(disp,cursor,blink);
	f_lcd_put_cmd(data);
}

void f_lcd_turn_dsp(_lcd_power power)
{
		if(power == on)
		{
			SET(PORT, BACKLIGHT);
		}
		else
		{
			CLR(PORT, BACKLIGHT);
		}
}

bool f_lcd_transmit_data_task()
{
	static _lcd_data *transmit;
	static struct
	{
		uint8_t position:	6;
		uint8_t nibble:		1;
	} sequence;
	if(transmit == NULL)
	{
		transmit = f_cb_dequeue(&Lcd_buffer);
		sequence.nibble = 0;
		sequence.position = 0;
	}

	if(transmit)
	{		
		char byte = transmit->data[sequence.position];
		if(sequence.nibble == 0) byte >>= 4;
		hd44780_outnibble_wo_pulse(byte & 0x0F, (sequence.position != 0));
		SET(PORT, HD44780_E);
		sequence.nibble++;
		
		if(sequence.nibble == 0) sequence.position++;
		if(sequence.position >= transmit->length)
		{
			free_mine(transmit);
			transmit = NULL;
		}
		CLR(PORT, HD44780_E);
		
		
		f_task_request(&f_lcd_transmit_data_task);
	}
	
	return true;
}