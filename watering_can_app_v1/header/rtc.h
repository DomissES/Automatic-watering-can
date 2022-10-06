/*
 * rtc.h
 *
 * Created: 01.09.2022 14:08:42
 *  Author: domis
 */ 


#ifndef RTC_H_
#define RTC_H_

#include <time.h>
#include <stdbool.h>

extern volatile time_t g_rtc_timestamp;
extern const struct tm* g_Rtc_time_ptr;

typedef struct  
{
	uint16_t counter:	15;
	bool last_tick:		1;
} _rtc_timer;

void f_init_rtc_timer();
bool f_rtc_timer_countdown(const uint8_t set, _rtc_timer *timer);



#endif /* RTC_H_ */