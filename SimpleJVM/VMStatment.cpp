#include "StdAfx.h"
#include "SJVM.h"
#include "SyntaxTree.h"
#include "VMStatment.h"

CStmtImage::CStmtImage()
{
	m_stmtIndex.push_back(NULL);
}

CStmtImage::~CStmtImage()
{

}

Statement *CStmtImage::newStmt(int nLineNo, StmtType st, stmt_index &index)
{
	Statement		*ps = new Statement();

	ps->nLineNo = nLineNo;
	ps->stmtType = st;
	m_stmtNewInMem.push_back(ps);
	index = (int)m_stmtIndex.size();
	m_stmtIndex.push_back(ps);

	return ps;
}

stmt_index CStmtImage::newStmtAssign(int nLineNo, stmt_index siVarAddr, stmt_index siExpression)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_ASSIGN, si);

	ps->sAssign.siVarAddress = siVarAddr;
	ps->sAssign.siExpression = siExpression;

	return si;
}

stmt_index CStmtImage::newStmtExpConstInt(int nLineNo, int nValue)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_EXPRESSION, si);

	ps->sExpression.expType = E_CONST;
	ps->sExpression.nConstValue = nValue;
	ps->sExpression.typeId = TID_INT;

	return si;
}

stmt_index CStmtImage::newStmtExpConstBool(int nLineNo, bool bValue)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_EXPRESSION, si);

	ps->sExpression.expType = E_CONST;
	ps->sExpression.nConstValue = bValue;
	ps->sExpression.typeId = TID_BOOLEAN;

	return si;
}

stmt_index CStmtImage::newStmtExpNull(int nLineNo)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_EXPRESSION, si);

	ps->sExpression.expType = E_CONST;
	ps->sExpression.nConstValue = 0;
	ps->sExpression.typeId = TID_NULL;

	return si;
}

stmt_index CStmtImage::newStmtExpConstString(int nLineNo, int nStrJObjIndex)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_EXPRESSION, si);

	ps->sExpression.expType = E_CONST;
	ps->sExpression.nConstValue = nStrJObjIndex;
	ps->sExpression.typeId = TID_STRING;

	return si;
}

stmt_index CStmtImage::newStmtExpNew(int nLineNo, TypeId typeId, int nMemberIndex, stmt_index siParameter)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_EXPRESSION, si);

	ps->sExpression.expType = E_NEW;
	ps->sExpression.typeId = typeId;
	ps->sExpression.newObj.nMemberIndex = nMemberIndex;
	ps->sExpression.newObj.siParameter = siParameter;

	return si;
}

stmt_index CStmtImage::newStmtExpVar(int nLineNo, bool bStatic, TypeId typeId, int nAddress, stmt_index siParentVar)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_EXPRESSION, si);

	ps->sExpression.expType = E_VAR;
	ps->sExpression.typeId = typeId;
	ps->sExpression.var.nAddress = nAddress;
	ps->sExpression.var.bStatic = bStatic;
	ps->sExpression.var.siParentVar = siParentVar;

	return si;
}

stmt_index CStmtImage::newStmtExpFuncCall(int nLineNo, bool bStatic, TypeId typeIdRet, TypeId typeIdMethodClass, stmt_index siParentVar, int nMemberIndex, stmt_index siParameter)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_EXPRESSION, si);

	ps->sExpression.expType = E_METHOD_CALL;
	ps->sExpression.typeId = typeIdRet;
	ps->sExpression.functionCall.bStatic = bStatic;
	ps->sExpression.functionCall.typeIdSpecialfiedClass = typeIdMethodClass;
	ps->sExpression.functionCall.nMemberIndex = nMemberIndex;
	ps->sExpression.functionCall.siParentVar = siParentVar;
	ps->sExpression.functionCall.siParameter = siParameter;

	return si;
}

stmt_index CStmtImage::newStmtExpOperate(int nLineNo, ExpressionType typeOperator, TypeId typeIdValue, stmt_index siExprLeft, stmt_index siExprRight, TypeId typeIdOperand)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_EXPRESSION, si);

	ps->sExpression.expType = typeOperator;
	ps->sExpression.typeId = typeIdValue;
	ps->sExpression.opParameter.siExprLeft = siExprLeft;
	ps->sExpression.opParameter.siExprRight = siExprRight;
	ps->sExpression.opParameter.typeIdOperand = typeIdOperand;

	return si;
}

stmt_index CStmtImage::newStmtIf(int nLineNo, stmt_index siCondition, stmt_index siTrueStmt, stmt_index siFalseStmt)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_IF, si);

	ps->sIf.siCondition = siCondition;
	ps->sIf.siTrueStmt = siTrueStmt;
	ps->sIf.siFalseStmt = siFalseStmt;

	return si;
}

stmt_index CStmtImage::newStmtFor(int nLineNo, stmt_index siInitalization, stmt_index siTermination, stmt_index siIncrement, stmt_index siActionStmt)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_FOR, si);

	ps->sFor.siInitalization = siInitalization;
	ps->sFor.siTermination = siTermination;
	ps->sFor.siIncrement = siIncrement;
	ps->sFor.siStmt = siActionStmt;

	return si;
}

