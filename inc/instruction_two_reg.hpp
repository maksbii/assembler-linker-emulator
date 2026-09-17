#ifndef INSTRUCTION_TWO_REG_HPP
#define INSTRUCTION_TWO_REG_HPP

#include <stdio.h>
#include "instruction_common.hpp"
#include "opcodes.hpp"

/*
 * emit_* helpers for the two-register instructions: xchg, add, sub, mul,
 * div, and, or, xor, shl, shr. Dispatched from asm_instr_two_reg() in
 * src/assembler.cpp.
 */

// Arithmetic mnemonics are 2-address (op %gprS, %gprD meaning
// gprD <= gprD op gprS) but the raw encoding is 3-address
// (gpr[A] <= gpr[B] op gpr[C]), so RegA=RegB=gprD (dest doubles as the
// first operand) and RegC=gprS.
inline void emit_add(long gprS, long gprD) {
    require_section(OPC_ADD);
    current_section->appendInstruction(OC_ARITH, MOD_ARITH_ADD, gprD, gprD, gprS, 0);
    printf("INSTR: %s %%r%ld, %%r%ld\n", op_name(OPC_ADD), gprS, gprD);
}

inline void emit_sub(long gprS, long gprD) {
    require_section(OPC_SUB);
    current_section->appendInstruction(OC_ARITH, MOD_ARITH_SUB, gprD, gprD, gprS, 0);
    printf("INSTR: %s %%r%ld, %%r%ld\n", op_name(OPC_SUB), gprS, gprD);
}

inline void emit_mul(long gprS, long gprD) {
    require_section(OPC_MUL);
    current_section->appendInstruction(OC_ARITH, MOD_ARITH_MUL, gprD, gprD, gprS, 0);
    printf("INSTR: %s %%r%ld, %%r%ld\n", op_name(OPC_MUL), gprS, gprD);
}

inline void emit_div(long gprS, long gprD) {
    require_section(OPC_DIV);
    current_section->appendInstruction(OC_ARITH, MOD_ARITH_DIV, gprD, gprD, gprS, 0);
    printf("INSTR: %s %%r%ld, %%r%ld\n", op_name(OPC_DIV), gprS, gprD);
}

// xchg swaps gpr[B] and gpr[C] directly; RegA is unused (spec leaves it
// unspecified, so left as 0).
inline void emit_xchg(long gprS, long gprD) {
    require_section(OPC_XCHG);
    current_section->appendInstruction(OC_XCHG, 0, 0, gprS, gprD, 0);
    printf("INSTR: %s %%r%ld, %%r%ld\n", op_name(OPC_XCHG), gprS, gprD);
}

// and/or/xor are 2-address like arithmetic (gprD <= gprD op gprS), same
// RegA=RegB=gprD / RegC=gprS layout as add/sub/mul/div, just under
// OC_LOGIC.
inline void emit_and(long gprS, long gprD) {
    require_section(OPC_AND);
    current_section->appendInstruction(OC_LOGIC, MOD_LOGIC_AND, gprD, gprD, gprS, 0);
    printf("INSTR: %s %%r%ld, %%r%ld\n", op_name(OPC_AND), gprS, gprD);
}

inline void emit_or(long gprS, long gprD) {
    require_section(OPC_OR);
    current_section->appendInstruction(OC_LOGIC, MOD_LOGIC_OR, gprD, gprD, gprS, 0);
    printf("INSTR: %s %%r%ld, %%r%ld\n", op_name(OPC_OR), gprS, gprD);
}

inline void emit_xor(long gprS, long gprD) {
    require_section(OPC_XOR);
    current_section->appendInstruction(OC_LOGIC, MOD_LOGIC_XOR, gprD, gprD, gprS, 0);
    printf("INSTR: %s %%r%ld, %%r%ld\n", op_name(OPC_XOR), gprS, gprD);
}

// shl/shr are the same 2-address layout again, under OC_SHIFT.
inline void emit_shl(long gprS, long gprD) {
    require_section(OPC_SHL);
    current_section->appendInstruction(OC_SHIFT, MOD_SHIFT_SHL, gprD, gprD, gprS, 0);
    printf("INSTR: %s %%r%ld, %%r%ld\n", op_name(OPC_SHL), gprS, gprD);
}

inline void emit_shr(long gprS, long gprD) {
    require_section(OPC_SHR);
    current_section->appendInstruction(OC_SHIFT, MOD_SHIFT_SHR, gprD, gprD, gprS, 0);
    printf("INSTR: %s %%r%ld, %%r%ld\n", op_name(OPC_SHR), gprS, gprD);
}

#endif /* INSTRUCTION_TWO_REG_HPP */
