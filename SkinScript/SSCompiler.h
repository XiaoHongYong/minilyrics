// SSCompiler.h: interface for the CSSCompiler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSCOMPILER_H__3CF1E6F7_A944_4C37_9674_9781D1BFEEAB__INCLUDED_)
#define AFX_SSCOMPILER_H__3CF1E6F7_A944_4C37_9674_9781D1BFEEAB__INCLUDED_

#include "SkinScript.h"

/*****************************************************************************
Compile step:
	1). flex
	2). yacc
	3). build symbol table, syntax check
	4). Generate statement tree


Skin script lanuage:
	1). (C++)--  :)
	2). Differences with C++
		A). 无指针, 但是所有用户定义的class/struct 需要用new/delete 来构造和析构, 简化堆栈的构造
		B). 无虚函数
		C). 单继承


函数调用和参数传递规则:
	1). 调用函数传递参数到栈中, 并且预留返回值. 顺序: 1) 返回值, 2) class 的 this指针, 3) 参数.

*****************************************************************************/


//
// two kind of stack address type(SSM_STACK_ADDR):
//   1) Global address: related from the bottom of stack, with 0x01 in high bytes.
//   2) Local address : related from the top of stack.
//   3) Heap address  : address index in heap.
//   ?) Direct address: address to memory directly.
typedef uint32					SSM_STACK_ADDR;

typedef vector<TYPE_NAME_ID>	V_TYPE_NAME_ID;

extern TYPE_NAME_ID			g_typeIDInt, g_typeIDString, g_typeIDVoid, g_typeIDAny;

struct TypeInfo
{
	TYPE_NAME_ID		typeID;
	bool				bReference;
	char				nClsMemberVar;	// -1 for unavailable.
};


#define  SSM_IS_GLOBAL_STACK_ADDR(addr)			(((addr) & 0x01000000) != 0)
#define	 SSM_SET_GLOBAL_STACK_ADDR_FLAG(addr)	(addr |= 0x01000000)
#define  SSM_GET_STACK_ADDR(addr)				((addr) & 0xFFFFFF)

#define MAKE_CLASS_FUNC_ID(nClassID, nFuncID)	(((nClassID) << 16) | (nFuncID))
#define GET_CLASS_ID(nID)						(((nID) & 0xFFFF0000) >> 16)
#define GET_FUNC_ID(nID)						((nID) & 0xFFFF)

//
// statement tree node, converted from SS_TREE_NODE, SSM executes this tree node.
//
struct STMT_TREE_NODE
{
	enum STMT_NODE_TYPE
	{
		SNT_UNKNOWN,
		SNT_BLOCK,
		SNT_IF,
		SNT_WHILE,
		SNT_DO_WHILE,
		SNT_ASSIGN,
		SNT_RETURN,
		SNT_ASSIGN_REF,	// assign to an reference value.
		SNT_LT_EQ,		// <=
		SNT_GT_EQ,		// >=
		SNT_LITTLE,		// <
		SNT_GREATER,	// >
		SNT_EQUAL,		// ==
		SNT_NOT_EQUAL,	// !=
		SNT_OP_INC,		// ++
		SNT_OP_DEC,		// --
		SNT_PLUS,		// +
		SNT_MINUS,		// -
		SNT_MULT,		// *
		SNT_DIV,		// /
		SNT_BOOL_AND,		// *
		SNT_BOOL_OR,		// /

		SNT_NEW_STACK_VAR,		// new a variable, and push to stack.
		SNT_POPUP_STACK_N,

		SNT_FUNC_CALL,
		SNT_VAR,
		SNT_VAR_REF,
		SNT_CLASS_VAR,
		SNT_CONST_INT,
		SNT_CONST_STR,
		SNT_NEW,				// alloc memory in heap
		SNT_DELETE,				// free memory in heap
	};

	STMT_TREE_NODE()
	{
		child[0] = child[1] = child[2] = NULL;
		sibling = NULL;
	}

	STMT_TREE_NODE	*child[MAX_CHILDREN];
	STMT_TREE_NODE	*sibling;

	STMT_NODE_TYPE	nodeType;
	int				nLineNo;

	// expression, id nodeType will store the value in nResult, after the statement is executed.
	int				nResult;

