/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_ASSEMBLER_TAB_H_INCLUDED
# define YY_YY_ASSEMBLER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    SYMBOL = 258,
    STRING = 259,
    NUMBER = 260,
    GPR = 261,
    CSR = 262,
    DIR_GLOBAL = 263,
    DIR_EXTERN = 264,
    DIR_SECTION = 265,
    DIR_WORD = 266,
    DIR_SKIP = 267,
    DIR_ASCII = 268,
    DIR_EQU = 269,
    DIR_END = 270,
    I_HALT = 271,
    I_INT = 272,
    I_IRET = 273,
    I_CALL = 274,
    I_RET = 275,
    I_JMP = 276,
    I_BEQ = 277,
    I_BNE = 278,
    I_BGT = 279,
    I_PUSH = 280,
    I_POP = 281,
    I_XCHG = 282,
    I_ADD = 283,
    I_SUB = 284,
    I_MUL = 285,
    I_DIV = 286,
    I_NOT = 287,
    I_AND = 288,
    I_OR = 289,
    I_XOR = 290,
    I_SHL = 291,
    I_SHR = 292,
    I_LD = 293,
    I_ST = 294,
    I_CSRRD = 295,
    I_CSRWR = 296,
    COLON = 297,
    COMMA = 298,
    LBRACKET = 299,
    RBRACKET = 300,
    PLUS = 301,
    MINUS = 302,
    STAR = 303,
    SLASH = 304,
    LPAREN = 305,
    RPAREN = 306,
    DOLLAR = 307,
    NEWLINE = 308,
    UMINUS = 309
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 31 "misc/assembler.y"

    long     ival;
    char    *sval;
    Operand *operand;

#line 118 "assembler.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ASSEMBLER_TAB_H_INCLUDED  */
