%{

#include <stdio.h>
#include "SkinScript.h"

// #define YYSTYPE		SS_TREE_NODE *

SS_TREE_NODE *		_syntaxTree;

int yylex (void);
void yyerror (char const *);

void Outputx(const char *str)
{
	printf("%s\n", str);
}


/*
 | stmt_function | stmt_class
*/
%}

%union
{
	SS_TREE_NODE		*node;
	VAR_KIND			varKind;
	EXP_OP				expOp;
	const char			*szName;
};

%token	TK_IF		TK_ELSE		TK_WHILE	TK_DO
%token	TK_GLOBAL	TK_FUNCTION	TK_CLASS
%token	TK_INT		TK_STRING	TK_VOID
%token <szName>		TK_ID		TK_STRING
%token <szName>		TK_NUMBER	TK_C_STRING
%token	TK_L_PARENTHESIS		TK_R_PARENTHESIS		TK_COLON	TK_SEMICOLON 
%token	TK_L_BRACE	TK_R_BRACE	TK_COMMA	TK_DOT		TK_ASSIGN
%token	<expOp>		TK_OP_INC	TK_OP_DEC	TK_PLUS		TK_MINUS	TK_MULT		TK_DIV
%token	<expOp>		TK_LT_EQ	TK_GT_EQ	TK_LITTLE	TK_GREATER	TK_EQUAL	TK_NOT_EQUAL
%token	TK_ERROR

%left	TK_LT_EQ	TK_GT_EQ	TK_LITTLE	TK_GREATER	TK_EQUAL	TK_NOT_EQUAL
%left	TK_PLUS		TK_MINUS
%left	TK_MULT		TK_DIV
%left	TK_OP_INC	TK_OP_DEC

%type <node>	program	decl_seq decl decl_var var_list decl_function fun_param_list
%type <node>	expression fun_param fun_body stmt_list stmt assign if_stmt 
%type <node>	while_stmt do_while_stmt stmt_block fun_call fun_call_param_list

%type <varKind>	var_type fun_return_type

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
					;

decl_var			:	var_type var_list TK_SEMICOLON
						{
							Outputx("decl_var");
							$$ = newVarDeclNode($1, $2);
						}
					;

var_list			:	TK_ID TK_COMMA var_list
						{
							$$ = InsertSiblingHeadNode($3, newVarNameNode($1));
						}
					|	TK_ID TK_ASSIGN expression TK_COMMA var_list
						{
							SS_TREE_NODE	*newHead = newVarNameNode($1);
							$$ = InsertSiblingHeadNode($5, newHead);
							newHead->child[0] = $3;
						}
					|	TK_ID { $$ = newVarNameNode($1); }
					|	TK_ID TK_ASSIGN expression
						{
							$$ = newVarNameNode($1);
							$$->child[0] = $3;
						}
					;

decl_function		:	TK_FUNCTION fun_return_type TK_ID TK_L_PARENTHESIS fun_param_list TK_R_PARENTHESIS
						fun_body
						{
							Outputx("decl_function with param");
							$$ = newFuncDeclNode($2, $3);
							$$->child[0] = $5;
							$$->child[1] = $7;
						}
					|	TK_FUNCTION fun_return_type TK_ID TK_L_PARENTHESIS TK_R_PARENTHESIS
						fun_body
						{
							Outputx("decl_function_without_param");
							$$ = newFuncDeclNode($2, $3);
							$$->child[1] = $6;
						}
					;

var_type			:	TK_INT	{ $$ = VK_INT; }
					|	TK_STRING	{ $$ = VK_STRING; }
					;

fun_return_type		:	var_type
					|	TK_VOID	{ $$ = VK_VOID; }
					;


fun_param_list		:	fun_param_list TK_COMMA fun_param
						{
							Outputx("fun_param_list , ");
							$$ = AppendSiblingNode($1, $3);
						}
					|	fun_param
						{
							Outputx("fun_param_list");
							$$ = $1;
						}
					;

fun_param			:	var_type TK_ID
						{ $$ = newVarDeclNode($1, newVarNameNode($2)); }
					;

fun_body			:	TK_L_BRACE stmt_list TK_R_BRACE
						{
							Outputx("fun_body");
							$$ = $2;
							if ($$ == NULL)
								$$ = newStmtNode(SK_NULL, NULL, NULL);
						}
					;

stmt_list			:	stmt_list decl_var
						{
							Outputx("stmt_list decl_var");
							$$ = AppendSiblingNode($1, $2);
						}
					|	stmt_list stmt_block
						{
							Outputx("stmt_list decl_var");
							$$ = AppendSiblingNode($1, $2);
						}
					|	stmt_block { $$ = $1; }
					|	decl_var { $$ = $1; /* empty */ }
					;

