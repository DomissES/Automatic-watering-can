/*
 * adc.c
 *
 * Created: 24.08.2022 22:24:32
 *  Author: domis
 */ 

#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/io.h>
#include "adc.h"
#include "system.h"

uint8_t g_adc_enable_flags;
volatile uint16_t g_adc_measures[8];

volatile uint16_t adc_sum;
volatile bool adc_busy;


ISR(ADC_vect)
{
	static uint8_t samples;
	static uint16_t accu;
	accu += ADC;
	samples++;
	if(samples == AVERAGE_MEASURES)
	{
		adc_sum = accu;
		accu = 0;
		samples = 0;
		adc_busy = false;
	}
	else ADC_START;
}

void f_init_adc()
{
	ADMUX |= _BV(REFS0);
	ADCSRA |= _BV(ADIE) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
	ADCSRB |= _BV(ADTS2);
}

static inline _adc_channels f_adc_jump_to_next_channel(_adc_channels channel)
{
	channel = (channel + 1) % 8;
	if((g_adc_enable_flags & (1 << channel)) == 0) channel = f_adc_jump_to_next_channel(channel);
	
	ADMUX = (ADMUX & 0xe0) | channel;
	return channel;
}

bool f_adc_update_task()
{
	static _adc_channels channel;
	
	if(!adc_busy)
	{
		uint16_t temp;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			temp = adc_sum;
		}
		g_adc_measures[channel] = temp/AVERAGE_MEASURES;
		
		channel = f_adc_jump_to_next_channel(channel);
		adc_busy = false;
		ADC_START;
		return true;
	}
	f_sys_report_error(err_adc, 2);
	return false;
}
