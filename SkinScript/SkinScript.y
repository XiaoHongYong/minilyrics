%{

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
%}

%token	TK_IF		TK_ELSE		TK_WHILE	TK_DO		TK_NEW		TK_DELETE
%token	TK_GLOBAL	TK_FUNCTION	TK_CLASS	TK_RETURN	TK_REFERENCE
%token	TK_ID
%token	TK_NUMBER	TK_C_STRING
%token	TK_L_PARENTHESIS		TK_R_PARENTHESIS		TK_COLON	TK_SEMICOLON 
%token	TK_L_BRACE	TK_R_BRACE	TK_COMMA	TK_DOT		TK_ASSIGN
%token	TK_OP_INC	TK_OP_DEC	TK_PLUS		TK_MINUS	TK_MULT		TK_DIV
%token	TK_LT_EQ	TK_GT_EQ	TK_LITTLE	TK_GREATER	TK_EQUAL	TK_NOT_EQUAL
%token	TK_BOOL_AND	TK_BOOL_OR
%token	TK_ERROR

%left	TK_BOOL_OR
%left	TK_BOOL_AND	
%left	TK_LT_EQ	TK_GT_EQ	TK_LITTLE	TK_GREATER	TK_EQUAL	TK_NOT_EQUAL
%left	TK_PLUS		TK_MINUS
%left	TK_MULT		TK_DIV
%left	TK_OP_INC	TK_OP_DEC

%%		/* Grammer for skin script */

program				:	decl_seq
						{	_syntaxTree = $$;	}
					;

decl_seq			:	decl_seq decl
						{
							$$ = AppendSiblingNode($1, $2);
						}
					|	decl
						{	$$ = $1;	}
					;

decl				:	decl_var	{	$$ = $1;	}
					|	decl_function	{	$$ = $1;	}
					|	decl_class	{	$$ = $1;	}
					;

decl_var			:	var_type var_list TK_SEMICOLON
						{
							$$ = newVarDeclNode($1->nLineNo, $1->szName, $2);
							delete $1;
						}
					;

var_list			:	TK_ID TK_COMMA var_list
						{
							$$ = InsertSiblingHeadNode($3, $1);
						}
					|	TK_ID TK_ASSIGN expression TK_COMMA var_list
						{
							SS_TREE_NODE	*newHead = $1;
							SetVarIDNodeAssignExp($1, $3);
							$$ = InsertSiblingHeadNode($5, newHead);
						}
					|	TK_ID { $$ = $1; }
					|	TK_ID TK_ASSIGN expression
						{
							$$ = $1;
							SetVarIDNodeAssignExp($1, $3);
						}
					;

decl_function		:	var_type TK_ID TK_L_PARENTHESIS fun_param_list TK_R_PARENTHESIS
						fun_body
						{
							$$ = newFuncDeclNode($2->nLineNo, $2->szName, $1, $4, $6);
							delete $2;
						}
					|	var_type TK_ID TK_L_PARENTHESIS TK_R_PARENTHESIS
						fun_body
						{
							$$ = newFuncDeclNode($2->nLineNo, $2->szName, $1, NULL, $5);
							delete $2;
						}
					;

var_type			:	TK_ID { $$ = $1; }
					;


fun_param_list		:	fun_param_list TK_COMMA fun_param
						{
							$$ = AppendSiblingNode($1, $3);
						}
					|	fun_param
						{
							$$ = $1;
						}
					;

fun_param			:	var_type TK_ID
						{
							$$ = newVarDeclNode($1->nLineNo, $1->szName, $2);
							delete $1;
						}
					|	TK_REFERENCE var_type TK_ID
						{
							$$ = newVarDeclRefNode($2->nLineNo, $2->szName, $3);
							delete $2;
						}
					;

fun_body			:	TK_L_BRACE stmt_list TK_R_BRACE
						{
							$$ = $2;
							if ($$ == NULL)
							{
								$$ = newStmtNode(SK_NULL, NULL, NULL);
								$$->nLineNo = g_srcLineNo;
							}
						}
					|	TK_L_BRACE TK_R_BRACE
						{
							$$ = newStmtNode(SK_NULL, NULL, NULL);
							$$->nLineNo = g_srcLineNo;
						}
					;

stmt_list			:	stmt_list stmt_block
						{
							$$ = AppendSiblingNode($1, $2);
						}
					|	stmt_block { $$ = $1; }
					;

stmt_block			:	TK_L_BRACE stmt_list TK_R_BRACE
						{ $$ = newStmtNode(SK_BLOCK, $2, NULL); }
					|	TK_L_BRACE TK_R_BRACE
						{ $$ = NULL; }
					|	stmt { $$ = $1; }
					|	decl_var { $$ = $1; /* empty */ }
					;


stmt				:	assign	{ $$ = $1; }
					|	if_stmt		{ $$ = $1; }
					|	while_stmt	{ $$ = $1; }
					|	do_while_stmt	{ $$ = $1; }
					|	TK_SEMICOLON	{ $$ = NULL; }
					|	expression TK_SEMICOLON	{ $$ = $1; }
					|	TK_RETURN expression TK_SEMICOLON { $$ = $1; $1->child[0] = $2; }
					|	TK_RETURN TK_SEMICOLON { $$ = $1; }
					|	TK_DELETE variable TK_SEMICOLON { $$ = newStmtNode(SK_DELETE, $2, NULL); }
					;

