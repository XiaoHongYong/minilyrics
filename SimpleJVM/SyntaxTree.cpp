#include "StdAfx.h"
#include "SyntaxTree.h"

#pragma warning (disable: 4996)

int			g_srcLineNo = 1; // source line number for listing
STreeNode	*_syntaxTree;

static char *dupStr(const char *str)
{
	if (str == NULL)
		return NULL;

	char	*temp;
	size_t		nLenMax;
	nLenMax = strlen(str) + 1;
	temp = new char[nLenMax];

	strcpy(temp, str);

	return temp;
}

STreeNode *AppendToSibling(STreeNode *pHead, STreeNode *pTail)
{
	STreeNode *p = pHead;
	while (p->pNext)
		p = p->pNext;
	p->pNext = pTail;

	return pHead;
}

void SJType::operator=(STreeNode *pNodeID)
{
	ASSERT(pNodeID->nodeType == STreeNode::T_STRING);
	nLineNo = pNodeID->nLineNo;
	szTypeName = dupStr(pNodeID->szString);
	typeId = 0;
}

STreeNode *newStnString(int nLineNo, LPCSTR szString)
{
	STreeNode	*pNode = new STreeNode(STreeNode::T_STRING, nLineNo);
	pNode->szString = dupStr(szString);
	return pNode;
}