stmt_index CStmtImage::newStmtWhile(int nLineNo, stmt_index siCondition, stmt_index siActionStmt)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_WHILE, si);

	ps->sWhile.siCondition = siCondition;
	ps->sWhile.siStmt = siActionStmt;

	return si;
}

stmt_index CStmtImage::newStmtDoWhile(int nLineNo, stmt_index siCondition, stmt_index siActionStmt)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_DO_WHILE, si);

	ps->sWhile.siCondition = siCondition;
	ps->sWhile.siStmt = siActionStmt;

	return si;
}

stmt_index CStmtImage::newStmtReturn(int nLineNo, stmt_index siExpression)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_RETURN, si);

	ps->sRetrun.siExpression = siExpression;

	return si;
}

stmt_index CStmtImage::newStmtBlock(int nLineNo, stmt_index &siChildBlock)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_BLOCK, si);

	ps->sBlock.siFirstChild = siChildBlock;

	return si;
}

stmt_index CStmtImage::newStmtStackPush(int nLineNo, TypeId typeId)
{
	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_PUSH_STACK, si);

	ps->sStackPush.typeIdVar = typeId;

	return si;
}

stmt_index CStmtImage::newStmtStackPop(int nLineNo, int nCountOfVarToPopup)
{
	ASSERT(nCountOfVarToPopup > 0);

	stmt_index		si;
	Statement		*ps = newStmt(nLineNo, ST_POP_STACK, si);
	ps->sStackPop.nCountOfVarToPopup = nCountOfVarToPopup;

	return si;
}

Statement *CStmtImage::GetStmt(stmt_index si)
{
	ASSERT(si > SI_INVALID && si < m_stmtIndex.size());
	if (si > SI_INVALID && si < m_stmtIndex.size())
		return m_stmtIndex[si];

	return NULL;
}

stmt_index *CStmtImage::GetTail(stmt_index *psi)
{
	ASSERT(psi);

	if (*psi == SI_INVALID)
		return psi;

	Statement		*ps = GetStmt(*psi);
	while (ps)
	{
		psi = &(ps->siNextStmt);
		if (ps->siNextStmt == SI_INVALID)
			break;
		ps = GetStmt(*psi);
	}

	return psi;
}

stmt_index CStmtImage::GetNext(stmt_index si)
{
	if (si == SI_INVALID)
		return si;

	Statement		*ps = GetStmt(si);
	if (ps)
		return ps->siNextStmt;

	return si;
}


#ifdef _DEBUG

void PrintSapces(int nIndent)
{
	for (int i = 0; i < nIndent; i++)
		printf("  ");
}

void CStmtImage::PrintStmtCodeTree(stmt_index si, int &nIndent)
{
	Statement	*ps = GetStmt(si);
	while (ps)
	{
		switch (ps->stmtType)
		{
		case ST_BLOCK:
			{
				StmtBlock	&sBlock = ps->sBlock;
				PrintSapces(nIndent); printf("{\n");
				nIndent++;
				if (sBlock.siFirstChild != SI_INVALID)
					PrintStmtCodeTree(sBlock.siFirstChild, nIndent);
				nIndent--;
				PrintSapces(nIndent); printf("}\n");
			}
			break;
		case ST_IF:
			{
				StmtIf	&sIf = ps->sIf;
				PrintSapces(nIndent); printf("if (");
				PrintStmtCodeTree(sIf.siCondition, nIndent);
				printf(")\n");

				if (sIf.siTrueStmt != SI_INVALID)
				{
					nIndent++;
					PrintStmtCodeTree(sIf.siTrueStmt, nIndent);
					nIndent--;
				}

				if (sIf.siFalseStmt != SI_INVALID)
				{
					nIndent++;
					PrintStmtCodeTree(sIf.siFalseStmt, nIndent);
					nIndent--;
				}
			}
			break;
		case ST_FOR:
			{
				StmtFor	sFor = ps->sFor;
				PrintSapces(nIndent); printf("for (");
				PrintStmtCodeTree(sFor.siInitalization, nIndent);
				printf(";");
				PrintStmtCodeTree(sFor.siTermination, nIndent);
				printf(";");
				PrintStmtCodeTree(sFor.siIncrement, nIndent);
				printf(")\n");

				nIndent++;
				if (sFor.siStmt != SI_INVALID)
					PrintStmtCodeTree(sFor.siStmt, nIndent);
				nIndent--;
			}
			break;
		case ST_WHILE:
			{
				StmtWhile	sWhile = ps->sWhile;
				PrintSapces(nIndent); printf("while (");
				PrintStmtCodeTree(sWhile.siCondition, nIndent);
				printf(")\n");

				nIndent++;
				if (sWhile.siStmt != SI_INVALID)
					PrintStmtCodeTree(sWhile.siStmt, nIndent);
				nIndent--;
			}
			break;
		case ST_DO_WHILE:
			{
				StmtWhile	sWhile = ps->sWhile;
				PrintSapces(nIndent); printf("do\n");
				nIndent++;
				if (sWhile.siStmt != SI_INVALID)
					PrintStmtCodeTree(sWhile.siStmt, nIndent);
				nIndent--;

				PrintSapces(nIndent); printf("while (");
				PrintExpression(sWhile.siCondition);
				printf(");\n");
			}
			break;
		case ST_ASSIGN:
			{
				StmtAssign	&sAssign = ps->sAssign;
				PrintSapces(nIndent); 
				PrintStmtCodeTree(sAssign.siVarAddress, nIndent);
				printf("=");
				PrintStmtCodeTree(sAssign.siExpression, nIndent);
			}
			break;
		case ST_RETURN:
			{
				StmtReturn	&sReturn = ps->sRetrun;
				PrintSapces(nIndent); printf("return ");
				if (sReturn.siExpression != SI_INVALID)
					PrintStmtCodeTree(sReturn.siExpression, nIndent);
				printf(";\n");
			}
			break;
		case ST_PUSH_STACK:
			{
				StmtStackPush	&sStackPush = ps->sStackPush;
				PrintSapces(nIndent); printf("LocalVarDeclare, type: %d\n", sStackPush.typeIdVar);
			}
			break;
		case ST_POP_STACK:
			{
				PrintSapces(nIndent); printf("Popup and Free type\n");
			}
			break;
		case ST_EXPRESSION:
			{
				PrintExpression(si);
			}
			break;
		default:
			ASSERT(0 && "Invalid statement type");
			return;
		}

		if (ps->siNextStmt == SI_INVALID)
			break;
		si = ps->siNextStmt;
		ps = GetStmt(ps->siNextStmt);

	}
}


