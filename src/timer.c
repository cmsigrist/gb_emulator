#include <stdlib.h>
#include <stdio.h>

#include "timer.h"
#include "cpu.h"
#include "error.h"
#include "cpu-storage.h"
#include "bit.h"
#include "memory.h"

#define INIT_VALUE 0
#define mask 3
#define TIMER_INC 4

int timer_init(gbtimer_t* timer, cpu_t* cpu){
	M_REQUIRE_NON_NULL(timer);
	M_REQUIRE_NON_NULL(cpu);

	timer->cpu = cpu;
	timer->counter = INIT_VALUE;

	return ERR_NONE;
}

int timer_cycle(gbtimer_t* timer){
	M_REQUIRE_NON_NULL(timer);
	
	bit_t state = timer_state(timer);
	timer->counter += TIMER_INC;
	
	cpu_write_at_idx(timer->cpu, REG_DIV, msb8(timer->counter));
	timer_inc_if_state_change(timer, state);
	
	return ERR_NONE;
}

int timer_bus_listener(gbtimer_t* timer, addr_t addr){
	M_REQUIRE_NON_NULL(timer);
	bit_t state = timer_state(timer);
	if(addr == REG_DIV){
		timer->counter = 0;
	}
	if(addr == REG_TAC || addr == REG_DIV){
		timer_inc_if_state_change(timer, state);
	}
	
	return ERR_NONE;
}

bit_t timer_state(gbtimer_t* timer){
	int used_bit;
	
	uint8_t tac = cpu_read_at_idx(timer->cpu, REG_TAC);
	switch (tac & mask){ 
		case 0 : used_bit = 9;
		break;
		case 1 : used_bit = 3;
		break;
		case 2 : used_bit = 5;
		break;
		case 3 : used_bit = 7;
		break;
	}

	return bit_get(tac, 2) & ((timer->counter >> used_bit) & 1);
}

int timer_inc_if_state_change(gbtimer_t* timer, bit_t old_state){
	M_REQUIRE_NON_NULL(timer);
	if(old_state == 1 && timer_state(timer) == 0){
		uint8_t tima = cpu_read_at_idx(timer->cpu, REG_TIMA);
		if(tima == 0xFF){
			uint8_t tma = cpu_read_at_idx(timer->cpu, REG_TMA);
			cpu_write_at_idx(timer->cpu, REG_TIMA, tma);
			cpu_request_interrupt(timer->cpu, TIMER);
		} else {
			cpu_write16_at_idx(timer->cpu, REG_TIMA, ++tima);
		}
	}
	return ERR_NONE;
}
