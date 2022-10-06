#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "error.h"

#define INIT_VALUE 0

int mem_create(memory_t* mem, size_t size) {
	M_REQUIRE_NON_NULL(mem);	

	if(size == 0) {
		return ERR_MEM;
	} else {
		if((mem->memory = calloc(1, size * sizeof(data_t))) == NULL) {
			free(mem->memory);
			return ERR_MEM;
		}
		mem->size = size;
		return ERR_NONE;
	}
}

void mem_free(memory_t* mem) {
	if(mem != NULL) {
		if(mem->memory != NULL) {
			free(mem->memory);
			mem->memory = NULL;
		}
		mem->size = INIT_VALUE;
		mem = NULL;
	}
}
