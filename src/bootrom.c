#include <stdio.h>
#include <stdlib.h>

#include "bootrom.h"
#include "error.h"
#include "component.h"
#include "memory.h"
#include "cartridge.h"


int bootrom_init(component_t* c){
	M_REQUIRE_NON_NULL(c);

	M_EXIT_IF_ERR(component_create(c, MEM_SIZE(BOOT_ROM)));
	
	data_t content[MEM_SIZE(BOOT_ROM)] = GAMEBOY_BOOT_ROM_CONTENT; 
	for(int i = 0; i < MEM_SIZE(BOOT_ROM); ++i) {
		c->mem->memory[i] = content[i];
	}
	
	return ERR_NONE;
}

int bootrom_bus_listener(gameboy_t* gameboy, addr_t addr){
	M_REQUIRE_NON_NULL(gameboy);

	if(gameboy->boot != 0 && (addr == REG_BOOT_ROM_DISABLE)){
		M_EXIT_IF_ERR(bus_unplug(gameboy->bus, &gameboy->bootrom));
		M_EXIT_IF_ERR(cartridge_plug(&gameboy->cartridge, gameboy->bus));
		gameboy->boot = 0;		
	}
	
	return ERR_NONE;
}


