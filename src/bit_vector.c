#include <stdlib.h>
#include <stdio.h>

#include "bit_vector.h"
#include "bit.h"
#include "image.h"
#include "error.h"

#define TRUE 1
#define FALSE 0

/**
 * @brief Given a size return the numbre of content[i] in a vector of size size
 * 
 */
int get_size(size_t size) {
	int length = size / IMAGE_LINE_WORD_BITS;
	if(size % IMAGE_LINE_WORD_BITS != 0) {
		length += 1;
	}
	return length;
}


bit_vector_t* bit_vector_create(size_t size, bit_t value){
	bit_vector_t* vector = calloc(1, sizeof(bit_vector_t));
	if(vector != NULL && (int) size > 0){
		int length = get_size(size);
		vector->size = size;
		vector->content = calloc(length, sizeof(uint32_t));
		if(vector->content != NULL) {
			if(value != 0) {
				for(int i = 0; i < size; i++) {
					vector->content[i / IMAGE_LINE_WORD_BITS] =
						vector->content[i / IMAGE_LINE_WORD_BITS] | (1 << i % IMAGE_LINE_WORD_BITS);
				}
			}
			return vector;
		} else {
			fprintf(stderr, "Impossible d'initialiser le contentu du vecteur\n");
			free(vector->content);
			vector->content = NULL;
			free(vector);
			return NULL;
		}
	} else {
		fprintf(stderr, "Impossible d'initialiser le vecteur\n");
		free(vector);
		return NULL;
	}
}

bit_vector_t* bit_vector_cpy(const bit_vector_t* pbv){
	if(pbv != NULL) {
		bit_vector_t* copy = calloc(1, sizeof(bit_vector_t));
		if(copy != NULL){
			copy->size = pbv->size;
			size_t length = get_size(copy->size);
			copy->content = calloc(length, sizeof(uint32_t));
			if(copy->content != NULL) {
				for(int i = 0; i < length; i++) {
					copy->content[i] = pbv->content[i];
				}
				return copy;
			} else {
				free(copy->content);
				copy->content = NULL;
				free(copy);
				return NULL;
			}
		} else {
			free(copy);
			return NULL;
		}
	} else {
		return NULL;
	}
}

bit_t bit_vector_get(const bit_vector_t* pbv, size_t index){
	if(pbv != NULL && index >= 0 && index < pbv->size){ 
		int i = index / IMAGE_LINE_WORD_BITS;
		int j = index % IMAGE_LINE_WORD_BITS;
		return ((pbv->content[i] >> j) & 1);
	} else {
		return 0;
	}
}

bit_vector_t* bit_vector_not(bit_vector_t* pbv) {
	if(pbv != NULL) {
		int length = get_size(pbv->size);
		for(int i = 0; i < length; i++) {
			pbv->content[i] = ~pbv->content[i];
		}
		
		if(pbv->size % IMAGE_LINE_WORD_BITS != 0) {
			bit_vector_shift(pbv, pbv->size % IMAGE_LINE_WORD_BITS);
			bit_vector_shift(pbv, -(pbv->size % IMAGE_LINE_WORD_BITS));
		}
		return pbv;
	} else {
		return NULL;
	}
}

