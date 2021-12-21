// SSCompiler.cpp: implementation of the CSSCompiler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSCompiler.h"
#include "SSMachine.h"

#define  SZ_THIS		"this"

#if 0
#define D_PRINT_SYNTAX_TREE	1
// #define D_PRINT_STACK		1
#define D_PRINT_EXE_STATEMENT		1
#endif

#ifdef D_PRINT_STACK
#define PRINT_STACK()		m_vLocalVarStack.Print()
#else
#define PRINT_STACK()		
#endif

void PrintStmtCodeTree(STMT_TREE_NODE *t, int &nIndent);

TYPE_NAME_ID			g_typeIDInt = 1, g_typeIDString = 2, g_typeIDVoid = 3, g_typeIDAny = 4;

bool IsSimpleKindTypeID(TYPE_NAME_ID typeID)
{
	return typeID == g_typeIDInt || typeID == g_typeIDString || typeID == g_typeIDVoid;
}

#define SSM_SYSAPI_FUNC_ID_BASE		1000

#define FORMATLASTERROR(p1, p2, p3)		FormatLastError(p1, p2, p3, __LINE__)

//////////////////////////////////////////////////////////////////////////
int yyparse();

extern SS_TREE_NODE *		_syntaxTree;
extern int yyleng;
extern FILE *yyin, *yyout;

void main(int  argc, char **argv )
{
	CSSCompiler		compiler;

    ++argv, --argc;
    if (argc <= 0)
	{
		printf("Please input the file to be compiled.");
		return;
	}

	compiler.Init();

	CSSMachine		machine;
	CSSMProcess		process(&machine);

	if (compiler.Compile(argv[0], process) != ERR_OK)
		return;

	machine.RunProcess(&process);
}

void FormatLastError(string &strOutput, int nLine, int nError, LPCSTR szStr, int nLineSrc)
{
	string str;
	switch (nError)
	{
	case ERR_C_UNDECLARED_ID:	str = "undeclared identifier"; break;
	case ERR_C_PARSE:	str = "Parse script file"; break;
	case ERR_C_REDEFINITION_ID:	str = "redefinition of identifier"; break;
	case ERR_C_OP_TYPE_NOT_MATCH:	str = "operand type not match"; break;
	case ERR_C_OP_REQUIRE_INT_STR_TYPE:	str = "operand type should be int or string"; break;
	case ERR_C_OP_REQUIRE_INT_TYPE:	str = "operand type should be int"; break;
	case ERR_C_REDEFINITION_TYPE:	str = "redefinition of type"; break;
	case ERR_C_CONVERT_VALUE:	str = "Can't convert value to another"; break;
	case ERR_C_UNEXPECTED_ERR:	str = "unexpected error occured"; break;
	case ERR_R_UNDECLARED_FUNCTION:	str = "undeclared function"; break;
	case ERR_R_FUNC_PARAM_COUNT:	str = "function parameter count not match"; break;
	case ERR_R_FUNC_PARAM_TYPE:	str = "function parameter type not match"; break;
	default:
		{
			// str = Error2Str(nError);
			str = "Unknown error code";
		}
		break;
	}

	char		szBuffer[512] = "";
	sprintf(szBuffer, "%4d: Line: %d, %s: %s", nLineSrc, nLine, str.c_str(), szStr);

	strOutput = szBuffer;
}

const char *GetStmtNodeTypeName(STMT_TREE_NODE::STMT_NODE_TYPE type)
{
	switch (type)
	{
		case STMT_TREE_NODE::SNT_UNKNOWN: return "SNT_UNKNOWN";
		case STMT_TREE_NODE::SNT_BLOCK: return "SNT_BLOCK";
		case STMT_TREE_NODE::SNT_IF: return "SNT_IF";
		case STMT_TREE_NODE::SNT_WHILE: return "SNT_WHILE";
		case STMT_TREE_NODE::SNT_DO_WHILE: return "SNT_DO_WHILE";
		case STMT_TREE_NODE::SNT_ASSIGN: return "SNT_ASSIGN";
		case STMT_TREE_NODE::SNT_RETURN: return "SNT_RETURN";
		case STMT_TREE_NODE::SNT_ASSIGN_REF: return "SNT_ASSIGN_REF";		// assign to an reference value.
		case STMT_TREE_NODE::SNT_LT_EQ: return "SNT_LT_EQ";		// <=
		case STMT_TREE_NODE::SNT_GT_EQ: return "SNT_GT_EQ";		// >=
		case STMT_TREE_NODE::SNT_LITTLE: return "SNT_LITTLE";		// <
		case STMT_TREE_NODE::SNT_GREATER: return "SNT_GREATER";	// >
		case STMT_TREE_NODE::SNT_EQUAL: return "SNT_EQUAL";		// ==
		case STMT_TREE_NODE::SNT_NOT_EQUAL: return "SNT_NOT_EQUAL";	// !=
		case STMT_TREE_NODE::SNT_OP_INC: return "SNT_OP_INC";		// ++
		case STMT_TREE_NODE::SNT_OP_DEC: return "SNT_OP_DEC";		// --
		case STMT_TREE_NODE::SNT_PLUS: return "SNT_PLUS";		// +
		case STMT_TREE_NODE::SNT_MINUS: return "SNT_MINUS";		// -
		case STMT_TREE_NODE::SNT_MULT: return "SNT_MULT";		// *
		case STMT_TREE_NODE::SNT_DIV: return "SNT_DIV";		// /
		case STMT_TREE_NODE::SNT_BOOL_AND: return "SNT_BOOL_AND";		// /
		case STMT_TREE_NODE::SNT_BOOL_OR: return "SNT_BOOL_OR";		// /

		case STMT_TREE_NODE::SNT_NEW_STACK_VAR: return "SNT_NEW_STACK_VAR";		// new a variable, and push to stack.
		case STMT_TREE_NODE::SNT_POPUP_STACK_N: return "SNT_POPUP_STACK_N";

		case STMT_TREE_NODE::SNT_FUNC_CALL: return "SNT_FUNC_CALL";
		case STMT_TREE_NODE::SNT_VAR: return "SNT_VAR";
		case STMT_TREE_NODE::SNT_VAR_REF: return "SNT_VAR_REF";
		case STMT_TREE_NODE::SNT_CONST_INT: return "SNT_CONST_INT";
		case STMT_TREE_NODE::SNT_CONST_STR: return "SNT_CONST_STR";
		case STMT_TREE_NODE::SNT_NEW: return "SNT_NEW";
		case STMT_TREE_NODE::SNT_DELETE: return "SNT_DELETE";
	}

	return "Unknown node type";
}

//////////////////////////////////////////////////////////////////////////

void STMT_TREE_NODE::AddTailSibing(STMT_TREE_NODE *pTail)
{
	PSTMT_TREE_NODE		*pNext = &sibling;
	while (*pNext)
	{
		pNext = &((*pNext)->sibling);
	}

	*pNext = pTail;
}

PSTMT_TREE_NODE newStmtAllocStackVar(TYPE_NAME_ID typeID)
{
	PSTMT_TREE_NODE	p = new STMT_TREE_NODE;
	p->nodeType = STMT_TREE_NODE::SNT_NEW_STACK_VAR;
	p->addr = typeID;
	return p;
}

PSTMT_TREE_NODE newStmt(STMT_TREE_NODE::STMT_NODE_TYPE nodeType)
{
	PSTMT_TREE_NODE	p = new STMT_TREE_NODE;
	p->nodeType = nodeType;
	return p;
}

void deleteStmt(STMT_TREE_NODE *pNode)
{
	while (pNode)
	{
		for (int i = 0; i < MAX_CHILDREN; i++)
		{
			if (pNode->child[i])
				deleteStmt(pNode->child[i]);
		}

		STMT_TREE_NODE	*p = pNode->sibling;

		delete pNode;

		pNode = p;
	}
}

//////////////////////////////////////////////////////////////////////////

CFunctionDeclTable::CFunctionDeclTable()
{
	FunctionDecl	*pfuncGlobal = new FunctionDecl;
	pfuncGlobal->typeIDRet = 0;
	pfuncGlobal->typeIDRet = g_typeIDVoid;
	m_vFunctions.push_back(pfuncGlobal);
}

CFunctionDeclTable::~CFunctionDeclTable()
{
	for (int i = 0; i < m_vFunctions.size(); i++)
	{
		delete m_vFunctions[i];
	}
	m_vFunctions.clear();
}

int CFunctionDeclTable::AddFunc(FunctionDecl *funcDecl)
{
	if (IsFuncDeclared(funcDecl->strName.c_str()))
	{
		delete funcDecl;
		return INVALID_FUNC_ID;
	}

	m_vFunctions.push_back(funcDecl);

	return m_vFunctions.size() - 1;
}

CFunctionDeclTable::FunctionDecl *CFunctionDeclTable::GetFuncByID(int nID)
{
	if (nID >= 0 && nID < m_vFunctions.size())
		return m_vFunctions[nID];
	else
		return NULL;
}

CFunctionDeclTable::FunctionDecl *CFunctionDeclTable::GetFuncByName(LPCSTR szName)
{
	for (int i = 0; i < m_vFunctions.size(); i++)
	{
		FunctionDecl	*func = m_vFunctions[i];
		if (strcmp(func->strName.c_str(), szName) == 0)
		{
			return m_vFunctions[i];
		}
	}
	return NULL;
}

int CFunctionDeclTable::GetFuncIDByName(LPCSTR szFuncName)
{
	for (int i = 0; i < m_vFunctions.size(); i++)
	{
		FunctionDecl	*func = m_vFunctions[i];
		if (strcmp(func->strName.c_str(), szFuncName) == 0)
		{
			return i;
		}
	}
	return -1;
}

