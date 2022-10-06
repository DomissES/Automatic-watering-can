/*
 * log.h
 *
 * Created: 18.09.2022 17:53:39
 *  Author: domis
 */ 


#ifndef LOG_H_
#define LOG_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {lt_program, lt_data, lt_error} _log_type_info;
typedef enum {lp_generic, lp_log_rollover, ld_watering} _log_data_info;

typedef struct
{
	uint32_t timestamp;
	_log_type_info type;
	_log_data_info data;
} _log_data;


void f_init_log();
bool f_log_append_entry(_log_data* entry);

_log_data f_log_get_entry(uint8_t number);
_log_data f_log_get_last_entry();

uint8_t f_log_get_entry_quantity();
bool f_log_delete_last_entry();
void f_log_reset_all();

char* f_log_entry_to_ascii(char** str, _log_data *entry);

#endif /* LOG_H_ */