/*
 * uart_service.h
 *
 * Created: 30.08.2022 22:39:41
 *  Author: domis
 */ 


#ifndef UART_SERVICE_H_
#define UART_SERVICE_H_

#include <stdbool.h>


bool f_uart_send_byte(const uint8_t byte);
bool f_uart_send_data(const __memx void* data, uint8_t length);
uint8_t *f_uart_get_data();
void f_init_uart();


#endif /* UART_SERVICE_H_ */