void CStmtImage::PrintExpression(stmt_index siExpr)
{
	ASSERT(siExpr != SI_INVALID);
	Statement		*ps = GetStmt(siExpr);
	ASSERT(ps->stmtType == ST_EXPRESSION);
	StmtExpression	&expr = ps->sExpression;

	switch (expr.expType)
	{
	case E_CONST:
		printf("%d", expr.nConstValue);
		return;
	case E_NEW:
		printf("new %d", expr.typeId);
		return;
	case E_VAR:
		{
			if (expr.var.bStatic)
			{
				printf("(static var)");
			}

			if (expr.var.siParentVar != SI_INVALID)
			{
				// A Jobject member varaible
				PrintExpression(expr.var.siParentVar);
				printf("(member var) %d ", expr.var.nAddress);
				return;
			}

			// Variable is in stack
			printf("(local var) %d ", expr.var.nAddress);

			return;
		}
	case E_METHOD_CALL:
		{
			// Variable is in stack
			printf("Function: %d ", expr.var.nAddress);
			Statement		*pStmt;

			if (!expr.functionCall.bStatic)
			{
				ASSERT(expr.functionCall.siParentVar != SI_INVALID);
				PrintExpression(expr.functionCall.siParentVar);
			}

			// Execute parameter expressions
			stmt_index		siParameter = expr.functionCall.siParameter;

			printf("Parameter: ");
			// Push parameter in stack
			while (siParameter != SI_INVALID)
			{
				pStmt = GetStmt(siParameter);
				ASSERT(pStmt);
				PrintExpression(siParameter);

				siParameter = GetNext(siParameter);
			}

			// Call method
			if (expr.functionCall.typeIdSpecialfiedClass != TID_INVALID)
				printf("CallSpecialfiedMethod: %d, %d", expr.functionCall.typeIdSpecialfiedClass, expr.functionCall.nMemberIndex);
			else
				printf("CallJObjectMethod: %d", expr.functionCall.nMemberIndex);

			return;
		}
	default:
		break;
	}

	PrintExpression(expr.opParameter.siExprLeft);

	switch (expr.expType)
	{
	case OP_LT_EQ:		// <=
		printf("<="); return;
	case OP_GT_EQ:		// >=
		printf(">="); return;
	case OP_LITTLE:		// <
		printf("<"); return;
	case OP_GREATER:	// >
		printf(">"); return;
	case OP_EQUAL:		// ==
		printf("=="); return;
	case OP_NOT_EQUAL:	// !=
		printf("!="); return;
	case OP_BOOL_AND:	// &&
		printf("&&"); return;
	case OP_BOOL_OR:	// ||
		printf("||"); return;
	case OP_PLUS:		// +
		printf("+"); return;
	case OP_MINUS:		// -
		printf("-"); return;
	case OP_MULT:		// *
		printf("*"); return;
	case OP_DIV:		// /
		printf("/"); return;
	case OP_OP_INC:		// ++
		printf("++"); return;
	case OP_OP_DEC:		// --
		printf("--"); return;
	default:
		ASSERT(0 && "Undefined expression type.");
	}

	if (expr.opParameter.siExprRight != SI_INVALID)
		PrintExpression(expr.opParameter.siExprRight);

	return ;
}

#endif
