#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include "../inc/asm_types.h"
#include "../inc/assembler_internal.hpp"
#include "../inc/symbol_table.hpp"
#include "../inc/relocation_table.hpp"
#include "../inc/opcodes.hpp"
#include "../inc/registers.hpp"
#include "../inc/instruction_common.hpp"
#include "../inc/instruction_no_operand.hpp"
#include "../inc/instruction_one_reg.hpp"
#include "../inc/instruction_two_reg.hpp"

extern int yyparse(void);
extern FILE *yyin;

std::map <std::string, Section> sections;
Section *current_section = nullptr;

DirectiveKind current_directive;

SymbolTable symbol_table;
RelocationTable relocation_table;

void asm_define_label(const char *name) {
    if (!current_section) {
        fprintf(stderr, "Error: label defined outside of any section: %s\n", name);
        exit(1);
    }
    if (symbol_table.checkSymbolDefinition(name)) {
        fprintf(stderr, "Error: multiple label definitions: %s\n", name);
        exit(1);
    }
    if (symbol_table.isDeclaredExtern(name)) {
        fprintf(stderr, "Error: cannot define symbol declared extern: %s\n", name);
        exit(1);
    }

    int section_index = symbol_table.getSectionIndex(current_section->getName());
    if (section_index == -1) {
        fprintf(stderr, "Error: current section not found in symbol table: %s\n", current_section->getName().c_str());
        exit(1);
    }
    if (!symbol_table.checkEntry(name)) {
        symbol_table.addEntry( SymbolTableEntry{
            current_section->getLocationCounter(), // value will be set later
            name,
            section_index,
            LOCAL,
            NOTYP,
            true, // defined
            {}
        });
    }
    else {
        symbol_table.defineSymbol(name, section_index, current_section->getLocationCounter());
    }


    printf("LABEL: %s\n", name);
}

void asm_directive_begin(DirectiveKind kind) {
    current_directive = kind;
    printf("DIRECTIVE BEGIN: %d\n", (int)kind);
}

void asm_directive_symbol(const char *name) {
    LocalityKind locality;
    if (current_directive == DIRECTIVE_GLOBAL) {
        locality = GLOBAL;
    }
    else if (current_directive == DIRECTIVE_EXTERN) {
        locality = EXTERN;
    }
    else {
        fprintf(stderr, "Error: unexpected symbol in directive: %s\n", name);
        exit(1);
    }

    if (!symbol_table.checkEntry(name)) {
        symbol_table.addEntry(SymbolTableEntry{
            0,
            name,
            -1,
            locality,
            NOTYP,
            false,
            {}
        });
    }
    else {
        symbol_table.setLocality(name, locality);
    }
    printf("  SYMBOL: %s\n", name);
}

void asm_directive_section(const char *name) {
    symbol_table.addEntry(SymbolTableEntry{
        0, // value will be set later
        name,
        -1, // section index will be set later
        LOCAL,
        SCTN,
        true, // defined
        {}
    });
    auto result = sections.emplace(name, Section(name));
    current_section = &result.first->second;
    printf("SECTION: %s\n", name);
}

void asm_directive_word_literal(long value) {
    if (!current_section) {
        fprintf(stderr, "Error: .word outside of any section\n");
        exit(1);
    }

    current_section->appendBytes(value, 4);

    printf("  WORD LITERAL: %ld\n", value);
}

void asm_directive_word_symbol(const char *name) {
    if (!current_section) {
        fprintf(stderr, "Error: .word outside of any section\n");
        exit(1);
    }

    if (!symbol_table.checkEntry(name)) {
        symbol_table.addEntry(SymbolTableEntry{
            0,          // value unknown until defined
            name,
            -1,         // section unknown until defined
            LOCAL,
            NOTYP,
            false,      // not defined yet
            {}
        });
    }

    int section_index = symbol_table.getSectionIndex(current_section->getName());
    relocation_table.addEntry(RelocationEntry{
        current_section->getLocationCounter(),
        section_index,
        name
    });

    current_section->appendBytes(0, 4);

    printf("  WORD SYMBOL: %s\n", name);
}

