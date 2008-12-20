%{
#define YYPARSER
/*
 | stmt_function | stmt_class
*/
%}

%token	TK_IF TK_ELSE TK_WHILE 
%token	TK_GLOBAL TK_CLASS TK_FUNCTION TK_CLASS
%token	TK_ID
%token	TK_INT TK_STRING TK_VOID
%token	TK_L_BRACKET TK_R_BRACKET TK_COMMA
%token	TK_ERROR

%%		/* Grammer for skin script */

program				:	stmt_seq
						{		}
					;

stmt_seq			:	stmt_seq | stmt
					;

stmt				:	stmt_global_viarible
					;

stmt_global_viarible:	TK_GLOBAL viar_type TK_ID TK_L_BRACKET fun_param_list TK_R_BRACKET
					|	TK_GLOBAL TK_VOID TK_ID TK_L_BRACKET fun_param_list TK_R_BRACKET
					;

fun_param_list		:	fun_param_list TK_COMMA
					|	viar_type TK_ID
					;

viar_type			:	TK_INT | TK_STRING


%%

int yyerror(char *message)
{
	printf("%s", message);
}