	// used to store the address of variable in stack, etc.
	SSM_STACK_ADDR	addr;

	void AddTailSibing(STMT_TREE_NODE *pTail);

};

typedef STMT_TREE_NODE *	PSTMT_TREE_NODE;

PSTMT_TREE_NODE newStmtAllocStackVar(TYPE_NAME_ID typeID);
PSTMT_TREE_NODE newStmt(STMT_TREE_NODE::STMT_NODE_TYPE nodeType);

void deleteStmt(STMT_TREE_NODE *pNode);

class CSSMachine;

enum SSC_ERROR
{
	ERR_C_UNDECLARED_ID		= 3000,
	ERR_C_PARSE,
	ERR_C_REDEFINITION_ID,
	ERR_C_OP_TYPE_NOT_MATCH,
	ERR_C_OP_REQUIRE_INT_STR_TYPE,
	ERR_C_OP_REQUIRE_INT_TYPE,
	ERR_C_REDEFINITION_TYPE,
	ERR_C_CONVERT_VALUE,
	ERR_C_UNEXPECTED_ERR,
	ERR_C_FUNC_RETURN_VALUE,
	ERR_C_DELETE_WRONG_TYPE,
	ERR_C_NEW_WRONG_TYPE,

	ERR_R_UNDECLARED_FUNCTION,		// undeclared function
	ERR_R_FUNC_PARAM_COUNT,			// incorrect function parameter count is passed
	ERR_R_FUNC_PARAM_TYPE,			// incorrect function parameter is passed
	ERR_R_FUNC_PARAM,
};

#define INVALID_TYPE_ID		0
#define INVALID_FUNC_ID		0

#define FUNC_ID_GLOBAL_CONSTRUCT	1
#define FUNC_ID_GLOBAL_DESTRUCT		2
#define FUNC_ID_CLASS_CONSTRUCT		1
#define FUNC_ID_CLASS_DESTRUCT		2

//
// function declaration table.
//
class CFunctionDeclTable
{
public:
	struct FunctionDecl
	{
		FunctionDecl()
		{
			m_pstmtTree = NULL;
			typeIDRet = g_typeIDVoid;
		}
		FunctionDecl(LPCSTR szFuncName, TYPE_NAME_ID typeRet)
		{
			strName = szFuncName;
			typeIDRet = typeRet;
			m_pstmtTree = NULL;
		}
		~FunctionDecl()
		{
			if (m_pstmtTree)
				deleteStmt(m_pstmtTree);
		}
		void AddParam(TYPE_NAME_ID typeID, bool bReference = false)
		{
			TypeInfo	paramType;
			paramType.typeID = typeID;
			paramType.bReference = bReference;
			paramType.nClsMemberVar = -1;
			vParamType.push_back(paramType);
		}

		string					strName;
		TYPE_NAME_ID			typeIDRet;
		vector<TypeInfo>		vParamType;
		PSTMT_TREE_NODE			m_pstmtTree;

	};

	CFunctionDeclTable();
	~CFunctionDeclTable();

	typedef vector<FunctionDecl*>		V_FUNCTION_DECL;

	int AddFunc(FunctionDecl *funcDecl);

	FunctionDecl *GetFuncByID(int nID);
	FunctionDecl *GetFuncByName(LPCSTR szName);
	int GetFuncIDByName(LPCSTR szFuncName);

	size_t GetFuncCount() { return m_vFunctions.size(); }

	bool IsFuncDeclared(LPCSTR szFuncName);

public:
	V_FUNCTION_DECL			m_vFunctions;

};

//
// class declaration table.
//
class CClassDeclarationTable
{
public:
	struct ClassMemberVar
	{
		TYPE_NAME_ID		typeID;
		string				strName;
		int					nLineNo;
	};
	typedef vector<ClassMemberVar>		V_MEMBER_VAR;

	struct ClassDecl
	{
		ClassDecl(LPCSTR szName = "");

		int GetMemberVarIndex(const char *szName);
		int AddMemberVar(TYPE_NAME_ID typeID, const char *szVarName, int nLineNo);
		int AddMemberMethod(CFunctionDeclTable::FunctionDecl *funcDecl);
		size_t GetClassSize() { return vMemberVar.size(); }

