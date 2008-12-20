// SkinScript.cpp: implementation of the CSkinScript class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include "stdafx.h"
#include "SkinScript.h"

/*

  

*/

int g_srcLineNo = 1; // source line number for listing

char *dupStr(const char *str)
{
	if (str == NULL)
		return NULL;

	char	*temp;
	int		nLenMax;
	nLenMax = strlen(str) + 1;
	temp = new char[nLenMax];

	strcpy(temp, str);

	return temp;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SS_TREE_NODE *newVarDeclNode(int nLineNo, const char *szVarTypeName, SS_TREE_NODE *varNameList)
{
	SS_TREE_NODE	*node = new SS_TREE_NODE;

	for (int i = 0; i < MAX_CHILDREN; i++)
		node->child[i] = NULL;
	node->child[0] = varNameList;

	node->nLineNo = nLineNo;
	node->sibling = NULL;
	node->szName = dupStr(szVarTypeName);
	node->nodeType = NT_VAR_DECL;

	// Set as id
	varNameList->kind.exp = EK_ID;

	return node;
}

SS_TREE_NODE *newVarDeclRefNode(int nLineNo, const char *szVarTypeName, SS_TREE_NODE *varNameList)
{
	SS_TREE_NODE	*node = new SS_TREE_NODE;

	for (int i = 0; i < MAX_CHILDREN; i++)
		node->child[i] = NULL;
	node->child[0] = varNameList;

	node->nLineNo = nLineNo;
	node->sibling = NULL;
	node->szName = dupStr(szVarTypeName);
	node->nodeType = NT_VAR_DECL;

	// Set as reference id
	varNameList->kind.exp = EK_ID_REF;

	return node;
}

SS_TREE_NODE *newVarIDNode(int nLineNo, const char * szVarName, SS_TREE_NODE *pAssignExp)
{
	SS_TREE_NODE	*node = new SS_TREE_NODE;

	node->child[0] = pAssignExp;

	node->nLineNo = nLineNo;
	node->szName = dupStr(szVarName);
	node->nodeType = NT_VAR_ID;

	return node;
}

void SetVarIDNodeAssignExp(SS_TREE_NODE *pIDTree, SS_TREE_NODE *pTreeExp)
{
	assert(pIDTree->nodeType == NT_VAR_ID);
	pIDTree->child[0] = pTreeExp;
}

SS_TREE_NODE *newFuncDeclNode(int nLineNo, const char *szFuncName, SS_TREE_NODE *funcRetType, SS_TREE_NODE *funcParam, SS_TREE_NODE *funcBody)
{
	SS_TREE_NODE	*node = new SS_TREE_NODE;

	node->child[0] = funcRetType;
	node->child[1] = funcParam;
	node->child[2] = funcBody;

	node->nLineNo = nLineNo;
	node->szName = dupStr(szFuncName);
	node->nodeType = NT_FUNC_IMP;

	return node;
}

SS_TREE_NODE *newClassDeclNode(int nLineNo, const char *szClassName, SS_TREE_NODE *parentClass, SS_TREE_NODE *classBody)
{
	SS_TREE_NODE	*node = new SS_TREE_NODE;

	node->child[0] = parentClass;
	node->child[1] = classBody;

	node->nLineNo = nLineNo;
	node->szName = dupStr(szClassName);
	node->nodeType = NT_CLASS_IMP;

	return node;
}

SS_TREE_NODE *newStmtNode(STMT_KIND stmtKind, SS_TREE_NODE *child0, SS_TREE_NODE *child1)
{
	SS_TREE_NODE	*node = new SS_TREE_NODE;

	if (child0)
		node->nLineNo = child0->nLineNo;
	else if (child1)
		node->nLineNo = child1->nLineNo;
	node->child[0] = child0;
	node->child[1] = child1;
	node->kind.stmt = stmtKind;
	node->szName = NULL;
	node->nodeType = NT_STMT;

	return node;
}

SS_TREE_NODE *IDNodeToExpNode(EXP_KIND expKind, SS_TREE_NODE *node)
{
	node->kind.exp = expKind;
	node->nodeType = NT_EXP;

	return node;
}

SS_TREE_NODE *newFuncCallNode(SS_TREE_NODE *funcName, SS_TREE_NODE *parameter)
{
	SS_TREE_NODE	*node = new SS_TREE_NODE;

	node->child[0] = funcName;
	node->child[1] = parameter;

	node->nLineNo = funcName->nLineNo;
	node->kind.exp = EK_FUNC_CALL;
	node->nodeType = NT_EXP;

	return node;
}

SS_TREE_NODE *newExpOpNode(EXP_OP op, SS_TREE_NODE *op1, SS_TREE_NODE *op2)
{
	SS_TREE_NODE	*node = new SS_TREE_NODE;

	node->child[0] = op1;
	node->child[1] = op2;

	if (op1)
		node->nLineNo = op1->nLineNo;
	else if (op2)
		node->nLineNo = op2->nLineNo;
	node->kind.exp = EK_OP;
	node->attr.op = op;
	node->nodeType = NT_EXP;

	return node;
}

SS_TREE_NODE *AppendChild0Node(SS_TREE_NODE *head, SS_TREE_NODE *childLast)
{
	SS_TREE_NODE *	t = head;
	if (t != NULL) {
		while (t->child[0] != NULL)
			t = t->child[0];
		t->child[0] = childLast;
		return head;
	}
	else
		return childLast;
}

SS_TREE_NODE *AppendSiblingNode(SS_TREE_NODE *head, SS_TREE_NODE *tail)
{
	SS_TREE_NODE *	t = head;
	if (t != NULL) {
		while (t->sibling != NULL)
			t = t->sibling;
		t->sibling = tail;
		return head;
	}
	else
		return tail;
}

SS_TREE_NODE *InsertSiblingHeadNode(SS_TREE_NODE *head, SS_TREE_NODE *newHead)
{
	if (newHead == NULL)
		return head;

	newHead->sibling = head;
	return newHead;
}

void PrintSapces(int nIndent)
{
	for (int i = 0; i < nIndent; i++)
		printf("  ");
}

const char *GetOpName(EXP_OP token)
{
	switch (token)
	{
	case EO_LT_EQ:		return "<=";
	case EO_GT_EQ:		return ">=";
	case EO_LITTLE:		return "<";
	case EO_GREATER:	return ">";
	case EO_EQUAL:		return "==";
	case EO_NOT_EQUAL:	return "!=";

	case EO_OP_INC:		return "++";
	case EO_OP_DEC:		return "--";
	case EO_PLUS:		return "+"	;
	case EO_MINUS:		return "-";
	case EO_MULT:		return "*";
	case EO_DIV:		return "/";
	case EO_BOOL_AND:	return "&&";
	case EO_BOOL_OR:	return "||";
	default:			return "Unknown";
	}
}

void PrintSyntaxTree(SS_TREE_NODE *t, int &nIndent)
{
	nIndent++;

	while (t != NULL)
	{
		printf("line: %2d  ", t->nLineNo);
		PrintSapces(nIndent);
		switch (t->nodeType)
		{
		case NT_VAR_DECL:
			printf("Variable decleration %s;\n", t->szName);
			break;
		case NT_VAR_ID:
			if (t->kind.exp == EK_ID_REF)
				printf("ref %s;\n", t->szName);
			else
				printf("%s;\n", t->szName);
			break;
		case NT_FUNC_IMP:
			{
				printf("Function %s()\n", t->szName);
/*				if (t->child[0])
				{
					PrintSapces(nIndent);
					printf("Param: \n");
					PrintSyntaxTree(t->child[0], nIndent);
					PrintSapces(nIndent);
					printf("End of Param\n");
				}
				ASSERT(t->child[1])
				if (t->child[1])
				{
					// function statment body
					PrintSapces(nIndent);
					printf("Function statment body: \n");
					PrintSyntaxTree(t->child[0], nIndent);
					PrintSapces(nIndent);
					printf("End of function statment body.\n");
				}*/
			}
			break;
		case NT_CLASS_IMP:
			printf("class %s\n", t->szName);
			break;
		case NT_STMT:
			{
				switch (t->kind.stmt)
				{
				case SK_NULL: printf("NULL statement;\n"); break;
				case SK_IF:
					printf("if\n");
// 					PrintSyntaxTree(t->child[0], nIndent);
// 					PrintSyntaxTree(t->child[1], nIndent);
// 					if (t->child[2])
// 						PrintSyntaxTree(t->child[2], nIndent);
					break;
				case SK_WHILE:
					printf("while\n");
// 					PrintSyntaxTree(t->child[0], nIndent);
// 					PrintSyntaxTree(t->child[1], nIndent);
					break;
				case SK_DO_WHILE:
					printf("do repeat\n");
// 					PrintSyntaxTree(t->child[0], nIndent);
// 					PrintSapces(nIndent);
// 					printf("repeat\n");
// 					PrintSyntaxTree(t->child[1], nIndent);
					break;
				case SK_ASSIGN:
					printf("%s = \n", t->szName);
// 					PrintSyntaxTree(t->child[0], nIndent);
					break;
				case SK_RETURN:
					printf("return : \n");
					break;
				case SK_BLOCK:
					printf("block : \n");
					break;
				case SK_DELETE:
					printf("delete: \n");
					break;
				default: printf("unknown statement kind;\n"); break;
				}
			}
			break;
		case NT_EXP:
			{
				switch (t->kind.exp)
				{
				case EK_OP:
					{
						printf("OP: %s\n", GetOpName(t->attr.op));
// 						if (t->child[0])
// 							PrintSyntaxTree(t->child[0], nIndent);
// 						if (t->child[1])
// 							PrintSyntaxTree(t->child[1], nIndent);
					}
					break;
				case EK_FUNC_CALL:
					printf("Function call: \n");
					break;
				case EK_CONST_INT:
					printf("Const int: %s\n", t->szName);
					break;
				case EK_CONST_STRING:
					printf("Const string: %s\n", t->szName);
					break;
				case EK_ID:
					printf("ID: %s\n", t->szName);
					break;
				case EK_ID_REF:
					printf("ref ID: %s\n", t->szName);
					break;
				case EK_NEW:
					printf("new : %s\n", t->szName);
					break;
				default:
					printf("Unkown expression: %s\n", t->szName);
					break;
				}
			}
			break;
		default:
			printf("Unknown Node type: %d\n", t->nodeType);
			break;
		}

		for (int i = 0; i < MAX_CHILDREN; i++)
			PrintSyntaxTree(t->child[i], nIndent);

		t = t->sibling;
	}

	nIndent--;
}

void deleteSSNode(SS_TREE_NODE *t)
{
	while (t)
	{
		for (int i = 0; i < MAX_CHILDREN; i++)
		{
			if (t->child[i])
				deleteSSNode(t->child[i]);
		}
		if (t->szName)
			delete[] (char*)t->szName;

		SS_TREE_NODE	*next = t->sibling;
		delete t;
		t = next;
	}
}

