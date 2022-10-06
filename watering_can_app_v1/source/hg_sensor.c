/*
 * hg_sensor.c
 *
 * Created: 02.09.2022 13:08:50
 *  Author: domis
 */ 

#include <stdfix.h>
#include "hg_sensor.h"
#include "adc.h"

#define HG_MIN 2.38k
#define HG_MAX 2.83k
#define HG_MIN_BIT (1024*(HG_MIN-2)*3)/ADC_VREF
#define HG_MAX_BIT (1024*(HG_MAX-2)*3)/ADC_VREF


static inline accum f_normalize(accum min, accum max, accum val, uint8_t nmax)
{
	accum delta = max - min;
	if(val <= min) val = min;
	else if(val >= max) val = max;
	
	return (val-min)*(accum)(nmax/delta);
}

uint8_t f_hg_get_soil_humidity()
{
	uint16_t measurement = f_adc_get_measure(HGB);
	uint8_t humidity = 100 - f_normalize(HG_MIN_BIT,HG_MAX_BIT, measurement, 100);
	return humidity;
}