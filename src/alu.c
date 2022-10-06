#include <stdio.h>
#include <stdint.h> 
#include "alu.h"
#include "bit.h"
#include "error.h"
//#include "tests.h"

#define Z 7
#define N 6
#define H 5
#define C 4

#define HC 0x30

#define lsbBit 0
#define msbBit 7

#define halfCarryBit 4
#define carryBit 8
#define carrySize 9
#define msbShift 7

#define one_shift 1

/**
 * @brief sets the Z flag if the given value is equal 0
 */
void set_Z_if_zero(alu_output_t* result) {
	if(result->value == 0) {
		set_Z(&(result->flags));
	}
}

/**
 * @brief sets the H and C flags if needed
 */
void set_H_and_C(alu_output_t* result, bit_t c[]) {
	if(c[halfCarryBit] != 0) {
		set_H(&(result->flags));
	} 
	if(c[carryBit] != 0) {
		set_C(&(result->flags));
	}
}

flag_bit_t get_flag(flags_t flags, flag_bit_t flag){
	switch(flag) {
		case(FLAG_Z): return bit_get(flags, Z)? FLAG_Z : 0;
		case(FLAG_N): return bit_get(flags, N)? FLAG_N : 0;
		case(FLAG_H): return bit_get(flags, H)? FLAG_H : 0;
		case(FLAG_C): return bit_get(flags, C)? FLAG_C : 0;
		default: return 0;
	}
}

void set_flag(flags_t* flags, flag_bit_t flag){	
	switch(flag) {
		case(FLAG_Z): bit_set(flags, Z); 
		break;
		case(FLAG_N): bit_set(flags, N);
		break;
		case(FLAG_H): bit_set(flags, H);
		break;
		case(FLAG_C): bit_set(flags, C);
		break;
		default:;
	}
}

int alu_add8(alu_output_t* result, uint8_t x, uint8_t y, bit_t c0){
	M_REQUIRE_NON_NULL(result);
	
	bit_t c[carrySize];
	c[0] = c0;
	
	uint8_t sumLSB = lsb4(x) + lsb4(y) + c0;
	uint8_t sumMSB = msb4(x) + msb4(y) + msb4(sumLSB);
	result->value = merge4(sumLSB, sumMSB);
	
	for(int i = 1; i < carrySize; ++i) {
		c[i] = bit_get((bit_get(x, i - 1) + bit_get(y, i - 1) + c[i - 1]), 1);
	}
	
	set_Z_if_zero(result);
	set_H_and_C(result, c);

	return ERR_NONE;
}

int alu_sub8(alu_output_t* result, uint8_t x, uint8_t y, bit_t b0){
	M_REQUIRE_NON_NULL(result);
	bit_t b[carrySize];
	b[0] = b0;
	
	set_N(&(result->flags));
	uint8_t subLSB = lsb4(x) - lsb4(y) - b0;
	uint8_t subMSB = msb4(x) - msb4(y) + msb4(subLSB);
	result->value = merge4(subLSB, subMSB);
	
	for(int i = 1; i < carrySize; ++i) {
		b[i] = ((uint8_t)(bit_get(x, i - 1) - bit_get(y, i - 1) - b[i - 1])) >> msbShift;
	}
	set_Z_if_zero(result);
	set_H_and_C(result, b);

	return ERR_NONE;
}

int alu_add16_low(alu_output_t* result, uint16_t x, uint16_t y) {
	M_REQUIRE_NON_NULL(result);
	
	alu_add8(result, lsb8(x), lsb8(y), 0);
	uint8_t sumLSB = result->value;
	
	bit_t c0 = 0;
	if(get_C(result->flags) == FLAG_C) {
		c0 = 1;
	}
	
	flags_t f = result->flags & HC;
	alu_add8(result, msb8(x), msb8(y), c0);
	uint8_t sumMSB = result->value;
	
	result->value = merge8(sumLSB, sumMSB);
	result->flags = f;

	set_Z_if_zero(result);
	
	return ERR_NONE;	
}

