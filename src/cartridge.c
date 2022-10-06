#include <stdio.h>
#include <stdlib.h>

#include "cartridge.h"
#include "component.h"
#include "error.h"
#include "memory.h"

int cartridge_init_from_file(component_t* c, const char* filename){
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE_NON_NULL(filename);

	FILE* input = fopen(filename, "r");
	M_EXIT_IF(input == NULL, ERR_IO, "Impossible de lire le fichier %s", filename);
	
	int index = 0;
	while(!feof(input) && !ferror(input) && index < BANK_ROM_SIZE){
		fscanf(input, "%c", &c->mem->memory[index]);
		index++;
	}
	
	if(c->mem->memory[CARTRIDGE_TYPE_ADDR] != 0){
		fclose(input);
		return ERR_NOT_IMPLEMENTED;
	}
	
	fclose(input);

	return ERR_NONE;
}

int cartridge_init(cartridge_t* ct, const char* filename){
	M_REQUIRE_NON_NULL(ct);	
	M_REQUIRE_NON_NULL(filename);
	M_EXIT_IF_ERR(component_create(&ct->c, BANK_ROM_SIZE)); ///////////////////////////////////////////
	
	return cartridge_init_from_file(&ct->c, filename);
}

int cartridge_plug(cartridge_t* ct, bus_t bus){
	M_REQUIRE_NON_NULL(ct);
	return bus_forced_plug(bus, &ct->c, BANK_ROM0_START, BANK_ROM1_END, BANK_ROM0_START);
}

void cartridge_free(cartridge_t* ct){ //component_free ->invalid free()
	if(ct != NULL){
		ct->c.mem = NULL;
		ct = NULL;
	}
}
	
