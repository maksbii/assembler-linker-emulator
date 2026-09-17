#ifndef INSTRUCTION_ONE_REG_HPP
#define INSTRUCTION_ONE_REG_HPP

#include <stdio.h>
#include "instruction_common.hpp"
#include "opcodes.hpp"
#include "registers.hpp"

/*
 * emit_* helpers for the one-register instructions: push, pop, not.
 * Dispatched from asm_instr_one_reg() in src/assembler.cpp.
 */

// push %gpr: sp <= sp-4; mem32[sp] <= gpr -> store, MOD_STORE_PUSH,
// A=sp, D=-4, C=gpr (RegB unused).
inline void emit_push(long gpr) {
    require_section(OPC_PUSH);
    current_section->appendInstruction(OC_STORE, MOD_STORE_PUSH, REG_SP, 0, gpr, -4);
    printf("INSTR: %s %%r%ld\n", op_name(OPC_PUSH), gpr);
}

// pop %gpr: gpr <= mem32[sp]; sp <= sp+4 -> load, MOD_LOAD_MEM_POSTINC,
// A=gpr, B=sp, D=4 (RegC unused).
inline void emit_pop(long gpr) {
    require_section(OPC_POP);
    current_section->appendInstruction(OC_LOAD, MOD_LOAD_MEM_POSTINC, gpr, REG_SP, 0, 4);
    printf("INSTR: %s %%r%ld\n", op_name(OPC_POP), gpr);
}

// not %gpr: gpr <= ~gpr, in place -> OC_LOGIC MOD_LOGIC_NOT, RegA=RegB=gpr
// (RegC unused).
inline void emit_not(long gpr) {
    require_section(OPC_NOT);
    current_section->appendInstruction(OC_LOGIC, MOD_LOGIC_NOT, gpr, gpr, 0, 0);
    printf("INSTR: %s %%r%ld\n", op_name(OPC_NOT), gpr);
}

#endif /* INSTRUCTION_ONE_REG_HPP */