stmt_block			:	TK_L_BRACE stmt_list TK_R_BRACE
						{ $$ = $2; }
					|	TK_L_BRACE TK_R_BRACE
						{ $$ = NULL; }
					|	stmt { $$ = $1; }


stmt				:	assign	{ $$ = $1; }
					|	if_stmt		{ $$ = $1; }
					|	while_stmt	{ $$ = $1; }
					|	do_while_stmt	{ $$ = $1; }
					|	fun_call	{ $$ = $1; }
					|	TK_SEMICOLON	{ $$ = NULL; }
					|	expression TK_SEMICOLON	{ $$ = $1; }
					;

assign				:	TK_ID TK_ASSIGN expression TK_SEMICOLON
						{
							// SS_TREE_NODE	*idNode = newExpNode(EK_ID, $1);
							$$ = newStmtNode(SK_ASSIGN, $3, NULL);
							$$->attr.szName = dupStr($1);
						}
					;

if_stmt				:	TK_IF TK_L_PARENTHESIS expression TK_R_PARENTHESIS stmt_block TK_ELSE stmt_block
						{
							Outputx("if_stmt");
							$$ = newStmtNode(SK_IF, $3, $5);
							$$->child[2] = $7;
						}
					|	TK_IF TK_L_PARENTHESIS expression TK_R_PARENTHESIS stmt_block
						{
							Outputx("if_stmt");
							$$ = newStmtNode(SK_IF, $3, $5);
						}
					;

while_stmt			:	TK_WHILE TK_L_PARENTHESIS expression TK_R_PARENTHESIS stmt_block
						{
							Outputx("while_stmt");
							$$ = newStmtNode(SK_WHILE, $3, $5);
						}
					;

do_while_stmt		:	TK_DO stmt_block TK_WHILE TK_L_PARENTHESIS expression TK_R_PARENTHESIS TK_SEMICOLON
						{
							Outputx("while_stmt");
							$$ = newStmtNode(SK_WHILE, $2, $5);
						}
					;

expression			:	TK_ID
						{ $$ = newExpNode(EK_ID, $1); }
					|	TK_NUMBER
						{ $$ = newExpNode(EK_CONST_INT, $1); }
					|	TK_C_STRING
						{ $$ = newExpNode(EK_CONST_STRING, $1); }
					|	TK_L_PARENTHESIS expression TK_R_PARENTHESIS
						{ $$ = $2; }
					|	expression TK_LT_EQ		 expression { $$ = newExpOpNode(EO_LT_EQ	, $1, $3); }
					|	expression TK_GT_EQ		 expression { $$ = newExpOpNode(EO_GT_EQ	, $1, $3); }
					|	expression TK_LITTLE	 expression { $$ = newExpOpNode(EO_LITTLE	, $1, $3); }
					|	expression TK_GREATER	 expression { $$ = newExpOpNode(EO_GREATER	, $1, $3); }
					|	expression TK_EQUAL		 expression { $$ = newExpOpNode(EO_EQUAL	, $1, $3); }
					|	expression TK_NOT_EQUAL  expression { $$ = newExpOpNode(EO_NOT_EQUAL, $1, $3); }
					|	expression TK_OP_INC	 expression { $$ = newExpOpNode(EO_OP_INC	, $1, $3); }
					|	expression TK_OP_DEC	 expression { $$ = newExpOpNode(EO_OP_DEC	, $1, $3); }
					|	expression TK_PLUS		 expression { $$ = newExpOpNode(EO_PLUS		, $1, $3); }
					|	expression TK_MINUS		 expression { $$ = newExpOpNode(EO_MINUS	, $1, $3); }
					|	expression TK_MULT		 expression { $$ = newExpOpNode(EO_MULT		, $1, $3); }
					|	expression TK_DIV		 expression { $$ = newExpOpNode(EO_DIV		, $1, $3); }
					;

fun_call			:	TK_ID TK_L_PARENTHESIS fun_call_param_list TK_R_PARENTHESIS TK_SEMICOLON
						{
							Outputx("fun_call");
							$$ = newFuncCallNode($1);
							$$->child[0] = $3;
						}
					;

fun_call_param_list	:	fun_call_param_list TK_COMMA expression
						{
							Outputx("fun_call_param_list");
							$$ = AppendSiblingNode($1, $3);
						}
					|	expression
						{
							Outputx("fun_call_param_list");
							$$ = $1;
						}
					|	{  /* no param */ $$ = NULL; }
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