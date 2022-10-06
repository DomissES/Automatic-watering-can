/*
 * hmi.c
 *
 * Created: 02.09.2022 13:18:04
 *  Author: domis
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "hmi.h"
#include "defines.h"
#include "state_machine.h"

#define DEBOUNCE_TIME 2

volatile struct _button{
	uint8_t state;
	uint8_t is_pressed;
	uint8_t pressed_seconds[3];
	uint8_t debounce_wait[3];
	} Hmi_buttons;

typedef const struct {
	uint8_t array_i;
	uint8_t int_enable;
	uint8_t reg_bit;
	volatile uint8_t *reg_adr;
	}_button_register;

volatile struct _buzzzer{
	uint8_t tone;
	uint8_t sequence;
	uint8_t counter;
	} Hmi_buzzer;

volatile struct _leds{
	uint8_t set_sequence;
	uint8_t act_sequence;
	uint8_t counter;
	} Hmi_leds[3];

bool g_hmi_lock_buttons;

/* funkcja malo czytelna, wymaga opisu:
	funkcja przyjmuje jako parametr strukture zawierajaca adresy rejestrow
	ktore nalezy zmienic, parametry wyzej zdefiniowane
*/
static bool f_hmi_button_interrupt_routine(_button_register *b)
{
	bool is_event = false;
	if(*b->reg_adr & _BV(b->reg_bit)) //jesli zbocze narastajace
	{
		if(Hmi_buttons.debounce_wait[b->array_i] == 0)
		{
			Hmi_buttons.pressed_seconds[b->array_i] = 0; //resetuj czas
			Hmi_buttons.state |= 1 << b->array_i; //przycisk jest wcisniety
			
			EIMSK &= ~_BV(b->int_enable); //zablokuj przerwanie
			*b->reg_adr &= ~_BV(b->reg_bit); //zmien na zbocze opadajace
			asm volatile("nop"::);
			EIMSK |= _BV(b->int_enable);
		}
	}
	else
	{
		Hmi_buttons.state &= ~(1 << b->array_i);
		Hmi_buttons.is_pressed |= 1 << b->array_i; //przycisk byl wcisniety
		
		Hmi_buttons.debounce_wait[b->array_i] = DEBOUNCE_TIME;
		
		EIMSK &= ~_BV(b->int_enable);
		*b->reg_adr |= _BV(b->reg_bit); //zmieñ na zbocze rosn¹ce
		asm volatile("nop"::);
		EIMSK |= _BV(b->int_enable);
		is_event = true;
	}
	return is_event;
}

ISR(INT2_vect) //INB_ENCB
{
	_button_register Button_ENCB = {0, INT2, ISC20, &EICRA};
	bool is_event = f_hmi_button_interrupt_routine(&Button_ENCB);
	if(is_event) f_sm_report_event();
}

ISR(INT6_vect) //INB_STA
{
	_button_register Button_STA = {1, INT6, ISC60, &EICRB};
	bool is_event = f_hmi_button_interrupt_routine(&Button_STA);
	if(is_event) f_sm_report_event();
}

ISR(INT7_vect) //INB_STB
{
	_button_register Button_STB = {2, INT7, ISC70, &EICRB};
	bool is_event = f_hmi_button_interrupt_routine(&Button_STB);
	if(is_event) f_sm_report_event();
}

void f_init_hmi_buttons()
{
	EICRA = 0b00110000; //0x30
	EICRB = 0b11110000; //0xc0 //0x30
	EIMSK = 0b11000100;
}

void f_hmi_update_buttons_task()
{
	if(!g_hmi_lock_buttons)
	{
		static uint8_t counter[3];
		uint8_t adr = 0x01;
		
		for(uint8_t i = 0; i < 3; i++)
		{
			if(Hmi_buttons.debounce_wait[i] != 0) Hmi_buttons.debounce_wait[i]--;
			
			if(Hmi_buttons.state & adr) counter[i]++;
			if(counter[i] == 16)
			{
				f_hmi_toggle_buzzer(hmi_one_impulse);
				Hmi_buttons.pressed_seconds[i]++;
				if(Hmi_buttons.pressed_seconds[b_encb] >= 3)
				{
					f_hmi_toggle_buzzer(hmi_two_impulse);
					f_sm_report_event();
				}
				counter[i] = 0;
			}
			adr <<= 1;
		}
	}
	
}

