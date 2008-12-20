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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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




/* Copy the first part of user declarations.  */
#line 1 "SkinScript.y"


#include "stdafx.h"
#include <stdio.h>
#include "SkinScript.h"

// #define YYSTYPE		_SSYYSTYPE
#define YYSTYPE		SS_TREE_NODE*

SS_TREE_NODE *		_syntaxTree;

int yylex (void);
void yyerror (char const *);

/*void Outputx(const char *str)
{
	printf("%s\n", str);
}*/


/*
 | stmt_function | stmt_class
*/


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 219 of yacc.c.  */
#line 201 "SkinScript.tab.c"

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T) && (defined (__STDC__) || defined (__cplusplus))
# include <stddef.h> /* INFRINGES ON USER NAME SPACE */
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if defined (__STDC__) || defined (__cplusplus)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     define YYINCLUDED_STDLIB_H
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2005 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM ((YYSIZE_T) -1)
#  endif
#  ifdef __cplusplus
extern "C" {
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if (! defined (malloc) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if (! defined (free) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifdef __cplusplus
}
#  endif
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  11
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   392

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  41
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  25
/* YYNRULES -- Number of rules. */
#define YYNRULES  73
/* YYNRULES -- Number of states. */
#define YYNSTATES  146

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   295

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    12,    14,    16,    20,
      24,    30,    32,    36,    43,    49,    51,    55,    57,    60,
      64,    68,    71,    74,    76,    80,    83,    85,    87,    89,
      91,    93,    95,    97,   100,   104,   107,   111,   116,   124,
     130,   136,   144,   146,   148,   151,   153,   157,   159,   163,
     167,   171,   175,   179,   183,   186,   189,   193,   197,   201,
     205,   209,   213,   218,   220,   224,   228,   230,   231,   238,
     247,   250,   252,   254
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      42,     0,    -1,    43,    -1,    43,    44,    -1,    44,    -1,
      45,    -1,    47,    -1,    63,    -1,    48,    46,    20,    -1,
      14,    23,    46,    -1,    14,    25,    59,    23,    46,    -1,
      14,    -1,    14,    25,    59,    -1,    48,    14,    17,    49,
      18,    51,    -1,    48,    14,    17,    18,    51,    -1,    14,
      -1,    49,    23,    50,    -1,    50,    -1,    48,    14,    -1,
      13,    48,    14,    -1,    21,    52,    22,    -1,    21,    22,
      -1,    52,    53,    -1,    53,    -1,    21,    52,    22,    -1,
      21,    22,    -1,    54,    -1,    45,    -1,    55,    -1,    56,
      -1,    57,    -1,    58,    -1,    20,    -1,    59,    20,    -1,
      12,    59,    20,    -1,    12,    20,    -1,     8,    61,    20,
      -1,    61,    25,    59,    20,    -1,     3,    17,    59,    18,
      53,     4,    53,    -1,     3,    17,    59,    18,    53,    -1,
       5,    17,    59,    18,    53,    -1,     6,    53,     5,    17,
      59,    18,    20,    -1,    15,    -1,    16,    -1,     7,    14,
      -1,    61,    -1,    17,    59,    18,    -1,    60,    -1,    59,
      32,    59,    -1,    59,    33,    59,    -1,    59,    34,    59,
      -1,    59,    35,    59,    -1,    59,    36,    59,    -1,    59,
      37,    59,    -1,    61,    26,    -1,    61,    27,    -1,    59,
      28,    59,    -1,    59,    29,    59,    -1,    59,    30,    59,
      -1,    59,    31,    59,    -1,    59,    38,    59,    -1,    59,
      39,    59,    -1,    61,    17,    62,    18,    -1,    14,    -1,
      61,    24,    14,    -1,    62,    23,    59,    -1,    59,    -1,
      -1,    11,    14,    21,    64,    22,    20,    -1,    11,    14,
      19,    14,    21,    64,    22,    20,    -1,    64,    65,    -1,
      44,    -1,    45,    -1,    47,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,    46,    46,    50,    54,    58,    59,    60,    63,    70,
      74,    80,    81,    88,    94,   102,   106,   110,   116,   121,
     128,   137,   144,   148,   151,   153,   155,   156,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   171,   177,   182,
     188,   194,   200,   202,   204,   206,   207,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   226,   232,   236,   242,   246,   250,   253,   258,
     265,   269,   273,   274
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TK_IF", "TK_ELSE", "TK_WHILE", "TK_DO",
  "TK_NEW", "TK_DELETE", "TK_GLOBAL", "TK_FUNCTION", "TK_CLASS",
  "TK_RETURN", "TK_REFERENCE", "TK_ID", "TK_NUMBER", "TK_C_STRING",
  "TK_L_PARENTHESIS", "TK_R_PARENTHESIS", "TK_COLON", "TK_SEMICOLON",
  "TK_L_BRACE", "TK_R_BRACE", "TK_COMMA", "TK_DOT", "TK_ASSIGN",
  "TK_OP_INC", "TK_OP_DEC", "TK_PLUS", "TK_MINUS", "TK_MULT", "TK_DIV",
  "TK_LT_EQ", "TK_GT_EQ", "TK_LITTLE", "TK_GREATER", "TK_EQUAL",
  "TK_NOT_EQUAL", "TK_BOOL_AND", "TK_BOOL_OR", "TK_ERROR", "$accept",
  "program", "decl_seq", "decl", "decl_var", "var_list", "decl_function",
  "var_type", "fun_param_list", "fun_param", "fun_body", "stmt_list",
  "stmt_block", "stmt", "assign", "if_stmt", "while_stmt", "do_while_stmt",
  "expression", "fun_call", "variable", "fun_call_param_list",
  "decl_class", "class_member_decl_seq", "class_member_decl", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    41,    42,    43,    43,    44,    44,    44,    45,    46,
      46,    46,    46,    47,    47,    48,    49,    49,    50,    50,
      51,    51,    52,    52,    53,    53,    53,    53,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    55,    56,    56,
      57,    58,    59,    59,    59,    59,    59,    59,    59,    59,
      59,    59,    59,    59,    59,    59,    59,    59,    59,    59,
      59,    59,    60,    61,    61,    62,    62,    62,    63,    63,
      64,    64,    65,    65
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     2,     1,     1,     1,     1,     3,     3,
       5,     1,     3,     6,     5,     1,     3,     1,     2,     3,
       3,     2,     2,     1,     3,     2,     1,     1,     1,     1,
       1,     1,     1,     2,     3,     2,     3,     4,     7,     5,
       5,     7,     1,     1,     2,     1,     3,     1,     3,     3,
       3,     3,     3,     3,     2,     2,     3,     3,     3,     3,
       3,     3,     4,     1,     3,     3,     1,     0,     6,     8,
       2,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,    15,     0,     2,     4,     5,     6,     0,     7,
       0,     1,     3,    11,     0,     0,     0,     0,     0,     0,
       8,     0,    71,     0,     0,     0,     0,     0,    17,    11,
       9,     0,    63,    42,    43,     0,    12,    47,    45,     0,
       0,    72,    73,    70,     0,     0,    14,    18,     0,     0,
      44,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    67,     0,    54,    55,     0,
      68,    19,     0,     0,     0,     0,     0,    63,    32,     0,
      21,    27,     0,     0,    23,    26,    28,    29,    30,    31,
       0,    45,    13,    16,    46,    10,    56,    57,    58,    59,
      48,    49,    50,    51,    52,    53,    60,    61,    66,     0,
      64,     0,     0,     0,     0,     0,    35,     0,    25,     0,
      20,    22,    33,     0,    62,     0,    69,     0,     0,     0,
      36,    34,    24,     0,    65,     0,     0,     0,    37,    39,
      40,     0,     0,     0,    38,    41
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     3,     4,    22,    81,    14,     7,    82,    27,    28,
      46,    83,    84,    85,    86,    87,    88,    89,    90,    37,
      38,   109,     9,    23,    43
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -67
static const short int yypact[] =
{
      -7,   -11,   -67,    13,    -7,   -67,   -67,   -67,     6,   -67,
      43,   -67,   -67,    55,    51,    76,    -7,    45,    89,    68,
     -67,    56,   -67,     5,    91,    80,    93,   -17,   -67,    77,
     -67,   102,   -67,   -67,   -67,    68,   320,   -67,   -15,    -7,
      97,   -67,   -67,   -67,   119,   195,   -67,   -67,    80,    98,
     -67,    92,    89,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    68,    68,    68,   120,   -67,   -67,    46,
     -67,   -67,   118,   121,   325,   122,     8,   123,   -67,   215,
     -67,   -67,    89,   235,   -67,   -67,   -67,   -67,   -67,   -67,
     233,    71,   -67,   -67,   -67,   -67,    83,    83,   -67,   -67,
       1,     1,     1,     1,     1,     1,   355,   344,   332,    63,
     -67,   135,    68,    68,   134,    41,   -67,   253,   -67,   290,
     -67,   -67,   -67,    68,   -67,    68,   -67,   114,   136,   124,
     -67,   -67,   -67,   288,   332,   325,   325,    68,   -67,   152,
     -67,   158,   325,   137,   -67,   -67
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
     -67,   -67,   -67,    87,    10,     3,   -18,    50,   -67,   109,
     111,    61,   -66,   -67,   -67,   -67,   -67,   -67,   -19,   -67,
     -27,   -67,   -67,   138,   -67
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -16
static const short int yytable[] =
{
      36,    48,    65,    10,     1,    42,    49,     2,   114,    66,
       6,    67,    68,    11,     6,    31,    51,   121,    91,     2,
      13,    30,    32,    33,    34,    35,     6,    40,   116,    53,
      54,    55,    56,    41,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,    91,   115,     6,
       8,    42,    91,   121,     8,    95,    91,   117,    24,     2,
       2,   130,    15,    25,    16,    66,     8,    26,   111,   139,
     140,    20,    17,     8,    44,    31,   144,    39,    18,    41,
      19,   124,    32,    33,    34,    35,   125,     5,    65,     8,
      21,    12,    91,   127,   128,    66,   123,    67,    68,    26,
      18,    45,    19,    29,   133,     2,   134,    47,    91,    91,
      94,    24,     2,    55,    56,    91,    50,    70,   141,     8,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,   135,    71,   110,   112,    32,   -15,   113,   129,
     119,   137,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,   136,   126,   142,   145,    93,    92,
       0,     0,     0,     0,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,   143,    69,     0,     0,
       0,     0,     0,     0,     0,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    72,     0,
      73,    74,    31,    75,     0,     0,     0,    76,     0,    77,
      33,    34,    35,     0,     0,    78,    79,    80,    72,     0,
      73,    74,    31,    75,     0,     0,     0,    76,     0,    77,
      33,    34,    35,     0,     0,    78,    79,   118,    72,     0,
      73,    74,    31,    75,     0,     0,     0,    76,     0,    77,
      33,    34,    35,   122,     0,    78,    79,   120,     0,     0,
       0,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,   131,     0,     0,     0,     0,     0,     0,
       0,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    72,     0,    73,    74,    31,    75,     0,
       0,     0,    76,     0,    77,    33,    34,    35,   138,     0,
      78,    79,   132,     0,     0,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    72,     0,
      73,    74,    31,    75,     0,     0,     0,    76,     0,    77,
      33,    34,    35,    52,     0,    78,    79,     0,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62
};

static const short int yycheck[] =
{
      19,    18,    17,    14,    11,    23,    23,    14,    74,    24,
       0,    26,    27,     0,     4,     7,    35,    83,    45,    14,
      14,    18,    14,    15,    16,    17,    16,    22,    20,    28,
      29,    30,    31,    23,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    74,    75,    39,
       0,    69,    79,   119,     4,    52,    83,    76,    13,    14,
      14,    20,    19,    18,    21,    24,    16,    17,    22,   135,
     136,    20,    17,    23,    24,     7,   142,    21,    23,    69,
      25,    18,    14,    15,    16,    17,    23,     0,    17,    39,
      14,     4,   119,   112,   113,    24,    25,    26,    27,    49,
      23,    21,    25,    14,   123,    14,   125,    14,   135,   136,
      18,    13,    14,    30,    31,   142,    14,    20,   137,    69,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    18,    14,    14,    17,    14,    14,    17,     5,
      79,    17,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    18,    20,     4,    20,    49,    48,
      -1,    -1,    -1,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    18,    39,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,     3,    -1,
       5,     6,     7,     8,    -1,    -1,    -1,    12,    -1,    14,
      15,    16,    17,    -1,    -1,    20,    21,    22,     3,    -1,
       5,     6,     7,     8,    -1,    -1,    -1,    12,    -1,    14,
      15,    16,    17,    -1,    -1,    20,    21,    22,     3,    -1,
       5,     6,     7,     8,    -1,    -1,    -1,    12,    -1,    14,
      15,    16,    17,    20,    -1,    20,    21,    22,    -1,    -1,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    20,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,     3,    -1,     5,     6,     7,     8,    -1,
      -1,    -1,    12,    -1,    14,    15,    16,    17,    20,    -1,
      20,    21,    22,    -1,    -1,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,     3,    -1,
       5,     6,     7,     8,    -1,    -1,    -1,    12,    -1,    14,
      15,    16,    17,    23,    -1,    20,    21,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    11,    14,    42,    43,    44,    45,    47,    48,    63,
      14,     0,    44,    14,    46,    19,    21,    17,    23,    25,
      20,    14,    44,    64,    13,    18,    48,    49,    50,    14,
      46,     7,    14,    15,    16,    17,    59,    60,    61,    21,
      22,    45,    47,    65,    48,    21,    51,    14,    18,    23,
      14,    59,    23,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    17,    24,    26,    27,    64,
      20,    14,     3,     5,     6,     8,    12,    14,    20,    21,
      22,    45,    48,    52,    53,    54,    55,    56,    57,    58,
      59,    61,    51,    50,    18,    46,    59,    59,    59,    59,
      59,    59,    59,    59,    59,    59,    59,    59,    59,    62,
      14,    22,    17,    17,    53,    61,    20,    59,    22,    52,
      22,    53,    20,    25,    18,    23,    20,    59,    59,     5,
      20,    20,    22,    59,    59,    18,    18,    17,    20,    53,
      53,    59,     4,    18,    53,    20
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (0)
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
              (Loc).first_line, (Loc).first_column,	\
              (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr,					\
                  Type, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname[yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      size_t yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

#endif /* YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);


# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()
    ;
#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 47 "SkinScript.y"
    {	_syntaxTree = (yyval);	;}
    break;

  case 3:
#line 51 "SkinScript.y"
    {
							(yyval) = AppendSiblingNode((yyvsp[-1]), (yyvsp[0]));
						;}
    break;

  case 4:
#line 55 "SkinScript.y"
    {	(yyval) = (yyvsp[0]);	;}
    break;

  case 5:
#line 58 "SkinScript.y"
    {	(yyval) = (yyvsp[0]);	;}
    break;

  case 6:
#line 59 "SkinScript.y"
    {	(yyval) = (yyvsp[0]);	;}
    break;

  case 7:
#line 60 "SkinScript.y"
    {	(yyval) = (yyvsp[0]);	;}
    break;

  case 8:
#line 64 "SkinScript.y"
    {
							(yyval) = newVarDeclNode((yyvsp[-2])->nLineNo, (yyvsp[-2])->szName, (yyvsp[-1]));
							delete (yyvsp[-2]);
						;}
    break;

  case 9:
#line 71 "SkinScript.y"
    {
							(yyval) = InsertSiblingHeadNode((yyvsp[0]), (yyvsp[-2]));
						;}
    break;

  case 10:
#line 75 "SkinScript.y"
    {
							SS_TREE_NODE	*newHead = (yyvsp[-4]);
							SetVarIDNodeAssignExp((yyvsp[-4]), (yyvsp[-2]));
							(yyval) = InsertSiblingHeadNode((yyvsp[0]), newHead);
						;}
    break;

  case 11:
#line 80 "SkinScript.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 12:
#line 82 "SkinScript.y"
    {
							(yyval) = (yyvsp[-2]);
							SetVarIDNodeAssignExp((yyvsp[-2]), (yyvsp[0]));
						;}
    break;

  case 13:
#line 90 "SkinScript.y"
    {
							(yyval) = newFuncDeclNode((yyvsp[-4])->nLineNo, (yyvsp[-4])->szName, (yyvsp[-5]), (yyvsp[-2]), (yyvsp[0]));
							delete (yyvsp[-4]);
						;}
    break;

  case 14:
#line 96 "SkinScript.y"
    {
							(yyval) = newFuncDeclNode((yyvsp[-3])->nLineNo, (yyvsp[-3])->szName, (yyvsp[-4]), NULL, (yyvsp[0]));
							delete (yyvsp[-3]);
						;}
    break;

  case 15:
#line 102 "SkinScript.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 16:
#line 107 "SkinScript.y"
    {
							(yyval) = AppendSiblingNode((yyvsp[-2]), (yyvsp[0]));
						;}
    break;

  case 17:
#line 111 "SkinScript.y"
    {
							(yyval) = (yyvsp[0]);
						;}
    break;

  case 18:
#line 117 "SkinScript.y"
    {
							(yyval) = newVarDeclNode((yyvsp[-1])->nLineNo, (yyvsp[-1])->szName, (yyvsp[0]));
							delete (yyvsp[-1]);
						;}
    break;

  case 19:
#line 122 "SkinScript.y"
    {
							(yyval) = newVarDeclRefNode((yyvsp[-1])->nLineNo, (yyvsp[-1])->szName, (yyvsp[0]));
							delete (yyvsp[-1]);
						;}
    break;

  case 20:
#line 129 "SkinScript.y"
    {
							(yyval) = (yyvsp[-1]);
							if ((yyval) == NULL)
							{
								(yyval) = newStmtNode(SK_NULL, NULL, NULL);
								(yyval)->nLineNo = g_srcLineNo;
							}
						;}
    break;

  case 21:
#line 138 "SkinScript.y"
    {
							(yyval) = newStmtNode(SK_NULL, NULL, NULL);
							(yyval)->nLineNo = g_srcLineNo;
						;}
    break;

  case 22:
#line 145 "SkinScript.y"
    {
							(yyval) = AppendSiblingNode((yyvsp[-1]), (yyvsp[0]));
						;}
    break;

  case 23:
#line 148 "SkinScript.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 24:
#line 152 "SkinScript.y"
    { (yyval) = newStmtNode(SK_BLOCK, (yyvsp[-1]), NULL); ;}
    break;

  case 25:
#line 154 "SkinScript.y"
    { (yyval) = NULL; ;}
    break;

  case 26:
#line 155 "SkinScript.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 27:
#line 156 "SkinScript.y"
    { (yyval) = (yyvsp[0]); /* empty */ ;}
    break;

  case 28:
#line 160 "SkinScript.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 29:
#line 161 "SkinScript.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 30:
#line 162 "SkinScript.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 31:
#line 163 "SkinScript.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 32:
#line 164 "SkinScript.y"
    { (yyval) = NULL; ;}
    break;

  case 33:
#line 165 "SkinScript.y"
    { (yyval) = (yyvsp[-1]); ;}
    break;

  case 34:
#line 166 "SkinScript.y"
    { (yyval) = (yyvsp[-2]); (yyvsp[-2])->child[0] = (yyvsp[-1]); ;}
    break;

  case 35:
#line 167 "SkinScript.y"
    { (yyval) = (yyvsp[-1]); ;}
    break;

  case 36:
#line 168 "SkinScript.y"
    { (yyval) = newStmtNode(SK_DELETE, (yyvsp[-1]), NULL); ;}
    break;

  case 37:
#line 172 "SkinScript.y"
    {
							(yyval) = newStmtNode(SK_ASSIGN, (yyvsp[-3]), (yyvsp[-1]));
						;}
    break;

  case 38:
#line 178 "SkinScript.y"
    {
							(yyval) = newStmtNode(SK_IF, (yyvsp[-4]), (yyvsp[-2]));
							(yyval)->child[2] = (yyvsp[0]);
						;}
    break;

  case 39:
#line 183 "SkinScript.y"
    {
							(yyval) = newStmtNode(SK_IF, (yyvsp[-2]), (yyvsp[0]));
						;}
    break;

  case 40:
#line 189 "SkinScript.y"
    {
							(yyval) = newStmtNode(SK_WHILE, (yyvsp[-2]), (yyvsp[0]));
						;}
    break;

  case 41:
#line 195 "SkinScript.y"
    {
							(yyval) = newStmtNode(SK_DO_WHILE, (yyvsp[-2]), (yyvsp[-5]));
						;}
    break;

  case 42:
#line 201 "SkinScript.y"
    { (yyval) = IDNodeToExpNode(EK_CONST_INT, (yyvsp[0])); ;}
    break;

  case 43:
#line 203 "SkinScript.y"
    { (yyval) = IDNodeToExpNode(EK_CONST_STRING, (yyvsp[0])); ;}
    break;

  case 44:
#line 205 "SkinScript.y"
    { (yyval) = IDNodeToExpNode(EK_NEW, (yyvsp[0])); ;}
    break;

  case 45:
#line 206 "SkinScript.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 46:
#line 208 "SkinScript.y"
    { (yyval) = (yyvsp[-1]); ;}
    break;

  case 47:
#line 209 "SkinScript.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 48:
#line 210 "SkinScript.y"
    { (yyval) = newExpOpNode(EO_LT_EQ	, (yyvsp[-2]), (yyvsp[0])); ;}
    break;

  case 49:
#line 211 "SkinScript.y"
    { (yyval) = newExpOpNode(EO_GT_EQ	, (yyvsp[-2]), (yyvsp[0])); ;}
    break;

  case 50:
#line 212 "SkinScript.y"
    { (yyval) = newExpOpNode(EO_LITTLE	, (yyvsp[-2]), (yyvsp[0])); ;}
    break;

  case 51:
#line 213 "SkinScript.y"
    { (yyval) = newExpOpNode(EO_GREATER	, (yyvsp[-2]), (yyvsp[0])); ;}
    break;

  case 52:
#line 214 "SkinScript.y"
    { (yyval) = newExpOpNode(EO_EQUAL	, (yyvsp[-2]), (yyvsp[0])); ;}
    break;

  case 53:
#line 215 "SkinScript.y"
    { (yyval) = newExpOpNode(EO_NOT_EQUAL, (yyvsp[-2]), (yyvsp[0])); ;}
    break;

  case 54:
#line 216 "SkinScript.y"
    { (yyval) = newExpOpNode(EO_OP_INC, (yyvsp[-1]), NULL); ;}
    break;

  case 55:
#line 217 "SkinScript.y"
    { (yyval) = newExpOpNode(EO_OP_DEC, (yyvsp[-1]), NULL); ;}
    break;

  case 56:
#line 218 "SkinScript.y"
    { (yyval) = newExpOpNode(EO_PLUS		, (yyvsp[-2]), (yyvsp[0])); ;}
    break;

  case 57:
#line 219 "SkinScript.y"
    { (yyval) = newExpOpNode(EO_MINUS	, (yyvsp[-2]), (yyvsp[0])); ;}
    break;

  case 58:
#line 220 "SkinScript.y"
    { (yyval) = newExpOpNode(EO_MULT		, (yyvsp[-2]), (yyvsp[0])); ;}
    break;

  case 59:
#line 221 "SkinScript.y"
    { (yyval) = newExpOpNode(EO_DIV		, (yyvsp[-2]), (yyvsp[0])); ;}
    break;

  case 60:
#line 222 "SkinScript.y"
    { (yyval) = newExpOpNode(EO_BOOL_AND	, (yyvsp[-2]), (yyvsp[0])); ;}
    break;

  case 61:
#line 223 "SkinScript.y"
    { (yyval) = newExpOpNode(EO_BOOL_OR	, (yyvsp[-2]), (yyvsp[0])); ;}
    break;

  case 62:
#line 227 "SkinScript.y"
    {
							(yyval) = newFuncCallNode((yyvsp[-3]), (yyvsp[-1]));
						;}
    break;

  case 63:
#line 233 "SkinScript.y"
    {
							(yyval) = IDNodeToExpNode(EK_ID, (yyvsp[0]));
						;}
    break;

  case 64:
#line 237 "SkinScript.y"
    {
							(yyval) = AppendChild0Node((yyvsp[-2]), IDNodeToExpNode(EK_ID, (yyvsp[0])));
						;}
    break;

  case 65:
#line 243 "SkinScript.y"
    {
							(yyval) = AppendSiblingNode((yyvsp[-2]), (yyvsp[0]));
						;}
    break;

  case 66:
#line 247 "SkinScript.y"
    {
							(yyval) = (yyvsp[0]);
						;}
    break;

  case 67:
#line 250 "SkinScript.y"
    {  /* no param */ (yyval) = NULL; ;}
    break;

  case 68:
#line 254 "SkinScript.y"
    {
							(yyval) = newClassDeclNode((yyvsp[-4])->nLineNo, (yyvsp[-4])->szName, NULL, (yyvsp[-2]));
							delete (yyvsp[-4]);
						;}
    break;

  case 69:
#line 259 "SkinScript.y"
    {
							(yyval) = newClassDeclNode((yyvsp[-6])->nLineNo, (yyvsp[-6])->szName, (yyvsp[-4]), (yyvsp[-2]));
							delete (yyvsp[-6]);
						;}
    break;

  case 70:
#line 266 "SkinScript.y"
    {
							(yyval) = AppendSiblingNode((yyvsp[-1]), (yyvsp[0]));
						;}
    break;

  case 71:
#line 270 "SkinScript.y"
    {	(yyval) = (yyvsp[0]);	;}
    break;

  case 72:
#line 273 "SkinScript.y"
    {	(yyval) = (yyvsp[0]);	;}
    break;

  case 73:
#line 274 "SkinScript.y"
    {	(yyval) = (yyvsp[0]);	;}
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 1801 "SkinScript.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  int yytype = YYTRANSLATE (yychar);
	  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
	  YYSIZE_T yysize = yysize0;
	  YYSIZE_T yysize1;
	  int yysize_overflow = 0;
	  char *yymsg = 0;
#	  define YYERROR_VERBOSE_ARGS_MAXIMUM 5
	  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
	  int yyx;

#if 0
	  /* This is so xgettext sees the translatable formats that are
	     constructed on the fly.  */
	  YY_("syntax error, unexpected %s");
	  YY_("syntax error, unexpected %s, expecting %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
#endif
	  char *yyfmt;
	  char const *yyf;
	  static char const yyunexpected[] = "syntax error, unexpected %s";
	  static char const yyexpecting[] = ", expecting %s";
	  static char const yyor[] = " or %s";
	  char yyformat[sizeof yyunexpected
			+ sizeof yyexpecting - 1
			+ ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
			   * (sizeof yyor - 1))];
	  char const *yyprefix = yyexpecting;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 1;

	  yyarg[0] = yytname[yytype];
	  yyfmt = yystpcpy (yyformat, yyunexpected);

	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
		  {
		    yycount = 1;
		    yysize = yysize0;
		    yyformat[sizeof yyunexpected - 1] = '\0';
		    break;
		  }
		yyarg[yycount++] = yytname[yyx];
		yysize1 = yysize + yytnamerr (0, yytname[yyx]);
		yysize_overflow |= yysize1 < yysize;
		yysize = yysize1;
		yyfmt = yystpcpy (yyfmt, yyprefix);
		yyprefix = yyor;
	      }

	  yyf = YY_(yyformat);
	  yysize1 = yysize + yystrlen (yyf);
	  yysize_overflow |= yysize1 < yysize;
	  yysize = yysize1;

	  if (!yysize_overflow && yysize <= YYSTACK_ALLOC_MAXIMUM)
	    yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg)
	    {
	      /* Avoid sprintf, as that infringes on the user's name space.
		 Don't have undefined behavior even if the translation
		 produced a string with the wrong number of "%s"s.  */
	      char *yyp = yymsg;
	      int yyi = 0;
	      while ((*yyp = *yyf))
		{
		  if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		    {
		      yyp += yytnamerr (yyp, yyarg[yyi++]);
		      yyf += 2;
		    }
		  else
		    {
		      yyp++;
		      yyf++;
		    }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    {
	      yyerror (YY_("syntax error"));
	      goto yyexhaustedlab;
	    }
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (YY_("syntax error"));
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (0)
     goto yyerrorlab;

yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token. */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK;
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 277 "SkinScript.y"


void yyerror (char const * s)
{
    printf("%s on line: %d\n", s, g_srcLineNo);
}

/*
void main(int  argc, char **argv )
{
    ++argv, --argc;
    if ( argc > 0 )
            yyin = fopen( argv[0], "r" );
    else
            yyin = stdin;

    yyparse();

}
*/
