#include "StdAfx.h"
#include "BatchMem.h"
#include "SyntaxTree.h"
#include "SJVM.h"

#pragma warning (disable: 4996)

int			g_srcLineNo = 1; // source line number for listing
STreeNode	*_syntaxTree;
CBatchMem	g_memSyntax;

//
// Using CBatchMem to manage small piece of memory, and free them togother.
//

static char *dupStr(const char *str)
{
	if (str == NULL)
		return NULL;

	return g_memSyntax.DupStr(str);
}

static char *dupStnStr(const char *str)
{
	return (char *)str;
}

STreeNode *NewSTreeNode(STreeNode::Type _t, int _nLineNo)
{
	STreeNode	*p = (STreeNode *)g_memSyntax.Alloc(sizeof(STreeNode));
	p->nodeType = _t;
	p->nLineNo = _nLineNo;
	p->pNext = NULL;
	return p;
}

STreeNode *AppendToSibling(PSTreeNode &pHead, STreeNode *pTail)
{
	if (!pHead)
	{
		pHead = pTail;
		return pHead;
	}

	STreeNode *p = pHead;
	while (p->pNext)
		p = p->pNext;
	p->pNext = pTail;

	return pHead;
}

void InsertToSibling(PSTreeNode &pHead, STreeNode *pToInsert)
{
	ASSERT(pHead && pToInsert);
	ASSERT(pToInsert->pNext == NULL);
	pToInsert->pNext = pHead->pNext;
	pHead->pNext = pToInsert;
}

void SJType::operator=(STreeNode *pNodeID)
{
	ASSERT(pNodeID->nodeType == STreeNode::T_STRING);
	nLineNo = pNodeID->nLineNo;
	szTypeName = dupStnStr(pNodeID->szString);
	typeId = 0;
}

STreeNode *newStnString(int nLineNo, LPCSTR szString)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_STRING, nLineNo);
	pNode->szString = dupStr(szString);
	return pNode;
}

STreeNode *newStnVarDeclare(STreeNode *pVarType, STreeNode *pVarNameList, bool bStatic)
{
	ASSERT(pVarType->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = NewSTreeNode(STreeNode::T_VAR_DECLAR, pVarType->nLineNo);
	StnVarDeclare &sVarDeclare = pNode->sVarDeclare;
	sVarDeclare.bStatic = bStatic;
	sVarDeclare.pVarList = pVarNameList;
	sVarDeclare.type = pVarType;

	deleteStn(pVarType);

	return pNode;
}

STreeNode *newStnVar(STreeNode *pVarIdString, STreeNode *pExp)
{
	ASSERT(pVarIdString->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = NewSTreeNode(STreeNode::T_VAR, pVarIdString->nLineNo);
	StnVar		&sVar = pNode->sVar;
	sVar.szVarName = dupStnStr(pVarIdString->szString);
	sVar.pExpAssign = pExp;
	sVar.typeId = 0;
	sVar.nAddress = -1;

	deleteStn(pVarIdString);

	return pNode;
}

STreeNode *newStnClassDeclare(STreeNode *pClassNameString, STreeNode *pParentClassString, STreeNode *pClassMember)
{
	ASSERT(pClassNameString->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = NewSTreeNode(STreeNode::T_CLASS, pClassNameString->nLineNo);
	StnClass	&sClass = pNode->sClass;
	sClass.szClassName = dupStnStr(pClassNameString->szString);
	sClass.pClassMember = pClassMember;
	if (pParentClassString)
	{
		ASSERT(pParentClassString->nodeType == STreeNode::T_STRING);
		sClass.typeParentClass = pParentClassString;
		deleteStn(pParentClassString);
	}
	else
		sClass.typeParentClass.szTypeName = NULL;
	sClass.typeIdClass = 0;

	deleteStn(pClassNameString);

	return pNode;
}

void newStnDefConstructorDeclare(LPCSTR szName, CMemberMethod *pMethod, int nLineNo)
{
	pMethod->pBodyNode = newStnBlock(nLineNo, NULL);
}

// For native function, body = NULL
STreeNode *newStnMethodDeclareNode(STreeNode *pMethodNameString, 
								   STreeNode *pRetTypeString, STreeNode *pParameterList, STreeNode *pBody,
								   bool bStatic)
{
	ASSERT(pMethodNameString->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = NewSTreeNode(STreeNode::T_MEMBER_METHOD, pMethodNameString->nLineNo);
	StnMemberMethod	&sMethod = pNode->sMemberMethod;
	sMethod.bStatic = bStatic;
	sMethod.pBody = pBody;
	sMethod.pParameterList = pParameterList;
	sMethod.szMethodName = dupStnStr(pMethodNameString->szString);
	sMethod.pMethod = NULL;

	if (pRetTypeString)
	{
		ASSERT(pRetTypeString->nodeType == STreeNode::T_STRING);
		sMethod.typeRet = pRetTypeString;
		deleteStn(pRetTypeString);
	}
	else
	{
		sMethod.typeRet.typeId = 0;
		sMethod.typeRet.szTypeName = NULL;
	}

	deleteStn(pMethodNameString);

	return pNode;
}

STreeNode *newStnTry(STreeNode *pTryStmt, STreeNode *pCatchSeq)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_TRY, pTryStmt->nLineNo);
	StnTry		&sTry = pNode->sTry;

	sTry.pTryStmt = pTryStmt;
	sTry.pCatch = pCatchSeq;

	return pNode;
}

STreeNode *newStnCatch(STreeNode *pException, STreeNode *pCatchStmt)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_CATCH, pException->nLineNo);
	StnCatch	&sCatch = pNode->sCatch;

	sCatch.pException = pException;
	sCatch.pCatchStmt = pCatchStmt;
	sCatch.pNextCatch = NULL;

	return pNode;
}