bool f_hmi_read_button(_hmi_button button)
{
	bool click = false;
	if(Hmi_buttons.is_pressed & (1 << button))
	{
		if(!g_hmi_lock_buttons || (button == b_encb))
		{
			f_hmi_toggle_buzzer(hmi_one_impulse);
			click = true;
		}
		Hmi_buttons.is_pressed &= ~(1 << button);
	}
	return click;
}

uint8_t f_hmi_get_pressed_seconds(_hmi_button button)
{
	return Hmi_buttons.pressed_seconds[button];
}

void f_hmi_toggle_buzzer(_hmi_sequence tone)
{
	Hmi_buzzer.sequence = tone;
	Hmi_buzzer.tone = tone;
	Hmi_buzzer.counter = 0;
}

void f_hmi_update_buzzer_task()
{
	if(Hmi_buzzer.counter == 0)
	{
		if(Hmi_buzzer.tone & 0xF0) Hmi_buzzer.sequence = Hmi_buzzer.tone;
		Hmi_buzzer.counter = 4;
	}
	else
	{
		if(Hmi_buzzer.sequence & 0x01) SET(PORT, OUT_BUZZ);
		else CLR(PORT, OUT_BUZZ);
		Hmi_buzzer.sequence >>= 1;
		Hmi_buzzer.counter--;
	}
	
}

void f_hmi_toggle_led(_hmi_led led, _hmi_sequence sequence)
{
	Hmi_leds[led].set_sequence = sequence;
	Hmi_leds[led].counter = 0; 
}

static inline void f_hmi_change_led_status(_hmi_led led, bool status)
{
	switch(led)
	{
		case led_a:
			if(status) CLR(PORT, LED_WRKA);
			else SET(PORT, LED_WRKA);
			break;
		case led_b:
			if(status) CLR(PORT, LED_WRKB);
			else SET(PORT, LED_WRKB);
			break;
		case led_error:
			if(status) CLR(PORT, LED_ERR);
			else SET(PORT, LED_ERR);
			break;
		default:
			break;
	}
}

void f_hmi_update_led_task()
{
	for (_hmi_led led = 0; led < 3; led++)
	{
		if(Hmi_leds[led].counter == 0)
		{
			if(Hmi_leds[led].set_sequence & 0xF0) Hmi_leds[led].act_sequence = Hmi_leds[led].set_sequence;
			Hmi_leds[led].counter = 4;
		}
		else
		{
			f_hmi_change_led_status(led, (Hmi_leds[led].act_sequence & 0x01));
			Hmi_leds[led].act_sequence >>= 1;
			Hmi_leds[led].counter--;
		}
	}
}

static inline int8_t f_hmi_get_encoder()
{
	if(!g_hmi_lock_buttons)
	{
		static int8_t last_state;
		int8_t state, has_changed;
		static int8_t delta;
		
		state = 0;
		if(GET(INB_ENC_A)) state = 3; //0b11
		if(GET(INB_ENC_B)) state ^= 1; //0b01
		has_changed = last_state - state;
		if(has_changed & 0x01)
		{
			last_state = state;
			delta += (has_changed & 2 ) - 1;
		}
		int8_t val = delta;
		delta = delta & 0x03; //maska dla delty, gdyby cos sie znalazlo na wyzszych bitach
		return val >> 2;
	}
	return 0;
}

int16_t f_hmi_encoder_change_value(int16_t min, int16_t max, uint8_t step, int16_t current_value)
{
	switch(f_hmi_get_encoder())
	{
		case -1:
			if(current_value > min) current_value -= step;
			if(current_value < min) current_value = min;
			break;
		case 0:
			break;
		case 1:
			if(current_value < max) current_value += step;
			if(current_value > max) current_value = max;
			break;
	}
	
	return current_value;
}