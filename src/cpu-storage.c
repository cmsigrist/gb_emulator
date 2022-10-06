/**
 * @file cpu-registers.c
 * @brief Game Boy CPU simulation, register part
 *
 * @date 2019
 */

#include "error.h"
#include "cpu-storage.h" // cpu_read_at_HL
#include "cpu-registers.h" // cpu_BC_get
#include "gameboy.h" // REGISTER_START
#include "util.h"
#include <inttypes.h> // PRIX8
#include <stdio.h> // fprintf

#define SP_CHANGE 2
#define SHIFT_ZERO 0

// ==== see cpu-storage.h ========================================
data_t cpu_read_at_idx(const cpu_t* cpu, addr_t addr)
{
    M_REQUIRE_NON_NULL(cpu);
    M_REQUIRE_NON_NULL(cpu->bus);
    
	data_t value = 0;
	bus_read(*(cpu->bus), addr, &value);
	return value;
}

// ==== see cpu-storage.h ========================================
addr_t cpu_read16_at_idx(const cpu_t* cpu, addr_t addr)
{
    M_REQUIRE_NON_NULL(cpu);
    M_REQUIRE_NON_NULL(cpu->bus);
    
	addr_t value = 0;
	bus_read16(*(cpu->bus), addr, &value);
	return value;
}

// ==== see cpu-storage.h ========================================
int cpu_write_at_idx(cpu_t* cpu, addr_t addr, data_t data)
{
	M_REQUIRE_NON_NULL(cpu);
	M_REQUIRE_NON_NULL(cpu->bus);
	 
	cpu->write_listener = addr;
	return bus_write(*(cpu->bus), addr, data);
}

// ==== see cpu-storage.h ========================================
int cpu_write16_at_idx(cpu_t* cpu, addr_t addr, addr_t data16)
{
	M_REQUIRE_NON_NULL(cpu);
	M_REQUIRE_NON_NULL(cpu->bus);
	 
	cpu->write_listener = addr;
	return bus_write16(*(cpu->bus), addr, data16);
}

// ==== see cpu-storage.h ========================================
int cpu_SP_push(cpu_t* cpu, addr_t data16)
{
	M_REQUIRE_NON_NULL(cpu);
	M_REQUIRE_NON_NULL(cpu->bus);
	 
	cpu->SP -= SP_CHANGE;
	return cpu_write16_at_idx(cpu, cpu->SP, data16);
}

// ==== see cpu-storage.h ========================================
addr_t cpu_SP_pop(cpu_t* cpu)
{
	M_REQUIRE_NON_NULL(cpu); 
	M_REQUIRE_NON_NULL(cpu->bus);
	
	addr_t addr = cpu_read16_at_idx(cpu, cpu->SP); 
	cpu->SP += SP_CHANGE;
	return addr;
}

