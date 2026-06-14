#ifndef ASM_TYPES_H
#define ASM_TYPES_H

/*
 * Ovaj fajl sadrzi zajednicke tipove koje koriste lexer.l i parser.y,
 * kao i deklaracije funkcija koje parser poziva tokom parsiranja.
 *
 * Sve funkcije asm_* treba implementirati u okviru assembler.cpp.
 * Ovaj header sluzi samo kao "ugovor" izmedju parsera i ostatka
 * implementacije asemblera.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Vrste operanada (videti odeljak "Asemblerske naredbe" u postavci)   */
/* ------------------------------------------------------------------ */
typedef enum {
    OPERAND_IMM_LIT,     /* $literal               */
    OPERAND_IMM_SYM,     /* $simbol                */
    OPERAND_MEM_LIT,     /* literal  (memdir)      */
    OPERAND_MEM_SYM,     /* simbol   (memdir)      */
    OPERAND_REG_DIR,     /* %reg                   */
    OPERAND_REG_IND,     /* [%reg]                 */
    OPERAND_REG_IND_LIT, /* [%reg + literal]       */
    OPERAND_REG_IND_SYM, /* [%reg + simbol]        */
    OPERAND_JUMP_LIT,    /* literal  (za jmp/call/b*) */
    OPERAND_JUMP_SYM     /* simbol   (za jmp/call/b*) */
} OperandKind;

typedef struct {
    OperandKind kind;
    long        reg;     /* broj registra, koristi se za REG_* tipove */
    long        literal; /* literal vrednost, koristi se za *_LIT tipove */
    char       *symbol;  /* ime simbola, koristi se za *_SYM tipove */
} Operand;

/* ------------------------------------------------------------------ */
/* Vrste asemblerskih naredbi bez operanada / sa operandima           */
/* ------------------------------------------------------------------ */
typedef enum {
    OPC_HALT, OPC_INT, OPC_IRET, OPC_RET,
    OPC_CALL, OPC_JMP,
    OPC_BEQ,  OPC_BNE,  OPC_BGT,
    OPC_PUSH, OPC_POP,  OPC_NOT,
    OPC_XCHG, OPC_ADD,  OPC_SUB, OPC_MUL, OPC_DIV,
    OPC_AND,  OPC_OR,   OPC_XOR, OPC_SHL, OPC_SHR,
    OPC_LD,   OPC_ST,
    OPC_CSRRD, OPC_CSRWR
} OpCode;

/* ------------------------------------------------------------------ */
/* Vrste asemblerskih direktiva                                       */
/* ------------------------------------------------------------------ */
typedef enum {
    DIRECTIVE_GLOBAL,
    DIRECTIVE_EXTERN,
    DIRECTIVE_WORD
} DirectiveKind;

/* ------------------------------------------------------------------ */
/* Funkcije koje pozivaju akcije u parser.y - implementirano u        */
/* assembler.cpp                                                      */
/* ------------------------------------------------------------------ */

/* obrada labele "ime:" */
void asm_define_label(const char *name);

/* pocetak parsiranja .global / .extern / .word direktive
   - koristi se sa promenljivim brojem parametara (lista simbola/inicijalizatora) */
void asm_directive_begin(DirectiveKind kind);

/* poziva se za svaki simbol u listi nakon .global ili .extern */
void asm_directive_symbol(const char *name);

/* .section <ime> */
void asm_directive_section(const char *name);

/* .word - inicijalizator je literal */
void asm_directive_word_literal(long value);

/* .word - inicijalizator je simbol */
void asm_directive_word_symbol(const char *name);

/* .skip <literal> */
void asm_directive_skip(long size);

/* .ascii "<string>" - prosledjuje se string zajedno sa znacima navoda */
void asm_directive_ascii(const char *literal_with_quotes);

/* .equ <novi_simbol>, <izraz> - vrednost izraza je vec izracunata u $4 */
void asm_directive_equ(const char *name, long value);

/* .end */
void asm_directive_end(void);

/* Ako se u .equ izrazu pojavi simbol, treba vratiti njegovu (do sada
   poznatu) vrednost - npr. iz tabele simbola. */
long asm_expr_symbol(const char *name);

/* ------------------------------------------------------------------ */
/* Konstruktori operanada - svaki vraca dinamicki alociranu strukturu */
/* Operand koju parser prenosi instrukcijama. Implementacija treba    */
/* da je oslobodi (free) kada vise nije potrebna.                     */
/* ------------------------------------------------------------------ */
Operand *asm_operand_imm_literal(long value);
Operand *asm_operand_imm_symbol(const char *name);
Operand *asm_operand_mem_literal(long value);
Operand *asm_operand_mem_symbol(const char *name);
Operand *asm_operand_reg_direct(long reg);
Operand *asm_operand_reg_indirect(long reg);
Operand *asm_operand_reg_indirect_literal(long reg, long disp);
Operand *asm_operand_reg_indirect_symbol(long reg, const char *symbol);

Operand *asm_operand_jump_literal(long value);
Operand *asm_operand_jump_symbol(const char *name);

/* ------------------------------------------------------------------ */
/* Generisanje instrukcija                                             */
/* ------------------------------------------------------------------ */
void asm_instr_no_operand(OpCode op);                       /* halt, int, iret, ret */
void asm_instr_jump(OpCode op, Operand *target);            /* call, jmp */
void asm_instr_branch(OpCode op, long gpr1, long gpr2, Operand *target); /* beq, bne, bgt */
void asm_instr_one_reg(OpCode op, long gpr);                /* push, pop, not */
void asm_instr_two_reg(OpCode op, long gprS, long gprD);    /* xchg, add, sub, ... */
void asm_instr_ld(Operand *src, long gprD);                 /* ld operand, %gpr */
void asm_instr_st(long gprS, Operand *dst);                 /* st %gpr, operand */
void asm_instr_csrrd(long csr, long gpr);                   /* csrrd %csr, %gpr */
void asm_instr_csrwr(long gpr, long csr);                   /* csrwr %gpr, %csr */

#ifdef __cplusplus
}
#endif

#endif /* ASM_TYPES_H */
