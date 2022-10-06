/**
 * @file cpu.c
 * @brief Game Boy CPU simulation
 *
 * @date 2019
 */

#include "error.h"
#include "opcode.h"
#include "cpu.h"
#include "cpu-alu.h"
#include "cpu-registers.h"
#include "cpu-storage.h"
#include "util.h"
#include "gameboy.h"

#include <inttypes.h> // PRIX8
#include <stdio.h> // fprintf
#include <stdlib.h>

#define INIT_VALUE 0
#define PREFIXE 0xCB

#define SHIFT 3

#define TRUE 1
#define FALSE 0

#define INTERRUPT_COUNT 5
#define MASK 1
#define RIGHT_SHIFT 1
#define NO_INTERRUPTION -1
#define INTERRUPT_ADDR 0x40
#define INTERRUPT_IDLE_TIME 5

#define NOT_ZERO 0
#define ZERO 1
#define NOT_CARRY 2
#define CARRY 3


// ======================================================================
int cpu_init(cpu_t* cpu) {
	M_REQUIRE_NON_NULL(cpu);
	cpu->idle_time = INIT_VALUE;
	cpu->alu.value = INIT_VALUE;
	cpu->alu.flags = INIT_VALUE;
	M_EXIT_IF_ERR(component_create(&cpu->high_ram, HIGH_RAM_SIZE)); 
	cpu->AF = INIT_VALUE;
	cpu->BC = INIT_VALUE;
	cpu->DE = INIT_VALUE;
	cpu->HL = INIT_VALUE;
	cpu->SP = INIT_VALUE;
	cpu->PC = INIT_VALUE;
	
	cpu->IE = INIT_VALUE;
	cpu->IF = INIT_VALUE;
	cpu->IME = FALSE;
	cpu->HALT = FALSE;

	cpu->write_listener = INIT_VALUE;

    return ERR_NONE;
}

// ======================================================================
int cpu_plug(cpu_t* cpu, bus_t* bus) {
	M_REQUIRE_NON_NULL(cpu);
	M_REQUIRE_NON_NULL(bus);
	
	(cpu->bus) = bus;
	M_EXIT_IF_ERR(bus_plug(*cpu->bus, &cpu->high_ram, HIGH_RAM_START, HIGH_RAM_END));

	(*bus)[REG_IF] = &cpu->IF;
	(*bus)[REG_IE] = &cpu->IE;
	
    return ERR_NONE;
}

// ======================================================================
void cpu_free(cpu_t* cpu) {
	if(cpu != NULL){    
		bus_unplug(*cpu->bus, &cpu->high_ram);
		component_free(&cpu->high_ram); 
		cpu = NULL;
	}
}

//=========================================================================
/**
 * @brief check if the condition of the jump is met
 * @param the condition cc
 * @return TRUE if it's true else FALSE
 * 
 */
int jumpConditionnal(const cpu_t* cpu,uint8_t cc) {
	bit_t flagF = bit_get(cpu->F, 7);
	bit_t flagC = bit_get(cpu->F, 4);
	switch(cc) {
		case NOT_ZERO: 
			if(flagF == 0) {
				return TRUE;
			}
			break;					
		case ZERO:
			if(flagF == 1) {
				return TRUE;		
			}
			break;	
		case NOT_CARRY:
			if(flagC == 0) {
				return TRUE;
			}
			break;	
		case CARRY:
			if(flagC == 1) {
				return TRUE;
			}
			break;
		default: break;
	}
	return FALSE;
}

int extractIrqBit(const uint8_t interruption) {
	uint8_t i = interruption;
	int index = 0;
	while(i <= INTERRUPT_COUNT) {
		if(i & MASK) {
			return index;
		}
		index++;
		i >>= RIGHT_SHIFT;
	}	
	return NO_INTERRUPTION;
}
//=========================================================================
/**
 * @brief Executes an instruction
 * @param lu instruction
 * @param cpu, the CPU which shall execute
 * @return error code
 *
 * See opcode.h and cpu.h
 */
