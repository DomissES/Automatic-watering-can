/*
 * lcd_gui.h
 *
 * Created: 05.09.2022 13:15:39
 *  Author: domis
 */ 


#ifndef LCD_GUI_H_
#define LCD_GUI_H_

#include <stdbool.h>
#include "defines.h"

typedef enum {
	gui_state,
	gui_time,
	gui_temperatue,
	gui_rh
	} _gui_main_pages;
	
typedef enum {
	soil_target,
	soil_current
	} _gui_soil_pages;
	
typedef enum {
	gui_up_a,
	gui_up_b,
	gui_down_a,
	gui_down_b
	} _gui_segments;

bool f_gui_dsp_next_page();
bool f_gui_dsp_page(_gui_main_pages page);
bool f_gui_update_page_value();
bool f_gui_dsp_text(_gui_segments segment, const __memx char* str);

bool f_gui_dsp_soil_h(uint16_t value, _gui_soil_pages page);

inline void f_gui_backlight_on()
{
	SET(PORT, BACKLIGHT);
}
inline void f_gui_backlight_off()
{
	CLR(PORT, BACKLIGHT);
}


#endif /* LCD_GUI_H_ */