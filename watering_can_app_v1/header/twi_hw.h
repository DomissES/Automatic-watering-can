/*
 * twi_hw.h
 *
 * Created: 11.09.2022 13:12:39
 *  Author: domis
 */ 


#ifndef TWI_HW_H_
#define TWI_HW_H_

void f_init_twi();
bool f_twi_send_data(uint8_t address, const __memx void* data, uint8_t length, bool rep_start);
bool f_twi_read_data(uint8_t address, void* data, uint8_t length, bool rep_start);



#endif /* TWI_HW_H_ */