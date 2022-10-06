/*
 * dht_sensor_hw.c
 *
 * Created: 01.09.2022 15:56:21
 *  Author: domis
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "dht_sensor_hw.h"
#include "defines.h"
#include "period_task.h"
#include "system.h"

//link do dokumentacji czujnika
//https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf

#define START_CAPTURE TCCR1B |= _BV(CS11);
#define STOP_CAPTURE TCCR1B &= ~_BV(CS11);

struct _dht_measure Dht_measure;


bool receive_active;
uint8_t receive_bytes[5];

bool f_dht_decode();

ISR(TIMER1_CAPT_vect)
{
	static uint8_t byte, receive_bit;
	
	if(receive_active)
	{
		byte <<= 1;
		if(ICR1 > 200) byte |= 1;
		
		if((receive_bit % 8) == 7)
		{
			receive_bytes[receive_bit/8] = byte;
			
			if(receive_bit == 39)
			{
				receive_active = false;
				STOP_CAPTURE;
				f_task_request(&f_dht_decode);
			}
		}
		receive_bit++;
	}
	else if((ICR1 > 290) && (ICR1 < 400)) // start bit: 160-200us
	{
		receive_active = true;
		receive_bit = 0;
	}
	
	if(ICR1 > 500) f_sys_report_error(err_dht, 2);
	
	TCNT1 = 0;
}

void f_init_dht()
{
	TIMSK |= _BV(TICIE1);
}

bool f_dht_measure_task()
{
	if(!receive_active)
	{
		CLR(PORT, DHT_THMM);
		_delay_ms(20);
		SET(PORT, DHT_THMM);
		
		START_CAPTURE;
		return true;
	}
	return false;
}

bool f_dht_decode()
{
	uint8_t checksum = 0;
	
	for(int i = 0; i < 4; i++) checksum += receive_bytes[i];
	
	if(checksum == receive_bytes[4])
	{
		Dht_measure.humidity = (uint8_t)((accum)((receive_bytes[0] << 8) + receive_bytes[1])/10);
		Dht_measure.temperature = (accum)((receive_bytes[2] << 8) + receive_bytes[3])/10;
		return true;
	}
	else f_sys_report_error(err_dht, 2);
	
	return false;
}