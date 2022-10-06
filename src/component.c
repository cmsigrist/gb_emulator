#include <stdio.h>
#include <stdlib.h>

#include "component.h"
#include "error.h"
#include "memory.h"

#define INIT_VALUE 0

int component_create(component_t* c, size_t mem_size) {
	M_REQUIRE_NON_NULL(c);
	c->start = INIT_VALUE;
	c->end = INIT_VALUE;	
	if(mem_size > 0) { 
		c->mem = calloc(1, sizeof(memory_t));
		if(c->mem != NULL) {
			int check = mem_create(c->mem, mem_size);
			if(check != ERR_NONE) {
				mem_free(c->mem);
				free(c->mem);
				c->mem = NULL;
				M_EXIT_ERR(check, "%s", "impossible de creer le composant");
			}
		}
	} else {
		c->mem = NULL;
	}
	return ERR_NONE;
}

void component_free(component_t* c) {
	if(c != NULL) {
		if(c->mem != NULL) {
			mem_free(c->mem);
			free(c->mem);
			c->mem = NULL;
		}
		c->start = INIT_VALUE;
		c->end = INIT_VALUE;
		c = NULL;
	}
}

int component_shared(component_t* c, component_t* c_old) {
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE_NON_NULL(c_old);
	M_REQUIRE_NON_NULL(c_old->mem);
	M_REQUIRE_NON_NULL(c_old->mem->memory);

	c->start = INIT_VALUE;
	c->end = INIT_VALUE;
	c->mem = c_old->mem;
	return ERR_NONE;
}
