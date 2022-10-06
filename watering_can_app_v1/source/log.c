/*
 * log.c
 *
 * Created: 18.09.2022 17:53:48
 *  Author: domis
 */ 

#include <stdlib.h>
#include <time.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "twi_eeprom.h"
#include "log.h"
#include "rtc.h"
#include "memory.h"

#include "defines.h"
#include "uart_service.h"

#define HEADING_ADDRESS 0
#define FIRST_DATA_ADDRESS 64
#define LOG_MAX_SIZE 64
#define INIT_CODE 0xCC

struct _log_heading
{
	uint8_t init_code;
	uint16_t first_data_address;
	uint8_t size;
}Log;


void f_init_log()
{
	struct _log_heading temp_heading;
	
	f_twi_eeprom_read_block(HEADING_ADDRESS, (uint8_t*)&temp_heading, sizeof(struct _log_heading));
	
	if(temp_heading.init_code != INIT_CODE) f_log_reset_all();
	
	Log = temp_heading;
}

static inline void f_log_update_heading()
{
	f_twi_eeprom_write_byte(HEADING_ADDRESS + 3, Log.size);
}

bool f_log_append_entry(_log_data* entry)
{
	uint16_t blank_address;
	
	blank_address = FIRST_DATA_ADDRESS + sizeof(_log_data)*(Log.size);
	
	bool isOk = f_twi_eeprom_write_block(blank_address, entry, sizeof(_log_data));
	f_log_update_heading();
	
	Log.size = (Log.size + 1) % LOG_MAX_SIZE;
	if(Log.size >= LOG_MAX_SIZE)
	{
		_log_data rollover = {g_rtc_timestamp, lt_program, lp_log_rollover};
		f_log_append_entry(&rollover);
	}
	
	return isOk;
}

_log_data f_log_get_entry(uint8_t number)
{
	uint16_t read_address = FIRST_DATA_ADDRESS + sizeof(_log_data) * number;
	_log_data entry;
	f_twi_eeprom_read_block(read_address, &entry, sizeof(_log_data));
	
	return entry;
}

_log_data f_log_get_last_entry()
{
	_log_data entry = f_log_get_entry(Log.size);
	return entry;
}

uint8_t f_log_get_entry_quantity()
{
	return Log.size;
}

bool f_log_delete_last_entry()
{
	if(Log.size == 0) return false;
	else Log.size--;
	f_log_update_heading();
	return true;
}

void f_log_reset_all()
{
	Log.init_code = INIT_CODE;
	Log.first_data_address = FIRST_DATA_ADDRESS;
	Log.size = 0;
	
	f_twi_eeprom_write_block(HEADING_ADDRESS, (uint8_t*)&Log, sizeof(struct _log_heading));
}

char* f_log_entry_to_ascii(char** str, _log_data *Entry)
{
	const __flash char* log_title[] = {PSTR("Time:\t"), PSTR("Type:\t"), PSTR("Info:\t")};
	const __flash char* log_type_info[] = {PSTR("Program\n"), PSTR("Data\n"), PSTR("Error\n")};
	const __flash char* log_data_info[] = {PSTR("No info\n"), PSTR("Log rollover\n"), PSTR("Watering\n")};
	
	*str = malloc_mine(60);
	
	char ascii_time[20];
	isotime_r(gmtime(&Entry->timestamp), (char*)ascii_time);
	memmove((char*)&ascii_time, (char*)&ascii_time + 11, 9);
	
	strcpy_P(*str, log_title[0]);
	strcat(*str, ascii_time);
	strcat(*str, "\n");
	
	strcat_P(*str, log_title[1]);
	strcat_P(*str, log_type_info[Entry->type]);
	
	strcat_P(*str, log_title[2]);
	if(Entry->type != lt_error) strcat_P(*str, log_data_info[Entry->data]);
	else
	{
		char txt[5];
		utoa(Entry->type, txt, 10);
		strcat(*str, (char*)&txt);
	}
	strcat(*str, "\n");
	
	
	realloc(*str, strlen(*str) + 1);
	
	return *str;
}