STreeNode *newStnVarDeclare(STreeNode *pVarType, STreeNode *pVarNameList, bool bStatic)
{
	ASSERT(pVarType->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = new STreeNode(STreeNode::T_VAR_DECLAR, pVarType->nLineNo);
	StnVarDeclare &sVarDeclare = pNode->sVarDeclare;
	sVarDeclare.bStatic = bStatic;
	sVarDeclare.pVarList = pVarNameList;
	sVarDeclare.type = pVarType;

	delete pVarType;

	return pNode;
}

STreeNode *newStnVar(STreeNode *pVarIdString, STreeNode *pExp)
{
	ASSERT(pVarIdString->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = new STreeNode(STreeNode::T_VAR, pVarIdString->nLineNo);
	StnVar		&sVar = pNode->sVar;
	sVar.szVarName = dupStr(pVarIdString->szString);
	sVar.pExpAssign = pExp;
	sVar.typeId = 0;
	sVar.nAddress = -1;

	delete pVarIdString;

	return pNode;
}

STreeNode *newStnClassDeclare(STreeNode *pClassNameString, STreeNode *pParentClassString, STreeNode *pClassMember)
{
	ASSERT(pClassNameString->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = new STreeNode(STreeNode::T_CLASS, pClassNameString->nLineNo);
	StnClass	&sClass = pNode->sClass;
	sClass.szClassName = dupStr(pClassNameString->szString);
	sClass.pClassMember = pClassMember;
	if (pParentClassString)
	{
		ASSERT(pParentClassString->nodeType == STreeNode::T_STRING);
		sClass.typeParentClass = pParentClassString;
		delete pParentClassString;
	}
	else
		sClass.typeParentClass.szTypeName = NULL;
	sClass.typeIdClass = 0;

	delete pClassNameString;

	return pNode;
}

// For native function, body = NULL
STreeNode *newStnMethodDeclareNode(STreeNode *pMethodNameString, 
								   STreeNode *pRetTypeString, STreeNode *pParameterList, STreeNode *pBody,
								   bool bStatic)
{
	ASSERT(pMethodNameString->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = new STreeNode(STreeNode::T_MEMBER_METHOD, pMethodNameString->nLineNo);
	StnMemberMethod	&sMethod = pNode->sMemberMethod;
	sMethod.bStatic = bStatic;
	sMethod.pBody = pBody;
	sMethod.pParameterList = pParameterList;
	sMethod.szMethodName = dupStr(pMethodNameString->szString);
	sMethod.pMethod = NULL;

	if (pRetTypeString)
	{
		ASSERT(pRetTypeString->nodeType == STreeNode::T_STRING);
		sMethod.typeRet = pRetTypeString;
		delete pRetTypeString;
	}
	else
	{
		sMethod.typeRet.typeId = 0;
		sMethod.typeRet.szTypeName = NULL;
	}

	delete pMethodNameString;

	return pNode;
}

STreeNode *newStnAssign(STreeNode *pTargetVar, STreeNode *pExpression)
{
	STreeNode	*pNode = new STreeNode(STreeNode::T_ASSIGN, pTargetVar->nLineNo);
	StnAssign	&sAssign = pNode->sAssign;

	sAssign.pRightExp = pExpression;
	sAssign.pVar = pTargetVar;

	return pNode;
}

STreeNode *newStnIf(STreeNode *pCondition, STreeNode *pTrueBlock, STreeNode *pFalseBlock)
{
	STreeNode	*pNode = new STreeNode(STreeNode::T_IF, pCondition->nLineNo);
	StnIf		&sIf = pNode->sIf;

	sIf.pCondition = pCondition;
	sIf.pTrueNode = pTrueBlock;
	sIf.pFalseNode = pFalseBlock;

	return pNode;
}

STreeNode *newStnFor(STreeNode *pInitalization, STreeNode *pTermination, STreeNode *pIncrement, STreeNode *pActionBlock)
{
	STreeNode	*pNode = new STreeNode(STreeNode::T_FOR, pInitalization->nLineNo);
	StnFor		&sFor = pNode->sFor;

	sFor.pInitalization = pInitalization;
	sFor.pTermination = pTermination;
	sFor.pIncrement = pIncrement;
	sFor.pActionNode = pActionBlock;

	return pNode;
}

STreeNode *newStnWhile(STreeNode *pCondition, STreeNode *pActionBlock)
{
	STreeNode	*pNode = new STreeNode(STreeNode::T_WHILE, pCondition->nLineNo);
	StnWhile	&sWhile = pNode->sWhile;

	sWhile.pCondition = pCondition;
	sWhile.pActionNode = pActionBlock;

	return pNode;
}

STreeNode *newStnDoWhile(STreeNode *pCondition, STreeNode *pActionBlock)
{
	STreeNode	*pNode = new STreeNode(STreeNode::T_DO_WHILE, pCondition->nLineNo);
	StnWhile	&sWhile = pNode->sWhile;

	sWhile.pCondition = pCondition;
	sWhile.pActionNode = pActionBlock;

	return pNode;
}

STreeNode *newStnExpVar(int nLineNo, LPCSTR szVarName, STreeNode *pParentVar)
{
	STreeNode	*pNode = new STreeNode(STreeNode::T_EXPRESSION, nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = E_VAR;
	sExp.var.szVarName = dupStr(szVarName);
	sExp.var.pVarParent = pParentVar;
	sExp.var.nAddress = -1;
	sExp.var.bStatic = false;

	return pNode;
}

STreeNode *newStnExpVar(STreeNode *pVarNameString, STreeNode *pParentVar)
{
	ASSERT(pVarNameString->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = newStnExpVar(pVarNameString->nLineNo, pVarNameString->szString, pParentVar);

	delete pVarNameString;

	return pNode;
}

STreeNode *newStnExprConstInt(STreeNode *pString)
{
	ASSERT(pString->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = new STreeNode(STreeNode::T_EXPRESSION, pString->nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = E_CONST_INT;
	sExp.nConstInt = atoi(pString->szString);

	delete pString;

	return pNode;
}

STreeNode *newStnExprConstString(STreeNode *pString)
{
	ASSERT(pString->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = new STreeNode(STreeNode::T_EXPRESSION, pString->nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = E_CONST_STRING;
	sExp.szConstString = dupStr(pString->szString);

	delete pString;

	return pNode;
}

STreeNode *newStnExpNew(STreeNode *pString, STreeNode *pParameterList)
{
	ASSERT(pString->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = new STreeNode(STreeNode::T_EXPRESSION, pString->nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = E_NEW;
	sExp.newObj.szNewType = dupStr(pString->szString);
	sExp.newObj.pParameter = pParameterList;

	delete pString;

	return pNode;
}

STreeNode *newStnExpOp(ExpressionType expType, STreeNode *pExpressionLeft, STreeNode *pExpressionRight)
{
	STreeNode	*pNode = new STreeNode(STreeNode::T_EXPRESSION, pExpressionLeft->nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = expType;
	sExp.opParameter.pExpressionLeft = pExpressionLeft;
	sExp.opParameter.pExpressionRight = pExpressionRight;

	return pNode;
}

STreeNode *newStnExpFuncCall(int nLineNo, LPSTR szMethodName, STreeNode *pVarToCall, STreeNode *pParameterList)
{
	STreeNode	*pNode = new STreeNode(STreeNode::T_EXPRESSION, nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = E_METHOD_CALL;
	sExp.functionCall.szFuncName = dupStr(szMethodName);
	sExp.functionCall.pVarParent = pVarToCall;
	sExp.functionCall.pParameter = pParameterList;

	return pNode;
}

STreeNode *newStnReturn(int nLineNo, STreeNode *pExpression)
{
	STreeNode	*pNode = new STreeNode(STreeNode::T_RETURN, nLineNo);
	StnReturn	&sReturn = pNode->sReturn;

	sReturn.pExpression = pExpression;

	return pNode;
}

STreeNode *newStnBlock(int nLineNo, STreeNode *pChildList)
{
	STreeNode	*pNode = new STreeNode(STreeNode::T_BLOCK, nLineNo);
	StnBlock	&sBlock = pNode->sBlock;

	sBlock.pChildList = pChildList;

	return pNode;
}

void PrintSapces(int nIndent, int nLineNo)
{
	printf("line: %03d  ", nLineNo);
	for (int i = 0; i < nIndent; i++)
		printf("  ");
}

void PrintSyntaxTree(STreeNode *t, int &nIndent)
{
	while (t != NULL)
	{
		int		nIndentOld = nIndent;
		switch (t->nodeType)
		{
		case STreeNode::T_STRING:
			printf(" %s ", t->szString);
			break;
		case STreeNode::T_CLASS:
			nIndent++;
			PrintSapces(nIndent, t->nLineNo);
			if (t->sClass.typeParentClass.szTypeName)
				printf("class %s { \n", t->sClass.szClassName);
			else
				printf("class %s extends %s { \n", t->sClass.szClassName, t->sClass.typeParentClass.szTypeName);
			if (t->sClass.pClassMember)
				PrintSyntaxTree(t->sClass.pClassMember, nIndent);
			PrintSapces(nIndent, t->nLineNo);
			printf("}\n");
			break;
		case STreeNode::T_VAR_DECLAR:
			{
				PrintSapces(nIndent, t->nLineNo);
				printf("%s	", t->sVarDeclare.type.szTypeName);
				STreeNode	*pVar = t->sVarDeclare.pVarList;
				while (pVar) {
					printf("%s(%d), ", pVar->sVar.szVarName, pVar->sVar.nAddress);
					if (pVar->sVar.pExpAssign)
						PrintSyntaxTree(pVar->sVar.pExpAssign, nIndent);
					pVar = pVar->pNext;
				}
				printf(";\n");
			}
			break;
		case STreeNode::T_VAR:
			printf("%s(%d), ", t->sVar.szVarName, t->sVar.nAddress);
			if (t->sVar.pExpAssign)
			{
				printf(" = ");
				PrintSyntaxTree(t->sVar.pExpAssign, nIndent);
			}
			break;
		case STreeNode::T_MEMBER_METHOD:
			{
				nIndent++;
				PrintSapces(nIndent, t->nLineNo);
				if (t->sMemberMethod.bStatic) printf("static ");
				if (t->sMemberMethod.pBody == NULL) printf("native ");
				printf("%s %s(\n", t->sMemberMethod.typeRet.szTypeName, t->sMemberMethod.szMethodName);
				if (t->sMemberMethod.pParameterList)
					PrintSyntaxTree(t->sMemberMethod.pParameterList, nIndent);
				PrintSapces(nIndent, t->nLineNo);
				printf(") {\n");
				nIndent++;
				if (t->sMemberMethod.pBody)
					PrintSyntaxTree(t->sMemberMethod.pBody, nIndent);
				nIndent--;
				PrintSapces(nIndent, t->nLineNo);
				printf("}\n");
			}
			break;
		case STreeNode::T_ASSIGN:
			{
				PrintSapces(nIndent, t->nLineNo);
				if (t->sAssign.pVar)
					PrintSyntaxTree(t->sAssign.pVar, nIndent);
				printf(" = ");
				if (t->sAssign.pRightExp)
					PrintSyntaxTree(t->sAssign.pRightExp, nIndent);
				printf("\n");
			}
			break;
		case STreeNode::T_IF:
			{
				PrintSapces(nIndent, t->nLineNo);
				printf("if (");
				if (t->sIf.pCondition)
					PrintSyntaxTree(t->sIf.pCondition, nIndent);
				printf(") {\n");
				if (t->sIf.pTrueNode)
				{
					nIndent++;
					PrintSyntaxTree(t->sIf.pTrueNode, nIndent);
					nIndent--;
					PrintSapces(nIndent, t->nLineNo);
					printf("}\n");
				}
				if (t->sIf.pFalseNode)
				{
					nIndent++;
					PrintSapces(nIndent, t->nLineNo);
					printf("else {\n");
					PrintSyntaxTree(t->sIf.pFalseNode, nIndent);
					nIndent--;
					PrintSapces(nIndent, t->nLineNo);
					printf("}\n");
				}
			}
			break;
		case STreeNode::T_FOR:
			{
				PrintSapces(nIndent, t->nLineNo);
				printf("for (");
				if (t->sFor.pInitalization) PrintSyntaxTree(t->sFor.pInitalization, nIndent);
				printf(";");
				if (t->sFor.pTermination) PrintSyntaxTree(t->sFor.pTermination, nIndent);
				printf(";");
				if (t->sFor.pIncrement) PrintSyntaxTree(t->sFor.pIncrement, nIndent);
				printf(") {\n");
				if (t->sFor.pActionNode)
				{
					nIndent++;
					PrintSyntaxTree(t->sFor.pActionNode, nIndent);
					nIndent--;
				}
				PrintSapces(nIndent, t->nLineNo);
				printf("}\n");
			}
			break;
		case STreeNode::T_WHILE:
			{
				PrintSapces(nIndent, t->nLineNo);
				printf("while (");
				if (t->sWhile.pCondition)
					PrintSyntaxTree(t->sWhile.pCondition, nIndent);
				printf(") {\n");
				if (t->sWhile.pActionNode)
				{
					nIndent++;
					PrintSyntaxTree(t->sWhile.pActionNode, nIndent);
					nIndent--;
				}
				PrintSapces(nIndent, t->nLineNo);
				printf("}\n");
			}
			break;
		case STreeNode::T_DO_WHILE:
			{
				PrintSapces(nIndent, t->nLineNo);
				printf("do {");
				if (t->sWhile.pActionNode)
				{
					nIndent++;
					PrintSyntaxTree(t->sWhile.pActionNode, nIndent);
					nIndent--;
				}
				PrintSapces(nIndent, t->nLineNo);
				printf("}\n");
				PrintSapces(nIndent, t->nLineNo);
				printf("while (");
				if (t->sWhile.pCondition)
					PrintSyntaxTree(t->sWhile.pCondition, nIndent);
				printf(")\n");
			}
			break;
		case STreeNode::T_EXPRESSION:
			{
				if (t->sExpression.type == E_CONST_INT)
				{
					printf("%d", t->sExpression.nConstInt);
					return;
				}
				else if (t->sExpression.type == E_CONST_STRING)
				{
					printf("%s", t->sExpression.szConstString);
					return;
				}
				else if (t->sExpression.type == E_NEW)
				{
					printf("new %s\n", t->sExpression.newObj.szNewType);
					return;
				}
				else if (t->sExpression.type == E_VAR)
				{
					STreeNode	*pParent = t->sExpression.var.pVarParent;
					if (pParent)
					{
						ASSERT(pParent->sExpression.type == E_VAR);
						PrintSyntaxTree(pParent, nIndent);
						printf(".");
						pParent = pParent->sExpression.var.pVarParent;
					}
					printf("%s(%d%s)", t->sExpression.var.szVarName, t->sExpression.var.nAddress, t->sExpression.var.bStatic ? " Static" : "");
					return;
				}
				else if (t->sExpression.type == E_METHOD_CALL)
				{
					PrintSapces(nIndent, t->nLineNo);
					if (t->sExpression.functionCall.pVarParent)
					{
						PrintSyntaxTree(t->sExpression.functionCall.pVarParent, nIndent);
						printf(".");
					}
					printf("%s(", t->sExpression.functionCall.szFuncName);

					if (t->sExpression.functionCall.pParameter)
					{
						PrintSyntaxTree(t->sExpression.functionCall.pParameter, nIndent);
					}
					printf(");\n");
					return;
				}

				if (t->sExpression.opParameter.pExpressionLeft)
					PrintSyntaxTree(t->sExpression.opParameter.pExpressionLeft, nIndent);
				switch (t->sExpression.type) {
					case OP_LT_EQ:		printf("<=");	break;	// <=
					case OP_GT_EQ:		printf(">=");	break;	// >=
					case OP_LITTLE:		printf("<");	break;	// <
					case OP_GREATER:		printf(">");	break;	// >
					case OP_EQUAL:		printf("==");	break;	// ==
					case OP_NOT_EQUAL:	printf("!=");	break;	// !=
					case OP_OP_INC:		printf("++");	break;	// ++
					case OP_OP_DEC:		printf("--");	break;	// --
					case OP_PLUS:		printf("+");	break;	// +
					case OP_MINUS:		printf("-");	break;	// -
					case OP_MULT:		printf("*");	break;	// *
					case OP_DIV:			printf("/");	break;	// /
					case OP_BOOL_AND:	printf("&&");	break;	// &&
					case OP_BOOL_OR:		printf("||");	break;	// ||
				}
				if (t->sExpression.opParameter.pExpressionRight)
					PrintSyntaxTree(t->sExpression.opParameter.pExpressionRight, nIndent);
			}
			break;
		case STreeNode::T_RETURN:
			{
				PrintSapces(nIndent, t->nLineNo);
				printf("return ");
				if (t->sReturn.pExpression)
				{
					PrintSyntaxTree(t->sReturn.pExpression, nIndent);
				}
				printf("\n");
			}
			break;
		case STreeNode::T_BLOCK:
			{
				PrintSapces(nIndent, t->nLineNo);
				printf("{\n");
				if (t->sBlock.pChildList)
				{
					nIndent++;
					PrintSyntaxTree(t->sBlock.pChildList, nIndent);
					nIndent--;
				}
				PrintSapces(nIndent, t->nLineNo);
				printf("}\n");
			}
			break;
		default:
			printf("Unknown Node type: %d, at line: %d\n", t->nodeType, t->nLineNo);
			break;
		}

		t = t->pNext;
		nIndent = nIndentOld;
	}

}
