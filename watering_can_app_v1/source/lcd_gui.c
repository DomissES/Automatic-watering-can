/*
 * lcd_gui.c
 *
 * Created: 05.09.2022 13:15:55
 *  Author: domis
 */ 

#include <avr/pgmspace.h>
#include <stdfix.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "lcd_gui.h"
#include "lcd_service.h"
#include "rtc.h"
#include "dht_sensor_hw.h"
#include "hg_sensor.h"
#include "state_machine.h"
#include "period_task.h"


#define PGM_STR(X) ((const __flash char[]){X})

_gui_main_pages active_page;

typedef char *(*value_to_string_ptr)(char *str);

typedef struct {
	const __flash char* title;
	value_to_string_ptr convert;
	} _gui_map;

static char *f_gui_time_to_str(char *str);
static char *f_gui_temp_to_str(char *str);
static char *f_gui_rh_to_str(char *str);
static char *f_gui_state_to_str(char *str);

const __flash _gui_map Gui_pages[] = {
	{PGM_STR("State   "), f_gui_state_to_str},
	{PGM_STR("Time    "), f_gui_time_to_str},
	{PGM_STR("Temp    "), f_gui_temp_to_str},
	{PGM_STR("RH      "), f_gui_rh_to_str}
};

const __flash char *State_names[] = {
	PGM_STR("Error "),
	PGM_STR("Idle  "),
	PGM_STR("Watch "),
};

static char *accumtostr(accum value, char *str, uint8_t digits)
{
	if(signbit(value) && ((int8_t)value < 1))
	{
		str[0] = 0x2D; // "-"
		utoa(value, &str[1], 10);
	}
	else itoa(value, str, 10);
	
	strcat(str, ",");
	value = value - (int16_t)value;
	
	
	if(signbit(value)) value =- value;
	
	while(digits)
	{
		value *= 10;
		if((int16_t)value == 0) strcat(str, "0");
		else utoa(value, &str[strlen(str)], 10);
		value = value - (int16_t)value;
		digits--;
	}
	return str;
}


static char *f_gui_time_to_str(char *str)
{
	char temp[20];
	isotime_r(g_Rtc_time_ptr, (char*)temp);
	strlcpy(str, temp + 11, 9);
	
	return str;
}

static char *f_gui_temp_to_str(char *str)
{
	accum temperature = f_dht_get_temperature();
	accumtostr(temperature, str, 1);
	strcat(str, "\xDF" "C");
	return str;
}

static char *f_gui_rh_to_str(char *str)
{
	uint8_t humidity = f_dht_get_humidity();
	utoa(humidity, str, 10);
	strcat(str, "%");
	return str;
}


static char *f_gui_state_to_str(char *str)
{
	uint8_t index;
	switch(f_sm_get_current_state())
	{
		case s_idle:
			index = 1;
			break;
		case s_work_watch:
			index = 2;
			break;
		default:
			index = 0;
	}
	return strcpy_P(str, State_names[index]);
}

bool f_gui_dsp_next_page()
{
	active_page = (active_page + 1) % (sizeof(Gui_pages) / sizeof(Gui_pages[0]));
	f_gui_dsp_page(active_page);
	
	return true;
}

bool f_gui_dsp_page(_gui_main_pages page)
{
	active_page = page;
	f_gui_dsp_text(gui_up_a, Gui_pages[page].title);
	f_gui_update_page_value();
	return true;
}

bool f_gui_update_page_value()
{
	char temp[11];
	(*Gui_pages[active_page].convert)(temp);
	while(strlen(temp) < 8) strcat(temp, " ");
	f_gui_dsp_text(gui_up_b, temp);
	return true;
}

static char *f_gui_soil_h_to_str(uint16_t value, char *str)
{
	utoa(value, str, 10);
	strcat(str, "%");
	while(strlen(str)<8) strcat(str, " ");
	return str;
}

bool f_gui_dsp_soil_h(uint16_t value, _gui_soil_pages page)
{
	const __flash char* title[2] = {
		PSTR("Target: "),
		PSTR("Current:")
	};
	 
	f_gui_dsp_text(gui_down_a, title[page]);
	
	char str[9];
	f_gui_soil_h_to_str(value, str);
	
	f_gui_dsp_text(gui_down_b, str);
	
	return true;
}

bool f_gui_dsp_text(_gui_segments segment, const __memx char* str)
{
	bool isOk;
	
	isOk = f_lcd_puttext((8*(segment & 0x01)), (segment >> 1), str);
	
	return isOk;
}

