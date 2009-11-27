#pragma once


typedef uint16					stmt_index;
#define SI_INVALID				0

enum StmtType
{
	ST_UNKNOWN,
	ST_BLOCK,
	ST_IF,
	ST_FOR,
	ST_WHILE,
	ST_DO_WHILE,
	ST_BREAK,
	ST_CONTINUE,
	ST_ASSIGN,
	ST_RETURN,
	ST_PUSH_STACK,
	ST_POP_STACK,

	ST_EXPRESSION,
};

// 
// struct StmtVariableAddress {
// 	int					*pResultAddress;
// 
// 	bool				bStaticAddr;
// 	TypeId				typeId;			// 
// 	int					nAddress;
// 	stmt_index			siParentVar;
// };

struct StmtBlock {
	stmt_index			siFirstChild;
};

struct StmtIf {
	stmt_index			siCondition;
	stmt_index			siTrueStmt;
	stmt_index			siFalseStmt;
};

struct StmtFor {
	stmt_index			siInitalization;
	stmt_index			siTermination;
	stmt_index			siIncrement;
	stmt_index			siStmt;
};

struct StmtWhile {
	stmt_index			siCondition;
	stmt_index			siStmt;
};

struct StmtDoWhile {
	stmt_index			siCondition;
	stmt_index			siStmt;
};

struct StmtAssign {
	stmt_index			siVarAddress;
	stmt_index			siExpression;
};

struct StmtReturn {
	stmt_index			siExpression;
};

struct StmtStackPush {
	// Push variable in stack, and save its typeId
	TypeId				typeIdVar;
};

struct StmtStackPop {
	// Push variable in stack, and save its typeId
	int					nCountOfVarToPopup;
};

struct StmtExpression {
	ExpressionType		expType;
	TypeId				typeId;				// E_NEW and typeId of this expression
	union {
		int					nConstValue;	// E_CONST_INT, E_CONST_STRING
		struct New {
			stmt_index		siParameter;	// Parameter of construction method
			int				nMemberIndex;
		}					newObj;			// E_NEW
		struct Var {						// E_VAR
			int				nAddress;
			bool			bStatic;
			stmt_index		siParentVar;
		}					var;
		struct FunctionCall {
			bool			bStatic;
			TypeId			typeIdSpecialfiedClass;	// SI_INVALID, indicate call from typeId of pVarParent.
			int				nMemberIndex;
			stmt_index		siParentVar;
			stmt_index		siParameter;
		}					functionCall;		// E_METHOD_CALL
		struct OpExression {
			stmt_index		siExprLeft;
			stmt_index		siExprRight;
			TypeId			typeIdOperand;		// The type of operand(left, right)
		}					opParameter;		// OP_XXXX
	};
};

struct StmtNewStackVar {
};

struct StmtFunctionCall {
	int					nRetValue;		// Return Value

	TypeId				typeIdRet;		// Return type
	TypeId				typeIdClass;	// Method class, if (typeIdClass == TID_INVALID), call dynamic method of siParentVar
	int					nMethodId;		// Method Id
	stmt_index			siParentVar;
	stmt_index			siParmeterExpStart;	// First Parameter: siParmeterExpStart, Second: siParmeterExpStart->siNextStmt
};

struct Statement
{
	uint16				nLineNo;
	stmt_index			siNextStmt;

	StmtType			stmtType;		// Type of the union.
	union {
		// StmtVariableAddress		sVar;
		StmtIf					sIf;
		StmtFor					sFor;
		StmtWhile				sWhile;
		StmtDoWhile				sDoWhile;
		StmtAssign				sAssign;
		StmtReturn				sRetrun;
		StmtBlock				sBlock;
		StmtExpression			sExpression;
		StmtNewStackVar			sNewStackVar;
		StmtFunctionCall		sFunctionCall;
		StmtStackPush			sStackPush;
		StmtStackPop			sStackPop;
	};
};

typedef Statement*		PStatement;

class CStmtImage
{
public:
	CStmtImage();
	virtual ~CStmtImage();

	Statement *newStmt(int nLineNo, StmtType st, stmt_index &index);

public:
	stmt_index newStmtAssign(int nLineNo, stmt_index siVarAddr, stmt_index siExpression);

	stmt_index newStmtExpConstInt(int nLineNo, int nValue);
	stmt_index newStmtExpConstBool(int nLineNo, bool bValue);
	stmt_index newStmtExpNull(int nLineNo);
	stmt_index newStmtExpConstString(int nLineNo, int nStrJObjIndex);
	stmt_index newStmtExpNew(int nLineNo, TypeId typeId, int nMemberIndex, stmt_index siParameter);
	stmt_index newStmtExpVar(int nLineNo, bool bStatic, TypeId typeId, int nAddress, stmt_index siParentVar);
	stmt_index newStmtExpFuncCall(int nLineNo, bool bStatic, TypeId typeIdRet, TypeId typeIdMethodClass, stmt_index siParentVar, int nMemberIndex, stmt_index siParameter);
	stmt_index newStmtExpOperate(int nLineNo, ExpressionType typeOperator, TypeId typeIdValue, stmt_index siExprLeft, stmt_index siExprRight, TypeId typeIdOperand);

	stmt_index newStmtIf(int nLineNo, stmt_index siCondition, stmt_index siTrueStmt, stmt_index siFalseStmt);
	stmt_index newStmtFor(int nLineNo, stmt_index siInitalization, stmt_index siTermination, stmt_index siIncrement, stmt_index siActionStmt);
	stmt_index newStmtWhile(int nLineNo, stmt_index siCondition, stmt_index siActionStmt);
	stmt_index newStmtDoWhile(int nLineNo, stmt_index siCondition, stmt_index siActionStmt);
	stmt_index newStmtBreak(int nLineNo);
	stmt_index newStmtContinue(int nLineNo);
	stmt_index newStmtReturn(int nLineNo, stmt_index siExpression);
	stmt_index newStmtBlock(int nLineNo, stmt_index &siChildBlock);

	stmt_index newStmtStackPush(int nLineNo, TypeId typeId);
	stmt_index newStmtStackPop(int nLineNo, int nCountOfVarToPopup);

	Statement *GetStmt(stmt_index si);
	stmt_index *GetTail(stmt_index *psi);
	stmt_index GetNext(stmt_index si);

#ifdef _DEBUG
	void PrintStmtCodeTree(stmt_index si, int &nIndent);
	void PrintExpression(stmt_index siExpr);
#endif

protected:
	string				m_memStmtOnDisk;
	list<Statement *>	m_stmtNewInMem;
	vector<Statement *>	m_stmtIndex;

};
