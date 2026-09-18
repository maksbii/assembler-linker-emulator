#ifndef INSTRUCTION_ST_HPP
#define INSTRUCTION_ST_HPP

#include <stdio.h>
#include <stdlib.h>
#include "instruction_common.hpp"
#include "opcodes.hpp"
#include "registers.hpp"
#include "symbol_table.hpp"

extern SymbolTable symbol_table;

/*
 * emit_st: `st %gprS, dst`. dst shares the same 8-kind `operand` grammar
 * rule as `ld`'s src, but only the kinds that name an actual memory
 * location make sense as a store target. REG_DIR/IMM_LIT/IMM_SYM are
 * "value" operands (a register's contents, or an immediate/its address as
 * a plain value) with no memory location to store into, so they're
 * assembler errors here -- see TRACKER.md's "st instruction" entry.
 */
inline void emit_st(long gprS, Operand *dst) {
    require_section(OPC_ST);

    switch (dst->kind) {
        // No memory location: storing gprS "into a register" isn't what
        // st does (ld's REG_DIR case is a register-to-register move, which
        // has no store counterpart).
        case OPERAND_REG_DIR:
            fprintf(stderr, "Error: invalid operand for st (register-direct has no memory destination): %%r%ld\n", dst->reg);
            exit(1);

        // mem32[rT] <= gprS -> MOD_STORE_DIRECT, A=rT, B=0, C=gprS, D=0.
        case OPERAND_REG_IND:
            current_section->appendInstruction(OC_STORE, MOD_STORE_DIRECT, dst->reg, 0, gprS, 0);
            printf("INSTR: st %%r%ld, [%%r%ld]\n", gprS, dst->reg);
            break;

        // mem32[rT + lit] <= gprS -> MOD_STORE_DIRECT, A=rT, B=0, C=gprS,
        // D=lit. Out-of-range is an assembler error, not a literal-pool
        // case, same rule as ld's [%reg + literal].
        case OPERAND_REG_IND_LIT:
            if (!fits_signed12(dst->literal)) {
                fprintf(stderr, "Error: displacement out of 12-bit signed range in st %%r%ld, [%%r%ld + %ld]: %ld\n",
                        gprS, dst->reg, dst->literal, dst->literal);
                exit(1);
            }
            current_section->appendInstruction(OC_STORE, MOD_STORE_DIRECT, dst->reg, 0, gprS, dst->literal);
            printf("INSTR: st %%r%ld, [%%r%ld + %ld]\n", gprS, dst->reg, dst->literal);
            break;

        // mem32[rT + sym] <= gprS -- only valid if sym is already a
        // resolved absolute constant (sectionIndex == -1) that fits 12
        // bits, same rule as ld's [%reg + symbol].
        case OPERAND_REG_IND_SYM: {
            SymbolTableEntry *entry = symbol_table.getEntry(dst->symbol);
            if (!entry || !entry->defined || entry->sectionIndex != -1) {
                fprintf(stderr, "Error: symbol used as displacement is not a resolved absolute constant: %s\n", dst->symbol);
                exit(1);
            }
            if (!fits_signed12(entry->value)) {
                fprintf(stderr, "Error: displacement out of 12-bit signed range in st %%r%ld, [%%r%ld + %s]: %ld\n",
                        gprS, dst->reg, dst->symbol, entry->value);
                exit(1);
            }
            current_section->appendInstruction(OC_STORE, MOD_STORE_DIRECT, dst->reg, 0, gprS, entry->value);
            printf("INSTR: st %%r%ld, [%%r%ld + %s]\n", gprS, dst->reg, dst->symbol);
            break;
        }

        // "$lit"/"$sym" name a value, not a memory location (mirrors ld's
        // immediate cases, which have no store counterpart).
        case OPERAND_IMM_LIT:
            fprintf(stderr, "Error: invalid operand for st (immediate has no memory destination): $%ld\n", dst->literal);
            exit(1);
        case OPERAND_IMM_SYM:
            fprintf(stderr, "Error: invalid operand for st (immediate has no memory destination): $%s\n", dst->symbol);
            exit(1);

        // mem32[lit] <= gprS -> MOD_STORE_DIRECT, A=0, B=0, C=gprS, D=lit
        // if it fits; otherwise the target address comes from a literal
        // pool slot. Since st has no scratch register available (unlike
        // ld's two-word oversized case), this uses MOD_STORE_INDIRECT in a
        // single word: A=pc, B=0, D=pool-disp (backpatched) so
        // gpr[A]+gpr[B]+D addresses the pool slot itself, and
        // mem32[mem32[pool_slot]] <= gpr[C] both reads the address out of
        // the pool and stores through it in one instruction.
        case OPERAND_MEM_LIT:
            if (fits_signed12(dst->literal)) {
                current_section->appendInstruction(OC_STORE, MOD_STORE_DIRECT, 0, 0, gprS, dst->literal);
            } else {
                current_section->appendPoolLiteral(OC_STORE, MOD_STORE_INDIRECT, REG_PC, 0, gprS, dst->literal);
            }
            printf("INSTR: st %%r%ld, %ld\n", gprS, dst->literal);
            break;

        // mem32[address-of(sym)] <= gprS: final address unknown until link
        // time, so this always needs a literal pool slot + relocation,
        // using the same single-word MOD_STORE_INDIRECT trick as the
        // oversized MEM_LIT case above.
        case OPERAND_MEM_SYM:
            current_section->appendPoolSymbol(OC_STORE, MOD_STORE_INDIRECT, REG_PC, 0, gprS, dst->symbol);
            printf("INSTR: st %%r%ld, %s\n", gprS, dst->symbol);
            break;

        default:
            fprintf(stderr, "Error: invalid operand kind for st\n");
            exit(1);
    }
}

#endif /* INSTRUCTION_ST_HPP */
