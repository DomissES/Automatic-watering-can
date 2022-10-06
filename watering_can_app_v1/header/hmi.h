/*
 * hmi.h
 *
 * Created: 02.09.2022 13:17:54
 *  Author: domis
 */ 


#ifndef HMI_H_
#define HMI_H_

#include <stdbool.h>


typedef enum
{
	hmi_off = 0,
	hmi_one_impulse = 0b00000001,
	hmi_two_impulse = 0b00000101,
	hmi_long_impulse = 0b00001111,
	hmi_blink = 0b11110011,
	hmi_on = 0b11111111
	} _hmi_sequence;

typedef enum {led_error, led_a, led_b} _hmi_led;
typedef enum {b_encb, b_sta, b_stb} _hmi_button;
	
extern bool g_hmi_lock_buttons;

void f_init_hmi_buttons();
void f_hmi_toggle_buzzer(_hmi_sequence tone);
void f_hmi_toggle_led(_hmi_led led, _hmi_sequence sequence);
bool f_hmi_read_button(_hmi_button button);
uint8_t f_hmi_get_pressed_seconds(_hmi_button button);


int16_t f_hmi_encoder_change_value(int16_t min, int16_t max, uint8_t step, int16_t current_value);

void f_hmi_update_buzzer_task();
void f_hmi_update_led_task();
void f_hmi_update_buttons_task();


#endif /* HMI_H_ */