STreeNode *appendCatchTail(STreeNode *pCatchSeq, STreeNode *pCatch)
{
	ASSERT(pCatchSeq->nodeType == STreeNode::T_CATCH);
	pCatchSeq->sCatch.pNextCatch = pCatch;

	return pCatchSeq;
}

STreeNode *newStnThrow(STreeNode *pExprThrow)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_THROW, pExprThrow->nLineNo);
	StnThrow	&sThrow = pNode->sThrow;

	sThrow.pNewExpr = pExprThrow;

	return pNode;
}

STreeNode *newStnIf(STreeNode *pCondition, STreeNode *pTrueBlock, STreeNode *pFalseBlock)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_IF, pCondition->nLineNo);
	StnIf		&sIf = pNode->sIf;

	sIf.pCondition = pCondition;
	sIf.pTrueNode = pTrueBlock;
	sIf.pFalseNode = pFalseBlock;

	return pNode;
}

STreeNode *newStnFor(STreeNode *pInitalization, STreeNode *pTermination, STreeNode *pIncrement, STreeNode *pActionBlock)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_FOR, pInitalization->nLineNo);
	StnFor		&sFor = pNode->sFor;

	sFor.pInitalization = pInitalization;
	sFor.pTermination = pTermination;
	sFor.pIncrement = pIncrement;
	sFor.pActionNode = pActionBlock;

	return pNode;
}

STreeNode *newStnWhile(STreeNode *pCondition, STreeNode *pActionBlock)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_WHILE, pCondition->nLineNo);
	StnWhile	&sWhile = pNode->sWhile;

	sWhile.pCondition = pCondition;
	sWhile.pActionNode = pActionBlock;

	return pNode;
}

STreeNode *newStnDoWhile(STreeNode *pCondition, STreeNode *pActionBlock)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_DO_WHILE, pCondition->nLineNo);
	StnWhile	&sWhile = pNode->sWhile;

	sWhile.pCondition = pCondition;
	sWhile.pActionNode = pActionBlock;

	return pNode;
}

STreeNode *newStnBreak(int nLineNo)
{
	return NewSTreeNode(STreeNode::T_BREAK, nLineNo);
}

STreeNode *newStnContinue(int nLineNo)
{
	return NewSTreeNode(STreeNode::T_CONTINUE, nLineNo);
}

STreeNode *newStnExpAssign(STreeNode *pTargetVar, STreeNode *pExpression)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_EXPRESSION, pTargetVar->nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = E_ASSIGN;

	sExp.assign.pRightExp = pExpression;
	sExp.assign.pVar = pTargetVar;

	return pNode;
}

STreeNode *newStnExpVar(int nLineNo, LPCSTR szVarName, STreeNode *pParentVar)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_EXPRESSION, nLineNo);
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

	deleteStn(pVarNameString);

	return pNode;
}

STreeNode *newStnExprConstInt(STreeNode *pString)
{
	ASSERT(pString->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = NewSTreeNode(STreeNode::T_EXPRESSION, pString->nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = E_CONST_INT;
	sExp.nConstInt = atoi(pString->szString);

	deleteStn(pString);

	return pNode;
}

STreeNode *newStnExprConstBool(int nLineNo, bool bTrue)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_EXPRESSION, nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = E_CONST_BOOLEAN;
	sExp.bConstBool = bTrue;

	return pNode;
}

STreeNode *newStnExprNull(int nLineNo)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_EXPRESSION, nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = E_CONST_NULL;

	return pNode;
}

