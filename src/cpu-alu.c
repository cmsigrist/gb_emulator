/**
 * @file cpu-alu.c
 * @brief Game Boy CPU simulation, ALU part asked to students
 *
 * @date 2019
 */

#include "error.h"
#include "bit.h"
#include "alu.h"
#include "cpu-alu.h"
#include "cpu-storage.h" // cpu_read_at_HL
#include "cpu-registers.h" // cpu_HL_get

// external library provided later to lower workload
extern int cpu_dispatch_alu_ext(const instruction_t* lu, cpu_t* cpu);

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#define INCREMENT 1
#define CARRY_ZERO 0
#define CARRY_ONE 1
#define NO_SHIFT 0

// ======================================================================
/**
 * @brief Returns flag bit value based on source preferences
 *
 * @param src source to select
 * @param cpu_f flags from cpu
 * @param alu_f flags from alu
 *
 * @return resulting flag bit value
 */
static bool flags_src_value(flag_src_t src, flag_bit_t cpu_f, flag_bit_t alu_f)
{
    switch (src) {
    case CLEAR:
        return false;

    case SET:
        return true;

    case ALU:
        return alu_f;

    case CPU:
        return cpu_f;

    default:
        return false;
    }

    return false;
}

// ======================================================================
/**
* @brief Checks if x is a valid flag source
*/
#define IS_VALID_FLAG_SRC(x) \
    (x == CLEAR || x == SET || x == ALU || x == CPU)