bool CFunctionDeclTable::IsFuncDeclared(LPCSTR szFuncName)
{
	V_FUNCTION_DECL::iterator	it, itEnd;

	itEnd = m_vFunctions.end();
	for (it = m_vFunctions.begin(); it != itEnd; ++it)
	{
		FunctionDecl	*func = *it;
		if (strcmp(func->strName.c_str(), szFuncName) == 0)
		{
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
//
CSymbolTable::CSymbolTable()
{
	StmtBlock	*pblock = new StmtBlock;

	// Add global symbol table
	pblock->bFunctionBody = false;
	pblock->m_pClass = NULL;
	m_symTabStack.push_back(pblock);
}

CSymbolTable::~CSymbolTable()
{
	for (int i = 0; i < m_symTabStack.size(); i++)
	{
		StmtBlock	*pblock = m_symTabStack[i];
		FreeStmtBlock(pblock);
	}
}

void CSymbolTable::Push(bool bFunctionBody)
{
	StmtBlock	*pblock = new StmtBlock;
	pblock->bFunctionBody = bFunctionBody;
	pblock->m_pClass = NULL;

	m_symTabStack.push_back(pblock);
}

void CSymbolTable::Push(CClassDeclarationTable::ClassDecl *pClass)
{
	StmtBlock	*pblock = new StmtBlock;
	pblock->bFunctionBody = false;
	pblock->m_pClass = pClass;

	m_symTabStack.push_back(pblock);
}

// enter a block
void CSymbolTable::Popup()
{
	ASSERT(!m_symTabStack.empty());

	StmtBlock	*pblock = m_symTabStack.back();
	m_symTabStack.pop_back();
	FreeStmtBlock(pblock);
}

void CSymbolTable::PopupToFunctionOut()
{
	while (!m_symTabStack.empty())
	{
		StmtBlock	*pblock = m_symTabStack.back();
		if (pblock->bFunctionBody)
			return;
		else
		{
			m_symTabStack.pop_back();
			FreeStmtBlock(pblock);
		}
	}
	ASSERT(0 && "Can't execute to here!");
}

void CSymbolTable::Close()
{
	for (int i = 0; i < m_symTabStack.size(); i++)
		FreeStmtBlock(m_symTabStack[i]);
	m_symTabStack.clear();

	StmtBlock	*pblock = new StmtBlock;

	// Add global symbol table
	pblock->bFunctionBody = false;
	pblock->m_pClass = NULL;
	m_symTabStack.push_back(pblock);
}

int CSymbolTable::AddSymbolDeclaration(LPCTSTR szName, int nLineNo, TYPE_NAME_ID symType, bool bReference)
{
	if (m_symTabStack.empty())
		return ERR_NOT_FOUND;

	StmtBlock	*pblock = m_symTabStack.back();

	MAP_SYMBOLTAB::iterator it = pblock->mapSymbols.find(szName);
	if (it == pblock->mapSymbols.end())
	{
		if (pblock->m_pClass)
		{
			int		n = pblock->m_pClass->GetMemberVarIndex(szName);
			if (n != -1)
				return ERR_EXIST;
		}

		SynbolProperty	*psymProperty = new SynbolProperty;
		psymProperty->listReferLines.push_back(nLineNo);
		psymProperty->symType = symType;
		psymProperty->bReference = bReference;
		psymProperty->memLoc = pblock->mapSymbols.size();
		pblock->mapSymbols[szName] = psymProperty;
		return ERR_OK;
	}

	return ERR_EXIST;
}

TypeInfo CSymbolTable::AddSymbolReference(LPCTSTR szName, int nLineNo)
{
	TypeInfo	typeInfo;

	typeInfo.bReference = false;
	typeInfo.typeID = INVALID_TYPE_ID;
	typeInfo.nClsMemberVar = -1;

	if (m_symTabStack.empty())
		return typeInfo;

	for (int i = (int)m_symTabStack.size() - 1; i >= 0; i--)
	{
		StmtBlock	*pblock = m_symTabStack[i];

		MAP_SYMBOLTAB::iterator it = pblock->mapSymbols.find(szName);
		if (it == pblock->mapSymbols.end())
		{
			if (pblock->m_pClass)
			{
				// Is it a class member?
				int		n = pblock->m_pClass->GetMemberVarIndex(szName);
				if (n != -1)
				{
					typeInfo.typeID = pblock->m_pClass->vMemberVar[n].typeID;
					typeInfo.bReference = false;
					ASSERT(n < 128);
					typeInfo.nClsMemberVar = (char)n;
					return typeInfo;
				}
			}
			continue;
		}

		SynbolProperty	*psymProperty = (*it).second;

		typeInfo.bReference = psymProperty->bReference;
		typeInfo.typeID = psymProperty->symType;
		return typeInfo;
	}

	return typeInfo;
}

bool CSymbolTable::IsClassMemberMethod(LPCSTR szName)
{
	for (int i = (int)m_symTabStack.size() - 1; i >= 0; i--)
	{
		StmtBlock	*pblock = m_symTabStack[i];

		if (pblock->m_pClass)
		{
			// Is it a class member?
			return pblock->m_pClass->m_methodTable.IsFuncDeclared(szName);
		}
	}
	return false;
}

SSM_STACK_ADDR CSymbolTable::GetSymbolMemLoc(LPCTSTR szName)
{
	if (m_symTabStack.empty())
	{
		ASSERT(0 && "Empty symbol table stack.");
		return 0;
	}

	int		nSize = 0;
	SSM_STACK_ADDR addr = 0;

	for (int i = (int)m_symTabStack.size() - 1; i >= 0; i--)
	{
		StmtBlock	*pblock = m_symTabStack[i];

		nSize += pblock->mapSymbols.size();

		MAP_SYMBOLTAB::iterator it = pblock->mapSymbols.find(szName);
		if (it == pblock->mapSymbols.end())
		{
			if (pblock->m_pClass)
			{
				// Is it a class member?
				int		n = pblock->m_pClass->GetMemberVarIndex(szName);
				if (n != -1)
				{
					// typeInfo.typeID = pblock->m_pClass->vMemberVar[n].typeID;
					return n;
				}
			}
			continue;
		}

		SynbolProperty	*psymProperty = (*it).second;
		if (i == 0)
		{
			// global variable
			addr = psymProperty->memLoc;
			return SSM_SET_GLOBAL_STACK_ADDR_FLAG(addr);
		}
		else
			return nSize - psymProperty->memLoc;
	}

	ASSERT(0 && "Can't get symbol location in stack");

	return 0;
}

SSM_STACK_ADDR CSymbolTable::GetCurFuncStackSize()
{
	SSM_STACK_ADDR		addr = 0;

	for (int i = m_symTabStack.size() - 1; i >= 0; i--)
	{
		StmtBlock	*pblock = m_symTabStack[i];
		addr += pblock->mapSymbols.size();
		if (pblock->bFunctionBody)
		{
			break;
		}
	}

	return addr;
}

void CSymbolTable::FreeStmtBlock(StmtBlock *pblock)
{
	MAP_SYMBOLTAB::iterator	it, itEnd;

	itEnd = pblock->mapSymbols.end();
	for (it = pblock->mapSymbols.begin(); it != itEnd; ++it)
		delete (*it).second;
	pblock->mapSymbols.clear();

	delete pblock;
}

//////////////////////////////////////////////////////////////////////////

//
// class declaration table.
//
CClassDeclarationTable::CClassDeclarationTable()
{
	// tacke the position of invalid typename id
	m_vClasses.push_back(new ClassDecl);

	AddSimpleType(SS_TYPE_INT);
	AddSimpleType(SS_TYPE_STRING);
	AddSimpleType(SS_TYPE_VOID);
	AddSimpleType(SS_TYPE_ANY);

	g_typeIDInt = GetClassIDByName(SS_TYPE_INT);
	g_typeIDString = GetClassIDByName(SS_TYPE_STRING);
	g_typeIDVoid = GetClassIDByName(SS_TYPE_VOID);
	g_typeIDAny = GetClassIDByName(SS_TYPE_ANY);
}

CClassDeclarationTable::~CClassDeclarationTable()
{

}

CClassDeclarationTable::ClassDecl::ClassDecl(LPCSTR szName)
{
	strName = szName;

	CFunctionDeclTable::FunctionDecl	*func;

	// add class construct function
	func = new CFunctionDeclTable::FunctionDecl;
	func->strName = "@construct";
	func->typeIDRet = g_typeIDVoid;
	m_methodTable.AddFunc(func);

	// add class destruct function
	func = new CFunctionDeclTable::FunctionDecl;
	func->strName = "@destruct";
	func->typeIDRet = g_typeIDVoid;
	m_methodTable.AddFunc(func);
}

int CClassDeclarationTable::ClassDecl::GetMemberVarIndex(const char *szName)
{
	for (int i = 0; i < vMemberVar.size(); i++)
	{
		if (strcmp(vMemberVar[i].strName.c_str(), szName) == 0)
			return i;
	}

	return -1;
}

int CClassDeclarationTable::ClassDecl::AddMemberVar(TYPE_NAME_ID typeID, const char *szVarName, int nLineNo)
{
	for (int i = 0; i < vMemberVar.size(); i++)
	{
		if (strcmp(vMemberVar[i].strName.c_str(), szVarName) == 0)
			return ERR_C_REDEFINITION_ID;
	}

	if (m_methodTable.IsFuncDeclared(szVarName))
		return ERR_C_REDEFINITION_ID;

	ClassMemberVar	var;
	var.typeID = typeID;
	var.strName = szVarName;
	var.nLineNo = nLineNo;

	vMemberVar.push_back(var);

	return ERR_OK;
}

int CClassDeclarationTable::ClassDecl::AddMemberMethod(CFunctionDeclTable::FunctionDecl *funcDecl)
{
	for (int i = 0; i < vMemberVar.size(); i++)
	{
		if (strcmp(vMemberVar[i].strName.c_str(), funcDecl->strName.c_str()) == 0)
		{
			delete funcDecl;
			return ERR_C_REDEFINITION_ID;
		}
	}

	int		nID = m_methodTable.AddFunc(funcDecl);
	if (nID == INVALID_FUNC_ID)
		return ERR_C_REDEFINITION_ID;
	else
		return ERR_OK;
}

int CClassDeclarationTable::AddClass(ClassDecl *classDecl)
{
	if (IsClassDeclared(classDecl->strName.c_str()))
	{
		delete classDecl;
		return INVALID_TYPE_ID;
	}

	m_vClasses.push_back(classDecl);

	return m_vClasses.size() - 1;
}

int CClassDeclarationTable::AddSimpleType(const char *szTypeName)
{
	if (IsClassDeclared(szTypeName))
		return ERR_C_REDEFINITION_ID;

	ClassDecl		*clsDecl;

	clsDecl = new ClassDecl;
	clsDecl->strName = szTypeName;
	m_vClasses.push_back(clsDecl);

	return ERR_OK;
}

CClassDeclarationTable::ClassDecl *CClassDeclarationTable::GetClassByID(TYPE_NAME_ID nID)
{
	if (nID <= 0 || nID >= m_vClasses.size())
		return NULL;

	return m_vClasses[nID];
}

CClassDeclarationTable::ClassDecl *CClassDeclarationTable::GetClassByName(LPCSTR szName)
{
	for (int i = 1; i < m_vClasses.size(); i++)
	{
		if (strcmp(szName, m_vClasses[i]->strName.c_str()) == 0)
			return m_vClasses[i];
	}

	return NULL;
}

TYPE_NAME_ID CClassDeclarationTable::GetClassIDByName(LPCSTR szName)
{
	for (int i = 1; i < m_vClasses.size(); i++)
	{
		if (strcmp(szName, m_vClasses[i]->strName.c_str()) == 0)
			return i;
	}

	return INVALID_TYPE_ID;
}

bool CClassDeclarationTable::IsClassDeclared(LPCSTR szName)
{
	for (int i = 1; i < m_vClasses.size(); i++)
	{
		if (strcmp(szName, m_vClasses[i]->strName.c_str()) == 0)
			return true;
	}

	return false;
}

CFunctionDeclTable::FunctionDecl *CClassDeclarationTable::GetClassMethod(TYPE_NAME_ID nClassID, int nMethodID)
{
	if (nClassID <= 0 || nClassID >= m_vClasses.size())
		return NULL;

	return m_vClasses[nClassID]->m_methodTable.GetFuncByID(nMethodID);

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
CSSMHeap::CSSMHeap()
{

}

CSSMHeap::~CSSMHeap()
{

}

HHEAPDATA CSSMHeap::New(size_t size)
{
	ASSERT(size >= 0 && size < 0x7FFF);
	HHEAPDATA		p = (HHEAPDATA)new char[(size + 1) * sizeof(int)];

	p->nData[0] = HEAP_FLAG | size;

	return p;
}

void CSSMHeap::Delete(HHEAPDATA hHeap)
{
	ASSERT(HeapIsValid(hHeap));
	delete[] (char*)hHeap;
}


//////////////////////////////////////////////////////////////////////////
//

CSSMProcess::CSSMProcess(CSSMachine *machine)
{
	m_pMachine = machine;
	m_nCallbackDeeper = 0;
}

CSSMProcess::~CSSMProcess()
{
}

/*
CSSMVariable *CSSMProcess::NewVar(TYPE_NAME_ID typeID)
{
	if (typeID == g_typeIDInt)
	{
		CSSMVariableInt	*pVar = new CSSMVariableInt;
		pVar->nVarType = typeID;
		return pVar;
	}
	else if (typeID == g_typeIDString)
	{
		CSSMVariableString	*pVar = new CSSMVariableString;
		pVar->nVarType = typeID;
		return pVar;
	}
	else
	{
		if (m_nCallbackDeeper >= 10)
		{
			// struct is deeper than 10 levels.
			m_nCallbackDeeper = 0;
			return NULL;
		}
		CTypeNameTable::TypeName	*typeName;
		typeName = GetTypeNameByID(typeID);
		ASSERT(typeName);
		if (typeName)
		{
			m_nCallbackDeeper++;
			CSSMVariableClass	*pVar = new CSSMVariableClass;
			for (int i = 0; i < typeName->vMemberTypeNameID.size(); ++i)
			{
				pVar->vMemberVars.push_back(NewVar(typeName->vMemberTypeNameID[i]));
			}
			m_nCallbackDeeper--;
			pVar->nVarType = typeID;
			return pVar;
		}
		else
			return NULL;
	}
}*/

int CSSMProcess::CallFun(int nClassID, int nFuncID, CSSMStack *params)
{
	int				nRet;

	if (nClassID < m_classDeclTable.GetClassCount())
	{
		CClassDeclarationTable::ClassDecl	*clsDecl = m_classDeclTable.GetClassByID(nClassID);
		CFunctionDeclTable::FunctionDecl	*func;

		if (!clsDecl)
			return ERR_NOT_FOUND;

		ASSERT(clsDecl);
		func = clsDecl->m_methodTable.GetFuncByID(nFuncID);
		ASSERT(func);

		if (!func)
			return ERR_NOT_FOUND;

		// alloc space for return value
		if (func->typeIDRet != g_typeIDVoid)
		{
			m_vLocalVarStack.Push(0);
			PRINT_STACK();
		}

		//
		// Call local functions
		//
		if (params)
		{
			m_vLocalVarStack.Push(*params);
			PRINT_STACK();
		}

		nRet = Execute(func->m_pstmtTree);

		return nRet;
	}
	else
	{
		int				nValue;
		nRet = m_pMachine->SysCall(nClassID - SSM_SYSAPI_FUNC_ID_BASE, nFuncID, *params, nValue);
		if (nRet == ERR_OK)
		{
			// push return value to stack
			m_vLocalVarStack.Push(nValue);

			PRINT_STACK();
		}
		return nRet;
	}
}

int CSSMProcess::CallFun(int nFuncID, CSSMStack *params)
{
	int				nRet;

	if (nFuncID < 0)
		return ERR_NOT_FOUND;

	else if (nFuncID < m_funcTable.GetFuncCount())
	{
		CFunctionDeclTable::FunctionDecl	*func = m_funcTable.m_vFunctions[nFuncID];

		// alloc space for return value
		if (func->typeIDRet != g_typeIDVoid)
		{
			m_vLocalVarStack.Push(0);
			PRINT_STACK();
		}

		//
		// Call local functions
		//
		if (params)
		{
			m_vLocalVarStack.Push(*params);
			PRINT_STACK();
		}

		nRet = Execute(m_funcTable.m_vFunctions[nFuncID]->m_pstmtTree);

		return nRet;
	}
	else
	{
		int				nValue;
		nRet = m_pMachine->SysCall(nFuncID - SSM_SYSAPI_FUNC_ID_BASE, *params, nValue);
		if (nRet == ERR_OK)
		{
			// push return value to stack
			m_vLocalVarStack.Push(nValue);

			PRINT_STACK();
		}
		return nRet;
	}
}

void CSSMProcess::PrintCodeTree()
{
	int			i, nIndent = 0;

	for (i = 0; i < m_funcTable.m_vFunctions.size(); i++)
	{
		nIndent = 0;
		printf("\n\nGlobal Function: %d, %s\n", i, m_funcTable.m_vFunctions[i]->strName.c_str());
		::PrintStmtCodeTree(m_funcTable.m_vFunctions[i]->m_pstmtTree, nIndent);
	}
	printf("\n\n");


	for (int k = 3; k < m_classDeclTable.GetClassCount(); k++)
	{
		CClassDeclarationTable::ClassDecl	*clsDecl = m_classDeclTable.GetClassByID(k);

		printf("Class: %s\n\n", clsDecl->strName.c_str());
		for (int i = 0; i < clsDecl->m_methodTable.m_vFunctions.size(); i++)
		{
			nIndent = 0;
			printf("\n\nFunction: %d, %s\n", i, clsDecl->m_methodTable.m_vFunctions[i]->strName.c_str());
			::PrintStmtCodeTree(clsDecl->m_methodTable.m_vFunctions[i]->m_pstmtTree, nIndent);
		}
		printf("\n\n");

	}
}

CFunctionDeclTable::FunctionDecl *CSSMProcess::GetFuncByID(int nID)
{
	if (nID < 0)
		return NULL;
	else if (nID < m_funcTable.GetFuncCount())
		return m_funcTable.GetFuncByID(nID);
	else
		return m_pMachine->GetFuncByID(nID - SSM_SYSAPI_FUNC_ID_BASE);
}

CFunctionDeclTable::FunctionDecl *CSSMProcess::GetFuncByName(LPCSTR szName)
{
	CFunctionDeclTable::FunctionDecl	*func;

	func = m_funcTable.GetFuncByName(szName);
	if (func)
		return func;

	return m_pMachine->GetFuncByName(szName);
}

int CSSMProcess::GetFuncIDByName(LPCSTR szFuncName)
{
	int		nID;

	nID = m_funcTable.GetFuncIDByName(szFuncName);
	if (nID != -1)
		return nID;

	nID = m_pMachine->GetFuncIDByName(szFuncName);
	if (nID == -1)
		return -1;
	else
		return nID + SSM_SYSAPI_FUNC_ID_BASE;
}

CClassDeclarationTable::ClassDecl *CSSMProcess::GetClassByID(TYPE_NAME_ID nID)
{
	if (nID <= INVALID_TYPE_ID)
		return NULL;
	else if (nID < m_classDeclTable.GetClassCount())
		return m_classDeclTable.GetClassByID(nID);
	else
		return m_pMachine->GetClassByID(nID - SSM_SYSAPI_FUNC_ID_BASE);
}

CClassDeclarationTable::ClassDecl *CSSMProcess::GetClassByName(LPCSTR szName)
{
	CClassDeclarationTable::ClassDecl	*pcls;

	pcls = m_classDeclTable.GetClassByName(szName);
	if (pcls)
		return pcls;

	return m_pMachine->GetClassByName(szName);
}

TYPE_NAME_ID CSSMProcess::GetClassIDByName(LPCSTR szName)
{
	TYPE_NAME_ID	nID;
	nID = m_classDeclTable.GetClassIDByName(szName);
	if (nID != INVALID_TYPE_ID)
		return nID;
	nID = m_pMachine->GetClassIDByName(szName);
	if (nID != INVALID_TYPE_ID)
		return nID + SSM_SYSAPI_FUNC_ID_BASE;

	return INVALID_TYPE_ID;
}

int CSSMProcess::Execute(STMT_TREE_NODE *stmtTree)
{
	int				nRet;
	STMT_TREE_NODE	*pTree = stmtTree;

	while (pTree)
	{
#ifdef D_PRINT_EXE_STATEMENT
		printf("line: %2d  %s	Result: %X, addr: %X\n", pTree->nLineNo, GetStmtNodeTypeName(pTree->nodeType), pTree->nResult, pTree->addr);
#endif
		switch (pTree->nodeType)
		{
		case STMT_TREE_NODE::SNT_BLOCK:
			{
				for (int i = 0; i < MAX_CHILDREN; i++)
				{
					if (pTree->child[i])
					{
						nRet = Execute(pTree->child[i]);
						ASSERT(nRet == ERR_OK);
					}
				}
			}
			break;
		case STMT_TREE_NODE::SNT_IF:
			{
				// execute condition
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				if (pTree->child[0]->nResult != 0)
				{
					// execute true statement
					nRet = Execute(pTree->child[1]);
					ASSERT(nRet == ERR_OK);
				}
				else
				{
					// execute else statement
					if (pTree->child[2])
					{
						nRet = Execute(pTree->child[2]);
						ASSERT(nRet == ERR_OK);
					}
				}
			}
			break;
		case STMT_TREE_NODE::SNT_WHILE:
			{
				int			nRunCycle = 0;
				while (1)
				{
					// condition
					nRet = Execute(pTree->child[0]);
					ASSERT(nRet == ERR_OK);
					if (pTree->child[0]->nResult == 0)
						break;

					// stmt
					nRet = Execute(pTree->child[1]);
					ASSERT(nRet == ERR_OK);

					nRunCycle++;
					if (nRunCycle > 10000)
						break;
				}
			}
			break;
		case STMT_TREE_NODE::SNT_DO_WHILE:
			{
				int			nRunCycle = 0;
				while (1)
				{
					// stmt
					nRet = Execute(pTree->child[1]);
					ASSERT(nRet == ERR_OK);

					// condition
					nRet = Execute(pTree->child[0]);
					ASSERT(nRet == ERR_OK);
					if (pTree->child[0]->nResult == 0)
						break;

					nRunCycle++;
					if (nRunCycle > 10000)
						break;
				}
			}
			break;
		case STMT_TREE_NODE::SNT_ASSIGN:
			{
				// expression value
				nRet = Execute(pTree->child[1]);
				ASSERT(nRet == ERR_OK);

				if (pTree->child[2])
				{
					// alloc variabe in stack?
					nRet = Execute(pTree->child[2]);
					ASSERT(nRet == ERR_OK);
				}

				// assign value
				if (pTree->child[0])
				{
					// set the value of class member
					HHEAPDATA			hHeapData;
					STMT_TREE_NODE		*pID = pTree->child[0];

					hHeapData = (HHEAPDATA)m_vLocalVarStack.GetValueByStackAddr(pID->addr);
					while (pID->child[0]->child[0])
					{
						hHeapData = (HHEAPDATA)HeapGetValue(hHeapData, pID->child[0]->addr);
						pID = pID->child[0];
					}

					// set class member variable value. all class are allocated in heap.
					HeapSetValue(hHeapData, pID->child[0]->addr, pTree->child[1]->nResult);
				}
				else
				{
					// set variable value in stack
					m_vLocalVarStack.SetValueByStackAddr(pTree->addr, pTree->child[1]->nResult);
				}

				PRINT_STACK();
			}
			break;
		case STMT_TREE_NODE::SNT_ASSIGN_REF:
			{
				// variable
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				// if (nRet != ERR_OK)

				// expression value
				nRet = Execute(pTree->child[1]);
				ASSERT(nRet == ERR_OK);

				if (pTree->child[2])
				{
					// alloc variabe in stack?
					nRet = Execute(pTree->child[2]);
					ASSERT(nRet == ERR_OK);
				}

				// assign reference value.
				m_vLocalVarStack.SetValueByStackAddr(
					m_vLocalVarStack.GetValueByStackAddr(pTree->addr),
					pTree->child[1]->nResult);

				PRINT_STACK();
			}
			break;
		case STMT_TREE_NODE::SNT_RETURN:
			{
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
			}
			break;
		case STMT_TREE_NODE::SNT_LT_EQ:
			{
				// <=
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				nRet = Execute(pTree->child[1]);
				ASSERT(nRet == ERR_OK);

				pTree->nResult = (pTree->child[0]->nResult <= pTree->child[1]->nResult);
			}
			break;
		case STMT_TREE_NODE::SNT_GT_EQ:
			{
				// >=
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				nRet = Execute(pTree->child[1]);
				ASSERT(nRet == ERR_OK);

				pTree->nResult = (pTree->child[0]->nResult >= pTree->child[1]->nResult);
			}
			break;
		case STMT_TREE_NODE::SNT_LITTLE:
			{
				// <
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				nRet = Execute(pTree->child[1]);
				ASSERT(nRet == ERR_OK);

				pTree->nResult = (pTree->child[0]->nResult < pTree->child[1]->nResult);
			}
			break;
		case STMT_TREE_NODE::SNT_GREATER:
			{
				// >
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				nRet = Execute(pTree->child[1]);
				ASSERT(nRet == ERR_OK);

				pTree->nResult = (pTree->child[0]->nResult > pTree->child[1]->nResult);
			}
			break;
		case STMT_TREE_NODE::SNT_EQUAL:
			{
				// ==
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				nRet = Execute(pTree->child[1]);
				ASSERT(nRet == ERR_OK);

				pTree->nResult = (pTree->child[0]->nResult == pTree->child[1]->nResult);
			}
			break;
		case STMT_TREE_NODE::SNT_NOT_EQUAL:
			{
				// !=
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				nRet = Execute(pTree->child[1]);
				ASSERT(nRet == ERR_OK);

				pTree->nResult = (pTree->child[0]->nResult != pTree->child[1]->nResult);
			}
			break;
		case STMT_TREE_NODE::SNT_OP_INC:
			{
				// ++
				PSTMT_TREE_NODE		pID;

				pID = pTree->child[0];

				nRet = Execute(pID);
				ASSERT(nRet == ERR_OK);

				pTree->nResult = pID->nResult + 1;

				if (pID->child[0])
					ExeSetClassMemberValue(pID, pTree->nResult);
				else
				{
					// assign reference value.
					if (pID->nodeType == STMT_TREE_NODE::SNT_VAR_REF)
					{
						m_vLocalVarStack.SetValueByStackAddr(
							m_vLocalVarStack.GetValueByStackAddr(pTree->addr),
							pTree->nResult);
					}
					else
						m_vLocalVarStack.SetValueByStackAddr(pID->addr, pTree->nResult);
				}

				PRINT_STACK();
			}
			break;
		case STMT_TREE_NODE::SNT_OP_DEC:
			{
				// --
				PSTMT_TREE_NODE		pID;

				pID = pTree->child[0];

				nRet = Execute(pID);
				ASSERT(nRet == ERR_OK);

				pTree->nResult = pID->nResult - 1;

				if (pID->child[0])
					ExeSetClassMemberValue(pID, pTree->nResult);
				else
				{
					// assign reference value.
					if (pID->nodeType == STMT_TREE_NODE::SNT_VAR_REF)
					{
						m_vLocalVarStack.SetValueByStackAddr(
							m_vLocalVarStack.GetValueByStackAddr(pTree->addr),
							pTree->nResult);
					}
					else
						m_vLocalVarStack.SetValueByStackAddr(pID->addr, pTree->nResult);
				}

				PRINT_STACK();
			}
			break;
		case STMT_TREE_NODE::SNT_PLUS:
			{
				// +
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				nRet = Execute(pTree->child[1]);
				ASSERT(nRet == ERR_OK);

				pTree->nResult = pTree->child[0]->nResult + pTree->child[1]->nResult;
			}
			break;
		case STMT_TREE_NODE::SNT_MINUS:
			{
				// -
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				nRet = Execute(pTree->child[1]);
				ASSERT(nRet == ERR_OK);

				pTree->nResult = pTree->child[0]->nResult - pTree->child[1]->nResult;
			}
			break;
		case STMT_TREE_NODE::SNT_MULT:
			{
				// *
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				nRet = Execute(pTree->child[1]);
				ASSERT(nRet == ERR_OK);

				pTree->nResult = pTree->child[0]->nResult * pTree->child[1]->nResult;
			}
			break;
		case STMT_TREE_NODE::SNT_DIV:
			{
				// /
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				nRet = Execute(pTree->child[1]);
				ASSERT(nRet == ERR_OK);

				if (pTree->child[1]->nResult != 0)
					pTree->nResult = pTree->child[0]->nResult + pTree->child[1]->nResult;
				else
					pTree->nResult = pTree->child[0]->nResult;
			}
			break;
		case STMT_TREE_NODE::SNT_BOOL_AND:
			{
				// /
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				nRet = Execute(pTree->child[1]);
				ASSERT(nRet == ERR_OK);

				pTree->nResult = pTree->child[0]->nResult != 0 && pTree->child[1]->nResult != 0;
			}
			break;
		case STMT_TREE_NODE::SNT_BOOL_OR:
			{
				// /
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				nRet = Execute(pTree->child[1]);
				ASSERT(nRet == ERR_OK);

				pTree->nResult = pTree->child[0]->nResult != 0 || pTree->child[1]->nResult != 0;
			}
			break;
		case STMT_TREE_NODE::SNT_NEW_STACK_VAR:
			{
				// alloc space on stack
				m_vLocalVarStack.Push(0);
				PRINT_STACK();
			}
			break;
		case STMT_TREE_NODE::SNT_POPUP_STACK_N:
			{
				// popup stack
				nRet = Execute(pTree->child[0]);
				ASSERT(nRet == ERR_OK);
				if (nRet != ERR_OK)
					return nRet;
				m_vLocalVarStack.PopupN(pTree->addr);
				PRINT_STACK();
			}
			break;
		case STMT_TREE_NODE::SNT_FUNC_CALL:
			{
				//
				// execute function call.
				//
				CSSMStack				params;
				STMT_TREE_NODE			*pNode;
				CFunctionDeclTable::FunctionDecl	*funcDecl = NULL;
				int						nClassID = -1, nFuncID = -1;

				if (pTree->child[0])
				{
					// class member call
					HHEAPDATA	hHeapThis = NULL;
					nRet = ExeGetClassMethod(pTree->child[0], &funcDecl, hHeapThis, nClassID, nFuncID);
					ASSERT(nRet == ERR_OK);
					if (nRet != ERR_OK)
						return nRet;

					// Push this in the stack...
					params.Push((int)hHeapThis);
				}
				else
				{
					// global function
					nFuncID = pTree->addr;
					funcDecl = GetFuncByID(nFuncID);
					if (!funcDecl)
						return ERR_R_UNDECLARED_FUNCTION;
				}

				// 2. check function parameter
				int			i = 0;
				pNode = pTree->child[1];
				while (pNode)
				{
					// is paramer count greater than declared?
					if (i >= funcDecl->vParamType.size())
						return ERR_R_FUNC_PARAM_COUNT;

					// execute parameter condition
					nRet = Execute(pNode);
					ASSERT(nRet == ERR_OK);
					if (nRet != ERR_OK)
						return nRet;

					// reference parameter can only pass from an ID variable.
					ASSERT(!(funcDecl->vParamType[i].bReference && pNode->nodeType != STMT_TREE_NODE::SNT_VAR && 
						pNode->nodeType != STMT_TREE_NODE::SNT_VAR_REF));
					if (funcDecl->vParamType[i].bReference)
					{
						// pass address
						SSM_STACK_ADDR	addr;
						if (pNode->nodeType == STMT_TREE_NODE::SNT_VAR_REF)
						{
							// pass original reference address
							addr = m_vLocalVarStack.GetValueByStackAddr(pNode->addr);
						}
						else
							addr = pNode->addr;

						if (!SSM_IS_GLOBAL_STACK_ADDR(addr))
						{
							addr = m_vLocalVarStack.size() - SSM_GET_STACK_ADDR(addr);
							SSM_SET_GLOBAL_STACK_ADDR_FLAG(addr);
						}
						ASSERT(SSM_GET_STACK_ADDR(addr) >= 0 && SSM_GET_STACK_ADDR(addr) < m_vLocalVarStack.size());
						params.Push(addr);
					}
					else
					{
						// pass value
						params.Push(pNode->nResult);
					}

					i++;
					pNode = pNode->sibling;
				}
				ASSERT(i == funcDecl->vParamType.size());
				if (i != funcDecl->vParamType.size())
					return ERR_R_FUNC_PARAM_COUNT;

				int		nStackSizeOld = m_vLocalVarStack.size();

				if (nClassID == -1)
					nRet = CallFun(nFuncID, &params);
				else
					nRet = CallFun(nClassID,  nFuncID, &params);
				ASSERT(nRet == ERR_OK);
				if (nRet != ERR_OK)
					return nRet;

				if (nStackSizeOld + 1 == m_vLocalVarStack.size())
				{
					// returned value was in the top of the stack
					pTree->nResult = m_vLocalVarStack.TopValue();
					m_vLocalVarStack.Popup();
					PRINT_STACK();
				}
			}
			break;
		case STMT_TREE_NODE::SNT_VAR:
			{
				if (pTree->child[0])
				{
					nRet = ExeGetClassMemberValue(pTree, pTree->nResult);
					ASSERT(nRet == ERR_OK);
				}
				else
					pTree->nResult = m_vLocalVarStack.GetValueByStackAddr(pTree->addr);
			}
			break;
		case STMT_TREE_NODE::SNT_VAR_REF:
			{
				if (pTree->child[0])
				{
					nRet = ExeGetClassMemberValue(pTree, pTree->nResult);
					ASSERT(nRet == ERR_OK);
				}
				else
				{
					SSM_STACK_ADDR	addr;
					addr = m_vLocalVarStack.GetValueByStackAddr(pTree->addr);
					pTree->nResult = m_vLocalVarStack.GetValueByStackAddr(addr);
				}
			}
			break;
		case STMT_TREE_NODE::SNT_CONST_INT:
			{
				pTree->nResult = (int)pTree->addr;
			}
			break;
		case STMT_TREE_NODE::SNT_CONST_STR:
			{
				pTree->nResult = (int)pTree->addr;
			}
			break;
		case STMT_TREE_NODE::SNT_NEW:
			{
				HHEAPDATA		hHeap;

				nRet = NewClass(pTree->addr, hHeap);
				if (nRet != ERR_OK)
					return nRet;

				pTree->nResult = (int)hHeap;
			}
			break;
		case STMT_TREE_NODE::SNT_DELETE:
			{
				HHEAPDATA			hHeap = NULL;
				PSTMT_TREE_NODE		pID = pTree->child[0];
				if (pID->child[0])
				{
					// class member variable
					int			nData;
					nRet = ExeGetClassMemberValue(pID, nData);
					if (nRet != ERR_OK)
						return nRet;
					hHeap = (HHEAPDATA)nData;
				}
				else
				{
					// object
					if (pID->nodeType == STMT_TREE_NODE::SNT_VAR)
						hHeap = (HHEAPDATA)m_vLocalVarStack.GetValueByStackAddr(pID->addr);
					else
					{
						SSM_STACK_ADDR	addr;
						addr = m_vLocalVarStack.GetValueByStackAddr(pID->addr);
						hHeap = (HHEAPDATA)m_vLocalVarStack.GetValueByStackAddr(addr);
					}
				}
				m_heap.Delete(hHeap);
			}
			break;
		default:
			ASSERT(0 && "Unknown statement type");
			break;
		}

		pTree = pTree->sibling;
	}

	return ERR_OK;
}

int CSSMProcess::ExeGetClassMemberValue(STMT_TREE_NODE *stmtVar, int &nValue)
{
	HHEAPDATA			hHeapData;
	STMT_TREE_NODE		*pID = stmtVar;

	if (pID->nodeType == STMT_TREE_NODE::SNT_VAR_REF)
	{
		SSM_STACK_ADDR	addr;
		addr = m_vLocalVarStack.GetValueByStackAddr(pID->addr);
		hHeapData = (HHEAPDATA)m_vLocalVarStack.GetValueByStackAddr(addr);
	}
	else
		hHeapData = (HHEAPDATA)m_vLocalVarStack.GetValueByStackAddr(pID->addr);

	while (pID->child[0]->child[0])
	{
		hHeapData = (HHEAPDATA)HeapGetValue(hHeapData, pID->child[0]->addr);
		pID = pID->child[0];
	}

	// set class member variable value. all class are allocated in heap.
	nValue = HeapGetValue(hHeapData, pID->child[0]->addr);

	return ERR_OK;
}

int CSSMProcess::ExeSetClassMemberValue(STMT_TREE_NODE *stmtVar, int nValue)
{
	HHEAPDATA			hHeapData;
	STMT_TREE_NODE		*pID = stmtVar;

	if (pID->nodeType == STMT_TREE_NODE::SNT_VAR_REF)
	{
		SSM_STACK_ADDR	addr;
		addr = m_vLocalVarStack.GetValueByStackAddr(pID->addr);
		hHeapData = (HHEAPDATA)m_vLocalVarStack.GetValueByStackAddr(addr);
	}
	else
		hHeapData = (HHEAPDATA)m_vLocalVarStack.GetValueByStackAddr(pID->addr);

	while (pID->child[0]->child[0])
	{
		hHeapData = (HHEAPDATA)HeapGetValue(hHeapData, pID->child[0]->addr);
		pID = pID->child[0];
	}

	// set class member variable value. all class are allocated in heap.
	HeapSetValue(hHeapData, pID->child[0]->addr, nValue);

	return ERR_OK;
}

int CSSMProcess::ExeGetClassMethod(STMT_TREE_NODE *stmtVar, CFunctionDeclTable::FunctionDecl **funcDecl, HHEAPDATA &hThis, int &nClassID, int &nFuncID)
{
	HHEAPDATA			hHeapData;
	STMT_TREE_NODE		*pID = stmtVar;

	*funcDecl = NULL;

	if (pID->nodeType == STMT_TREE_NODE::SNT_VAR_REF)
	{
		SSM_STACK_ADDR	addr;
		addr = m_vLocalVarStack.GetValueByStackAddr(pID->addr);
		hHeapData = (HHEAPDATA)m_vLocalVarStack.GetValueByStackAddr(addr);
	}
	else
		hHeapData = (HHEAPDATA)m_vLocalVarStack.GetValueByStackAddr(pID->addr);

	while (pID->child[0]->child[0])
	{
		hHeapData = (HHEAPDATA)HeapGetValue(hHeapData, pID->child[0]->addr);
		pID = pID->child[0];
	}

	CClassDeclarationTable::ClassDecl	*clsDecl;

	nClassID = GET_CLASS_ID(pID->child[0]->addr);
	nFuncID = GET_FUNC_ID(pID->child[0]->addr);

	clsDecl = GetClassByID(nClassID);
	ASSERT(clsDecl);
	if (!clsDecl)
	{
		FORMATLASTERROR(stmtVar->nLineNo, ERR_C_UNEXPECTED_ERR, "");
		return ERR_C_UNEXPECTED_ERR;
	}

	*funcDecl = clsDecl->m_methodTable.GetFuncByID(nFuncID);
	ASSERT(*funcDecl);
	if (!*funcDecl)
	{
		FORMATLASTERROR(stmtVar->nLineNo, ERR_C_UNEXPECTED_ERR, "");
		return ERR_C_UNEXPECTED_ERR;
	}

	hThis = hHeapData;

	return ERR_OK;
}

int CSSMProcess::NewClass(int nClassID, HHEAPDATA &hHeap)
{
	if (nClassID < 0)
		return ERR_NOT_FOUND;

	if (nClassID < m_classDeclTable.GetClassCount())
	{
		CClassDeclarationTable::ClassDecl	*clsDecl;
		int				nSize, nRet;

		clsDecl = m_classDeclTable.GetClassByID(nClassID);
		ASSERT(clsDecl);

		hHeap = m_heap.New(clsDecl->GetClassSize());

		nSize = m_vLocalVarStack.size();
		m_vLocalVarStack.Push((int)hHeap);

		nRet = CallFun(nClassID, FUNC_ID_CLASS_CONSTRUCT, NULL);

		if (nSize + 1 == m_vLocalVarStack.size())
			m_vLocalVarStack.Popup();
		else
			ASSERT(0 && "Invalid Stack size when call class construction function");

		return nRet;
	}
	else
		return m_pMachine->NewClass(nClassID - SSM_SYSAPI_FUNC_ID_BASE, hHeap, m_heap);
}

void CSSMProcess::FormatLastError(int nLine, int nError, LPCSTR szStr, int nLineSrc)
{
	::FormatLastError(m_strLastErr, nLine, nError, szStr, nLineSrc);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSSCompiler::CSSCompiler()
{
	m_syntaxTree = NULL;
	m_pProcess = NULL;
	m_CurClass = NULL;
}

CSSCompiler::~CSSCompiler()
{

}

int CSSCompiler::Init()
{

	return ERR_OK;
}

int CSSCompiler::Compile(LPCTSTR szFile, CSSMProcess &process)
{
	int		nRet;

	m_pProcess = &process;

	yyin = fopen(szFile, "r" );
	if (!yyin)
	{
		printf("Can't open file: %s", szFile);
		return ERR_OPEN_FILE;
	}

	nRet = yyparse();
	if (nRet != 0)
		return ERR_C_PARSE;
	m_syntaxTree = _syntaxTree;

#ifdef D_PRINT_SYNTAX_TREE
	int		nIndent = 0;
	PrintSyntaxTree(m_syntaxTree, nIndent);
#endif

	// build symbol table
	nRet = BuildSymbolTable();
	if (nRet != ERR_OK)
		return nRet;

	nRet = BuildCodeStmt();
	if (nRet != ERR_OK)
		return nRet;

//	deleteSSNode(m_syntaxTree);
	m_syntaxTree = NULL;

	return ERR_OK;
}

TYPE_NAME_ID CSSCompiler::GetIDOfTypeName(LPCTSTR szTypeName)
{
	return m_pProcess->GetClassIDByName(szTypeName);
}

const char *CSSCompiler::GetTypeNameStrByID(TYPE_NAME_ID id)
{
	return m_pProcess->GetClassByID(id)->strName.c_str();
}

int CSSCompiler::BuildSymbolTable()
{
	int		nRet;

	//
	// Add class and function declaration first.
	//
	nRet = PrevBST_AddClassDefinition();
	if (nRet != ERR_OK)
	{
		printf("Build Error: %s\n", m_strLastErr.c_str());
		return nRet;
	}

	nRet = PrevBST_AddFunctionAndVariable(m_syntaxTree, NULL);
	if (nRet != ERR_OK)
	{
		printf("Build Error: %s\n", m_strLastErr.c_str());
		return nRet;
	}

	nRet = BST_Node(m_syntaxTree);
	if (nRet != ERR_OK)
	{
		printf("Build Error: %s\n", m_strLastErr.c_str());
		return nRet;
	}

	return ERR_OK;
}

int CSSCompiler::PrevBST_AddClassDefinition()
{
	int					nRet;
	SS_TREE_NODE		*pTree;
	CClassDeclarationTable::ClassDecl	*pClassDecl;

	//
	// 1). Add class name first
	//
	pTree = m_syntaxTree;
	while (pTree)
	{
		if (pTree->nodeType == NT_CLASS_IMP)
		{
			// add class 
			pClassDecl = new CClassDeclarationTable::ClassDecl;
			pClassDecl->strName = pTree->szName;
			nRet = m_pProcess->AddClass(pClassDecl);
			if (nRet == INVALID_TYPE_ID)
			{
				FORMATLASTERROR(pTree->nLineNo, nRet, pTree->szName);
				return nRet;
			}
		}

		pTree = pTree->sibling;
	}

	//
	// 2). Add class member variables and methods declarations...
	//
	pTree = m_syntaxTree;
	while (pTree)
	{
		if (pTree->nodeType == NT_CLASS_IMP)
		{
			// get class declaration
			pClassDecl = m_pProcess->GetClassByName(pTree->szName);
			ASSERT(pClassDecl);

			// in build symbol table stage, don't set current symbol table as a class.
			m_symTable.Push(false);

			// add class member variables and methods.
			nRet = PrevBST_AddFunctionAndVariable(pTree->child[1], pClassDecl);
			if (nRet != ERR_OK)
				return nRet;

			m_symTable.Popup();
		}

		pTree = pTree->sibling;
	}

	return ERR_OK;
}

int CSSCompiler::PrevBST_AddFunctionAndVariable(SS_TREE_NODE *pTree, CClassDeclarationTable::ClassDecl *pClassDecl)
{
	int					nRet;

	//
	// Add all function declarations...
	//
	while (pTree)
	{
		if (pTree->nodeType == NT_FUNC_IMP)
		{
			// function definition
			SS_TREE_NODE	*pNode;

			m_symTable.Push(true);

			// check function declaration, update type info.
			nRet = BST_FunctionDecl(pTree);
			if (nRet != ERR_OK)
				return nRet;

			CFunctionDeclTable::FunctionDecl	*funcDecl = new CFunctionDeclTable::FunctionDecl;

			funcDecl->strName = pTree->szName;
			funcDecl->typeIDRet = pTree->child[0]->typeId;	// return value type

			// function parameter
			pNode = pTree->child[1];
			while (pNode)
			{
				funcDecl->AddParam(pNode->typeId, pNode->child[0]->kind.exp == EK_ID_REF);
				pNode = pNode->sibling;
			}

			if (pClassDecl)
				nRet = pClassDecl->AddMemberMethod(funcDecl);
			else
			{
				nRet = m_pProcess->AddFunc(funcDecl);
				if (nRet == INVALID_FUNC_ID)
					nRet = ERR_C_REDEFINITION_ID;
				else
					nRet = ERR_OK;
			}
			if (nRet != ERR_OK)
			{
				FORMATLASTERROR(pTree->nLineNo, nRet, pTree->szName);
				return nRet;
			}

			m_symTable.Popup();
		}
		else if (pTree->nodeType == NT_VAR_DECL && pClassDecl)
		{
			nRet = BST_VariableDecl(pTree, pClassDecl);
			if (nRet  != ERR_OK)
				return nRet;
		}

		pTree = pTree->sibling;
	}

	return ERR_OK;
}

int CSSCompiler::BST_Node(SS_TREE_NODE *pTree)
{
	int					nRet;

	while (pTree)
	{
		if (pTree->nodeType == NT_VAR_DECL)
		{
			nRet = BST_VariableDecl(pTree);
			if (nRet != ERR_OK)
				return nRet;
		}
		else if (pTree->nodeType == NT_FUNC_IMP)
		{
			// function definition
			m_symTable.Push(true);

			nRet = BST_FunctionDecl(pTree);
			if (nRet != ERR_OK)
				return nRet;

			// 4. check function body/statement
			nRet = BST_Node(pTree->child[2]);
			if (nRet != ERR_OK)
				return nRet;

			m_symTable.Popup();
		}
		else if (pTree->nodeType == NT_CLASS_IMP)
		{
			// class definition
			m_symTable.Push(false);

			nRet = BST_ClassDefinition(pTree);
			if (nRet != ERR_OK)
				return nRet;

			m_symTable.Popup();
		}
		else if (pTree->nodeType == NT_STMT)
		{
			nRet = BST_Stmt(pTree);
			if (nRet != ERR_OK)
				return nRet;
		}
		else if (pTree->nodeType == NT_EXP)
		{
			nRet = BST_Expression(pTree);
			if (nRet != ERR_OK)
				return nRet;
		}
		else
			ASSERT(0);

		pTree = pTree->sibling;
	}

	return ERR_OK;
}

int CSSCompiler::BST_ClassDefinition(SS_TREE_NODE *pTree)
{
	SS_TREE_NODE	*pNode = NULL;
	int				nRet;
	CClassDeclarationTable::ClassDecl	*classDecl = NULL;

	// 1. get class declaration
	classDecl = m_pProcess->GetClassByName(pTree->szName);
	ASSERT(classDecl);
	if (!classDecl)
	{
		FORMATLASTERROR(pTree->nLineNo, ERR_C_UNEXPECTED_ERR, pTree->szName);
		return ERR_C_UNEXPECTED_ERR;
	}

	// 2. class memeber variables was checked in PrevBST_AddClassDefinition, don't check it here.
	//    So, just add them in symbol table.
	for (int i = 0; i < classDecl->vMemberVar.size(); i++)
	{
		m_symTable.AddSymbolDeclaration(classDecl->vMemberVar[i].strName.c_str(), 
			classDecl->vMemberVar[i].nLineNo, classDecl->vMemberVar[i].typeID, false);
	}

	// 3. Check class memeber methods only.
	pNode = pTree->child[1];
	while (pNode)
	{
		if (pNode->nodeType == NT_FUNC_IMP)
		{
			// function definition
			m_symTable.Push(classDecl);

			// 3.1 Add this before all parameters
			m_symTable.AddSymbolDeclaration(SZ_THIS, pNode->nLineNo, 
				m_pProcess->GetClassIDByName(pTree->szName), false);

			nRet = BST_FunctionDecl(pNode);
			if (nRet != ERR_OK)
				return nRet;

			// 4. check function body/statement
			nRet = BST_Node(pNode->child[2]);
			if (nRet != ERR_OK)
				return nRet;

			m_symTable.Popup();
		}
		else
			ASSERT(pNode->nodeType == NT_VAR_DECL);

		pNode = pNode->sibling;
	}

	return ERR_OK;
}

int CSSCompiler::BST_FunctionDecl(SS_TREE_NODE *pTree)
{
	SS_TREE_NODE	*pNode;
	int				nRet;

	// 1. check function return type
	pNode = pTree->child[0];
	pNode->typeId = GetIDOfTypeName(pNode->szName);
	if (pNode->typeId == INVALID_TYPE_ID)
	{
		// report error.
		FORMATLASTERROR(pNode->nLineNo, ERR_C_UNDECLARED_ID, pNode->szName);
		return ERR_C_UNDECLARED_ID;
	}
	m_typeIDCurFunc = pTree->typeId = pNode->typeId;

	// 2. check function name
	//   1) Is it a type name?
	if (GetIDOfTypeName(pTree->szName) != INVALID_TYPE_ID)
	{
		FORMATLASTERROR(pTree->nLineNo, ERR_C_REDEFINITION_ID, pTree->szName);
		return ERR_C_REDEFINITION_ID;
	}

	// 3. check function parameter declaration
	pNode = pTree->child[1];
	while (pNode)
	{
		nRet = BST_VariableDecl(pNode);
		if (nRet != ERR_OK)
			return nRet;
		pNode = pNode->sibling;
	}

	return ERR_OK;
}

int CSSCompiler::BST_VariableDecl(SS_TREE_NODE *pTree, CClassDeclarationTable::ClassDecl *classDecl)
{
	int		nRet;

	// check varible declaration
	ASSERT(pTree->nodeType == NT_VAR_DECL);

	// 1. check type name
	ASSERT(pTree->szName);
	pTree->typeId = GetIDOfTypeName(pTree->szName);
	if (pTree->typeId == INVALID_TYPE_ID)
	{
		// report error.
		FORMATLASTERROR(pTree->nLineNo, ERR_C_UNDECLARED_ID, pTree->szName);
		return ERR_C_UNDECLARED_ID;
	}

	// 2. check varible id
	TYPE_NAME_ID	typeId = pTree->typeId;
	SS_TREE_NODE	*pNode = pTree->child[0];
	while (pNode)
	{
		ASSERT(pNode->nodeType == NT_VAR_ID);

		// 1) Is it a type name?
		if (GetIDOfTypeName(pNode->szName) != INVALID_TYPE_ID)
		{
			FORMATLASTERROR(pNode->nLineNo, ERR_C_REDEFINITION_ID, pNode->szName);
			return ERR_C_REDEFINITION_ID;
		}

		if (classDecl)
		{
			// 2) Add class member variable
			nRet = classDecl->AddMemberVar(typeId, pNode->szName, pNode->nLineNo);
			if (nRet != ERR_OK)
			{
				FORMATLASTERROR(pNode->nLineNo, nRet, pNode->szName);
				return nRet;
			}
		}

		// 3) check varible assign expression.
		if (pNode->child[0])
		{
			nRet = BST_Expression(pNode->child[0]);
			if (nRet != ERR_OK)
				return nRet;
		}

		// 4) Add symbol declaration
		nRet = m_symTable.AddSymbolDeclaration(pNode->szName, pNode->nLineNo, typeId, pNode->kind.exp == EK_ID_REF);
		if (nRet != ERR_OK)
		{
			FORMATLASTERROR(pNode->nLineNo, ERR_C_REDEFINITION_ID, pNode->szName);
			return nRet;
		}

		pNode = pNode->sibling;
	}

	return ERR_OK;
}

int CSSCompiler::BST_Expression(SS_TREE_NODE *pTree)
{
	int					nRet;

	ASSERT(pTree->nodeType == NT_EXP);
	switch (pTree->kind.exp)
	{
	case EK_ID:
	case EK_ID_REF:
		{
			ASSERT(pTree->kind.exp != EK_ID_REF);
			TypeInfo	typeInfo;

			// is id defined?
			typeInfo = m_symTable.AddSymbolReference(pTree->szName, pTree->nLineNo);
			if (typeInfo.typeID == INVALID_TYPE_ID)
			{
				FORMATLASTERROR(pTree->nLineNo, ERR_C_UNDECLARED_ID, pTree->szName);
				return ERR_C_UNDECLARED_ID;
			}
			if (typeInfo.nClsMemberVar != -1)
			{
				// it's a class member variable, insert this node
				SS_TREE_NODE	*pMemberVar = new SS_TREE_NODE;
				*pMemberVar = *pTree;
				pTree->szName = dupStr(SZ_THIS);
				pTree->child[0] = pMemberVar;

				typeInfo = m_symTable.AddSymbolReference(pTree->szName, pTree->nLineNo);
			}

			SS_TREE_NODE	*pNode;
			pNode = pTree->child[0];
			while (pNode)
			{
				// check class member variable
				CClassDeclarationTable::ClassDecl	*classDecl;
				int					nMemberIndex;

				classDecl = m_pProcess->GetClassByID(typeInfo.typeID);
				ASSERT(classDecl);

				nMemberIndex = classDecl->GetMemberVarIndex(pNode->szName);
				if (nMemberIndex == -1)
				{
					FORMATLASTERROR(pNode->nLineNo, ERR_C_UNDECLARED_ID, pNode->szName);
					return ERR_C_UNDECLARED_ID;
				}
				typeInfo.bReference = false;
				typeInfo.typeID = classDecl->vMemberVar[nMemberIndex].typeID;

				pNode = pNode->child[0];
			}

			pTree->typeId = typeInfo.typeID;

			if (typeInfo.bReference)
				pTree->kind.exp = EK_ID_REF;
		}
		break;
	case EK_CONST_STRING:
		pTree->typeId = g_typeIDString;
		break;
	case EK_NEW:
		{
			pTree->typeId = GetIDOfTypeName(pTree->szName);
			if (pTree->typeId == INVALID_TYPE_ID)
			{
				FORMATLASTERROR(pTree->nLineNo, ERR_C_UNDECLARED_ID, pTree->szName);
				return ERR_C_UNDECLARED_ID;
			}
			if (IsSimpleKindTypeID(pTree->typeId))
			{
				FORMATLASTERROR(pTree->nLineNo, ERR_C_DELETE_WRONG_TYPE, pTree->szName);
				return ERR_C_DELETE_WRONG_TYPE;
			}
		}
		break;
	case EK_CONST_INT:
		pTree->typeId = g_typeIDInt;
		break;
	case EK_FUNC_CALL:
		{
			CFunctionDeclTable::FunctionDecl	*funcDecl = NULL;
			SS_TREE_NODE	*pID;
			nRet = ERR_OK;

			pID = pTree->child[0];

			// 0.1 Is class method call? if so, insert this node.
			if (m_symTable.IsClassMemberMethod(pID->szName))
			{
				SS_TREE_NODE	*pMemberMethod = new SS_TREE_NODE;
				*pMemberMethod = *pID;
				pID->szName = dupStr(SZ_THIS);
				pID->child[0] = pMemberMethod;
			}

			// 1. Is function name defined?
			if (pID->child[0])
			{
				//
				// class member method call
				//

				// Is var declared before?
				TypeInfo typeInfo = m_symTable.AddSymbolReference(pID->szName, pID->nLineNo);
				if (typeInfo.typeID == INVALID_TYPE_ID)
				{
					FORMATLASTERROR(pTree->nLineNo, ERR_C_UNDECLARED_ID, pID->szName);
					return ERR_C_UNDECLARED_ID;
				}
				if (typeInfo.nClsMemberVar != -1)
				{
					// it's a class member variable, insert this
					SS_TREE_NODE	*pMemberVar = new SS_TREE_NODE;
					*pMemberVar = *pID;
					pID->szName = dupStr(SZ_THIS);
					pID->child[0] = pMemberVar;

					typeInfo = m_symTable.AddSymbolReference(pID->szName, pID->nLineNo);
				}

				// is class member correct?
				TYPE_NAME_ID	typeID =  typeInfo.typeID;
				SS_TREE_NODE	*pMemberVar = pID->child[0];
				int				nMemberIndex;
				while (pMemberVar)
				{
					CClassDeclarationTable::ClassDecl	*classDecl;
					classDecl = m_pProcess->GetClassByID(typeID);
					ASSERT(classDecl);

					if (pMemberVar->child[0])
					{
						// class member variable?
						nMemberIndex = classDecl->GetMemberVarIndex(pMemberVar->szName);
						if (nMemberIndex == -1)
						{
							FORMATLASTERROR(pTree->nLineNo, ERR_C_UNDECLARED_ID, pMemberVar->szName);
							return ERR_C_UNDECLARED_ID;
						}
						typeID = classDecl->vMemberVar[nMemberIndex].typeID;
					}
					else
					{
						// class member function
						funcDecl = classDecl->m_methodTable.GetFuncByName(pMemberVar->szName);
						if (funcDecl == NULL)
						{
							FORMATLASTERROR(pTree->nLineNo, ERR_C_UNDECLARED_ID, pMemberVar->szName);
							return ERR_C_UNDECLARED_ID;
						}
					}
					pMemberVar = pMemberVar->child[0];
				}
			}
			else
			{
				// normal function call
				funcDecl = m_pProcess->GetFuncByName(pID->szName);
				if (!funcDecl)
				{
					FORMATLASTERROR(pID->nLineNo, ERR_C_UNDECLARED_ID, pID->szName);
					return ERR_C_UNDECLARED_ID;
				}
			}

			// 2. get function return type.
			pTree->typeId = funcDecl->typeIDRet;

			// 3. Is function parameter correct?
			int			i = 0;
			SS_TREE_NODE *pNode = pTree->child[1];
			while (pNode)
			{
				// is paramer count greater than declared?
				if (i >= funcDecl->vParamType.size())
				{
					FORMATLASTERROR(pNode->nLineNo, ERR_R_FUNC_PARAM_COUNT, "");
					return ERR_R_FUNC_PARAM_COUNT;
				}

				// check parameter expression
				nRet = BST_Expression(pNode);
				if (nRet != ERR_OK)
					return nRet;

				// Is expression type match function delcaration?
				if (pNode->typeId != funcDecl->vParamType[i].typeID 
					&& funcDecl->vParamType[i].typeID != g_typeIDAny)
				{
					char	szMsg[256];
					sprintf(szMsg, "Can't convert parameter: %d from type '%s' to '%s'", i + 1, 
						GetTypeNameStrByID(pNode->typeId),
						GetTypeNameStrByID(funcDecl->vParamType[i].typeID));
					FORMATLASTERROR(pNode->nLineNo, ERR_R_FUNC_PARAM_TYPE, szMsg);
					return ERR_R_FUNC_PARAM_TYPE;
				}

				// reference parameter can only pass from an ID variable.
				if (funcDecl->vParamType[i].bReference && 
					(pNode->kind.exp != EK_ID && pNode->kind.exp != EK_ID_REF))
				{
					char	szMsg[256];
					sprintf(szMsg, "reference parameter: %d can't accept const value.", i + 1);
					FORMATLASTERROR(pNode->nLineNo, ERR_R_FUNC_PARAM_TYPE, szMsg);
					return ERR_R_FUNC_PARAM_TYPE;
				}

				i++;
				pNode = pNode->sibling;
			}
			if (i != funcDecl->vParamType.size())
			{
				FORMATLASTERROR(pTree->nLineNo, ERR_R_FUNC_PARAM_COUNT, "");
				return ERR_R_FUNC_PARAM_COUNT;
			}
		}
		break;
	case EK_OP:
		{
			// Check variable id 1
			if (pTree->child[0])
			{
				nRet = BST_Expression(pTree->child[0]);
				if (nRet != ERR_OK)
					return nRet;
			}

			// Check variable id 2
			if (pTree->child[1])
			{
				nRet = BST_Expression(pTree->child[1]);
				if (nRet != ERR_OK)
					return nRet;
			}

			if (pTree->child[0] && pTree->child[1])
			{
				// is variable type same?
				if (pTree->child[0]->typeId != pTree->child[1]->typeId)
				{
					string		str;

					str = "type1: ";
					str += GetTypeNameStrByID(pTree->child[0]->typeId);

					str += "type2: ";
					str += GetTypeNameStrByID(pTree->child[1]->typeId);

					FORMATLASTERROR(pTree->nLineNo, ERR_C_OP_TYPE_NOT_MATCH, str.c_str());
					return ERR_C_OP_TYPE_NOT_MATCH;
				}
			}
			pTree->typeId = pTree->child[0]->typeId;
			if (pTree->typeId != g_typeIDInt)
			{
				FORMATLASTERROR(pTree->nLineNo, ERR_C_OP_REQUIRE_INT_TYPE, GetTypeNameStrByID(pTree->typeId));
				return ERR_C_OP_REQUIRE_INT_TYPE;
			}
/*			switch (pTree->typeId)
			{
			case EO_EQUAL:
			case EO_NOT_EQUAL:
			case EO_PLUS:
			case EO_LT_EQ	:
			case EO_GT_EQ	:
			case EO_LITTLE	:
			case EO_GREATER	:
			case EO_OP_INC	:
			case EO_OP_DEC	:
			case EO_MINUS	:
			case EO_MULT	:
			case EO_DIV		:
				if (pTree->typeId != g_typeIDInt)
				{
					FORMATLASTERROR(pTree->nLineNo, ERR_C_OP_REQUIRE_INT_TYPE, GetTypeNameStrByID(pTree->typeId));
					return ERR_C_OP_REQUIRE_INT_TYPE;
				}
				break;
			default:
				FORMATLASTERROR(pTree->nLineNo, ERR_C_UNEXPECTED_ERR, "Unknown expression operation");
				return ERR_C_UNEXPECTED_ERR;
				break;
			}*/
			return ERR_OK;
		}
		break;
	default:
		ASSERT(0 && "Unknown expression kind");
		return ERR_C_UNEXPECTED_ERR;
	}

	return ERR_OK;
}

int CSSCompiler::BST_Stmt(SS_TREE_NODE *pTree)
{
	int					nRet;

	// ASSERT(pTree->nodeType == NT_STMT);
	switch (pTree->kind.stmt)
	{
	case SK_NULL:
		break;
	case SK_IF:
		// 1. check expression
		nRet = BST_Expression(pTree->child[0]);
		if (nRet != ERR_OK)
			return nRet;

		// 2. Check statement
		m_symTable.Push(false);
		nRet = BST_Node(pTree->child[1]);
		if (nRet != ERR_OK)
			return nRet;
		m_symTable.Popup();

		// 3. Check else statement
		if (pTree->child[2])
		{
			m_symTable.Push(false);
			nRet = BST_Node(pTree->child[2]);
			if (nRet != ERR_OK)
				return nRet;
			m_symTable.Popup();
		}
		break;
	case SK_WHILE:
		// 1. check expression
		nRet = BST_Expression(pTree->child[0]);
		if (nRet != ERR_OK)
			return nRet;

		// 2. Check statement
		m_symTable.Push(false);
		nRet = BST_Node(pTree->child[1]);
		if (nRet != ERR_OK)
			return nRet;
		m_symTable.Popup();
		break;
	case SK_DO_WHILE:
		// 1. Check statement
		m_symTable.Push(false);
		nRet = BST_Node(pTree->child[1]);
		if (nRet != ERR_OK)
			return nRet;
		m_symTable.Popup();

		// 2. check expression
		nRet = BST_Expression(pTree->child[0]);
		if (nRet != ERR_OK)
			return nRet;
		break;
	case SK_ASSIGN:
		{
			SS_TREE_NODE	*pID = pTree->child[0];

			// 1. left node should be an ID
			nRet = BST_Expression(pID);
			if (nRet != ERR_OK)
				return nRet;
			pTree->typeId = pID->typeId;

			// 4. check expression
			SS_TREE_NODE	*pExp = pTree->child[1];
			nRet = BST_Expression(pExp);
			if (nRet != ERR_OK)
				return nRet;

			// 5. left and right must have same type
			if (pID->typeId != pExp->typeId)
			{
				char		szMsg[512];
				sprintf(szMsg, "Can't convert from type '%s' to '%s'.", 
					GetTypeNameStrByID(pID->typeId),
					GetTypeNameStrByID(pExp->typeId));
				FORMATLASTERROR(pTree->nLineNo, ERR_C_CONVERT_VALUE, "");
				return ERR_C_CONVERT_VALUE;
			}
		}
		break;
	case SK_RETURN:
		{
			SS_TREE_NODE	*pNode;

			// 1. check return expression 
			pNode = pTree->child[0];
			if (pNode)
			{
				nRet = BST_Expression(pNode);
				if (nRet != ERR_OK)
					return nRet;
			}
			pTree->typeId = pNode->typeId;

			// 2. Check return type math?
			if (m_typeIDCurFunc != pTree->typeId)
			{
				char	szMsg[265];
				sprintf(szMsg, "'Return' can't convert from type '%s' to '%s'.", 
					GetTypeNameStrByID(pTree->typeId),
					GetTypeNameStrByID(m_typeIDCurFunc));
				FORMATLASTERROR(pTree->nLineNo, ERR_C_CONVERT_VALUE, szMsg);
				return ERR_C_CONVERT_VALUE;
			}
		}
		break;
	case SK_BLOCK:
		if (pTree->child[0])
		{
			m_symTable.Push();
			nRet = BST_Node(pTree->child[0]);
			if (nRet != ERR_OK)
				return nRet;
			m_symTable.Popup();
		}
		break;
	case SK_DELETE:
		{
			SS_TREE_NODE	*pID = pTree->child[0];

			nRet = BST_Expression(pID);
			if (nRet != ERR_OK)
				return nRet;

			if (IsSimpleKindTypeID(pID->typeId))
			{
				FORMATLASTERROR(pID->nLineNo, ERR_C_DELETE_WRONG_TYPE, pID->szName);
				return ERR_C_DELETE_WRONG_TYPE;
			}
		}
		break;
	default:
		FORMATLASTERROR(pTree->nLineNo, ERR_C_UNEXPECTED_ERR, "Unknown statement kind.");
		return ERR_C_UNEXPECTED_ERR;
	}

	return ERR_OK;
}
// 
// int CSSCompiler::BST_Variable(SS_TREE_NODE *pTree)
// {
// 
// }

void CSSCompiler::FormatLastError(const char *szFormat, ...)
{
	va_list		args;

	int			nBuf;
	char		szBuffer[512] = "";
	char		*szMsg = "";
	int			nLen = 0;

#if defined(_WIN32) && !defined(_WINCE_)
	__try
#endif
	{
		// 格式化日志
		va_start(args, szFormat);
		szMsg = szBuffer + nLen;
		nBuf = _vsntprintf(szMsg, CountOf(szBuffer) - nLen, szFormat, args);
		va_end(args);

		// was there an error? was the expanded string too long?
		if (nBuf < 0)
			strcpy(szBuffer, _T("Insufficient log buffer space."));
	}
#if defined(_WIN32) && !defined(_WINCE_)
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// 有异常发生！
		strcpy(szBuffer, _T("Exception error in format."));
	}
#endif
	m_strLastErr = szBuffer;
}

void CSSCompiler::FormatLastError(int nLine, int nError, LPCSTR szStr, int nLineSrc)
{
	::FormatLastError(m_strLastErr, nLine, nError, szStr, nLineSrc);
}

int CSSCompiler::BuildCodeStmt()
{
	int					nRet;
	SS_TREE_NODE		*pTree = m_syntaxTree;
	CFunctionDeclTable::FunctionDecl	*func;

	m_symTable.Close();

	func = m_pProcess->GetFuncByID(0);
	nRet = BCS_Tree(m_syntaxTree, func->m_pstmtTree);
	ASSERT(nRet == ERR_OK);

	m_pProcess->PrintCodeTree();

	return ERR_OK;
}

int CSSCompiler::BCS_Tree(SS_TREE_NODE *pTree, PSTMT_TREE_NODE &pstmtOut)
{
	int					nRet;
	PSTMT_TREE_NODE		*ppstmtTail = &pstmtOut;
	PSTMT_TREE_NODE		pstmt = NULL;

	while (pTree)
	{
		if (pTree->nodeType == NT_VAR_DECL)
		{
			// variable declaration
			nRet = BCS_VarDeclaration(pTree, pstmt);
			ASSERT(nRet == ERR_OK);
		}
		else if (pTree->nodeType == NT_FUNC_IMP)
		{
			// function definition
			nRet = BCS_FunctionDefinition(pTree);
			ASSERT(nRet == ERR_OK);

			pTree = pTree->sibling;

			// continue next node.
			continue;
		}
		else if (pTree->nodeType == NT_CLASS_IMP)
		{
			// class definition
			BCS_ClassDefinition(pTree);

			pTree = pTree->sibling;

			// continue next node.
			continue;
		}
		else if (pTree->nodeType == NT_STMT)
		{
			nRet = BCS_Stmt(pTree, pstmt);
			ASSERT(nRet == ERR_OK);
			if (nRet != ERR_OK)
				return nRet;
		}
		else if (pTree->nodeType == NT_EXP)
		{
			nRet = BCS_Expression(pTree, pstmt);
			ASSERT(nRet == ERR_OK);
			if (nRet != ERR_OK)
				return nRet;
		}
		else
			ASSERT(0 && "Unknown node type");

		*ppstmtTail = pstmt;
		ppstmtTail = &(pstmt->sibling);

		pTree = pTree->sibling;
	}

	return ERR_OK;
}

int CSSCompiler::BCS_FunctionDefinition(SS_TREE_NODE *pTree, CClassDeclarationTable::ClassDecl *pClassDecl)
{
	int					nRet;
	PSTMT_TREE_NODE		pstmtFunc = NULL;
	CFunctionDeclTable::FunctionDecl	*pCurFunc;

	m_symTable.Push(true);

	// Get function declaration
	if (pClassDecl)
	{
		// in class definition
		pCurFunc = pClassDecl->m_methodTable.GetFuncByName(pTree->szName);
	}
	else
	{
		// in global function and viariable definition
		pCurFunc = m_pProcess->GetFuncByName(pTree->szName);
		ASSERT(pCurFunc);
	}

	pstmtFunc = newStmt(STMT_TREE_NODE::SNT_BLOCK);
	pstmtFunc->nLineNo = pTree->nLineNo;

	// 3.1 Add this before all parameters
	if (pClassDecl)
	{
		m_symTable.AddSymbolDeclaration(SZ_THIS, pTree->nLineNo, 
			m_pProcess->GetClassIDByName(pClassDecl->strName.c_str()), false);
	}

	// 3.2 Add function parameter declaration
	//    Just add function parameter variable in symbol table
	//    Don't allocate space in stack. Caller function should do that.
	PSTMT_TREE_NODE	*pstmtParam;
	SS_TREE_NODE	*pNode;
	pstmtParam = &(pstmtFunc->child[1]);
	pNode = pTree->child[1];
	while (pNode)
	{
		nRet = BST_VariableDecl(pNode, NULL);
		ASSERT(nRet == ERR_OK);
		if (nRet != ERR_OK)
			return nRet;
		pNode = pNode->sibling;
		pstmtParam = &((*pstmtParam)->sibling);
	}

	// 4. check function body/statement
	nRet = BCS_Tree(pTree->child[2], pstmtFunc->child[2]);
	ASSERT(nRet == ERR_OK);
	if (nRet != ERR_OK)
		return nRet;

	// clear stack
	STMT_TREE_NODE		*pstmt;
	pstmt = newStmt(STMT_TREE_NODE::SNT_POPUP_STACK_N);
	pstmt->nLineNo = pTree->nLineNo;
	pstmt->addr = m_symTable.GetCurFuncStackSize();

	pstmtFunc->sibling = pstmt;

	pCurFunc->m_pstmtTree = pstmtFunc;

	m_symTable.Popup();

	return ERR_OK;
}

int CSSCompiler::BCS_ClassDefinition(SS_TREE_NODE *pTree)
{
	SS_TREE_NODE	*pNode = NULL;
	int				nRet;
	CClassDeclarationTable::ClassDecl *pClassDecl;

	// 1. get class declearation
	pClassDecl = m_pProcess->GetClassByName(pTree->szName);
	ASSERT(pClassDecl);
	if (!pClassDecl)
	{
		FORMATLASTERROR(pTree->nLineNo, ERR_C_UNEXPECTED_ERR, pTree->szName);
		return ERR_C_UNEXPECTED_ERR;
	}

	CFunctionDeclTable::FunctionDecl	*funcConstruct;

	funcConstruct = pClassDecl->m_methodTable.GetFuncByID(FUNC_ID_CLASS_CONSTRUCT);
	ASSERT(funcConstruct);


	// 2. Check class memeber variables only
	// Add variable init statement in construction function
	m_symTable.Push(pClassDecl);
	m_symTable.AddSymbolDeclaration(SZ_THIS, pTree->nLineNo, 
		m_pProcess->GetClassIDByName(pTree->szName), false);
	pNode = pTree->child[1];
	while (pNode)
	{
		if (pNode->nodeType == NT_VAR_DECL)
		{
			PSTMT_TREE_NODE		pstmtInit = NULL;
			nRet = BCS_ClassMemberVariable(pNode, pstmtInit, pClassDecl);
			if (nRet != ERR_OK)
				return nRet;
			if (pstmtInit)
			{
				if (funcConstruct->m_pstmtTree)
					funcConstruct->m_pstmtTree->AddTailSibing(pstmtInit);
				else
					funcConstruct->m_pstmtTree = pstmtInit;
			}
		}
		else
			ASSERT(pNode->nodeType == NT_FUNC_IMP);

		pNode = pNode->sibling;
	}
	m_symTable.Popup();

	m_symTable.Push(pClassDecl);
	// 3. Check class memeber methods only.
	pNode = pTree->child[1];
	while (pNode)
	{
		if (pNode->nodeType == NT_FUNC_IMP)
		{
			// function definition
			nRet = BCS_FunctionDefinition(pNode, pClassDecl);
			if (nRet != ERR_OK)
				return nRet;
		}

		pNode = pNode->sibling;
	}
	m_symTable.Popup();

	return ERR_OK;
}

int CSSCompiler::BCS_ClassMemberVariable(SS_TREE_NODE *pTree, PSTMT_TREE_NODE &pstmtConstruct, CClassDeclarationTable::ClassDecl *pClassDecl)
{
	// build varible declaration
	ASSERT(pTree->nodeType == NT_VAR_DECL);

	pstmtConstruct = NULL;

	int					nRet;
	PSTMT_TREE_NODE		*ppstmtTail = &pstmtConstruct;

	// 2. add every variable initial statement
	SS_TREE_NODE	*pVarNode = pTree->child[0];
	while (pVarNode)
	{
		STMT_TREE_NODE	*pstmt = NULL;
		SS_TREE_NODE	*pExp;

		pExp = pVarNode->child[0];
		if (pExp)
		{
			// 3. add variable assign expression code.
			pstmt = newStmt(STMT_TREE_NODE::SNT_ASSIGN);
			pstmt->nLineNo = pVarNode->nLineNo;

			STMT_TREE_NODE	*pThis;
			pThis = newStmt(STMT_TREE_NODE::SNT_VAR);
			pThis->addr = m_symTable.GetSymbolMemLoc(SZ_THIS);
			pThis->nLineNo = pVarNode->nLineNo;
			pstmt->child[0] = pThis;

			STMT_TREE_NODE	*pMemberVar;
			pMemberVar = newStmt(STMT_TREE_NODE::SNT_VAR);
			pMemberVar->addr = pClassDecl->GetMemberVarIndex(pVarNode->szName);
			pMemberVar->nLineNo = pVarNode->nLineNo;
			pThis->child[0] = pMemberVar;

			STMT_TREE_NODE	*pExression = NULL;
			nRet = BCS_Expression(pExp, pExression);
			ASSERT(nRet == ERR_OK);
			if (nRet != ERR_OK)
				return nRet;

			pstmt->child[1] = pExression;
			*ppstmtTail = pstmt;
			ppstmtTail = &(pstmt->sibling);
		}

		// add symbol declaration
		nRet = m_symTable.AddSymbolDeclaration(pVarNode->szName, pVarNode->nLineNo, 
			pTree->typeId, pVarNode->kind.exp == EK_ID_REF);

		pVarNode = pVarNode->sibling;
	}
	return ERR_OK;
}

int CSSCompiler::BCS_VarDeclaration(SS_TREE_NODE *pTree, PSTMT_TREE_NODE &pstmtOut)
{
	// build varible declaration
	ASSERT(pTree->nodeType == NT_VAR_DECL);

	pstmtOut = NULL;

	TYPE_NAME_ID		typeId;
	int					nRet;
	PSTMT_TREE_NODE		*ppstmtTail = &pstmtOut;

	// 1. get type name
	typeId = GetIDOfTypeName(pTree->szName);
	ASSERT(typeId != INVALID_TYPE_ID);

	// 2. add every variable
	SS_TREE_NODE	*pNode = pTree->child[0];
	while (pNode)
	{
		STMT_TREE_NODE	*pstmt = NULL;
		SS_TREE_NODE	*pExp;

		pExp = pNode->child[0];
		if (pExp)
		{
			// 3. add variable assign expression code.
			pstmt = newStmt(STMT_TREE_NODE::SNT_ASSIGN);
			pstmt->nLineNo = pNode->nLineNo;

			STMT_TREE_NODE	*pExression = NULL;
			nRet = BCS_Expression(pExp, pExression);
			ASSERT(nRet == ERR_OK);
			if (nRet != ERR_OK)
				return nRet;

			pstmt->child[1] = pExression;

			pstmt->child[2] = newStmtAllocStackVar(typeId);
			pstmt->child[2]->nLineNo = pNode->nLineNo;
		}
		else
		{
			// 
			pstmt = newStmtAllocStackVar(typeId);
			pstmt->nLineNo = pNode->nLineNo;
		}

		// add symbol declaration
		nRet = m_symTable.AddSymbolDeclaration(pNode->szName, pNode->nLineNo, 
			typeId, pNode->kind.exp == EK_ID_REF);
		ASSERT(nRet == ERR_OK);
		pstmt->addr = m_symTable.GetSymbolMemLoc(pNode->szName);

		*ppstmtTail = pstmt;
		ppstmtTail = &(pstmt->sibling);

		pNode = pNode->sibling;
	}
	return ERR_OK;
}

int CSSCompiler::BCS_Expression(SS_TREE_NODE *pTree, PSTMT_TREE_NODE &pstmtOut)
{
	ASSERT(pTree->nodeType == NT_EXP);
	int					nRet = ERR_OK;

	if (pTree->kind.exp == EK_ID)
	{
		if (pTree->child[0])
			nRet = BCS_ClassVariable(pTree, pstmtOut);
		else
		{
			pstmtOut = new STMT_TREE_NODE;
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_VAR;
			pstmtOut->addr = m_symTable.GetSymbolMemLoc(pTree->szName);
		}
	}
	else if (pTree->kind.exp == EK_ID_REF)
	{
		if (pTree->child[0])
			nRet = BCS_ClassVariable(pTree, pstmtOut);
		else
		{
			pstmtOut = new STMT_TREE_NODE;
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_VAR_REF;
			pstmtOut->addr = m_symTable.GetSymbolMemLoc(pTree->szName);
		}
	}
	else if (pTree->kind.exp == EK_CONST_INT)
	{
		pstmtOut = new STMT_TREE_NODE;
		pstmtOut->nodeType = STMT_TREE_NODE::SNT_CONST_INT;
		pstmtOut->addr = atoi(pTree->szName);
	}
	else if (pTree->kind.exp == EK_CONST_STRING)
	{
		pstmtOut = new STMT_TREE_NODE;
		pstmtOut->nodeType = STMT_TREE_NODE::SNT_CONST_STR;
		pstmtOut->addr = (SSM_STACK_ADDR)m_pProcess->m_strTable.Add(pTree->szName);
	}
	else if (pTree->kind.exp == EK_NEW)
	{
		pstmtOut = new STMT_TREE_NODE;
		pstmtOut->nodeType = STMT_TREE_NODE::SNT_NEW;
		pstmtOut->addr = pTree->typeId;
	}
	else if (pTree->kind.exp == EK_FUNC_CALL)
	{
		pstmtOut = new STMT_TREE_NODE;
		pstmtOut->nodeType = STMT_TREE_NODE::SNT_FUNC_CALL;

		// Get function id

		// 1. Is function name defined?
		SS_TREE_NODE	*pID;
		pID = pTree->child[0];
		ASSERT(pID);
		if (pID->child[0])
		{
			nRet = BCS_ClassMethodCall(pID, pstmtOut->child[0]);
			if (nRet != ERR_OK)
				return nRet;
		}
		else
		{
			// normal function call
			pstmtOut->addr = m_pProcess->GetFuncIDByName(pID->szName);
			if (pstmtOut->addr == -1)
			{
				FORMATLASTERROR(pID->nLineNo, ERR_C_UNDECLARED_ID, pID->szName);
				return ERR_C_UNDECLARED_ID;
			}
		}

		// 2. function parameter
		PSTMT_TREE_NODE		*pstmtParamTail = &(pstmtOut->child[1]);
		SS_TREE_NODE *pNode = pTree->child[1];
		while (pNode)
		{
			// Is id defined?
			nRet = BCS_Expression(pNode, *pstmtParamTail);
			ASSERT(nRet == ERR_OK);
			if (nRet != ERR_OK)
				return nRet;

			pstmtParamTail = &((*pstmtParamTail)->sibling);

			pNode = pNode->sibling;
		}
	}
	else if (pTree->kind.exp == EK_OP)
	{
		PSTMT_TREE_NODE		pstmtOp1 = NULL, pstmtOp2 = NULL;

		pstmtOut = new STMT_TREE_NODE;
		// Check variable type
		if (pTree->child[0])
		{
			nRet = BCS_Expression(pTree->child[0], pstmtOp1);
			ASSERT(nRet == ERR_OK);
		}
		if (pTree->child[1])
		{
			nRet = BCS_Expression(pTree->child[1], pstmtOp2);
			ASSERT(nRet == ERR_OK);
		}
		pstmtOut->child[0] = pstmtOp1;
		pstmtOut->child[1] = pstmtOp2;
		switch (pTree->attr.op)
		{
		case EO_LT_EQ	:
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_LT_EQ;
			break;
		case EO_GT_EQ	:
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_GT_EQ;
			break;
		case EO_LITTLE	:
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_LITTLE;
			break;
		case EO_GREATER	:
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_GREATER;
			break;
		case EO_EQUAL:
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_EQUAL;
			break;
		case EO_NOT_EQUAL:
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_NOT_EQUAL;
			break;
		case EO_OP_INC	:
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_OP_INC;
			break;
		case EO_OP_DEC	:
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_OP_DEC;
			break;
		case EO_PLUS:
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_PLUS;
			break;
		case EO_MINUS	:
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_MINUS;
			break;
		case EO_MULT	:
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_MULT;
			break;
		case EO_DIV		:
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_DIV;
			break;
		case EO_BOOL_AND:
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_BOOL_AND;
			break;
		case EO_BOOL_OR	:
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_BOOL_OR;
			break;
		default:
			ASSERT(0);
			FORMATLASTERROR(pTree->nLineNo, ERR_C_UNEXPECTED_ERR, "");
			return ERR_C_UNEXPECTED_ERR;
			break;
		}
	}
	else
	{
		ASSERT(0);
		return ERR_C_UNEXPECTED_ERR;
	}

	pstmtOut->nLineNo = pTree->nLineNo;

	return nRet;
}

int CSSCompiler::BCS_ClassMethodCall(SS_TREE_NODE *pID, PSTMT_TREE_NODE &pstmtOut)
{
	//
	// class member method call
	//
	TYPE_NAME_ID	typeID;
	SS_TREE_NODE	*pMemberVar;
	PSTMT_TREE_NODE	*pstmtMemberTail = &pstmtOut;
	STMT_TREE_NODE	*stmtClsMember = NULL;

	// Is var declared before?
	TypeInfo typeInfo = m_symTable.AddSymbolReference(pID->szName, pID->nLineNo);
	if (typeInfo.typeID == INVALID_TYPE_ID)
	{
		FORMATLASTERROR(pID->nLineNo, ERR_C_UNDECLARED_ID, pID->szName);
		return ERR_C_UNDECLARED_ID;
	}
	typeID =  typeInfo.typeID;

	if (typeInfo.nClsMemberVar != -1)
	{
		// It's a class member variable.
		stmtClsMember = newStmt(STMT_TREE_NODE::SNT_VAR);
		stmtClsMember->addr = m_symTable.GetSymbolMemLoc(SZ_THIS);
		stmtClsMember->nLineNo = pID->nLineNo;

		typeInfo = m_symTable.AddSymbolReference(SZ_THIS, pID->nLineNo);
		typeID = typeInfo.typeID;

		*pstmtMemberTail = stmtClsMember;
		pstmtMemberTail = &(stmtClsMember->child[0]);

		pMemberVar = pID;
	}
	else
	{
		// 
		stmtClsMember = newStmt(STMT_TREE_NODE::SNT_VAR);
		if (typeInfo.bReference && typeInfo.nClsMemberVar != -1)
			stmtClsMember->nodeType = STMT_TREE_NODE::SNT_VAR_REF;
		stmtClsMember->addr = m_symTable.GetSymbolMemLoc(pID->szName);
		stmtClsMember->nLineNo = pID->nLineNo;
		*pstmtMemberTail = stmtClsMember;
		pstmtMemberTail = &(stmtClsMember->child[0]);

		pMemberVar = pID->child[0];
	}

	// is class member correct?
	while (pMemberVar)
	{
		CClassDeclarationTable::ClassDecl	*classDecl;
		int				nMemberIndex;

		classDecl = m_pProcess->GetClassByID(typeID);
		ASSERT(classDecl);

		if (pMemberVar->child[0])
		{
			// class member variable?
			nMemberIndex = classDecl->GetMemberVarIndex(pMemberVar->szName);
			if (nMemberIndex == -1)
			{
				FORMATLASTERROR(pMemberVar->nLineNo, ERR_C_UNDECLARED_ID, pMemberVar->szName);
				return ERR_C_UNDECLARED_ID;
			}
			typeID = classDecl->vMemberVar[nMemberIndex].typeID;
		}
		else
		{
			// class member function
			nMemberIndex = classDecl->m_methodTable.GetFuncIDByName(pMemberVar->szName);
			if (nMemberIndex == -1)
			{
				FORMATLASTERROR(pMemberVar->nLineNo, ERR_C_UNDECLARED_ID, pMemberVar->szName);
				return ERR_C_UNDECLARED_ID;
			}
			nMemberIndex = MAKE_CLASS_FUNC_ID(typeID, nMemberIndex);
		}
		stmtClsMember = newStmt(STMT_TREE_NODE::SNT_VAR);
		stmtClsMember->addr = nMemberIndex;
		stmtClsMember->nLineNo = pMemberVar->nLineNo;

		*pstmtMemberTail = stmtClsMember;
		pstmtMemberTail = &(stmtClsMember->child[0]);
		pMemberVar = pMemberVar->child[0];
	}

	return ERR_OK;
}

int CSSCompiler::BCS_ClassVariable(SS_TREE_NODE *pID, PSTMT_TREE_NODE &pstmtOut)
{
	//
	// class member method call
	//
	TYPE_NAME_ID	typeID;
	SS_TREE_NODE	*pMemberVar;
	PSTMT_TREE_NODE	*pstmtMemberTail = &pstmtOut;
	STMT_TREE_NODE	*stmtClsMember = NULL;

	// Is var declared before?
	TypeInfo typeInfo = m_symTable.AddSymbolReference(pID->szName, pID->nLineNo);
	if (typeInfo.typeID == INVALID_TYPE_ID)
	{
		FORMATLASTERROR(pID->nLineNo, ERR_C_UNDECLARED_ID, pID->szName);
		return ERR_C_UNDECLARED_ID;
	}
	typeID =  typeInfo.typeID;


	if (typeInfo.nClsMemberVar != -1)
	{
		// It's a class member variable.
		stmtClsMember = newStmt(STMT_TREE_NODE::SNT_VAR);
		stmtClsMember->addr = m_symTable.GetSymbolMemLoc(SZ_THIS);
		stmtClsMember->nLineNo = pID->nLineNo;

		typeInfo = m_symTable.AddSymbolReference(SZ_THIS, pID->nLineNo);
		typeID = typeInfo.typeID;

		*pstmtMemberTail = stmtClsMember;
		pstmtMemberTail = &(stmtClsMember->child[0]);

		pMemberVar = pID;
	}
	else
	{
		// 
		stmtClsMember = newStmt(STMT_TREE_NODE::SNT_VAR);
		if (typeInfo.bReference)
			stmtClsMember->nodeType = STMT_TREE_NODE::SNT_VAR_REF;
		stmtClsMember->addr = m_symTable.GetSymbolMemLoc(pID->szName);
		stmtClsMember->nLineNo = pID->nLineNo;
		*pstmtMemberTail = stmtClsMember;
		pstmtMemberTail = &(stmtClsMember->child[0]);

		pMemberVar = pID->child[0];
	}

	// is class member variable correct?
	while (pMemberVar)
	{
		CClassDeclarationTable::ClassDecl	*classDecl;
		int				nMemberIndex;

		classDecl = m_pProcess->GetClassByID(typeID);
		ASSERT(classDecl);

		// class member variable?
		nMemberIndex = classDecl->GetMemberVarIndex(pMemberVar->szName);
		if (nMemberIndex == -1)
		{
			FORMATLASTERROR(pMemberVar->nLineNo, ERR_C_UNDECLARED_ID, pMemberVar->szName);
			return ERR_C_UNDECLARED_ID;
		}
		typeID = classDecl->vMemberVar[nMemberIndex].typeID;

		stmtClsMember = newStmt(STMT_TREE_NODE::SNT_VAR);
		stmtClsMember->addr = nMemberIndex;
		stmtClsMember->nLineNo = pMemberVar->nLineNo;

		*pstmtMemberTail = stmtClsMember;
		pstmtMemberTail = &(stmtClsMember->child[0]);
		pMemberVar = pMemberVar->child[0];
	}

	pID->typeId = typeID;

	return ERR_OK;
}


int CSSCompiler::BCS_Stmt(SS_TREE_NODE *pTree, PSTMT_TREE_NODE &pstmtOut)
{
	int					nRet;

	pstmtOut = new STMT_TREE_NODE;
	pstmtOut->nLineNo = pTree->nLineNo;

	// ASSERT(pTree->nodeType == NT_STMT);
	switch (pTree->kind.stmt)
	{
	case SK_NULL:
		pstmtOut->nodeType = STMT_TREE_NODE::SNT_BLOCK;
		break;
	case SK_IF:
		{
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_IF;

			STMT_TREE_NODE		*pCondition = NULL, *ptruestmt = NULL, *pelsestmt = NULL;
			// 1. check expression
			nRet = BCS_Expression(pTree->child[0], pCondition);
			ASSERT(nRet == ERR_OK);
			if (nRet != ERR_OK)
				return nRet;

			// 2. Check statement
			m_symTable.Push(false);
			nRet = BCS_Tree(pTree->child[1], ptruestmt);
			ASSERT(nRet == ERR_OK);
			if (nRet != ERR_OK)
				return nRet;
			m_symTable.Popup();

			// 3. Check else statement
			if (pTree->child[2])
			{
				m_symTable.Push(false);
				nRet = BCS_Tree(pTree->child[2], pelsestmt);
				ASSERT(nRet == ERR_OK);
				if (nRet != ERR_OK)
					return nRet;
				m_symTable.Popup();
			}
			pstmtOut->child[0] = pCondition;
			pstmtOut->child[1] = ptruestmt;
			pstmtOut->child[2] = pelsestmt;
		}
		break;
	case SK_WHILE:
		{
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_WHILE;
			STMT_TREE_NODE		*pCondition = NULL, *ptruestmt = NULL;

			// 1. check expression
			nRet = BCS_Expression(pTree->child[0], pCondition);
			ASSERT(nRet == ERR_OK);
			if (nRet != ERR_OK)
				return nRet;

			// 2. Check statement
			m_symTable.Push(false);
			nRet = BCS_Tree(pTree->child[1], ptruestmt);
			ASSERT(nRet == ERR_OK);
			if (nRet != ERR_OK)
				return nRet;
			m_symTable.Popup();

			pstmtOut->child[0] = pCondition;
			pstmtOut->child[1] = ptruestmt;
		}
		break;
	case SK_DO_WHILE:
		{
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_DO_WHILE;
			STMT_TREE_NODE		*pCondition = NULL, *ptruestmt = NULL;

			// 1. Check statement
			m_symTable.Push(false);
			nRet = BCS_Tree(pTree->child[1], ptruestmt);
			ASSERT(nRet == ERR_OK);
			if (nRet != ERR_OK)
				return nRet;
			m_symTable.Popup();

			// 2. check expression
			nRet = BCS_Expression(pTree->child[0], pCondition);
			ASSERT(nRet == ERR_OK);
			if (nRet != ERR_OK)
				return nRet;

			pstmtOut->child[0] = pCondition;
			pstmtOut->child[1] = ptruestmt;
		}
		break;
	case SK_ASSIGN:
		{
			STMT_TREE_NODE		*pExpression = NULL;
			SS_TREE_NODE		*pID = pTree->child[0];

			if (pID->kind.exp == EK_ID_REF)
				pstmtOut->nodeType = STMT_TREE_NODE::SNT_ASSIGN_REF;
			else
				pstmtOut->nodeType = STMT_TREE_NODE::SNT_ASSIGN;

			// 1. left node should be an ID
			if (pID->child[0])
			{
				// class variable...
				BCS_ClassVariable(pID, pstmtOut->child[0]);
			}
			else
				pstmtOut->addr = m_symTable.GetSymbolMemLoc(pID->szName);

			// 4. check expression
			nRet = BCS_Expression(pTree->child[1], pExpression);
			ASSERT(nRet == ERR_OK);
			if (nRet != ERR_OK)
				return nRet;

			pstmtOut->child[1] = pExpression;
		}
		break;
	case SK_RETURN:
		{
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_RETURN;
			STMT_TREE_NODE		*pExpression = NULL;
			SS_TREE_NODE		*pNode;

			// 1. check return expression 
			pNode = pTree->child[0];
			if (pNode)
			{
				nRet = BCS_Expression(pNode, pExpression);
				ASSERT(nRet == ERR_OK);
				if (nRet != ERR_OK)
					return nRet;

				// assign return expression in stack pos...
				STMT_TREE_NODE	*pAssign;
				pAssign = newStmt(STMT_TREE_NODE::SNT_ASSIGN);
				pAssign->nLineNo = pNode->nLineNo;
				pAssign->addr = m_symTable.GetCurFuncStackSize() + 1;
				pAssign->child[1] = pExpression;
				pstmtOut->child[0] = pAssign;
			}
		}
		break;
	case SK_BLOCK:
		if (pTree->child[0])
		{
			m_symTable.Push();
			STMT_TREE_NODE		*pstmtChild = NULL;
			int		nSizeOld = m_symTable.GetCurFuncStackSize();
			nRet = BCS_Tree(pTree->child[0], pstmtChild);
			ASSERT(nRet == ERR_OK);
			if (nRet != ERR_OK)
				return nRet;
			if (nSizeOld != m_symTable.GetCurFuncStackSize())
			{
				pstmtOut->nodeType = STMT_TREE_NODE::SNT_POPUP_STACK_N;
				pstmtOut->addr = m_symTable.GetCurFuncStackSize() - nSizeOld;
			}
			else
			{
				pstmtOut->nodeType = STMT_TREE_NODE::SNT_BLOCK;
			}
			pstmtOut->child[0] = pstmtChild;
			m_symTable.Popup();
		}
		break;
	case SK_DELETE:
		{
			pstmtOut->nodeType = STMT_TREE_NODE::SNT_DELETE;
			STMT_TREE_NODE		*pExpression = NULL;
			// 1. check return expression 
			if (pTree->child[0])
			{
				nRet = BCS_Expression(pTree->child[0], pExpression);
				ASSERT(nRet == ERR_OK);
				if (nRet != ERR_OK)
					return nRet;

				pstmtOut->child[0] = pExpression;
			}
		}
		break;
	default:
		ASSERT(0);
		FORMATLASTERROR(pTree->nLineNo, ERR_C_UNEXPECTED_ERR, "");
		return ERR_C_UNEXPECTED_ERR;
		break;
	}
// 
// 		pTree = pTree->sibling;
// 	}

	return ERR_OK;
}

void PrintSapces(int nIndent);

void PrintStmtCodeTree(STMT_TREE_NODE *t, int &nIndent)
{
	nIndent++;

	while (t != NULL)
	{
		printf("line: %2d  ", t->nLineNo);
		PrintSapces(nIndent);
		switch (t->nodeType)
		{
		case STMT_TREE_NODE::SNT_UNKNOWN:
			printf("SNT_UNKNOWN\n");
			break;
		case STMT_TREE_NODE::SNT_BLOCK:
			printf("SNT_BLOCK\n");
			break;
		case STMT_TREE_NODE::SNT_IF:
			printf("if\n");
			break;
		case STMT_TREE_NODE::SNT_WHILE:
			printf("while\n");
			break;
		case STMT_TREE_NODE::SNT_DO_WHILE:
			printf("do while\n");
			break;
		case STMT_TREE_NODE::SNT_ASSIGN:
			printf("=\n");
			break;
		case STMT_TREE_NODE::SNT_ASSIGN_REF:
			printf("ref =\n");
			break;
		case STMT_TREE_NODE::SNT_RETURN:
			printf("return\n");
			break;
		case STMT_TREE_NODE::SNT_LT_EQ:		// <=
			printf("<=\n");
			break;
		case STMT_TREE_NODE::SNT_GT_EQ:		// >=
			printf(">=\n");
			break;
		case STMT_TREE_NODE::SNT_LITTLE:		// <
			printf("<\n");
			break;
		case STMT_TREE_NODE::SNT_GREATER:	// >
			printf(">\n");
			break;
		case STMT_TREE_NODE::SNT_EQUAL:		// ==
			printf("==\n");
			break;
		case STMT_TREE_NODE::SNT_NOT_EQUAL:	// !=
			printf("!=\n");
			break;
		case STMT_TREE_NODE::SNT_OP_INC:		// ++
			printf("++\n");
			break;
		case STMT_TREE_NODE::SNT_OP_DEC:		// --
			printf("--\n");
			break;
		case STMT_TREE_NODE::SNT_PLUS:		// +
			printf("+\n");
			break;
		case STMT_TREE_NODE::SNT_MINUS:		// -
			printf("-\n");
			break;
		case STMT_TREE_NODE::SNT_MULT:		// *
			printf("*\n");
			break;
		case STMT_TREE_NODE::SNT_DIV:		// /
			printf("/\n");
			break;
		case STMT_TREE_NODE::SNT_BOOL_AND:		// /
			printf("&&\n");
			break;
		case STMT_TREE_NODE::SNT_BOOL_OR:		// /
			printf("||\n");
			break;

		case STMT_TREE_NODE::SNT_NEW_STACK_VAR:		// new a variable: and push to stack.
			printf("new Variable\n");
			break;
		case STMT_TREE_NODE::SNT_POPUP_STACK_N:
			printf("Popup N: %d\n", t->addr);
			break;

		case STMT_TREE_NODE::SNT_FUNC_CALL:
			printf("Call func: %d\n", t->addr);
			break;
		case STMT_TREE_NODE::SNT_VAR:
			if (SSM_IS_GLOBAL_STACK_ADDR(t->addr))
				printf("Global Id: %d\n", SSM_GET_STACK_ADDR(t->addr));
			else
				printf("Id: %d\n", SSM_GET_STACK_ADDR(t->addr));
			break;
		case STMT_TREE_NODE::SNT_VAR_REF:
			printf("ref Id: %d\n", SSM_GET_STACK_ADDR(t->addr));
			break;
		case STMT_TREE_NODE::SNT_CONST_INT:
			printf("const int: %d\n", t->addr);
			break;
		case STMT_TREE_NODE::SNT_CONST_STR:
			printf("const str: %s\n", t->addr);
			break;
		case STMT_TREE_NODE::SNT_NEW:
			printf("new: %d\n", t->addr);
			break;
		case STMT_TREE_NODE::SNT_DELETE:
			printf("delete: %X\n", t->addr);
			break;
		default:
			printf("Unknown Node type: %d\n", t->nodeType);
			break;
		}

		for (int i = 0; i < MAX_CHILDREN; i++)
			PrintStmtCodeTree(t->child[i], nIndent);

		t = t->sibling;
	}

	nIndent--;
}


