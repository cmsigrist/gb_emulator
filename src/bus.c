#include <stdio.h>
#include "bus.h"
#include "error.h"
#include "bit.h"

#define DEFAULT_VALUE 0xFF
#define MEMORY_MAP_END 0xFFFF
#define INIT_VALUE 0
#define next 1

int bus_remap(bus_t bus, component_t* c, addr_t offset) {
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE_NON_NULL(bus);
	M_REQUIRE_NON_NULL(c->mem);
	M_REQUIRE_NON_NULL(c->mem->memory);
	
	if(c->start > c->end) {
		return ERR_BAD_PARAMETER;
	}
	if(c->end - c->start + offset > c->mem->size) {
		return ERR_ADDRESS;
	} else {
		for(int i = 0; i <= c->end - c->start; ++i) {
			bus[i + c->start] = &(c->mem->memory[i + offset]);
		}
		return ERR_NONE;
	}

}

int bus_forced_plug(bus_t bus, component_t* c, addr_t start, addr_t end, addr_t offset) {
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE_NON_NULL(c->mem);
	M_REQUIRE_NON_NULL(bus);
	if(end - start + offset > c->mem->size) {
		return ERR_ADDRESS;
	} else {
		c->start = start;
		c->end = end;
		return bus_remap(bus, c, offset);
	}
}

int bus_plug(bus_t bus, component_t* c, addr_t start, addr_t end) {
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE_NON_NULL(bus);
	
	for(int i = start; i <= end; ++i) {
		if(bus[i] != NULL) {
			return ERR_ADDRESS;
		}
	}
	if(c->mem != NULL) {
		if(end - start > c->mem->size || end <= start) {
			c->start = INIT_VALUE;
			c->end = INIT_VALUE;
			return ERR_ADDRESS;
		} else {
			c->start = start;
			c->end = end;
			bus_remap(bus, c, INIT_VALUE);
			return ERR_NONE;
		}
	} else {
		if(end <= start) {
			c->start = INIT_VALUE;
			c->end = INIT_VALUE;
			return ERR_BAD_PARAMETER;
		} else {	
			c->start = start;
			c->end = end;	
			return ERR_NONE;
		}
	}
}

int bus_unplug(bus_t bus, component_t* c) {
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE_NON_NULL(bus);
	
	for(int i = c->start; i <= c->end; ++i) {
		bus[i] = NULL;
	}
	c->start = INIT_VALUE;
	c->end = INIT_VALUE;
	return ERR_NONE;
}

int bus_read(const bus_t bus, addr_t address, data_t* data) {
	M_REQUIRE_NON_NULL(data);
	M_REQUIRE_NON_NULL(bus);

	if(bus[address] == NULL) {
		*data = DEFAULT_VALUE;
		return ERR_NONE;
	} else {
		*data = *bus[address];
		return ERR_NONE;
	}	
}

int bus_read16(const bus_t bus, addr_t address, addr_t* data16) {
	M_REQUIRE_NON_NULL(data16);
	M_REQUIRE_NON_NULL(bus);
	
	if(bus[address] == NULL || bus[address + next] == NULL || address == MEMORY_MAP_END) {
		*data16 = DEFAULT_VALUE;
		return ERR_NONE;
	} else {
		*data16 = merge8(*bus[address], *bus[address + next]);
		return ERR_NONE;
	}
}

int bus_write(bus_t bus, addr_t address, data_t data) {
	M_REQUIRE_NON_NULL(bus[address]);
	M_REQUIRE_NON_NULL(bus);
	#ifdef TETRIS
		if(address < 0x8000) {
			return ERR_BAD_PARAMETER;
		}
	#endif
	*bus[address] = data;
	return ERR_NONE;
}

int bus_write16(bus_t bus, addr_t address, addr_t data16) {
	M_REQUIRE_NON_NULL(bus);
	M_REQUIRE_NON_NULL(bus[address]);
	M_REQUIRE_NON_NULL(bus[address + next]);
	
	*bus[address] = lsb8(data16);
	*bus[address + next] = msb8(data16);
	return ERR_NONE;
}