#define CHECK_FLAG_SRC(x) \
    M_REQUIRE(IS_VALID_FLAG_SRC(x), ERR_BAD_PARAMETER, "Parameter %d for " #x " is not valid", x)

// ==== see cpu-alu.h ========================================
int cpu_combine_alu_flags(cpu_t* cpu,
                          flag_src_t Z, flag_src_t N, flag_src_t H, flag_src_t C)
{
    M_REQUIRE_NON_NULL(cpu);
    CHECK_FLAG_SRC(Z);
    CHECK_FLAG_SRC(N);
    CHECK_FLAG_SRC(H);
    CHECK_FLAG_SRC(C);

    flags_t res_f = 0;

    if (flags_src_value(Z, get_Z(cpu->F), get_Z(cpu->alu.flags)))
        set_Z(&res_f);

    if (flags_src_value(N, get_N(cpu->F), get_N(cpu->alu.flags)))
        set_N(&res_f);

    if (flags_src_value(H, get_H(cpu->F), get_H(cpu->alu.flags)))
        set_H(&res_f);

    if (flags_src_value(C, get_C(cpu->F), get_C(cpu->alu.flags)))
        set_C(&res_f);

    cpu->F = res_f;

    return ERR_NONE;
}

// ======================================================================
/**
* @brief Tool function usefull for CHG_U3_R8:
*        Do a SET or a RESET(=unset) of data bit,
*          according to SR and N3 bits of instruction's opcode
*/
static void do_set_or_res(const instruction_t* lu, data_t* data)
{
    assert(lu   != NULL);
    assert(data != NULL);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"

    if (extract_sr_bit(lu->opcode)) {
        *data |=   (data_t) (1 << extract_n3(lu->opcode)) ;
    } else {
        *data &= ~((data_t) (1 << extract_n3(lu->opcode)));
    }

#pragma GCC diagnostic pop
}

// ==== see cpu-alu.h ========================================
int cpu_dispatch_alu(const instruction_t* lu, cpu_t* cpu)
{
    M_REQUIRE_NON_NULL(cpu);
    switch (lu->family) {

    // ADD
    case ADD_A_HLR: {
		do_cpu_arithm(cpu, alu_add8, cpu_read_at_HL(cpu), ADD_FLAGS_SRC);
		//cpu_A_set(cpu, cpu->alu.value);
    } break;

    case ADD_A_N8: {
		do_cpu_arithm(cpu, alu_add8, cpu_read_data_after_opcode(cpu), ADD_FLAGS_SRC);
		//cpu_A_set(cpu, cpu->alu.value);
    } break;

    case ADD_A_R8: {
		uint8_t r = extract_reg(lu->opcode, NO_SHIFT);
		do_cpu_arithm(cpu, alu_add8, cpu_reg_get(cpu, r), ADD_FLAGS_SRC);
		//cpu_A_set(cpu, cpu->alu.value);
    } break;

    case INC_HLR: {		
		alu_add8(&cpu->alu, cpu_read_at_HL(cpu), INCREMENT, extract_carry(cpu, lu->opcode));
		cpu_write_at_HL(cpu, cpu->alu.value);
		cpu_combine_alu_flags(cpu, INC_FLAGS_SRC);
    } break;

    case INC_R8: {
		uint8_t r = extract_n3(lu->opcode);
		alu_add8(&cpu->alu, cpu_reg_get(cpu, r), INCREMENT, CARRY_ZERO);
		cpu_reg_set(cpu, r, cpu->alu.value);
		cpu_combine_alu_flags(cpu, INC_FLAGS_SRC);
    } break;
    
    case DEC_R8: {
		uint8_t r = extract_n3(lu->opcode);
		alu_sub8(&cpu->alu, cpu_reg_get(cpu, r), INCREMENT, CARRY_ZERO);
		cpu_reg_set(cpu, r, cpu->alu.value);
		cpu_combine_alu_flags(cpu, DEC_FLAGS_SRC);
	} break;

    case ADD_HL_R16SP: {
		uint8_t r = extract_reg_pair(lu->opcode);
		if(r == REG_AF_CODE) {
			alu_add16_high(&cpu->alu, cpu_HL_get(cpu), cpu_reg_pair_SP_get(cpu, REG_AF_CODE)); 
		} else {
			alu_add16_high(&cpu->alu, cpu_HL_get(cpu), cpu_reg_pair_get(cpu, r)); 
		}			
		cpu_HL_set(cpu, cpu->alu.value);
		cpu_combine_alu_flags(cpu, CPU, CLEAR, ALU, ALU);
    } break;

    case INC_R16SP: {
		uint8_t r = extract_reg_pair(lu->opcode);
		if(r == REG_AF_CODE){
			alu_add16_high(&cpu->alu, cpu_reg_pair_SP_get(cpu, REG_AF_CODE), CARRY_ONE);
			cpu_reg_pair_SP_set(cpu, REG_AF_CODE, cpu->alu.value);
			cpu_combine_alu_flags(cpu, CPU, CPU, CPU, CPU);
		 } else {
			alu_add16_high(&cpu->alu, cpu_reg_pair_get(cpu,r), CARRY_ONE);
			cpu_reg_pair_set(cpu, r, cpu->alu.value);
			cpu_combine_alu_flags(cpu, CPU, CPU, CPU, CPU);
		}
    } break;


    // COMPARISONS
    case CP_A_R8: {
		uint8_t r = extract_reg(lu->opcode, NO_SHIFT);
		alu_sub8(&cpu->alu, cpu_A_get(cpu), cpu_reg_get(cpu, r), CARRY_ZERO);
		cpu_combine_alu_flags(cpu, SUB_FLAGS_SRC);
    } break;

	case CP_A_N8: {
		uint8_t n = cpu_read_data_after_opcode(cpu);
		alu_sub8(&cpu->alu, cpu_A_get(cpu), n, CARRY_ZERO);
		cpu_combine_alu_flags(cpu, SUB_FLAGS_SRC);
    } break;

    // BIT MOVE (rotate, shift)
    case SLA_R8: {
		uint8_t r = extract_reg(lu->opcode, NO_SHIFT);
		alu_shift(&cpu->alu, cpu_reg_get(cpu, r), LEFT);
		cpu_reg_set(cpu, r, cpu->alu.value);
		cpu_combine_alu_flags(cpu, SHIFT_FLAGS_SRC);

    } break;

    case ROT_R8: {
		uint8_t r = extract_reg(lu->opcode, NO_SHIFT);
		rot_dir_t dir = extract_rot_dir(lu->opcode);
			
		alu_carry_rotate(&cpu->alu, cpu_reg_get(cpu, r), dir, cpu->F);
		cpu_reg_set(cpu, r, cpu->alu.value);
		cpu_combine_alu_flags(cpu, SHIFT_FLAGS_SRC);
    } break;


    // BIT TESTS (and set)
    case BIT_U3_R8: {
		uint8_t r = extract_reg(lu->opcode, NO_SHIFT);
		uint8_t n3 = extract_n3(lu->opcode);
		if(bit_get(cpu_reg_get(cpu, r), n3)){
			cpu_combine_alu_flags(cpu, CLEAR, CLEAR, SET, CPU);
		} else {
			cpu_combine_alu_flags(cpu, SET, CLEAR, SET, CPU);
		}
    } break;

    case CHG_U3_R8: {
		uint8_t r = extract_reg(lu->opcode, NO_SHIFT);
		data_t data = cpu_reg_get(cpu, r);
		do_set_or_res(lu, &data);
		cpu_reg_set(cpu, r, data);
		cpu_combine_alu_flags(cpu, CPU, CPU, CPU, CPU);
    } break;

    // ---------------------------------------------------------
    // All the others are handled elsewhere by provided library
    default:
        // uncomment this line if you have the cs212gbcpuext library
        #ifdef ALUEXT
			M_EXIT_IF_ERR(cpu_dispatch_alu_ext(lu, cpu));
		#endif
        break;
    } // switch

    return ERR_NONE;
}
