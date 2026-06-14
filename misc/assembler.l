%{
/*
 * lexer.l
 *
 * Leksicki analizator za asembler apstraktnog racunarskog sistema.
 * Pokriva sve opste detalje sintakse, sve asemblerske direktive
 * (.global, .extern, .section, .word, .skip, .ascii, .equ, .end)
 * i sve asemblerske naredbe iz postavke projekta.
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
#include "assembler.tab.h"

%}

%option noyywrap
%option yylineno

DIGIT     [0-9]
HEXDIGIT  [0-9a-fA-F]
ID        [a-zA-Z_][a-zA-Z0-9_]*

%%

    /* --------------------------------------------------------------- */
    /* Komentari i razmaci                                              */
    /* --------------------------------------------------------------- */
[ \t]+              { /* belina se ignorise */ }
"#".*               { /* jednolinijski komentar - ignorisi do kraja linije */ }
\r?\n               { return NEWLINE; }

    /* --------------------------------------------------------------- */
    /* Asemblerske direktive                                            */
    /* --------------------------------------------------------------- */
"."global           { return DIR_GLOBAL; }
"."extern           { return DIR_EXTERN; }
"."section          { return DIR_SECTION; }
"."word             { return DIR_WORD; }
"."skip             { return DIR_SKIP; }
"."ascii            { return DIR_ASCII; }
"."equ              { return DIR_EQU; }
"."end              { return DIR_END; }

    /* --------------------------------------------------------------- */
    /* Asemblerske naredbe                                              */
    /* --------------------------------------------------------------- */
"halt"              { return I_HALT; }
"int"               { return I_INT; }
"iret"              { return I_IRET; }
"call"              { return I_CALL; }
"ret"               { return I_RET; }
"jmp"               { return I_JMP; }
"beq"               { return I_BEQ; }
"bne"               { return I_BNE; }
"bgt"               { return I_BGT; }
"push"              { return I_PUSH; }
"pop"               { return I_POP; }
"xchg"              { return I_XCHG; }
"add"               { return I_ADD; }
"sub"               { return I_SUB; }
"mul"               { return I_MUL; }
"div"               { return I_DIV; }
"not"               { return I_NOT; }
"and"               { return I_AND; }
"or"                { return I_OR; }
"xor"               { return I_XOR; }
"shl"               { return I_SHL; }
"shr"               { return I_SHR; }
"ld"                { return I_LD; }
"st"                { return I_ST; }
"csrrd"             { return I_CSRRD; }
"csrwr"             { return I_CSRWR; }

    /* --------------------------------------------------------------- */
    /* Opstenamenski registri: r0-r15, sa aliasima sp (r14) i pc (r15)  */
    /* --------------------------------------------------------------- */
"%sp"               { yylval.ival = 14; return GPR; }
"%pc"               { yylval.ival = 15; return GPR; }
"%r"(1[0-5]|[0-9])  { yylval.ival = atoi(yytext + 2); return GPR; }

    /* --------------------------------------------------------------- */
    /* Kontrolni i statusni registri                                    */
    /* --------------------------------------------------------------- */
"%status"           { yylval.ival = 0; return CSR; }
"%handler"          { yylval.ival = 1; return CSR; }
"%cause"            { yylval.ival = 2; return CSR; }

    /* --------------------------------------------------------------- */
    /* Brojevne konstante: heksadekadne i dekadne                       */
    /* --------------------------------------------------------------- */
0[xX]{HEXDIGIT}+    { yylval.ival = strtol(yytext, NULL, 16); return NUMBER; }
{DIGIT}+            { yylval.ival = strtol(yytext, NULL, 10); return NUMBER; }

    /* --------------------------------------------------------------- */
    /* String literal za .ascii direktivu                               */
    /* --------------------------------------------------------------- */
\"([^"\\\n]|\\.)*\" {
                        /* yytext sadrzi i znake navoda - obrada (uklanjanje
                           navoda i interpretacija escape sekvenci) se vrsi
                           u asm_directive_ascii */
                        yylval.sval = strdup(yytext);
                        return STRING;
                    }

    /* --------------------------------------------------------------- */
    /* Simboli (imena labela, sekcija, .equ simbola, itd.)              */
    /* --------------------------------------------------------------- */
{ID}                { yylval.sval = strdup(yytext); return SYMBOL; }

    /* --------------------------------------------------------------- */
    /* Interpunkcija i operatori                                        */
    /* --------------------------------------------------------------- */
":"                 { return COLON; }
","                 { return COMMA; }
"["                 { return LBRACKET; }
"]"                 { return RBRACKET; }
"+"                 { return PLUS; }
"-"                 { return MINUS; }
"*"                 { return STAR; }
"/"                 { return SLASH; }
"("                 { return LPAREN; }
")"                 { return RPAREN; }
"$"                 { return DOLLAR; }

    /* --------------------------------------------------------------- */
    /* Sve sto nije prepoznato - leksicka greska                        */
    /* --------------------------------------------------------------- */
.                   {
                        fprintf(stderr,
                                "Leksicka greska na liniji %d: neocekivan karakter '%s'\n",
                                yylineno, yytext);
                    }

%%
