/*
 * twi_eeprom.h
 *
 * Created: 14.09.2022 10:26:57
 *  Author: domis
 */ 


#ifndef TWI_EEPROM_H_
#define TWI_EEPROM_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

bool f_twi_eeprom_write_byte(const uint16_t address, uint8_t byte);
bool f_twi_eeprom_write_block(const uint16_t address, const void* data, size_t length);
uint8_t f_twi_eeprom_read_byte(const uint16_t address);
void* f_twi_eeprom_read_block(const uint16_t address, void* data, size_t length);



#endif /* TWI_EEPROM_H_ */