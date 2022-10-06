/*
 * memory.h
 *
 * Created: 24.08.2022 23:33:29
 *  Author: domis
 */ 


#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define MEMORY_LOW_LIMIT 1024

//link do strony, jak sa ulozone ponizsze zmienne
//https://www.nongnu.org/avr-libc/user-manual/malloc.html

extern uint16_t __data_start;
extern uint16_t __data_end;
extern uint16_t __bss_start;
extern uint16_t __bss_end;
extern uint16_t __heap_start;
extern uint16_t __heap_end; //tylko do zainicjalizowania na samym poczatku!
extern uint8_t * __brkval;
extern uint16_t * stack_p;


uint16_t f_memory_check_free_ram();
bool f_memory_check_if_ram_isOk_task();
void * malloc_mine(size_t size);
void free_mine(void *ptr);

inline uint16_t f_memory_check_stack_size()
{
	uint8_t local_variable;
	return (uint16_t)&local_variable - (uint16_t)*stack_p;
}
inline uint16_t f_memory_check_heap_size()
{
	return (uint16_t)(__brkval - (uint16_t)&__heap_start);
}


#endif /* MEMORY_H_ */