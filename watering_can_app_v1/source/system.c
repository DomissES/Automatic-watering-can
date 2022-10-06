/*
 * system.c
 *
 * Created: 31.08.2022 12:17:11
 *  Author: domis
 */ 

#include <avr/interrupt.h>
#include <avr/io.h>
#include "system.h"
#include "state_machine.h"

_error Sys_error;

ISR(BADISR_vect)
{
	
}

void f_init_gpio()
{
	DDRA = 0b00011111;
	PORTA = 0b00001111;
	
	DDRB = 0xFF;
	PORTB = 0xFF;
	
	DDRC = 0xFF;
	PORTC= 0x00;
	
	DDRD = 0b11101000;
	PORTD = 0b11111000;
	
	DDRE = 0b00111100;
	PORTE = 0b00111000;
	
	DDRF = 0x00;
	PORTF = 0x00;
	
	DDRG = 0b00000;
	PORTG = 0b00000;
}

void f_sys_report_error(_error_code code, uint8_t priority)
{
	Sys_error.status = true;
	Sys_error.code = code;
	Sys_error.priority = priority;
	f_sm_report_event();
	g_Next_state.state = s_error;
}

void f_sys_delete_error()
{
	Sys_error.status = false;
}

_error f_sys_check_last_error()
{
	return Sys_error;
}