bit_vector_t* bit_vector_and(bit_vector_t* pbv1, const bit_vector_t* pbv2) {
	if(pbv1 != NULL && pbv2 != NULL) {
		int s1 = get_size(pbv1->size);
		int s2 = get_size(pbv2->size);
		if(s1 == s2) {
			int length = get_size(pbv1->size);
			for(int i = 0; i < length; i++) {
				pbv1->content[i] = pbv1->content[i] & pbv2->content[i];
			}
			return pbv1;
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

bit_vector_t* bit_vector_or(bit_vector_t* pbv1, const bit_vector_t* pbv2) {
	if(pbv1 != NULL && pbv2 != NULL) {
		int s1 = get_size(pbv1->size);
		int s2 = get_size(pbv2->size);
		if(s1 == s2) {
			int length = get_size(pbv1->size);
			for(int i = 0; i < length; i++) {
				pbv1->content[i] = pbv1->content[i] | pbv2->content[i];
			}
			return pbv1;
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

bit_vector_t* bit_vector_xor(bit_vector_t* pbv1, const bit_vector_t* pbv2) {
	if(pbv1 != NULL && pbv2 != NULL) {
		int s1 = get_size(pbv1->size);
		int s2 = get_size(pbv2->size);
		if(s1 == s2) {
			int length = get_size(pbv1->size);
			for(int i = 0; i < length; i++) {
				pbv1->content[i] = pbv1->content[i] ^ pbv2->content[i];
			}
			return pbv1;
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

/**
 * @brief helper function for bit_vector_extract_zero_ext 
 * 						  and bit_vector_extract_wrap_ext
 * 	zero = TRUE for extract_zero_ext
 * 	zero = FALSE for extract_wrap_ext
 * */

bit_vector_t* extract(const bit_vector_t* pbv, int64_t index, size_t size, bit_t zero) {
	bit_vector_t* new_vector = bit_vector_create(size, 0);
	if(new_vector == NULL) {
		return NULL;
	}
	for(int i = 0; i < size; i++) {
		if((i + index >= 0 && i + index < pbv->size && zero == TRUE) || zero == FALSE ) {
			new_vector->content[i / IMAGE_LINE_WORD_BITS] |= 
				((pbv->content[((i + index) % pbv->size) / IMAGE_LINE_WORD_BITS] >> 
					((i + index) % pbv->size)) & 1) << (i % IMAGE_LINE_WORD_BITS);
		}
	}
	return new_vector;
}


bit_vector_t* bit_vector_extract_zero_ext(const bit_vector_t* pbv, int64_t index, size_t size) {
	if(pbv != NULL) {
		if(size == 0){
			return NULL;
		} else {
			return extract(pbv, index, size, TRUE);
		}
	} else {
		return bit_vector_create(size, 0);
	}
}


bit_vector_t* bit_vector_extract_wrap_ext(const bit_vector_t* pbv, int64_t index, size_t size) {
	if(pbv != NULL) {
		if(size == 0){
			return NULL;
		} else {
			return extract(pbv, index, size, FALSE);
		}
	} else {
		return NULL;
	}
}



bit_vector_t* bit_vector_shift(const bit_vector_t* pbv, int64_t shift) {
	if(pbv != NULL) {
		return bit_vector_extract_zero_ext(pbv, -shift, pbv->size);
	} else {
		return NULL;
	}
}

bit_vector_t* bit_vector_join(const bit_vector_t* pbv1, const bit_vector_t* pbv2, int64_t shift) {
	if(pbv1 != NULL && pbv2 != NULL && pbv1->size == pbv2->size && shift >= 0 && shift <= pbv1->size) {	
		bit_vector_t* join = bit_vector_create(pbv1->size, 0);
		
		for(int i = 0; i < pbv1->size; i++) {
			if(i < shift) {
				join->content[i / IMAGE_LINE_WORD_BITS] |= ((pbv1->content[i / IMAGE_LINE_WORD_BITS] 
																>> (i % IMAGE_LINE_WORD_BITS)) & 1) << (i % IMAGE_LINE_WORD_BITS);
			} else {
				join->content[i / IMAGE_LINE_WORD_BITS] |= ((pbv2->content[i / IMAGE_LINE_WORD_BITS] 
																>> (i % IMAGE_LINE_WORD_BITS)) & 1) << (i % IMAGE_LINE_WORD_BITS);
			}
		}
		return join;
	} else {
		return NULL;
	}
}

int bit_vector_print(const bit_vector_t* pbv) {
	int length = get_size(pbv->size);
	for(int i = length - 1; i >= 0; i--) {
		for(int j = 31; j >= 0; j--) {
			printf("%s", ((pbv->content[i] >> j) & 1) == 1 ? "1" : "0");
		}
	}
	return pbv->size;
}

int bit_vector_println(const char* prefix, const bit_vector_t* pbv) {
	int count = printf("%s ", prefix);
	count += bit_vector_print(pbv);
	printf("\n");
	return ++count;
}

void bit_vector_free(bit_vector_t** pbv) {
	free((*pbv)->content);
	(*pbv)->content = NULL;
	free(*pbv);
	*pbv = NULL;
}