static int cpu_dispatch(const instruction_t* lu, cpu_t* cpu)
{
    M_REQUIRE_NON_NULL(lu);
    M_REQUIRE_NON_NULL(cpu);

	cpu->alu.value = INIT_VALUE;
	cpu->alu.flags = INIT_VALUE;
	cpu->idle_time = INIT_VALUE;
	//printf("op = 0x%X\n", lu->opcode);
    switch (lu->family) {

    // ALU
    case ADD_A_HLR: 
    case ADD_A_N8:
    case ADD_A_R8:
    case INC_HLR:
    case INC_R8:
    case ADD_HL_R16SP:
    case INC_R16SP:
    case SUB_A_HLR:
    case SUB_A_N8:
    case SUB_A_R8:
    case DEC_HLR:
    case DEC_R8:
    case DEC_R16SP:
    case AND_A_HLR:
    case AND_A_N8:
    case AND_A_R8:
    case OR_A_HLR:
    case OR_A_N8:
    case OR_A_R8:
    case XOR_A_HLR:
    case XOR_A_N8:
    case XOR_A_R8:
    case CPL:
    case CP_A_HLR:
    case CP_A_N8:
    case CP_A_R8:
    case SLA_HLR:
    case SLA_R8:
    case SRA_HLR:
    case SRA_R8:
    case SRL_HLR:
    case SRL_R8:
    case ROTCA:
    case ROTA:
    case ROTC_HLR:
    case ROT_HLR:
    case ROTC_R8:
    case ROT_R8:
    case SWAP_HLR:
    case SWAP_R8:
    case BIT_U3_HLR:
    case BIT_U3_R8:
    case CHG_U3_HLR:
    case CHG_U3_R8:
    case LD_HLSP_S8:
    case DAA:
    case SCCF:
        M_EXIT_IF_ERR(cpu_dispatch_alu(lu, cpu));
        break;

    // STORAGE
    case LD_A_BCR: 
    case LD_A_CR: 
    case LD_A_DER: 
    case LD_A_HLRU:
    case LD_A_N16R:
    case LD_A_N8R:
    case LD_BCR_A:
    case LD_CR_A:
    case LD_DER_A:
    case LD_HLRU_A:
    case LD_HLR_N8:
    case LD_HLR_R8:
    case LD_N16R_A:
    case LD_N16R_SP:
    case LD_N8R_A: 
    case LD_R16SP_N16:
    case LD_R8_HLR:
    case LD_R8_N8:	
    case LD_R8_R8:
    case LD_SP_HL:
    case POP_R16:			
    case PUSH_R16:
        M_EXIT_IF_ERR(cpu_dispatch_storage(lu, cpu));       
        break;


    // JUMP
    case JP_CC_N16: 
		{
			uint16_t nn = cpu_read_addr_after_opcode(cpu);
			uint8_t cc = extract_cc(lu->opcode);
			if(jumpConditionnal(cpu, cc)) {
				cpu->PC = nn - lu->bytes;
				cpu->idle_time = lu->xtra_cycles;
			} 
		}
        break;

    case JP_HL:
		cpu->PC = cpu_HL_get(cpu) - lu->bytes;
        break;

    case JP_N16:
		{
			uint16_t nn = cpu_read_addr_after_opcode(cpu); //////////////////////////////////
			cpu->PC = nn - lu->bytes;
		}
        break;

    case JR_CC_E8:
		{
			signed char e = cpu_read_data_after_opcode(cpu);  ////////////////////////////////////////////////
			uint8_t cc = extract_cc(lu->opcode);
			if(jumpConditionnal(cpu, cc)) {
				cpu->PC += e;
				cpu->idle_time = lu->xtra_cycles;
			}
		}
        break;

    case JR_E8:
		{
			signed char e = cpu_read_data_after_opcode(cpu); 
			cpu->PC += e;
		}
        break;


    // CALLS
    case CALL_CC_N16:
		{
			uint16_t nn = cpu_read_addr_after_opcode(cpu);
			uint8_t cc = extract_cc(lu->opcode);
			if(jumpConditionnal(cpu, cc)) {
				uint16_t PCPrime = cpu->PC + lu->bytes;
				M_EXIT_IF_ERR(cpu_SP_push(cpu, PCPrime));
				cpu->PC = nn - lu->bytes;
				cpu->idle_time = lu->xtra_cycles;
			}
		}		
        break;

    case CALL_N16:
		{
			uint16_t nn = cpu_read_addr_after_opcode(cpu);
			uint16_t PCPrime = cpu->PC + lu->bytes;
			M_EXIT_IF_ERR(cpu_SP_push(cpu, PCPrime));
			cpu->PC = nn - lu->bytes;
		}
        break;


    // RETURN (from call)
    case RET:
		cpu->PC = cpu_SP_pop(cpu) - lu->bytes;
        break;

    case RET_CC:
		{
			uint8_t cc = extract_cc(lu->opcode);
			if(jumpConditionnal(cpu, cc)) {
				cpu->PC = cpu_SP_pop(cpu) - lu->bytes;
				cpu->idle_time = lu->xtra_cycles;
			}
		}
        break;

    case RST_U3:
		{
			uint8_t n = extract_reg(lu->opcode, SHIFT);
			uint16_t PCPrime = cpu->PC + lu->bytes;
			M_EXIT_IF_ERR(cpu_SP_push(cpu, PCPrime));
			cpu->PC = (n << SHIFT) - lu->bytes;
		}
        break;


    // INTERRUPT & MISC.
    case EDI:
		cpu->IME = extract_ime(lu->opcode);
        break;

    case RETI:
		cpu->IME = TRUE;
		cpu->PC = cpu_SP_pop(cpu) - lu->bytes;
        break;

    case HALT:
		cpu->HALT = TRUE;
        break;

    case STOP:
    case NOP: 
        // ne rien faire
        break;

    default: {
        fprintf(stderr, "Unknown instruction, Code: 0x%" PRIX8 "\n", cpu_read_at_idx(cpu, cpu->PC));
        return ERR_INSTR;
    } break;

    } // switch

	cpu->idle_time += lu->cycles - 1;	
	cpu->PC += lu->bytes;
    return ERR_NONE;
}

