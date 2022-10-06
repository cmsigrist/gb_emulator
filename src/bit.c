#include <stdio.h>
#include <stdint.h>   // for uint8_t and uint16_t types
#include <inttypes.h> // for PRIx8, etc.
#include "bit.h"

// ======================================================================
/**
 * @brief a type to represent 1 single bit.
 */
/* Nous vous fournission ici un type à n'utiliser QUE lorsque vous
 * voulez représenter UN SEUL bit ; p.ex. :
 *     bit_t un_bit_tout_seul = 1;
 */
typedef uint8_t bit_t;


// ======================================================================
/**
 * @brief clamp a value to be a bit index between 0 and 7
 */
/* Nous vous fournission ici une macro (les macros seront présentées dans
 *     le cours, bien plus tard dans le semestre) permettant de forcer
 * une valeur entre 0 et 7.
 * Par exemple :
 *     i = CLAMP07(j);
 * fera que la variable i contiendra :
 *     + la même valeur que celle de j si celle-ci est comprise entre 0 et 7 ;
 *     + 0 si la valeur de j est inférieure ou égale à 0 ;
 *     + 0 si la vlaeur de j est supérieure ou égale à 8.
 */
#define CLAMP07(x) (((x) < 0) || ((x) > 7) ? 0 : (x))

#define fourBitMask 0xF
#define msb4Shift 4
#define eightBitMask 0xFF
#define msb8Shift 8
#define sizeOfByte 8
// ======================================================================
uint8_t lsb4(uint8_t value){
	return value & fourBitMask; // Mettez ici votre code pour LSB4 sur value
}

uint8_t msb4(uint8_t value){
	return value >> msb4Shift; // Mettez ici votre code pour MSB4 sur value
}

uint8_t lsb8(uint16_t value){
	return value & eightBitMask;
}

uint8_t msb8(uint16_t value){
	return value >> msb8Shift; 
}

uint16_t merge8(uint8_t v1, uint8_t v2){
	return v2 << msb8Shift | v1;
}

uint8_t merge4(uint8_t v1, uint8_t v2){
	return ((v2 & fourBitMask) << msb4Shift) | (v1 & fourBitMask);
}

bit_t bit_get(uint8_t value, int index){
	int i = CLAMP07(index);
	return (value >> i) & 1;
}

void bit_set(uint8_t* value, int index){
	int i = CLAMP07(index);
	*value = *value | (1 << i);
}

void bit_unset(uint8_t* value, int index){
	int i = CLAMP07(index);
	*value = ~((~*value) | (1 << i));
}

void bit_rotate(uint8_t* value, rot_dir_t dir, int d){
	int i = CLAMP07(d);
	if(dir == LEFT) {
		*value = (*value << i) | (*value >> (sizeOfByte - i));
	} else {
		*value = (*value >> i) | (*value << (sizeOfByte - i));
	}
} 

void bit_edit(uint8_t* value, int index, uint8_t v){
	if(v == 0) {
		bit_unset(value, index);
	} else {
		bit_set(value, index);		
	}
}

 