		string					strName;
		// string				strParentName;		// parent class name.
		// TYPE_NAME_ID			typeIDParent;
		V_MEMBER_VAR			vMemberVar;

		CFunctionDeclTable		m_methodTable;
	};

	typedef vector<ClassDecl *>		V_CLASS_DECL;

	CClassDeclarationTable();
	~CClassDeclarationTable();

	int AddClass(ClassDecl *classDecl);

	int AddSimpleType(const char *szTypeName);

	ClassDecl *GetClassByID(TYPE_NAME_ID nID);
	ClassDecl *GetClassByName(LPCSTR szName);
	TYPE_NAME_ID GetClassIDByName(LPCSTR szName);

	size_t GetClassCount() { return m_vClasses.size(); }

	bool IsClassDeclared(LPCSTR szName);

	CFunctionDeclTable::FunctionDecl *GetClassMethod(TYPE_NAME_ID nClassID, int nMethodID);

protected:
	V_CLASS_DECL				m_vClasses;

};

//
// machine stack
//
class CSSMStack
{
public:
	void Push(int nInitValue) { m_vInt.push_back(nInitValue); }
	void Push(CSSMStack &stack)
	{
		for (vector<int>::iterator it = stack.m_vInt.begin(); it != stack.m_vInt.end(); ++it)
			m_vInt.push_back(*it);
	}

	void InsertFront(int nValue) { m_vInt.insert(m_vInt.begin(), nValue); }

	void Popup() { m_vInt.pop_back(); }
	void PopupN(size_t nSize) { m_vInt.erase(m_vInt.end() - nSize, m_vInt.end()); }

	int TopValue() { return m_vInt.back(); }
	size_t size() { return m_vInt.size(); }

	int GetValue(int nPos)
	{
		if (nPos >= 0 && nPos < m_vInt.size())
			return m_vInt[nPos];
		else
		{
			ASSERT(0 && "Out of stack range.");
			return 0;
		}
	}

	void SetValue(int nPos, int nValue)
	{
		if (nPos >= 0 && nPos < m_vInt.size())
			m_vInt[nPos] = nValue;
		else
			ASSERT(0 && "Out of stack range.");
	}

	int GetValueByStackAddr(SSM_STACK_ADDR	addr)
	{
		int		nPos;

		if (SSM_IS_GLOBAL_STACK_ADDR(addr))
			nPos = SSM_GET_STACK_ADDR(addr);
		else
			nPos = m_vInt.size() - SSM_GET_STACK_ADDR(addr);
		ASSERT(nPos >= 0 && nPos < m_vInt.size());

		if (nPos >= 0 && nPos < m_vInt.size())
			return m_vInt[nPos];
		else
		{
			ASSERT(0 && "Out of stack range.");
			return 0;
		}
	}

	void SetValueByStackAddr(SSM_STACK_ADDR	addr, int nValue)
	{
		int		nPos;

		if (SSM_IS_GLOBAL_STACK_ADDR(addr))
			nPos = SSM_GET_STACK_ADDR(addr);
		else
			nPos = m_vInt.size() - SSM_GET_STACK_ADDR(addr);
		ASSERT(nPos >= 0 && nPos < m_vInt.size());

		if (nPos >= 0 && nPos < m_vInt.size())
			m_vInt[nPos] = nValue;
		else
			ASSERT(0 && "Out of stack range.");
	}

	void Print()
	{
		printf("Stack %d: ", m_vInt.size());
		for (int i = 0; i < m_vInt.size(); i++)
			printf("%X	", m_vInt[i]);
		printf("\n");
	}

protected:
	vector<int>			m_vInt;

};

struct HeapData
{
	int		nData[1];
};

typedef HeapData *	HHEAPDATA;

#define HEAP_FLAG		0x67890000

inline bool HeapIsValid(HHEAPDATA heapData)
{
	ASSERT(heapData);
	return heapData != NULL && ((heapData->nData[0] & 0xFFFF0000) == HEAP_FLAG);
}

inline int HeapLength(HHEAPDATA heapData)
{
	ASSERT(HeapIsValid(heapData));

	return (heapData->nData[0] & 0xFFFF);
}

inline int *HeapGetDataPtr(HHEAPDATA heapData)
{
	ASSERT(HeapIsValid(heapData));
	return heapData->nData + 1;
}