STreeNode *newStnExprConstString(STreeNode *pString)
{
	ASSERT(pString->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = NewSTreeNode(STreeNode::T_EXPRESSION, pString->nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = E_CONST_STRING;
	sExp.szConstString = dupStnStr(pString->szString);

	deleteStn(pString);

	return pNode;
}

STreeNode *newStnExpNew(STreeNode *pString, STreeNode *pParameterList)
{
	ASSERT(pString->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = NewSTreeNode(STreeNode::T_EXPRESSION, pString->nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = E_NEW;
	sExp.newObj.szNewType = dupStnStr(pString->szString);
	sExp.newObj.pParameter = pParameterList;

	deleteStn(pString);

	return pNode;
}

STreeNode *newStnExpDynamicCast(STreeNode *pIDStr, STreeNode *pVar)
{
	ASSERT(pIDStr->nodeType == STreeNode::T_STRING);

	STreeNode	*pNode = NewSTreeNode(STreeNode::T_EXPRESSION, pIDStr->nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = E_DYNAMIC_CAST;
	sExp.dynCast.szCastToType = dupStnStr(pIDStr->szString);
	sExp.dynCast.pVar = pVar;

	deleteStn(pIDStr);

	return pNode;
}

STreeNode *newStnExpOp(ExpressionType expType, STreeNode *pExpressionLeft, STreeNode *pExpressionRight)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_EXPRESSION, pExpressionLeft->nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = expType;
	sExp.opParameter.pExpressionLeft = pExpressionLeft;
	sExp.opParameter.pExpressionRight = pExpressionRight;

	return pNode;
}

STreeNode *newStnExpFuncCall(int nLineNo, LPSTR szMethodName, STreeNode *pVarToCall, STreeNode *pParameterList)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_EXPRESSION, nLineNo);
	StnExpression	&sExp = pNode->sExpression;
	sExp.type = E_METHOD_CALL;
	sExp.functionCall.szFuncName = dupStr(szMethodName);
	sExp.functionCall.pVarParent = pVarToCall;
	sExp.functionCall.pParameter = pParameterList;

	return pNode;
}

STreeNode *newStnReturn(int nLineNo, STreeNode *pExpression)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_RETURN, nLineNo);
	StnReturn	&sReturn = pNode->sReturn;

	sReturn.pExpression = pExpression;

	return pNode;
}

STreeNode *newStnBlock(int nLineNo, STreeNode *pChildList)
{
	STreeNode	*pNode = NewSTreeNode(STreeNode::T_BLOCK, nLineNo);
	StnBlock	&sBlock = pNode->sBlock;

	sBlock.pChildList = pChildList;
	sBlock.nCountOfVarToPopup = 0;

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
		case STreeNode::T_TRY:
			{
				PrintSapces(nIndent, t->nLineNo);
				printf("try\n");
				if (t->sTry.pTryStmt)
					PrintSyntaxTree(t->sTry.pTryStmt, nIndent);
				ASSERT(t->sTry.pCatch);
				PrintSyntaxTree(t->sTry.pCatch, nIndent);
			}
			break;
		case STreeNode::T_CATCH:
			{
				PrintSapces(nIndent, t->nLineNo);
				printf("catch\n");
				if (t->sCatch.pException)
					PrintSyntaxTree(t->sCatch.pException, nIndent);
				ASSERT(t->sCatch.pCatchStmt);
				PrintSyntaxTree(t->sCatch.pCatchStmt, nIndent);

				if (t->sCatch.pNextCatch)
					PrintSyntaxTree(t->sCatch.pNextCatch, nIndent);
			}
			break;
		case STreeNode::T_THROW:
			{
				PrintSapces(nIndent, t->nLineNo);
				printf("throw");
				if (t->sThrow.pNewExpr)
					PrintSyntaxTree(t->sThrow.pNewExpr, nIndent);
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
				else if (t->sExpression.type == E_CONST_BOOLEAN)
				{
					printf("%s", t->sExpression.bConstBool ? "true" : "false");
					return;
				}
				else if (t->sExpression.type == E_CONST_NULL)
				{
					printf("null");
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
				else if (t->sExpression.type == E_DYNAMIC_CAST)
				{
					printf("DynamicCast %s\n", t->sExpression.dynCast.szCastToType);
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
				else if (t->sExpression.type == E_ASSIGN || t->sExpression.type == E_ASSIGN_STATIC_VAR_INIT)
				{
					PrintSapces(nIndent, t->nLineNo);
					if (t->sExpression.assign.pVar)
						PrintSyntaxTree(t->sExpression.assign.pVar, nIndent);
					printf(" = ");
					if (t->sExpression.assign.pRightExp)
						PrintSyntaxTree(t->sExpression.assign.pRightExp, nIndent);
					printf("\n");
					return;
				}

				if (t->sExpression.opParameter.pExpressionLeft)
					PrintSyntaxTree(t->sExpression.opParameter.pExpressionLeft, nIndent);
				switch (t->sExpression.type) {
					case OP_LT_EQ:		printf("<=");	break;	// <=
					case OP_GT_EQ:		printf(">=");	break;	// >=
					case OP_LITTLE:		printf("<");	break;	// <
					case OP_GREATER:	printf(">");	break;	// >
					case OP_EQUAL:		printf("==");	break;	// ==
					case OP_NOT_EQUAL:	printf("!=");	break;	// !=
					case OP_OP_INC:		printf("++");	break;	// ++
					case OP_OP_DEC:		printf("--");	break;	// --
					case OP_PLUS:		printf("+");	break;	// +
					case OP_MINUS:		printf("-");	break;	// -
					case OP_MULT:		printf("*");	break;	// *
					case OP_DIV:		printf("/");	break;	// /
					case OP_BOOL_AND:	printf("&&");	break;	// &&
					case OP_BOOL_OR:	printf("||");	break;	// ||
					case OP_PLUS_ASSIGN:printf("+=");	break;	// +=
					case OP_MINUS_ASSIGN:printf("-=");	break;	// -=
					default:
						ASSERT(0 && "Invalid expression Type."); break;
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
