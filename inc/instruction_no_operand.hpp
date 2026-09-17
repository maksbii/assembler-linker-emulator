#ifndef INSTRUCTION_NO_OPERAND_HPP
#define INSTRUCTION_NO_OPERAND_HPP

#include <stdio.h>
#include "instruction_common.hpp"
#include "opcodes.hpp"
#include "registers.hpp"

/*
 * emit_* helpers for the no-operand instructions: halt, int, ret, iret.
 * Dispatched from asm_instr_no_operand() in src/assembler.cpp.
 */

inline void emit_halt() {
    require_section(OPC_HALT);
    current_section->appendInstruction(OC_HALT, 0x0, 0, 0, 0, 0);
    printf("INSTR: %s\n", op_name(OPC_HALT));
}

inline void emit_int() {
    require_section(OPC_INT);
    current_section->appendInstruction(OC_INT, 0x0, 0, 0, 0, 0);
    printf("INSTR: %s\n", op_name(OPC_INT));
}

// iret = pop pc; pop status (spec wording, matching push order: status
// pushed first, pc pushed last so pc sits on top of stack). Can't be two
// plain postinc pops in that order though: popping pc first would hand
// control to the restored address before the status pop ever executed.
// So status is read non-destructively from [sp+4] first (leaving sp
// untouched), and pc is popped last via postinc with D=8, which both
// reads the correct [sp] value and accounts for both words' worth of
// stack space in one go.
inline void emit_iret() {
    require_section(OPC_IRET);
    current_section->appendInstruction(OC_LOAD, MOD_LOAD_CSRRD_MEM, CSR_STATUS, REG_SP, 0, 4);
    current_section->appendInstruction(OC_LOAD, MOD_LOAD_MEM_POSTINC, REG_PC, REG_SP, 0, 8);
    printf("INSTR: %s\n", op_name(OPC_IRET));
}

// ret = pop pc: gpr[pc] <= mem32[sp]; sp <= sp + 4 (load, ld-mem-postinc).
inline void emit_ret() {
    require_section(OPC_RET);
    current_section->appendInstruction(OC_LOAD, MOD_LOAD_MEM_POSTINC, REG_PC, REG_SP, 0, 4);
    printf("INSTR: %s\n", op_name(OPC_RET));
}

#endif /* INSTRUCTION_NO_OPERAND_HPP */