void asm_directive_skip(long size) {
    if (!current_section) {
        fprintf(stderr, "Error: .skip outside of any section\n");
        exit(1);
    }
    if (size < 0) {
        fprintf(stderr, "Error: .skip size must be non-negative: %ld\n", size);
        exit(1);
    }

    current_section->appendBytes(0, size);

    printf("SKIP: %ld\n", size);
}

void asm_directive_ascii(const char *literal_with_quotes) {
    if (!current_section) {
        fprintf(stderr, "Error: .ascii outside of any section\n");
        exit(1);
    }

    size_t len = strlen(literal_with_quotes);
    if (len < 2 || literal_with_quotes[0] != '"' || literal_with_quotes[len - 1] != '"') {
        fprintf(stderr, "Error: malformed .ascii string literal: %s\n", literal_with_quotes);
        exit(1);
    }

    for (size_t i = 1; i < len - 1; i++) {
        char c = literal_with_quotes[i];

        if (c == '\\' && i + 1 < len - 1) {
            i++;
            switch (literal_with_quotes[i]) {
                case 'n':  c = '\n'; break;
                case 't':  c = '\t'; break;
                case 'r':  c = '\r'; break;
                case '0':  c = '\0'; break;
                case '\\': c = '\\'; break;
                case '"':  c = '"';  break;
                default:   c = literal_with_quotes[i]; break;
            }
        }

        current_section->appendBytes((unsigned char)c, 1);
    }

    printf("ASCII: %s\n", literal_with_quotes);
}

void asm_directive_equ(const char *name, long value) {
    printf("EQU: %s = %ld\n", name, value);
}

void asm_directive_end(void) {
    printf("END\n");
}

long asm_expr_symbol(const char *name) {
    printf("  (expr symbol %s -> 0)\n", name);
    return 0;
}

static Operand *make_op(OperandKind k) {
    Operand *o = (Operand*)calloc(1, sizeof(Operand));
    o->kind = k;
    return o;
}

Operand *asm_operand_imm_literal(long value) {
    Operand *o = make_op(OPERAND_IMM_LIT);
    o->literal = value;
    return o;
}
Operand *asm_operand_imm_symbol(const char *name) {
    Operand *o = make_op(OPERAND_IMM_SYM);
    o->symbol = strdup(name);
    return o;
}
Operand *asm_operand_mem_literal(long value) {
    Operand *o = make_op(OPERAND_MEM_LIT);
    o->literal = value;
    return o;
}
Operand *asm_operand_mem_symbol(const char *name) {
    Operand *o = make_op(OPERAND_MEM_SYM);
    o->symbol = strdup(name);
    return o;
}
Operand *asm_operand_reg_direct(long reg) {
    Operand *o = make_op(OPERAND_REG_DIR);
    o->reg = reg;
    return o;
}
Operand *asm_operand_reg_indirect(long reg) {
    Operand *o = make_op(OPERAND_REG_IND);
    o->reg = reg;
    return o;
}
Operand *asm_operand_reg_indirect_literal(long reg, long disp) {
    Operand *o = make_op(OPERAND_REG_IND_LIT);
    o->reg = reg;
    o->literal = disp;
    return o;
}
Operand *asm_operand_reg_indirect_symbol(long reg, const char *symbol) {
    Operand *o = make_op(OPERAND_REG_IND_SYM);
    o->reg = reg;
    o->symbol = strdup(symbol);
    return o;
}
Operand *asm_operand_jump_literal(long value) {
    Operand *o = make_op(OPERAND_JUMP_LIT);
    o->literal = value;
    return o;
}
Operand *asm_operand_jump_symbol(const char *name) {
    Operand *o = make_op(OPERAND_JUMP_SYM);
    o->symbol = strdup(name);
    return o;
}