inline int HeapGetValue(HHEAPDATA heapData, int nIndex)
{
	ASSERT(HeapIsValid(heapData));
	ASSERT(nIndex >= 0 && nIndex < HeapLength(heapData));
	return heapData->nData[nIndex + 1];
}

inline void HeapSetValue(HHEAPDATA heapData, int nIndex, int nValue)
{
	ASSERT(HeapIsValid(heapData));
	ASSERT(nIndex >= 0 && nIndex < HeapLength(heapData));
	heapData->nData[nIndex + 1] = nValue;
}

//
// The heap : alloc and free dynamic memory
//
class CSSMHeap
{
public:
	CSSMHeap();
	~CSSMHeap();

//	LPVOID New(size_t size);
//	void Delete(LPVOID lpData);

	HHEAPDATA New(size_t size);
	void Delete(HHEAPDATA hHeap);

protected:

};

class CConstStrTable
{
public:
	~CConstStrTable()
	{
		for (int i = 0; i < m_vStr.size(); i++)
		{
			delete[] (char*) m_vStr[i];
		}
	}

	const char *Add(const char*str)
	{
		for (int i = 0; i < m_vStr.size(); i++)
		{
			if (strcmp(str, m_vStr[i]) == 0)
				return m_vStr[i];
		}

		str = dupStr(str);
		m_vStr.push_back(str);
		return str;
	}

protected:
	typedef vector<const char*>	V_CSTR;
	V_CSTR			m_vStr;

};

//
// The process definition of skin script machine.
//
class CSSMProcess
{
public:
	CSSMProcess(CSSMachine *machine);
	~CSSMProcess();

	int CallFun(int nClassID, int nFuncID, CSSMStack *params);
	int CallFun(int nFuncID, CSSMStack *params);

	void PrintCodeTree();

	//
	// function declaration table method
	//
	int AddFunc(CFunctionDeclTable::FunctionDecl *funcDecl) { return m_funcTable.AddFunc(funcDecl); }

	CFunctionDeclTable::FunctionDecl *GetFuncByID(int nID);
	CFunctionDeclTable::FunctionDecl *GetFuncByName(LPCSTR szName);
	int GetFuncIDByName(LPCSTR szFuncName);

	//
	// class table method
	//
	int AddClass(CClassDeclarationTable::ClassDecl *classDecl) { return m_classDeclTable.AddClass(classDecl); }

	CClassDeclarationTable::ClassDecl *GetClassByID(TYPE_NAME_ID nID);
	CClassDeclarationTable::ClassDecl *GetClassByName(LPCSTR szName);
	TYPE_NAME_ID GetClassIDByName(LPCSTR szName);

protected:
	int Execute(STMT_TREE_NODE *stmtTree);

	int ExeGetClassMemberValue(STMT_TREE_NODE *stmtVar, int &nValue);
	int ExeSetClassMemberValue(STMT_TREE_NODE *stmtVar, int nValue);
	int ExeGetClassMethod(STMT_TREE_NODE *stmtVar, CFunctionDeclTable::FunctionDecl **funcDecl, HHEAPDATA &hThis, int &nClassID, int &nFuncID);

	int NewClass(int nClassID, HHEAPDATA &hHeap);

	void FormatLastError(int nLine, int nError, LPCSTR szStr, int nLineSrc);

public:
	CConstStrTable				m_strTable;

	CSSMachine					*m_pMachine;

	CFunctionDeclTable			m_funcTable;
	CClassDeclarationTable		m_classDeclTable;

	CSSMStack					m_vLocalVarStack;
	CSSMHeap					m_heap;

	// to protect virtual machine and compiler
	int							m_nCallbackDeeper;

	string						m_strLastErr;

};

//
// CSymbolTable 
//
class CSymbolTable
{
public:
	

	struct SynbolProperty
	{
		TYPE_NAME_ID			symType;
		bool					bReference;
		SSM_STACK_ADDR			memLoc;				// memory location in stack.
		list<int>				listReferLines;
	};

	typedef map<string, SynbolProperty*>	MAP_SYMBOLTAB;

	// block is the name space of variables, for example: { }
	struct StmtBlock
	{
		bool					bFunctionBody;		// is this block a function body?
		CClassDeclarationTable::ClassDecl	*m_pClass;	// is this block a class?
		MAP_SYMBOLTAB			mapSymbols;
	};

