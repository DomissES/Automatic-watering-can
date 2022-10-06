/*
 * adc.h
 *
 * Created: 24.08.2022 22:24:19
 *  Author: domis
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <stdfix.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include "defines.h"


#define AVERAGE_MEASURES 16
#define ADC_VREF 4.990k

#define ADC_ENABLE ADCSRA |= _BV(ADEN);
#define ADC_DISABLE ADCSRA &= ~_BV(ADEN);
#define ADC_START ADCSRA |= _BV(ADSC);

typedef enum{BCU, BVL, SVL, HGB, HGA, LPHB, LPHM, LPHA} _adc_channels;
extern uint8_t g_adc_enable_flags;
extern volatile uint16_t g_adc_measures[8];

void f_init_adc();
bool f_adc_update_task();

inline uint16_t f_adc_get_measure(_adc_channels chan)
{
	return g_adc_measures[chan];
}
inline void f_adc_enable_channel(_adc_channels channel)
{
	g_adc_enable_flags |= 1 << channel;
	if(!(ADCSRA & _BV(ADEN))) ADC_ENABLE;
}
inline void f_adc_disable_channel(_adc_channels channel)
{
	g_adc_enable_flags &= ~(1 << channel);
	if(!g_adc_enable_flags) ADC_DISABLE;
}




#endif /* ADC_H_ */