// ---------------------------------------------------------------------
static int cpu_do_cycle(cpu_t* cpu)
{
    M_REQUIRE_NON_NULL(cpu);
	uint8_t interrupt = cpu->IE & cpu->IF;

	if(interrupt != 0 && cpu->IME) {
		interrupt_t interruption = extractIrqBit(interrupt);
		if(interruption == NO_INTERRUPTION) {
			return ERR_BAD_PARAMETER;
		}
		cpu->IME = FALSE;
		bit_unset(&(cpu->IF), interruption);
		M_EXIT_IF_ERR(cpu_SP_push(cpu, cpu->PC));
		cpu->PC = INTERRUPT_ADDR + (interruption << SHIFT);
		cpu->idle_time += INTERRUPT_IDLE_TIME;
		return ERR_NONE;
	}

	if(cpu->idle_time <= 0){
		uint8_t op = cpu_read_at_idx(cpu, cpu->PC); //////////////////////////////////////////
		if(op == PREFIXE) {
			M_EXIT_IF_ERR(cpu_dispatch(&instruction_prefixed[cpu_read_data_after_opcode(cpu)], cpu));
		} else {
			M_EXIT_IF_ERR(cpu_dispatch(&instruction_direct[op], cpu));
		}

		return ERR_NONE;
	} else {
		cpu->idle_time -= 1;
		return ERR_BAD_PARAMETER;
	}
}

// ======================================================================
/**
 * See cpu.h
 */
int cpu_cycle(cpu_t* cpu)
{
    M_REQUIRE_NON_NULL(cpu);
    M_REQUIRE_NON_NULL(cpu->bus);
	cpu->write_listener = INIT_VALUE;
    if(cpu->HALT == FALSE || (cpu->HALT == TRUE && cpu->IF != 0 && cpu->idle_time == 0)) {
		cpu->HALT = FALSE;
		cpu_do_cycle(cpu);
	}
    return ERR_NONE;
}

// ======================================================================
void cpu_request_interrupt(cpu_t* cpu, interrupt_t i) {
	cpu->IF = (cpu->IF | 1 << i);
}
