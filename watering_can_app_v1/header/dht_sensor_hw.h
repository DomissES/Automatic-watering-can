/*
 * dht_sensor_hw.h
 *
 * Created: 01.09.2022 15:56:10
 *  Author: domis
 */ 


#ifndef DHT_SENSOR_HW_H_
#define DHT_SENSOR_HW_H_

#include <stdint.h>
#include <stdfix.h>

extern struct _dht_measure
{
	uint8_t humidity;
	accum temperature;
	} Dht_measure;

void f_init_dht();
bool f_dht_measure_task();


inline uint8_t f_dht_get_humidity()
{
	return Dht_measure.humidity;
}
inline accum f_dht_get_temperature()
{
	return Dht_measure.temperature;
}



#endif /* DHT_SENSOR_HW_H_ */