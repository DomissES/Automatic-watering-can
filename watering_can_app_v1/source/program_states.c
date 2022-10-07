/*
 * program_states.c
 *
 * Created: 17.08.2022 14:14:43
 *  Author: domis
 */ 
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "adc.h"
#include "program_states.h"
#include "system.h"
#include "state_machine.h"
#include "hmi.h"
#include "lcd_gui.h"
#include "rtc.h"
#include "uart_service.h"
#include "lcd_service.h"
#include "period_task.h"
#include "dht_sensor_hw.h"
#include "hg_sensor.h"
#include "memory.h"
#include "twi_hw.h"
#include "log.h"


const __flash _state_machine g_Program_states[]=
{
	{e_error, ps_error},
	{s_init, ps_init},
	{s_idle_prologue, ps_idle_prologue},
	{s_idle, ps_idle},
	{s_time_in, ps_time_in},
	{s_export_log_prologue, ps_export_log_prologue},
	{s_export_log, ps_export_log},
	{s_value_in, ps_value_in},
	{s_work_watch, ps_work_watch},
	{s_work_check, ps_work_check},
	{s_fini, ps_fini}
};


_state_flow ps_error()
{
	f_lcd_clear_all();
	_error error = f_sys_check_last_error();
	_log_data log_info = {g_rtc_timestamp, lt_error, error.code};	
	f_log_append_entry(&log_info);
	
	f_hmi_toggle_led(led_error, hmi_on);
	if(error.priority != 0)
	{
		const __flash char* entry = PSTR("ERROR!  ");
		f_gui_dsp_text(gui_up_a, entry);
		
		char str[7];
		utoa(error.code, str, 16);
		f_gui_dsp_text(gui_up_b, str);
		
		while(!g_event_flag)
		;
		
		f_sys_delete_error();
		f_hmi_toggle_led(led_error, hmi_off);
		
		switch(f_sm_read_event())
		{
			case e_button_encb:
				return (_state_flow){s_idle_prologue, NULL};
				break;
			default:
				return (_state_flow){s_error, NULL};
		}
		
		
	}
	return (_state_flow){s_fini, NULL};
}

_state_flow ps_init()
{
	f_init_gpio();
	
	f_init_hmi_buttons();
	f_init_adc();
	f_init_dht();
	f_init_lcd();
	f_init_uart();
	f_init_twi();
	f_init_task_timer();
	TASK_TIMER_ENABLE;
	f_init_rtc_timer();
	sei();
	f_gui_backlight_on();
	
	f_init_log();
	
	return (_state_flow){s_idle_prologue, NULL};
}

_state_flow ps_idle_prologue()
{
	f_adc_enable_channel(HGB);
	
	
	return (_state_flow){s_idle, NULL};
}

_state_flow ps_idle()
{
	_rtc_timer timer_page;
	timer_page.counter = 1;
	
	_rtc_timer timer_value;
	timer_value.counter = 1;
	
	_rtc_timer timer_lock;
	timer_lock.counter = 4*20;
	
	uint8_t soil_h;
		
	while (!g_event_flag)
	{
		if(f_rtc_timer_countdown(12, &timer_page)) f_gui_dsp_next_page();		
		if(f_rtc_timer_countdown(1, &timer_value))
		{
			f_gui_update_page_value();
			soil_h = f_hg_get_soil_humidity();
			f_gui_dsp_soil_h(soil_h, soil_current);
		}
		if(!g_hmi_lock_buttons)
		{
			if(f_rtc_timer_countdown(80, &timer_lock))
			{
				f_gui_backlight_off();
				g_hmi_lock_buttons = true;
			}
		}
		_delay_ms(10);
	}
	
	switch(f_sm_read_event())
	{
		case e_error:
			return (_state_flow){s_error, NULL};
		case e_button_encb:
			if(g_hmi_lock_buttons)
			{
				g_hmi_lock_buttons = false;
				f_gui_backlight_on();
				return (_state_flow){s_idle, NULL};
			}
			else return (_state_flow){s_value_in, NULL};
		case e_button_encb_long:
			return (_state_flow){s_fini, NULL};
		case e_button_sta:
			return (_state_flow){s_time_in, NULL};
		case e_button_stb:
			return (_state_flow){s_export_log_prologue, NULL};
		default:
			f_hmi_toggle_buzzer(hmi_two_impulse);
			return (_state_flow){s_idle, NULL};
	}
}