// ==== see cpu-storage.h ========================================
int cpu_dispatch_storage(const instruction_t* lu, cpu_t* cpu)
{
    M_REQUIRE_NON_NULL(cpu);
    M_REQUIRE_NON_NULL(lu);
    
    switch (lu->family) {
    case LD_A_BCR:
        cpu_A_set(cpu, cpu_read_at_idx(cpu, cpu_BC_get(cpu)));
		break;	

    case LD_A_CR:
		cpu_A_set(cpu, cpu_read_at_idx(cpu, REGISTERS_START + cpu_reg_get(cpu, REG_C_CODE)));
		break;

    case LD_A_DER:
        cpu_A_set(cpu, cpu_read_at_idx(cpu, cpu_DE_get(cpu))); /////////////////////////////////////////////
		break;

    case LD_A_HLRU:
        cpu_A_set(cpu, cpu_read_at_HL(cpu)); ////////////////////////////////////////////////////////
		cpu_HL_set(cpu, cpu_HL_get(cpu) + extract_HL_increment(lu->opcode));
		break;

    case LD_A_N16R:
        {
			addr_t nn = cpu_read_addr_after_opcode(cpu);
			cpu_A_set(cpu, cpu_read_at_idx(cpu, nn));
		}
		break;

    case LD_A_N8R:
        {
			addr_t addr = REGISTERS_START + cpu_read_data_after_opcode(cpu);
			cpu_A_set(cpu, cpu_read_at_idx(cpu, addr));
		}
		break;

    case LD_BCR_A:    
		cpu_write_at_idx(cpu, cpu_BC_get(cpu), cpu_A_get(cpu));
		break;

    case LD_CR_A:
        cpu_write_at_idx(cpu, cpu_C_get(cpu) + REGISTERS_START, cpu_A_get(cpu));
		break;

    case LD_DER_A:
        cpu_write_at_idx(cpu, cpu_DE_get(cpu), cpu_A_get(cpu));
		break;

    case LD_HLRU_A:
        cpu_write_at_HL(cpu, cpu_A_get(cpu));
		cpu_HL_set(cpu, cpu_HL_get(cpu) + extract_HL_increment(lu->opcode));
		break;

    case LD_HLR_N8:
        cpu_write_at_HL(cpu, cpu_read_data_after_opcode(cpu));
		break;

    case LD_HLR_R8:
		{
		uint8_t r = extract_reg(lu->opcode, SHIFT_ZERO);
        cpu_write_at_HL(cpu, cpu_reg_get(cpu, r)); ////////////////////////////////////
		}
		break;

    case LD_N16R_A:
       {
			addr_t nn = cpu_read_addr_after_opcode(cpu);
			cpu_write_at_idx(cpu, nn, cpu_A_get(cpu));
		}
		break;

    case LD_N16R_SP:
        {
			addr_t nn = cpu_read_addr_after_opcode(cpu);
			cpu_write16_at_idx(cpu, nn, cpu_reg_pair_SP_get(cpu, REG_AF_CODE));
		}
		break;

    case LD_N8R_A:
        {
			addr_t addr = REGISTERS_START + cpu_read_data_after_opcode(cpu);
			cpu_write_at_idx(cpu, addr, cpu_A_get(cpu));
		}
		break;

    case LD_R16SP_N16:
        {
			uint8_t r = extract_reg_pair(lu->opcode);
			addr_t nn = cpu_read_addr_after_opcode(cpu); /////////////////////////////////////////
			if(r == REG_AF_CODE){
				cpu_reg_pair_SP_set(cpu, REG_AF_CODE, nn);
			} else {
			cpu_reg_pair_set(cpu, r, nn);
			}
		}
		break;

    case LD_R8_HLR:
        {
			uint8_t r = extract_n3(lu->opcode);
			cpu_reg_set(cpu, r, cpu_read_at_idx(cpu, cpu_HL_get(cpu)));
		}
		break;

    case LD_R8_N8:
        {
			uint8_t r = extract_n3(lu->opcode);
			uint8_t n = cpu_read_data_after_opcode(cpu); //////////////////////////////////////
			cpu_reg_set(cpu, r, n);
		}
		break;	

    case LD_R8_R8: 
		{
			uint8_t r = extract_n3(lu->opcode);
			uint8_t s = extract_reg(lu->opcode, SHIFT_ZERO);
			if(r != s) {
				cpu_reg_set(cpu, r, cpu_reg_get(cpu, s));
			}
		}
		break;

    case LD_SP_HL:
		cpu_reg_pair_SP_set(cpu, REG_AF_CODE, cpu_HL_get(cpu));
		break;

    case POP_R16:
        {
			uint8_t r = extract_reg_pair(lu->opcode);
			cpu_reg_pair_set(cpu, r, cpu_SP_pop(cpu));
		}
		break;

    case PUSH_R16:
		{
			uint16_t r = extract_reg_pair(lu->opcode);
			cpu_SP_push(cpu, cpu_reg_pair_get(cpu, r));
		}
        break;

    default:
        fprintf(stderr, "Unknown STORAGE instruction, Code: 0x%" PRIX8 "\n", cpu_read_at_idx(cpu, cpu->PC));
        return ERR_INSTR;
        break;
    } // switch

    return ERR_NONE;
}
