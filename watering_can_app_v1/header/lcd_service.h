/*
 * lcd_service.h
 *
 * Created: 24.08.2022 23:25:17
 *  Author: domis
 */ 


#ifndef LCD_SERVICE_H_
#define LCD_SERVICE_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {left_up, right_up, left_down, right_down} _lcd_segments;
typedef enum {on, off} _lcd_power;
void f_init_lcd();

bool f_lcd_puttext(uint8_t x, uint8_t y, const char __memx *txt);

void f_lcd_clear_all();
void f_lcd_clear_segment(_lcd_segments segment);
void f_lcd_dispctl(bool disp, bool cursor, bool blink);
void f_lcd_turn_dsp(_lcd_power power);

bool f_lcd_transmit_data_task();

#endif /* LCD_SERVICE_H_ */