_state_flow ps_time_in()
{
	f_lcd_clear_all();
	const __flash char * entry = PSTR("Set time:");
	f_gui_dsp_text(gui_up_a, entry);
		
	_rtc_timer timer_timeout;
	timer_timeout.counter = 4*10;
	char str[20];
	uint8_t ranges[2][3] = {{0, 23, 1}, {0, 60, 1}};
	struct tm temp_time = *g_Rtc_time_ptr;
	int8_t *input[2] = {&temp_time.tm_hour, &temp_time.tm_min};
	
	for(uint8_t i = 0; i < 2; i++)
	{
		int8_t last = *input[i] + 1;
		
		while(!g_event_flag)
		{
			*input[i] = f_hmi_encoder_change_value(ranges[i][0], ranges[i][1], ranges[i][2], *input[i]);
			
			if(*input[i] != last)
			{
				last = *input[i];
				isotime_r(&temp_time, (char*)str);
				strlcpy((char*)str, (char*)str+11, 9);
				f_gui_dsp_text(gui_down_a, str);
				timer_timeout.counter = 4*10;
			}
			if(f_rtc_timer_countdown(4*10, &timer_timeout))
			{
				return (_state_flow){s_idle_prologue, NULL};
			}
		}
		
		switch (f_sm_read_event())
		{
			case e_error:
				return (_state_flow){s_error, NULL};
				break;
			case e_timeout:
				return (_state_flow){s_idle_prologue, NULL};
			case e_button_encb:
				break;
			default:
				i--;
		}
	}
	
	g_rtc_timestamp = mktime(&temp_time);
	return (_state_flow){s_idle_prologue, NULL};
	
}

_state_flow ps_export_log_prologue()
{
	const __flash char * entry[] = {PSTR("Export?"), PSTR("Yes"), PSTR("No/ RS=>")};
	
	f_lcd_clear_all();
	f_gui_dsp_text(gui_up_a, entry[0]);
	f_gui_dsp_text(gui_down_a, entry[1]);
	f_gui_dsp_text(gui_down_b, entry[2]);
	
	_rtc_timer timer_timeout;
	timer_timeout.counter = 4*10;
	
	while(true)
	{
		while(!g_event_flag)
		{
			if(f_rtc_timer_countdown(0, &timer_timeout)) return (_state_flow){s_idle_prologue, NULL};
			_delay_ms(10);
		}
		
		switch(f_sm_read_event())
		{
			case e_button_sta:
				return (_state_flow){s_export_log, NULL};
			case e_button_encb_long:
				f_log_reset_all();
			case e_button_stb:
				return (_state_flow){s_idle_prologue, NULL};
			
			default:
				break;
		}
	}
}

_state_flow ps_export_log()
{
	const __flash char* entry[] = {PSTR("Exporting"), PSTR("Export OK")};
	const __flash char* uart_entry[] = {PSTR("Export log:\n"), PSTR("Export end\n")};
	const __flash char* txt_number = PSTR("Log number:\t");
	
	f_lcd_clear_all();
	f_gui_dsp_text(gui_up_a, entry[0]);
	
	uint8_t log_size = f_log_get_entry_quantity();
	char * eeprom_txt;
	char log_number[5];
	_log_data eeprom_input;
	
	f_uart_send_data(uart_entry[0], strlen_P(uart_entry[0]));
	
	for(uint8_t i = 0; i < log_size; i++)
	{
		f_uart_send_data(txt_number, strlen_P(txt_number));
		utoa(i, (char*)&log_number, 10);
		strcat((char*)&log_number, "\n");
		f_uart_send_data(&log_number, strlen((char*)&log_number));
		
		eeprom_input = f_log_get_entry(i);
		eeprom_txt = f_log_entry_to_ascii(&eeprom_txt, &eeprom_input);
		
		f_uart_send_data(eeprom_txt, strlen(eeprom_txt));
		
		_delay_ms(20);
	}
	f_uart_send_data(uart_entry[1], strlen_P(uart_entry[1]));
	
	f_gui_dsp_text(gui_up_a, entry[1]);
	_delay_ms(1500);
	
	return (_state_flow){s_idle_prologue, NULL};
}

