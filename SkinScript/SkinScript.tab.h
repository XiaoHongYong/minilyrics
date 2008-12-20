/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TK_IF = 258,
     TK_ELSE = 259,
     TK_WHILE = 260,
     TK_DO = 261,
     TK_NEW = 262,
     TK_DELETE = 263,
     TK_GLOBAL = 264,
     TK_FUNCTION = 265,
     TK_CLASS = 266,
     TK_RETURN = 267,
     TK_REFERENCE = 268,
     TK_ID = 269,
     TK_NUMBER = 270,
     TK_C_STRING = 271,
     TK_L_PARENTHESIS = 272,
     TK_R_PARENTHESIS = 273,
     TK_COLON = 274,
     TK_SEMICOLON = 275,
     TK_L_BRACE = 276,
     TK_R_BRACE = 277,
     TK_COMMA = 278,
     TK_DOT = 279,
     TK_ASSIGN = 280,
     TK_OP_INC = 281,
     TK_OP_DEC = 282,
     TK_PLUS = 283,
     TK_MINUS = 284,
     TK_MULT = 285,
     TK_DIV = 286,
     TK_LT_EQ = 287,
     TK_GT_EQ = 288,
     TK_LITTLE = 289,
     TK_GREATER = 290,
     TK_EQUAL = 291,
     TK_NOT_EQUAL = 292,
     TK_BOOL_AND = 293,
     TK_BOOL_OR = 294,
     TK_ERROR = 295
   };
#endif
/* Tokens.  */
#define TK_IF 258
#define TK_ELSE 259
#define TK_WHILE 260
#define TK_DO 261
#define TK_NEW 262
#define TK_DELETE 263
#define TK_GLOBAL 264
#define TK_FUNCTION 265
#define TK_CLASS 266
#define TK_RETURN 267
#define TK_REFERENCE 268
#define TK_ID 269
#define TK_NUMBER 270
#define TK_C_STRING 271
#define TK_L_PARENTHESIS 272
#define TK_R_PARENTHESIS 273
#define TK_COLON 274
#define TK_SEMICOLON 275
#define TK_L_BRACE 276
#define TK_R_BRACE 277
#define TK_COMMA 278
#define TK_DOT 279
#define TK_ASSIGN 280
#define TK_OP_INC 281
#define TK_OP_DEC 282
#define TK_PLUS 283
#define TK_MINUS 284
#define TK_MULT 285
#define TK_DIV 286
#define TK_LT_EQ 287
#define TK_GT_EQ 288
#define TK_LITTLE 289
#define TK_GREATER 290
#define TK_EQUAL 291
#define TK_NOT_EQUAL 292
#define TK_BOOL_AND 293
#define TK_BOOL_OR 294
#define TK_ERROR 295




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



