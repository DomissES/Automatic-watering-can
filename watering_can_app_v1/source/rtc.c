/*
 * rtc.c
 *
 * Created: 01.09.2022 14:08:52
 *  Author: domis
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "rtc.h"
#include "period_task.h"

#include "dht_sensor_hw.h"
#include "adc.h"
#include "hmi.h"

volatile time_t g_rtc_timestamp;
bool rtc_timer_tick;

struct tm Rtc_time;
const struct tm *g_Rtc_time_ptr = &Rtc_time;

static inline void f_rtc_interrupt() __attribute__((always_inline));

ISR(TIMER0_OVF_vect)
{
	f_rtc_interrupt();
	if(g_adc_enable_flags) f_task_request(&f_adc_update_task);
	f_task_request(&f_hmi_update_buzzer_task);
	f_task_request(&f_hmi_update_buttons_task);
	f_task_request(&f_hmi_update_led_task);
}

void f_init_rtc_timer()
{
	ASSR |= _BV(AS0);
	TCNT0=0;
	TCCR0 |= _BV(CS01);
	while(ASSR & 0x07);
	TIMSK |= _BV(TOIE0);
}

void f_rtc_4Hz_timer()
{
	rtc_timer_tick ^= true;
}

void f_rtc_1Hz_timer()
{
	localtime_r(&g_rtc_timestamp, &Rtc_time);
	if(g_rtc_timestamp == ONE_DAY) g_rtc_timestamp = 0;
}

void f_rtc_025Hz_timer()
{
	f_task_request(&f_dht_measure_task);
}

static inline void f_rtc_interrupt()
{
	static uint8_t count_4Hz, count_1Hz, count_025Hz;
	
	if(count_4Hz == 4)
	{
		f_rtc_4Hz_timer();
		count_4Hz = 0;
	}
	
	if(count_1Hz == 16)
	{
		g_rtc_timestamp++;
		f_rtc_1Hz_timer();
		count_1Hz = 0;
	}
	
	if(count_025Hz == 64)
	{
		f_rtc_025Hz_timer();
		count_025Hz = 0;
	}
	
	count_4Hz++;
	count_1Hz++;
	count_025Hz++;
}

bool f_rtc_timer_countdown(const uint8_t set, _rtc_timer *timer)
{
	bool timeout = false;
	
	if(timer->last_tick != rtc_timer_tick)
	{
		timer->last_tick = rtc_timer_tick;
		
		if(timer->counter == 0)
		{
			timer->counter = set;
			timeout = true;
		}
		timer->counter--;
	}
	return timeout;
}