_state_flow ps_value_in()
{
	const __flash char * entry[] =
	{
		PSTR("Set settings:"),
		PSTR("Target: "),
		PSTR("Pump On:"),
		PSTR("Check  :"),
	};
	f_lcd_clear_all();
	f_gui_dsp_text(gui_up_a, entry[0]);
	
	union
	{
		_ps_work_data structure;
		uint8_t indexed[3];
	} *Input_data;
	Input_data = malloc_mine(sizeof(_ps_work_data));
	
	_rtc_timer timer_timeout;
	timer_timeout.counter = 4*10;
	uint8_t ranges[3][3] = {{0, 100, 5}, {1, 10, 1}, {10, 240, 10}};
	char str[9];
	
	for(uint8_t i = 0; i < 3; i++)
	{
		Input_data->indexed[i] = ranges[i][0];
		uint16_t last = Input_data->indexed[i] + 1;
		
		f_gui_dsp_text(gui_down_a, entry[i+1]);
		
		while(!g_event_flag)
		{
			Input_data->indexed[i] = f_hmi_encoder_change_value(ranges[i][0], ranges[i][1], ranges[i][2], Input_data->indexed[i]);
			
			if(Input_data->indexed[i] != last)
			{
				last = Input_data->indexed[i];
				utoa(Input_data->indexed[i], str, 10);
				while(strlen(str) < 8) strcat(str, " ");
				f_gui_dsp_text(gui_down_b, str);
				timer_timeout.counter = 4*10;
			}
			if(f_rtc_timer_countdown(4*10, &timer_timeout))
			{
				free_mine(Input_data);			
				return (_state_flow){s_idle_prologue, NULL};
			}
		}
		
		
		switch (f_sm_read_event())
		{
			case e_error:
				return (_state_flow){e_error, NULL};
				break;
			case e_button_encb:
				break;
			default:
				i--;
		}
	}
	Input_data->structure.timer_page_counter = 1;
	Input_data->structure.timer_lock_counter = 4*20;
	return(_state_flow){s_work_watch, Input_data};
}

_state_flow ps_work_watch(_ps_work_data *Parameter)
{
	uint8_t soil_h;
	
	_rtc_timer timer_page;
	timer_page.counter = Parameter->timer_page_counter;
	_rtc_timer timer_value;
	timer_value.counter = 1;
	_rtc_timer timer_lock;
	timer_lock.counter = Parameter->timer_lock_counter;
	_rtc_timer timer_check;
	timer_check.counter = 4 * Parameter->time_check_set;
	
	bool soil_page = false;
	
	while(1)
	{
		while(!g_event_flag)
		{
			if(f_rtc_timer_countdown(12, &timer_page))
			{
				f_gui_dsp_next_page();
				soil_page ^= 1;
			}
			
			if(f_rtc_timer_countdown(1, &timer_value))
			{
				f_gui_update_page_value();
				
				if(soil_page)
				{
					f_gui_dsp_soil_h(Parameter->hg_set, soil_target);
				}
				else
				{
					soil_h = f_hg_get_soil_humidity();
					f_gui_dsp_soil_h(soil_h, soil_current);
				}
				
			}
			if(f_rtc_timer_countdown(4 * Parameter->time_check_set, &timer_check))
			{
				Parameter->timer_page_counter = timer_page.counter;
				Parameter->timer_lock_counter = timer_lock.counter;
				return (_state_flow){s_work_check, Parameter};
			}
			
			if(!g_hmi_lock_buttons)
			{
				if(f_rtc_timer_countdown(80, &timer_lock))
				{
					f_gui_backlight_off();
					g_hmi_lock_buttons = true;
					
				}
			}
			_delay_ms(100);
		}
			
		
		switch(f_sm_read_event())
		{
			case e_error:
				return (_state_flow){s_error, NULL};
			case e_button_encb:
				g_hmi_lock_buttons = false;
				f_gui_backlight_on();
				break;
			case e_button_encb_long:
				free_mine(Parameter);
				return (_state_flow){s_idle_prologue, NULL};
			default:
				break;
		}
	}
}

_state_flow ps_work_check(_ps_work_data *Parameter)
{
	const __flash char *entry = PSTR("Watering");
	
	uint8_t soil_h = f_hg_get_soil_humidity();
	
	f_hmi_toggle_led(led_a, hmi_one_impulse);
	
	if(soil_h < Parameter->hg_set)
	{
		_rtc_timer timer_water;
		timer_water.counter = 4 * Parameter->time_on_set;
		
		f_lcd_clear_segment(right_down);
		f_gui_dsp_text(gui_down_a, entry);
		
		f_hmi_toggle_led(led_a, hmi_blink);
		CLR(PORT, GL_PMPB);
		
		
		while (!f_rtc_timer_countdown(0, &timer_water))
		;
		
		_log_data log_info = {g_rtc_timestamp, lt_data, ld_watering};
		f_log_append_entry(&log_info);
		SET(PORT, GL_PMPB);
		f_hmi_toggle_led(led_a, hmi_off);
	}
	
	return (_state_flow){s_work_watch, Parameter};
}

_state_flow ps_fini()
{
	f_hmi_toggle_buzzer(hmi_on);
	_delay_ms(1500);
	SET(PORT, OUT_POFF);
	while(1);
	return (_state_flow){s_error, NULL};
}