assign				:	variable TK_ASSIGN expression TK_SEMICOLON
						{
							$$ = newStmtNode(SK_ASSIGN, $1, $3);
						}
					;

if_stmt				:	TK_IF TK_L_PARENTHESIS expression TK_R_PARENTHESIS stmt_block TK_ELSE stmt_block
						{
							$$ = newStmtNode(SK_IF, $3, $5);
							$$->child[2] = $7;
						}
					|	TK_IF TK_L_PARENTHESIS expression TK_R_PARENTHESIS stmt_block
						{
							$$ = newStmtNode(SK_IF, $3, $5);
						}
					;

while_stmt			:	TK_WHILE TK_L_PARENTHESIS expression TK_R_PARENTHESIS stmt_block
						{
							$$ = newStmtNode(SK_WHILE, $3, $5);
						}
					;

do_while_stmt		:	TK_DO stmt_block TK_WHILE TK_L_PARENTHESIS expression TK_R_PARENTHESIS TK_SEMICOLON
						{
							$$ = newStmtNode(SK_DO_WHILE, $5, $2);
						}
					;

expression			:	TK_NUMBER
						{ $$ = IDNodeToExpNode(EK_CONST_INT, $1); }
					|	TK_C_STRING
						{ $$ = IDNodeToExpNode(EK_CONST_STRING, $1); }
					|	TK_NEW TK_ID
						{ $$ = IDNodeToExpNode(EK_NEW, $2); }
					|	variable { $$ = $1; }
					|	TK_L_PARENTHESIS expression TK_R_PARENTHESIS
						{ $$ = $2; }
					|	fun_call { $$ = $1; }
					|	expression TK_LT_EQ		 expression { $$ = newExpOpNode(EO_LT_EQ	, $1, $3); }
					|	expression TK_GT_EQ		 expression { $$ = newExpOpNode(EO_GT_EQ	, $1, $3); }
					|	expression TK_LITTLE	 expression { $$ = newExpOpNode(EO_LITTLE	, $1, $3); }
					|	expression TK_GREATER	 expression { $$ = newExpOpNode(EO_GREATER	, $1, $3); }
					|	expression TK_EQUAL		 expression { $$ = newExpOpNode(EO_EQUAL	, $1, $3); }
					|	expression TK_NOT_EQUAL  expression { $$ = newExpOpNode(EO_NOT_EQUAL, $1, $3); }
					|	variable TK_OP_INC				{ $$ = newExpOpNode(EO_OP_INC, $1, NULL); }
					|	variable TK_OP_DEC				{ $$ = newExpOpNode(EO_OP_DEC, $1, NULL); }
					|	expression TK_PLUS		 expression { $$ = newExpOpNode(EO_PLUS		, $1, $3); }
					|	expression TK_MINUS		 expression { $$ = newExpOpNode(EO_MINUS	, $1, $3); }
					|	expression TK_MULT		 expression { $$ = newExpOpNode(EO_MULT		, $1, $3); }
					|	expression TK_DIV		 expression { $$ = newExpOpNode(EO_DIV		, $1, $3); }
					|	expression TK_BOOL_AND	 expression { $$ = newExpOpNode(EO_BOOL_AND	, $1, $3); }
					|	expression TK_BOOL_OR	 expression { $$ = newExpOpNode(EO_BOOL_OR	, $1, $3); }
					;

fun_call			:	variable TK_L_PARENTHESIS fun_call_param_list TK_R_PARENTHESIS
						{
							$$ = newFuncCallNode($1, $3);
						}
					;

variable			:	TK_ID
						{
							$$ = IDNodeToExpNode(EK_ID, $1);
						}
					|	variable TK_DOT TK_ID
						{
							$$ = AppendChild0Node($1, IDNodeToExpNode(EK_ID, $3));
						}
					;

fun_call_param_list	:	fun_call_param_list TK_COMMA expression
						{
							$$ = AppendSiblingNode($1, $3);
						}
					|	expression
						{
							$$ = $1;
						}
					|	{  /* no param */ $$ = NULL; }
					;

decl_class			:	TK_CLASS TK_ID TK_L_BRACE class_member_decl_seq TK_R_BRACE TK_SEMICOLON
						{
							$$ = newClassDeclNode($2->nLineNo, $2->szName, NULL, $4);
							delete $2;
						}
					|	TK_CLASS TK_ID TK_COLON TK_ID TK_L_BRACE class_member_decl_seq TK_R_BRACE TK_SEMICOLON
						{
							$$ = newClassDeclNode($2->nLineNo, $2->szName, $4, $6);
							delete $2;
						}
					;

class_member_decl_seq	:	class_member_decl_seq class_member_decl
						{
							$$ = AppendSiblingNode($1, $2);
						}
					|	decl
						{	$$ = $1;	}
					;

class_member_decl		:	decl_var	{	$$ = $1;	}
					|	decl_function	{	$$ = $1;	}
					;

%%

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