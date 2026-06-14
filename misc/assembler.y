%{
/*
 * parser.y
 *
 * Sintaksni analizator za asembler apstraktnog racunarskog sistema.
 *
 * Gramatika prati "Opste detalje", sve asemblerske direktive i sve
 * asemblerske naredbe iz postavke projekta. Semanticke akcije pozivaju
 * funkcije deklarisane u asm_types.h, koje treba implementirati u
 * okviru assembler.cpp (generisanje masinskog koda, tabela simbola,
 * relokacioni zapisi, itd.).
 *
 * Build (primer):
 *   bison -d -o parser.tab.c parser.y
 *   flex  -o lexer.c lexer.l
 *   g++ -o asembler lexer.c parser.tab.c assembler.cpp ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inc/asm_types.h"

int yylex(void);
int yyerror(const char *s);
extern int yylineno;
extern char *yytext;

%}

%union {
    long     ival;
    char    *sval;
    Operand *operand;
}

/* Terminali sa semantickom vrednoscu */
%token <sval> SYMBOL STRING
%token <ival> NUMBER GPR CSR

/* Direktive */
%token DIR_GLOBAL DIR_EXTERN DIR_SECTION DIR_WORD DIR_SKIP DIR_ASCII DIR_EQU DIR_END

/* Naredbe */
%token I_HALT I_INT I_IRET I_CALL I_RET I_JMP I_BEQ I_BNE I_BGT I_PUSH I_POP I_XCHG
%token I_ADD I_SUB I_MUL I_DIV I_NOT I_AND I_OR I_XOR I_SHL I_SHR I_LD I_ST I_CSRRD I_CSRWR

/* Interpunkcija, operatori i kraj linije */
%token COLON COMMA LBRACKET RBRACKET PLUS MINUS STAR SLASH LPAREN RPAREN DOLLAR NEWLINE

%type <operand> operand jump_operand
%type <ival>    expr

/* Prioriteti operatora za .equ izraz (od najnizeg ka najvisem) */
%left PLUS MINUS
%left STAR SLASH
%precedence UMINUS

%%

/* ====================================================================
 * Program se sastoji od niza linija. Svaka linija moze biti prazna,
 * sadrzati samo labelu, samo naredbu/direktivu, ili oboje. Linija se
 * zavrsava NEWLINE tokenom.
 * ==================================================================== */

program:
      /* prazno */
    | program line
    ;

line:
      NEWLINE
    | label NEWLINE
    | label statement NEWLINE
    | statement NEWLINE
    | label
    | statement
    | error NEWLINE
        {
            /* opcionalan oporavak od greske - nastavi sa narednom linijom */
            yyerrok;
        }
    ;

/* Labela se uvek nalazi na pocetku linije (eventualno nakon belina koje
 * lexer ignorise) i zavrsava se dvotackom. */
label:
      SYMBOL COLON
        {
            asm_define_label($1);
            free($1);
        }
    ;

statement:
      directive
    | instruction
    ;

/* ====================================================================
 * ASEMBLERSKE DIREKTIVE
 * ==================================================================== */

directive:
      DIR_GLOBAL
        { asm_directive_begin(DIRECTIVE_GLOBAL); }
      symbol_list

    | DIR_EXTERN
        { asm_directive_begin(DIRECTIVE_EXTERN); }
      symbol_list

    | DIR_SECTION SYMBOL
        {
            asm_directive_section($2);
            free($2);
        }

    | DIR_WORD
        { asm_directive_begin(DIRECTIVE_WORD); }
      init_list

    | DIR_SKIP NUMBER
        { asm_directive_skip($2); }

    | DIR_ASCII STRING
        {
            asm_directive_ascii($2);
            free($2);
        }

    | DIR_EQU SYMBOL COMMA expr
        {
            asm_directive_equ($2, $4);
            free($2);
        }

    | DIR_END
        { asm_directive_end(); }
    ;

/* Lista simbola za .global / .extern, razdvojena zapetama */
symbol_list:
      SYMBOL
        {
            asm_directive_symbol($1);
            free($1);
        }
    | symbol_list COMMA SYMBOL
        {
            asm_directive_symbol($3);
            free($3);
        }
    ;

/* Lista inicijalizatora za .word - literal ili simbol, razdvojeni zapetama */
init_list:
      init
    | init_list COMMA init
    ;

init:
      NUMBER
        { asm_directive_word_literal($1); }
    | SYMBOL
        {
            asm_directive_word_symbol($1);
            free($1);
        }
    ;

/* Jednostavan aritmeticki izraz za .equ direktivu (nivo C).
 * Vrednosti simbola koji se javljaju u izrazu se razresavaju preko
 * asm_expr_symbol (npr. iz tabele simbola koja je do tog trenutka
 * popunjena). */
expr:
      NUMBER                    { $$ = $1; }
    | SYMBOL                    { $$ = asm_expr_symbol($1); free($1); }
    | MINUS expr %prec UMINUS    { $$ = -$2; }
    | LPAREN expr RPAREN        { $$ = $2; }
    | expr PLUS expr            { $$ = $1 + $3; }
    | expr MINUS expr           { $$ = $1 - $3; }
    | expr STAR expr            { $$ = $1 * $3; }
    | expr SLASH expr           { $$ = $1 / $3; }
    ;

/* ====================================================================
 * ASEMBLERSKE NAREDBE
 * ==================================================================== */

instruction:
      /* Naredbe bez operanada */
      I_HALT  { asm_instr_no_operand(OPC_HALT); }
    | I_INT   { asm_instr_no_operand(OPC_INT);  }
    | I_IRET  { asm_instr_no_operand(OPC_IRET); }
    | I_RET   { asm_instr_no_operand(OPC_RET);  }

      /* Naredbe skoka i poziva potprograma */
    | I_CALL jump_operand
        { asm_instr_jump(OPC_CALL, $2); }
    | I_JMP jump_operand
        { asm_instr_jump(OPC_JMP, $2); }

      /* Uslovni skokovi: %gpr1, %gpr2, operand */
    | I_BEQ GPR COMMA GPR COMMA jump_operand
        { asm_instr_branch(OPC_BEQ, $2, $4, $6); }
    | I_BNE GPR COMMA GPR COMMA jump_operand
        { asm_instr_branch(OPC_BNE, $2, $4, $6); }
    | I_BGT GPR COMMA GPR COMMA jump_operand
        { asm_instr_branch(OPC_BGT, $2, $4, $6); }

      /* Naredbe sa jednim registrom */
    | I_PUSH GPR
        { asm_instr_one_reg(OPC_PUSH, $2); }
    | I_POP GPR
        { asm_instr_one_reg(OPC_POP, $2); }
    | I_NOT GPR
        { asm_instr_one_reg(OPC_NOT, $2); }

      /* Naredbe sa dva registra: %gprS, %gprD */
    | I_XCHG GPR COMMA GPR
        { asm_instr_two_reg(OPC_XCHG, $2, $4); }
    | I_ADD GPR COMMA GPR
        { asm_instr_two_reg(OPC_ADD, $2, $4); }
    | I_SUB GPR COMMA GPR
        { asm_instr_two_reg(OPC_SUB, $2, $4); }
    | I_MUL GPR COMMA GPR
        { asm_instr_two_reg(OPC_MUL, $2, $4); }
    | I_DIV GPR COMMA GPR
        { asm_instr_two_reg(OPC_DIV, $2, $4); }
    | I_AND GPR COMMA GPR
        { asm_instr_two_reg(OPC_AND, $2, $4); }
    | I_OR GPR COMMA GPR
        { asm_instr_two_reg(OPC_OR, $2, $4); }
    | I_XOR GPR COMMA GPR
        { asm_instr_two_reg(OPC_XOR, $2, $4); }
    | I_SHL GPR COMMA GPR
        { asm_instr_two_reg(OPC_SHL, $2, $4); }
    | I_SHR GPR COMMA GPR
        { asm_instr_two_reg(OPC_SHR, $2, $4); }

      /* Naredbe za rad sa podacima */
    | I_LD operand COMMA GPR
        { asm_instr_ld($2, $4); }
    | I_ST GPR COMMA operand
        { asm_instr_st($2, $4); }

      /* Naredbe za kontrolne i statusne registre */
    | I_CSRRD CSR COMMA GPR
        { asm_instr_csrrd($2, $4); }
    | I_CSRWR GPR COMMA CSR
        { asm_instr_csrwr($2, $4); }
    ;

/* ====================================================================
 * OPERANDI
 * ==================================================================== */

/* Operandi za naredbe skoka/poziva (call, jmp, beq, bne, bgt):
 *   <literal> ili <simbol> */
jump_operand:
      NUMBER
        { $$ = asm_operand_jump_literal($1); }
    | SYMBOL
        {
            $$ = asm_operand_jump_symbol($1);
            free($1);
        }
    ;

/* Operandi za naredbe za rad sa podacima (ld, st):
 *   $literal | $simbol | literal | simbol
 *   %reg | [%reg] | [%reg + literal] | [%reg + simbol] */
operand:
      DOLLAR NUMBER
        { $$ = asm_operand_imm_literal($2); }
    | DOLLAR SYMBOL
        {
            $$ = asm_operand_imm_symbol($2);
            free($2);
        }
    | NUMBER
        { $$ = asm_operand_mem_literal($1); }
    | SYMBOL
        {
            $$ = asm_operand_mem_symbol($1);
            free($1);
        }
    | GPR
        { $$ = asm_operand_reg_direct($1); }
    | LBRACKET GPR RBRACKET
        { $$ = asm_operand_reg_indirect($2); }
    | LBRACKET GPR PLUS NUMBER RBRACKET
        { $$ = asm_operand_reg_indirect_literal($2, $4); }
    | LBRACKET GPR MINUS NUMBER RBRACKET
        { $$ = asm_operand_reg_indirect_literal($2, -$4); }
    | LBRACKET GPR PLUS SYMBOL RBRACKET
        {
            $$ = asm_operand_reg_indirect_symbol($2, $4);
            free($4);
        }
    ;

%%

int yyerror(const char *s)
{
    fprintf(stderr, "Sintaksna greska na liniji %d (blizu '%s'): %s\n",
            yylineno, yytext, s);
    return 0;
}
