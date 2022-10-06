#include <stdio.h>
#include <stdlib.h>
#include "gameboy.h"
#include "error.h"
#include "bus.h"
#include "bootrom.h"
#include "timer.h"
#include "cpu-storage.h"

#define INIT_VALUE 0

#define W_RAM 0
#define REG 1
#define E_RAM 2
#define V_RAM 3
#define G_RAM 4
#define U 5
#define BOOT_INIT 1


#ifdef BLARGG
static int blargg_bus_listener(gameboy_t* gameboy, addr_t addr) {
	M_REQUIRE_NON_NULL(gameboy);
	if(addr == BLARGG_REG){
		printf("%c", cpu_read_at_idx(&(gameboy->cpu), addr));
	}
	return ERR_NONE;
}
#endif

int gameboy_create(gameboy_t* gameboy, const char* filename) {
	M_REQUIRE_NON_NULL(gameboy);
	//gameboy->cycles = INIT_VALUE;
	gameboy->cycles = 1;
	gameboy->nb_components = 0;
	
	for(int i = 0; i < BUS_SIZE; ++i) {
		gameboy->bus[i] = NULL;
	}
	M_EXIT_IF_ERR(component_create(&gameboy->components[W_RAM], MEM_SIZE(WORK_RAM)));
	M_EXIT_IF_ERR(component_create(&gameboy->components[REG], MEM_SIZE(REGISTERS)));
	M_EXIT_IF_ERR(component_create(&gameboy->components[E_RAM], MEM_SIZE(EXTERN_RAM)));
	M_EXIT_IF_ERR(component_create(&gameboy->components[V_RAM], MEM_SIZE(VIDEO_RAM)));
	M_EXIT_IF_ERR(component_create(&gameboy->components[G_RAM], MEM_SIZE(GRAPH_RAM)));
	M_EXIT_IF_ERR(component_create(&gameboy->components[U], MEM_SIZE(USELESS)));
	gameboy->nb_components = GB_NB_COMPONENTS;
	
	component_t echo;
	M_EXIT_IF_ERR(component_create(&echo, MEM_SIZE(ECHO_RAM))); /////////////////////////////////////////////
	M_EXIT_IF_ERR(bus_plug(gameboy->bus, &gameboy->components[W_RAM], WORK_RAM_START, WORK_RAM_END));
	M_EXIT_IF_ERR(bus_plug(gameboy->bus, &gameboy->components[REG], REGISTERS_START, REGISTERS_END));
	M_EXIT_IF_ERR(bus_plug(gameboy->bus, &gameboy->components[E_RAM], EXTERN_RAM_START, EXTERN_RAM_END));
	M_EXIT_IF_ERR(bus_plug(gameboy->bus, &gameboy->components[V_RAM], VIDEO_RAM_START, VIDEO_RAM_END));				
	M_EXIT_IF_ERR(bus_plug(gameboy->bus, &gameboy->components[G_RAM], GRAPH_RAM_START, GRAPH_RAM_END));		
	M_EXIT_IF_ERR(bus_plug(gameboy->bus, &gameboy->components[U], USELESS_START, USELESS_END));			
	M_EXIT_IF_ERR(component_shared(&echo, &gameboy->components[W_RAM]));			
	M_EXIT_IF_ERR(bus_plug(gameboy->bus, &echo, ECHO_RAM_START, ECHO_RAM_END));
	
	M_EXIT_IF_ERR(cpu_init(&gameboy->cpu));
	
	gameboy->boot = BOOT_INIT;		
		
	M_EXIT_IF_ERR(timer_init(&gameboy->timer, &gameboy->cpu));	
	M_EXIT_IF_ERR(cartridge_init(&gameboy->cartridge, filename));
	M_EXIT_IF_ERR(cartridge_plug(&gameboy->cartridge, gameboy->bus));
	M_EXIT_IF_ERR(bootrom_init(&gameboy->bootrom));
	M_EXIT_IF_ERR(bootrom_plug(&gameboy->bootrom, gameboy->bus));
	M_EXIT_IF_ERR(cpu_plug(&gameboy->cpu, &gameboy->bus));
	
	M_EXIT_IF_ERR(lcdc_init(gameboy)); ////////////////////////////////////////////////////////////////////
	M_EXIT_IF_ERR(lcdc_plug(&gameboy->screen, gameboy->bus));
	M_EXIT_IF_ERR(joypad_init_and_plug(&gameboy->pad, &gameboy->cpu));

	return ERR_NONE;
}

void gameboy_free(gameboy_t* gameboy) { 
	if(gameboy != NULL) {
		for(int i = 0; i < GB_NB_COMPONENTS; ++i) {
			bus_unplug(gameboy->bus, &gameboy->components[i]);
			component_free(&gameboy->components[i]);
		}
		cartridge_free(&gameboy->cartridge);
		component_free(&gameboy->bootrom);
		for(int i = ECHO_RAM_START; i < ECHO_RAM_END; i++) {
			gameboy->bus[i] = NULL;
		}
		cpu_free(&gameboy->cpu);
		component_free(&gameboy->bootrom);
		lcdc_free(&gameboy->screen);
		gameboy->timer.counter = 0;
		gameboy->nb_components = 0;
		gameboy = NULL;
	}
}

int gameboy_run_until(gameboy_t* gameboy, uint64_t cycle) {
	M_REQUIRE_NON_NULL(gameboy);
	while(gameboy->cycles < cycle) {	
		M_EXIT_IF_ERR(timer_cycle(&gameboy->timer));
		M_EXIT_IF_ERR(cpu_cycle(&gameboy->cpu));
		if(gameboy->screen.on) {
			M_EXIT_IF_ERR(lcdc_cycle(&gameboy->screen, gameboy->cycles));
		}
		
		M_EXIT_IF_ERR(timer_bus_listener(&gameboy->timer, gameboy->cpu.write_listener));		
		M_EXIT_IF_ERR(lcdc_bus_listener(&gameboy->screen, gameboy->cpu.write_listener));
		M_EXIT_IF_ERR(bootrom_bus_listener(gameboy, gameboy->cpu.write_listener));
		M_EXIT_IF_ERR(joypad_bus_listener(&gameboy->pad, gameboy->cpu.write_listener));
		
		(gameboy->cycles)++;
	
		#ifdef BLARGG
			M_EXIT_IF_ERR(blargg_bus_listener(gameboy, gameboy->cpu.write_listener));
		#endif
		
	}
	return ERR_NONE;
}