	typedef vector<StmtBlock *>		SYMTABL_STACK;

public:
	CSymbolTable();
	~CSymbolTable();

	// travel in block
	void Push(bool bFunctionBody = false);
	void Push(CClassDeclarationTable::ClassDecl *pClass);

	// travel out of block
	void Popup();
	void PopupToFunctionOut();

	void Close();

	// Add new symbol declaration 
	int AddSymbolDeclaration(LPCTSTR szName, int nLineNo, TYPE_NAME_ID symType, bool bReference = false);

	// Add reference of id, and return the TypeInfo.
	TypeInfo AddSymbolReference(LPCTSTR szName, int nLineNo);

	bool IsClassMemberMethod(LPCSTR szName);
	
	SSM_STACK_ADDR GetSymbolMemLoc(LPCTSTR szName);
	SSM_STACK_ADDR GetCurFuncStackSize();

protected:
	void FreeStmtBlock(StmtBlock *pblock);

protected:
	SYMTABL_STACK			m_symTabStack;

};

//
// The compiler used to compile skin script.
//
class CSSCompiler  
{
public:
	CSSCompiler();
	virtual ~CSSCompiler();

	int Init();

	int Compile(LPCTSTR szFile, CSSMProcess &process);

protected:
	TYPE_NAME_ID GetIDOfTypeName(LPCTSTR szTypeName);
	const char *GetTypeNameStrByID(TYPE_NAME_ID id);

	//
	// Build symbol table and check for syntax error
	//
	int BuildSymbolTable();

	int PrevBST_AddClassDefinition();

	int PrevBST_AddFunctionAndVariable(SS_TREE_NODE *pTree, CClassDeclarationTable::ClassDecl *pClassDecl);

	int BST_Node(SS_TREE_NODE *pTree);
	int BST_FunctionDecl(SS_TREE_NODE *pTree);
	int BST_ClassDefinition(SS_TREE_NODE *pTree);
	int BST_VariableDecl(SS_TREE_NODE *pTree, CClassDeclarationTable::ClassDecl *classDecl = NULL);
	int BST_Stmt(SS_TREE_NODE *pTree);
	int BST_Expression(SS_TREE_NODE *pTree);
	// int BST_Variable(SS_TREE_NODE *pTree);

	//
	// Build code
	//
	int BuildCodeStmt();

	int BCS_Tree(SS_TREE_NODE *pTree, PSTMT_TREE_NODE &pstmtOut);
	int BCS_FunctionDefinition(SS_TREE_NODE *pTree, CClassDeclarationTable::ClassDecl *pClassDecl = NULL);
	int BCS_ClassMemberVariable(SS_TREE_NODE *pTree, PSTMT_TREE_NODE &pstmtConstruct, CClassDeclarationTable::ClassDecl *pClassDecl);
	int BCS_ClassDefinition(SS_TREE_NODE *pTree);
	int BCS_VarDeclaration(SS_TREE_NODE *pTree, PSTMT_TREE_NODE &pstmtOut);
	int BCS_Expression(SS_TREE_NODE *pTree, PSTMT_TREE_NODE &pstmtOut);
	int BCS_Stmt(SS_TREE_NODE *pTree, PSTMT_TREE_NODE &pstmtOut);
	int BCS_ClassVariable(SS_TREE_NODE *pID, PSTMT_TREE_NODE &pstmtOut);
	int BCS_ClassMethodCall(SS_TREE_NODE *pID, PSTMT_TREE_NODE &pstmtOut);

	void FormatLastError(const char *szFormat, ...);
	void FormatLastError(int nLine, int nError, LPCSTR szStr, int nLineSrc);

protected:
	CSymbolTable				m_symTable;

	SS_TREE_NODE				*m_syntaxTree;
	TYPE_NAME_ID				m_typeIDCurFunc;

	CSSMProcess					*m_pProcess;
	CClassDeclarationTable::ClassDecl	*m_CurClass;

	string						m_strLastErr;

};

#endif // !defined(AFX_SSCOMPILER_H__3CF1E6F7_A944_4C37_9674_9781D1BFEEAB__INCLUDED_)
