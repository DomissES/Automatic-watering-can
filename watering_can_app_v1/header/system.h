/*
 * system.h
 *
 * Created: 31.08.2022 12:16:59
 *  Author: domis
 */ 


#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <stdbool.h>

typedef enum{
	err_no_error,
	err_general,
	err_malloc,
	err_memory_low,
	err_buf_ovf,
	err_tx_ovf,
	err_rx_ovf,
	err_twi_ovf,
	err_twi_gen,
	err_twi_eeprom,
	err_task,
	err_adc = 0x0C,
	err_dht = 0x10,
	} _error_code;

typedef struct  
{
	bool status;
	_error_code code;
	uint8_t priority;
} _error;

void f_init_gpio();
void f_sys_report_error(_error_code code, uint8_t priority);
void f_sys_delete_error();
_error f_sys_check_last_error();


#endif /* SYSTEM_H_ */