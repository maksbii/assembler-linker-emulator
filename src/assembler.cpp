#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/asm_types.h"

extern int yyparse(void);
extern FILE *yyin;

void asm_define_label(const char *name) {
    printf("LABEL: %s\n", name);
}

void asm_directive_begin(DirectiveKind kind) {
    printf("DIRECTIVE BEGIN: %d\n", (int)kind);
}

void asm_directive_symbol(const char *name) {
    printf("  SYMBOL: %s\n", name);
}

void asm_directive_section(const char *name) {
    printf("SECTION: %s\n", name);
}

void asm_directive_word_literal(long value) {
    printf("  WORD LITERAL: %ld\n", value);
}

void asm_directive_word_symbol(const char *name) {
    printf("  WORD SYMBOL: %s\n", name);
}

void asm_directive_skip(long size) {
    printf("SKIP: %ld\n", size);
}

void asm_directive_ascii(const char *literal_with_quotes) {
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

static const char *op_name(OpCode op) {
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
    printf("INSTR: %s\n", op_name(op));
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
    printf("INSTR: %s %%r%ld\n", op_name(op), gpr);
}
void asm_instr_two_reg(OpCode op, long gprS, long gprD) {
    printf("INSTR: %s %%r%ld, %%r%ld\n", op_name(op), gprS, gprD);
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