// SkinScript.h: interface for the CSkinScript class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINSCRIPT_H__4BBA1D6A_B5A9_4BE3_B54C_AFF2B445D5FB__INCLUDED_)
#define AFX_SKINSCRIPT_H__4BBA1D6A_B5A9_4BE3_B54C_AFF2B445D5FB__INCLUDED_

/*
	Skin script compiler interface
*/


extern int g_srcLineNo; // source line number for listing

typedef uint16		TYPE_NAME_ID;

#define MAX_CHILDREN		3

#define SS_TYPE_STRING		"string"
#define SS_TYPE_INT			"int"
#define SS_TYPE_VOID		"void"
#define SS_TYPE_ANY			"any"

enum SS_NODE_TYPE
{
	NT_VAR_DECL,		// varable declearation
	NT_VAR_ID,			// varable, EK_ID_REF and EK_ID is available for kind.exp
	NT_FUNC_IMP,		// function implementation
	NT_CLASS_IMP,		// class implementation
	NT_STMT,			// statement
	NT_EXP,				// expression
};

enum STMT_KIND
{
	SK_NULL,
	SK_IF,
	SK_WHILE,
	SK_DO_WHILE,
	SK_ASSIGN,
	SK_RETURN,
	SK_BLOCK,
	SK_DELETE,
};

enum EXP_KIND
{
	EK_OP,
	EK_FUNC_CALL,
	EK_CONST_INT,
	EK_CONST_STRING,
	EK_ID,
	EK_ID_REF,
	EK_NEW,
};

enum EXP_OP
{
	EO_LT_EQ,		// <=
	EO_GT_EQ,		// >=
	EO_LITTLE,		// <
	EO_GREATER,		// >
	EO_EQUAL,		// ==
	EO_NOT_EQUAL,	// !=
	EO_OP_INC,		// ++
	EO_OP_DEC,		// --
	EO_PLUS,		// +
	EO_MINUS,		// -
	EO_MULT,		// *
	EO_DIV,			// /
	EO_BOOL_AND,	// &&
	EO_BOOL_OR,		// ||
};

struct SS_TREE_NODE;

struct SS_TREE_NODE
{
	SS_TREE_NODE()
	{
		for (int i = 0; i < MAX_CHILDREN; i++)
			child[i] = NULL;
		sibling = NULL;
		szName = NULL;
#ifdef _DEBUG
// 		nLineNo = 0;
// 		kind.stmt = 0;
// 		attr.lValue = 0;
#endif
	}
	SS_TREE_NODE	*child[MAX_CHILDREN];
	SS_TREE_NODE	*sibling;
	int				nLineNo;

	SS_NODE_TYPE	nodeType;
	union
	{
		STMT_KIND	stmt;
		EXP_KIND	exp;
	}kind;
	union
	{
		EXP_OP		op;			// for EXP_KIND
		int			lValue;
	}attr;
	const char		*szName;

	TYPE_NAME_ID	typeId;
};

SS_TREE_NODE *newVarDeclNode(int nLineNo, const char *szVarTypeName, SS_TREE_NODE *varNameList);

SS_TREE_NODE *newVarDeclRefNode(int nLineNo, const char *szVarTypeName, SS_TREE_NODE *varNameList);

SS_TREE_NODE *newVarIDNode(int nLineNo, const char * szVarName, SS_TREE_NODE *pAssignExp = NULL);

void SetVarIDNodeAssignExp(SS_TREE_NODE *pIDTree, SS_TREE_NODE *pTreeExp);

SS_TREE_NODE *newFuncDeclNode(int nLineNo, const char *szFuncName, SS_TREE_NODE *funcRetType, SS_TREE_NODE *funcParam, SS_TREE_NODE *funcBody);

SS_TREE_NODE *newClassDeclNode(int nLineNo, const char *szClassName, SS_TREE_NODE *parentClass, SS_TREE_NODE *classBody);

SS_TREE_NODE *newStmtNode(STMT_KIND stmtKind, SS_TREE_NODE *child0, SS_TREE_NODE *child1);

SS_TREE_NODE *IDNodeToExpNode(EXP_KIND expKind, SS_TREE_NODE *node);

SS_TREE_NODE *newFuncCallNode(SS_TREE_NODE *funcName, SS_TREE_NODE *parameter);

SS_TREE_NODE *newExpOpNode(EXP_OP op, SS_TREE_NODE *op1, SS_TREE_NODE *op2);

SS_TREE_NODE *AppendChild0Node(SS_TREE_NODE *head, SS_TREE_NODE *childLast);

SS_TREE_NODE *AppendSiblingNode(SS_TREE_NODE *head, SS_TREE_NODE *tail);

SS_TREE_NODE *InsertSiblingHeadNode(SS_TREE_NODE *head, SS_TREE_NODE *newHead);

void PrintSyntaxTree(SS_TREE_NODE *t, int &nIndent);

char *dupStr(const char *str);

void deleteSSNode(SS_TREE_NODE *t);

#endif // !defined(AFX_SKINSCRIPT_H__4BBA1D6A_B5A9_4BE3_B54C_AFF2B445D5FB__INCLUDED_)
