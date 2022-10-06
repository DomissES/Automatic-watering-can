/*
 * memory.c
 *
 * Created: 30.08.2022 20:10:50
 *  Author: domis
 */ 


#include <stdlib.h>
#include <util/atomic.h>
#include "memory.h"
#include "system.h"

uint16_t *stack_ptr = (uint16_t*)0x5D;


uint16_t f_memory_check_free_ram()
{
	uint16_t heap_end = (uint16_t)__brkval == 0? (uint16_t)&__heap_start : (uint16_t)__brkval;
	uint16_t local_variable = ((uint16_t)&local_variable - heap_end);
	return local_variable;
}

bool f_memory_check_if_ram_isOk_task()
{
	bool isOk = true;
	uint8_t free_ram = f_memory_check_free_ram();
	if(free_ram < MEMORY_LOW_LIMIT)
	{
		f_sys_report_error(err_memory_low, 1);
		isOk = false;
	}
	
	return isOk;
}

void * malloc_mine(size_t size)
{
	void *ptr;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		ptr = malloc(size);
	}
	if(ptr == NULL) f_sys_report_error(err_malloc, 0);
	
	return ptr;
}

void free_mine(void *ptr)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		free(ptr);
	}
}