int alu_add16_high(alu_output_t* result, uint16_t x, uint16_t y) { ////////////////////////////////////////////////////////////
	M_REQUIRE_NON_NULL(result);	

	alu_add8(result, lsb8(x), lsb8(y), 0);
	uint8_t sumLSB = result->value;
	
	bit_t c0 = 0;
	if(get_C(result->flags) == FLAG_C) {
		c0 = 1;
	}
	result->flags = 0;
	alu_add8(result, msb8(x), msb8(y), c0);
	uint8_t sumMSB = result->value;
	result->flags = result->flags & HC;
	result->value = merge8(sumLSB, sumMSB);
	
	set_Z_if_zero(result);
		
	return ERR_NONE;	
}

int alu_shift(alu_output_t* result, uint8_t x, rot_dir_t dir) {
	M_REQUIRE_NON_NULL(result);
	M_EXIT_IF(dir < LEFT || dir > RIGHT, ERR_BAD_PARAMETER, "dir = %d doit etre égal a LEFT(0) ou RIGHT(1)", dir);
	if(dir == LEFT) {
		if(bit_get(x, msbBit) == 1) {
			set_C(&(result->flags));
		}
		bit_unset(&x, msbBit);
	} else {
		if(bit_get(x, lsbBit) == 1) {
			set_C(&(result->flags));
		}
		bit_unset(&x, lsbBit);
	}
	bit_rotate(&x, dir, one_shift);
	result->value = x;

	set_Z_if_zero(result);
	
	return ERR_NONE;
}

int alu_shiftR_A(alu_output_t* result, uint8_t x) {
	M_REQUIRE_NON_NULL(result);
	
	if(bit_get(x, lsbBit) == 1) {
		set_C(&(result->flags));
	}
	bit_t mask = 0;
	if(bit_get(x, msbBit) == 1) {
		mask = 1;
	}
	bit_rotate(&x, RIGHT, one_shift);
	if(mask == 1) {
		bit_set(&x, msbBit);
	} else {
		bit_unset(&x, msbBit);
	}	
	result->value = x;

	set_Z_if_zero(result);
	
	return ERR_NONE;
}

int alu_rotate(alu_output_t* result, uint8_t x, rot_dir_t dir) {
	M_REQUIRE_NON_NULL(result);
	M_EXIT_IF(dir < LEFT || dir > RIGHT, ERR_BAD_PARAMETER, "dir = %d doit etre égal a LEFT(0) ou RIGHT(1)", dir);
	
	if(dir == LEFT) {
		if(bit_get(x, msbBit) == 1) {
			set_C(&(result->flags));
		}
	} else {
		if(bit_get(x, lsbBit) == 1) {
			set_C(&(result->flags));
		}
	}
	
	bit_rotate(&x, dir, one_shift);
	result->value = x;	
	
	set_Z_if_zero(result);
	
	return ERR_NONE;
}

int alu_carry_rotate(alu_output_t* result, uint8_t x, rot_dir_t dir, flags_t flags) {
	M_REQUIRE_NON_NULL(result);
	M_EXIT_IF(dir < LEFT || dir > RIGHT, ERR_BAD_PARAMETER, "dir = %d doit etre égal a LEFT(0) ou RIGHT(1)", dir);
	
	bit_t s = 0;
	if(dir == LEFT) {
		s = bit_get(x, msbBit);		
	} else {
		s = bit_get(x, lsbBit);
	}	
	alu_shift(result, x, dir);
	
	if(dir == LEFT && get_C(flags) == FLAG_C) {
		uint8_t lsb = lsb8(result->value);
		bit_set(&lsb, lsbBit);
		result->value = lsb;
	} 
	if(dir == RIGHT && get_C(flags) == FLAG_C) {
		uint8_t lsb = lsb8(result->value);
		bit_set(&lsb, msbBit);
		result->value = lsb;	
	}
	
	result->flags = result->flags & 1;
	if(s != 0) {
		set_flag(&(result->flags), FLAG_C);
	}

	set_Z_if_zero(result);
	
	return ERR_NONE;	
}
		
