/*
 * twi_eeprom.c
 *
 * Created: 14.09.2022 10:27:10
 *  Author: domis
 */ 

#include <string.h>
#include <util/delay.h>
#include "twi_eeprom.h"
#include "twi_hw.h"
#include "memory.h"
#include "system.h"

#define PAGE_SIZE 64
#define LAST_PAGE_ADDRESS 250
#define EEPROM_ADDRESS 0xA0

static inline void f_twi_eeprom_write_address_to_table(uint16_t address, uint8_t *element)
{
	element[0] = (uint8_t)(address >> 8);
	element[1] = (uint8_t)(address & 0xFF);
}

bool f_twi_eeprom_write_byte(const uint16_t address, uint8_t byte)
{
	uint8_t element[3];
			
	f_twi_eeprom_write_address_to_table(address, (uint8_t*)&element);
	element[2] = byte;
	
	bool isOk = f_twi_send_data(EEPROM_ADDRESS, (uint8_t*)&element, 3, false);
	
	return isOk;	
}

bool f_twi_eeprom_write_block(const uint16_t address, const void* data, size_t length)
{
	bool isOk = false;
	size_t bytes_for_page;
	size_t temp_address = address;
	uint8_t i = 0;
	do
	{
		bytes_for_page = PAGE_SIZE - (temp_address % PAGE_SIZE);
		if(bytes_for_page >= length) bytes_for_page = length;
		length -= bytes_for_page;
		
		uint8_t *element = malloc_mine(bytes_for_page + 2);
		
		f_twi_eeprom_write_address_to_table(temp_address, element);
		memcpy((uint8_t*)&element[2], (uint8_t*)&data[temp_address - address], bytes_for_page);
		
		isOk = f_twi_send_data(EEPROM_ADDRESS, element, bytes_for_page + 2, false);
		
		free_mine(element);
		if(!isOk)
		{
			f_sys_report_error(err_twi_eeprom, 1);
			return false;
		}
		
		temp_address += bytes_for_page;		
		i++;
	}while(length);
	
	
	return isOk;
}

uint8_t f_twi_eeprom_read_byte(const uint16_t address)
{
	
}

void* f_twi_eeprom_read_block(const uint16_t address, void* data, size_t length)
{
	uint8_t temp_address[2];
	f_twi_eeprom_write_address_to_table(address, (uint8_t*)&temp_address);
	
	f_twi_send_data(EEPROM_ADDRESS, (uint8_t*)&temp_address, 2, false);
	bool isOk = f_twi_read_data(EEPROM_ADDRESS, data, length, true);
	
	if(!isOk) f_sys_report_error(err_twi_gen, 2);
	return data;
}
