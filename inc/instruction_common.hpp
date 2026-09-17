#ifndef INSTRUCTION_COMMON_HPP
#define INSTRUCTION_COMMON_HPP

#include <stdio.h>
#include <stdlib.h>
#include "asm_types.h"
#include "assembler_internal.hpp"

/*
 * Shared helpers used by every instruction_*.hpp emit_* family: naming an
 * OpCode for the "INSTR: ..." trace lines, and guarding that an
 * instruction only ever gets emitted while inside a section.
 */

inline const char *op_name(OpCode op) {
    switch (op) {
        case OPC_HALT: return "halt";
        case OPC_INT: return "int";
        case OPC_IRET: return "iret";
        case OPC_RET: return "ret";
        case OPC_CALL: return "call";
        case OPC_JMP: return "jmp";
        case OPC_BEQ: return "beq";
        case OPC_BNE: return "bne";
        case OPC_BGT: return "bgt";
        case OPC_PUSH: return "push";
        case OPC_POP: return "pop";
        case OPC_NOT: return "not";
        case OPC_XCHG: return "xchg";
        case OPC_ADD: return "add";
        case OPC_SUB: return "sub";
        case OPC_MUL: return "mul";
        case OPC_DIV: return "div";
        case OPC_AND: return "and";
        case OPC_OR: return "or";
        case OPC_XOR: return "xor";
        case OPC_SHL: return "shl";
        case OPC_SHR: return "shr";
        case OPC_LD: return "ld";
        case OPC_ST: return "st";
        case OPC_CSRRD: return "csrrd";
        case OPC_CSRWR: return "csrwr";
    }
    return "?";
}

inline void require_section(OpCode op) {
    if (!current_section) {
        fprintf(stderr, "Error: instruction outside of any section: %s\n", op_name(op));
        exit(1);
    }
}

#endif /* INSTRUCTION_COMMON_HPP */