static void print_operand(const Operand *o) {
    switch (o->kind) {
        case OPERAND_IMM_LIT: printf("$%ld", o->literal); break;
        case OPERAND_IMM_SYM: printf("$%s", o->symbol); break;
        case OPERAND_MEM_LIT: printf("%ld", o->literal); break;
        case OPERAND_MEM_SYM: printf("%s", o->symbol); break;
        case OPERAND_REG_DIR: printf("%%r%ld", o->reg); break;
        case OPERAND_REG_IND: printf("[%%r%ld]", o->reg); break;
        case OPERAND_REG_IND_LIT: printf("[%%r%ld + %ld]", o->reg, o->literal); break;
        case OPERAND_REG_IND_SYM: printf("[%%r%ld + %s]", o->reg, o->symbol); break;
        case OPERAND_JUMP_LIT: printf("%ld", o->literal); break;
        case OPERAND_JUMP_SYM: printf("%s", o->symbol); break;
    }
}

void asm_instr_no_operand(OpCode op) {
    switch (op) {
        case OPC_HALT: emit_halt(); break;
        case OPC_INT:  emit_int();  break;
        case OPC_IRET: emit_iret(); break;
        case OPC_RET:  emit_ret();  break;
        default:       printf("INSTR: %s\n", op_name(op)); break;
    }
}
void asm_instr_jump(OpCode op, Operand *target) {
    printf("INSTR: %s ", op_name(op));
    print_operand(target);
    printf("\n");
    free(target);
}
void asm_instr_branch(OpCode op, long gpr1, long gpr2, Operand *target) {
    printf("INSTR: %s %%r%ld, %%r%ld, ", op_name(op), gpr1, gpr2);
    print_operand(target);
    printf("\n");
    free(target);
}
void asm_instr_one_reg(OpCode op, long gpr) {
    switch (op) {
        case OPC_PUSH: emit_push(gpr); break;
        case OPC_POP:  emit_pop(gpr);  break;
        case OPC_NOT:  emit_not(gpr);  break;
        default: printf("INSTR: %s %%r%ld\n", op_name(op), gpr); break;
    }
}
void asm_instr_two_reg(OpCode op, long gprS, long gprD) {
    switch (op) {
        case OPC_ADD:  emit_add(gprS, gprD);  break;
        case OPC_SUB:  emit_sub(gprS, gprD);  break;
        case OPC_MUL:  emit_mul(gprS, gprD);  break;
        case OPC_DIV:  emit_div(gprS, gprD);  break;
        case OPC_XCHG: emit_xchg(gprS, gprD); break;
        case OPC_AND:  emit_and(gprS, gprD);  break;
        case OPC_OR:   emit_or(gprS, gprD);   break;
        case OPC_XOR:  emit_xor(gprS, gprD);  break;
        case OPC_SHL:  emit_shl(gprS, gprD);  break;
        case OPC_SHR:  emit_shr(gprS, gprD);  break;
        default: printf("INSTR: %s %%r%ld, %%r%ld\n", op_name(op), gprS, gprD); break;
    }
}
void asm_instr_ld(Operand *src, long gprD) {
    printf("INSTR: ld ");
    print_operand(src);
    printf(", %%r%ld\n", gprD);
    free(src);
}
void asm_instr_st(long gprS, Operand *dst) {
    printf("INSTR: st %%r%ld, ", gprS);
    print_operand(dst);
    printf("\n");
    free(dst);
}
void asm_instr_csrrd(long csr, long gpr) {
    printf("INSTR: csrrd csr%ld, %%r%ld\n", csr, gpr);
}
void asm_instr_csrwr(long gpr, long csr) {
    printf("INSTR: csrwr %%r%ld, csr%ld\n", gpr, csr);
}

int main(int argc, char **argv) {

    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            perror("fopen");
            return 1;
        }
    }
    return yyparse();
}