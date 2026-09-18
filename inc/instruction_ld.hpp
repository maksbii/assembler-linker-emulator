#ifndef INSTRUCTION_LD_HPP
#define INSTRUCTION_LD_HPP

#include <stdio.h>
#include <stdlib.h>
#include "instruction_common.hpp"
#include "opcodes.hpp"
#include "registers.hpp"
#include "symbol_table.hpp"

extern SymbolTable symbol_table;

/*
 * emit_ld: `ld src, %gprD`. Handles the addressing modes that encode
 * directly as a single word. Operand kinds that need a literal pool
 * (IMM_LIT/MEM_LIT out of 12-bit range, IMM_SYM, MEM_SYM) aren't
 * implemented yet -- see TRACKER.md's "ld instruction planning" entry.
 */
inline void emit_ld(Operand *src, long gprD) {
    require_section(OPC_LD);

    switch (src->kind) {
        // gprD <= rS -> MOD_LOAD_IMM_DISP, A=gprD, B=rS, D=0
        case OPERAND_REG_DIR:
            current_section->appendInstruction(OC_LOAD, MOD_LOAD_IMM_DISP, gprD, src->reg, 0, 0);
            printf("INSTR: ld %%r%ld, %%r%ld\n", src->reg, gprD);
            break;

        // gprD <= mem32[rS] -> MOD_LOAD_MEM, A=gprD, B=rS, C=0, D=0
        case OPERAND_REG_IND:
            current_section->appendInstruction(OC_LOAD, MOD_LOAD_MEM, gprD, src->reg, 0, 0);
            printf("INSTR: ld [%%r%ld], %%r%ld\n", src->reg, gprD);
            break;

        // gprD <= mem32[rS + lit] -> MOD_LOAD_MEM, A=gprD, B=rS, C=0, D=lit.
        // Out-of-range is an assembler error, not a literal-pool case (spec:
        // [%reg + literal] outside the 12-bit signed range must be reported
        // as an error).
        case OPERAND_REG_IND_LIT:
            if (!fits_signed12(src->literal)) {
                fprintf(stderr, "Error: displacement out of 12-bit signed range in ld [%%r%ld + %ld], %%r%ld: %ld\n",
                        src->reg, src->literal, gprD, src->literal);
                exit(1);
            }
            current_section->appendInstruction(OC_LOAD, MOD_LOAD_MEM, gprD, src->reg, 0, src->literal);
            printf("INSTR: ld [%%r%ld + %ld], %%r%ld\n", src->reg, src->literal, gprD);
            break;

        // gprD <= mem32[rS + sym] -- only valid if sym is already a resolved
        // absolute constant (.equ-style, sectionIndex == -1) that fits 12
        // bits; section-relative symbols need relocation and can't be
        // folded into a plain displacement at assembly time.
        case OPERAND_REG_IND_SYM: {
            SymbolTableEntry *entry = symbol_table.getEntry(src->symbol);
            if (!entry || !entry->defined || entry->sectionIndex != -1) {
                fprintf(stderr, "Error: symbol used as displacement is not a resolved absolute constant: %s\n", src->symbol);
                exit(1);
            }
            if (!fits_signed12(entry->value)) {
                fprintf(stderr, "Error: displacement out of 12-bit signed range in ld [%%r%ld + %s], %%r%ld: %ld\n",
                        src->reg, src->symbol, gprD, entry->value);
                exit(1);
            }
            current_section->appendInstruction(OC_LOAD, MOD_LOAD_MEM, gprD, src->reg, 0, entry->value);
            printf("INSTR: ld [%%r%ld + %s], %%r%ld\n", src->reg, src->symbol, gprD);
            break;
        }

        // gprD <= lit -> MOD_LOAD_IMM_DISP, A=gprD, B=r0, D=lit if it fits;
        // otherwise gprD <= lit via a literal-pool slot holding the value,
        // loaded PC-relative (MOD_LOAD_MEM, B=pc).
        case OPERAND_IMM_LIT:
            if (fits_signed12(src->literal)) {
                current_section->appendInstruction(OC_LOAD, MOD_LOAD_IMM_DISP, gprD, 0, 0, src->literal);
            } else {
                current_section->appendPoolLiteral(OC_LOAD, MOD_LOAD_MEM, gprD, REG_PC, 0, src->literal);
            }
            printf("INSTR: ld $%ld, %%r%ld\n", src->literal, gprD);
            break;

        // gprD <= mem32[lit] -> MOD_LOAD_MEM, A=gprD, B=r0, C=0, D=lit if it
        // fits; otherwise load the address from the literal pool first
        // (word 1), then dereference it (word 2).
        case OPERAND_MEM_LIT:
            if (fits_signed12(src->literal)) {
                current_section->appendInstruction(OC_LOAD, MOD_LOAD_MEM, gprD, 0, 0, src->literal);
            } else {
                current_section->appendPoolLiteral(OC_LOAD, MOD_LOAD_MEM, gprD, REG_PC, 0, src->literal);
                current_section->appendInstruction(OC_LOAD, MOD_LOAD_MEM, gprD, gprD, 0, 0);
            }
            printf("INSTR: ld %ld, %%r%ld\n", src->literal, gprD);
            break;

        // gprD <= address-of(sym): final address unknown until link time,
        // so this always needs a literal pool slot + relocation entry.
        case OPERAND_IMM_SYM:
            current_section->appendPoolSymbol(OC_LOAD, MOD_LOAD_MEM, gprD, REG_PC, 0, src->symbol);
            printf("INSTR: ld $%s, %%r%ld\n", src->symbol, gprD);
            break;

        // gprD <= mem32[address-of(sym)]: literal pool + relocation for the
        // address (word 1), then dereference it (word 2).
        case OPERAND_MEM_SYM:
            current_section->appendPoolSymbol(OC_LOAD, MOD_LOAD_MEM, gprD, REG_PC, 0, src->symbol);
            current_section->appendInstruction(OC_LOAD, MOD_LOAD_MEM, gprD, gprD, 0, 0);
            printf("INSTR: ld %s, %%r%ld\n", src->symbol, gprD);
            break;

        default:
            fprintf(stderr, "Error: invalid operand kind for ld\n");
            exit(1);
    }
}

#endif /* INSTRUCTION_LD_HPP */
