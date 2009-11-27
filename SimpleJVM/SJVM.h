#pragma once

#include "SyntaxTree.h"
#include "VMStatment.h"

enum SIMPLE_JAVA_COMPILE_ERROR
{
	ERR_C_UNDECLARED_ID		= 3000,
	ERR_C_UNDECLARED_TYPE,
	ERR_C_UNDECLARED_METHOD,
	ERR_C_UNDECLARED_VAR,
	ERR_C_UNDECLARED_MEMBER_VAR,
	ERR_C_PARSE,
	ERR_C_REDEFINITION_ID,
	ERR_C_OP_TYPE_NOT_MATCH,
	ERR_C_OP_REQUIRE_INT_STR_TYPE,
	ERR_C_OP_REQUIRE_INT_TYPE,
	ERR_C_OP_REQUIRE_BOOL_TYPE,
	ERR_C_OP_MUST_TARGET_VAR,
	ERR_C_REDEFINITION_TYPE,
	ERR_C_REDEFINITION_METHOD,
	ERR_C_CONVERT_VALUE,
	ERR_C_UNEXPECTED_ERR,
	ERR_C_FUNC_RETURN_VALUE,
	ERR_C_NEW_WRONG_TYPE,
	ERR_C_NOT_CLASS_TYPE,
	ERR_C_CAN_NOT_CALL_NONE_STATIC_MEMBER,
	ERR_C_REQUIRE_LOGICAL_CONDITION,
	ERR_C_METHOD_CALL_PARAMETER_COUNT,
	ERR_C_CLASS_TYPE_NOT_VAR,
	ERR_C_CLASS_HIERARCHY_INCONSISTENT,
	ERR_C_OVERRIDE_METHOD_RET_DIFF,
	ERR_C_DUMPLICATE_METHOD,
	ERR_C_CONSTRUCTOR_MUST_BE_FIRST_STMT,
	ERR_C_CONSTRUCTOR_OBJ_UNDEFINED,
	ERR_C_CONSTRUCTOR_NO_RETRUN_VALUE,
	ERR_C_RETRUN_TYPE_MISSING,
	ERR_C_NO_THIS_SUPER_IN_STATIC_METHOD,

	ERR_R_UNDECLARED_FUNCTION,		// undeclared function
	ERR_R_FUNC_PARAM_COUNT,			// incorrect function parameter count is passed
	ERR_R_FUNC_PARAM_TYPE,			// incorrect function parameter is passed
	ERR_R_FUNC_PARAM,
};


enum TypeIdPredefined
{
	TID_INVALID = 0,
	TID_VOID,
	TID_CHAR,
	TID_BOOLEAN,
	TID_SHORT,
	TID_INT,
	TID_NULL,
	TID_STRING,
};

inline bool IsSimpleType(TypeId typeId) { return typeId < TID_NULL; }

#define JOBJ_NULL		0

#define SZ_THIS		"this"
#define	SZ_SUPER	"super"
#define SZ_NULL		"null"

typedef vector<TypeId>		VecParamType;
typedef vector<int>			VecInt;
typedef vector<tstring>		VecString;

typedef int					Variable;
typedef vector<Variable>	VecVariable;
typedef map<tstring, int>	MapStrIndex;

class CMemberMethod
{
public:
	CMemberMethod() {
		siMethod = SI_INVALID;
		pNativeFunc = NULL;
		typeIDRet = TID_INVALID;
		pBodyNode = NULL;
		bStatic = false;
	}

public:
	int						nLineNo;
	int						nVirtualIndex;
	VecParamType			vParamType;
	TypeId					typeIDRet;
	bool					bStatic;		// Is Static function?
	string					strName;

	STreeNode				*pBodyNode;

	LPVOID					pNativeFunc;

	stmt_index				siMethod;

public:
	void AddParam(TypeId typeId) { vParamType.push_back(typeId); }

	bool IsSameNameAndParameter(LPCSTR szMethodName, VecParamType &vParameter);

	bool IsSameNameAndParameter(CMemberMethod *pMethod);

	bool IsSame(CMemberMethod *pMethod);

};

typedef vector<CMemberMethod *>			VecMethod;

class CMemberVar
{
public:
	TypeId					typeId;
	bool					bStatic;		// Is Static variable?
	int						nAddr;			// Static address or index in CJObject.
	string					strName;

};
typedef vector<CMemberVar *>		VecMemberVar;

class CClass
{
public:
	CClass() {
		pClassParent = NULL;
		typeId = TID_INVALID;
	}

public:
	TypeId					typeId;
	CClass					*pClassParent;
	VecMethod				vMethod;
	VecMethod				vVirtualMethod;
	CMemberMethod			methodVarInit;
	VecMemberVar			vMemberVar;
	string					strName;

	// typedef vector<class CClass *>		VecClass;
	// VecClass				m_vChildClass;

public:
	int AddMethod(CMemberMethod *pMethod);

	int AddMemberVar(TypeId typeId, LPCSTR szVarName, bool bStatic, int nStaticAddr);

	int GetTotalMemberCount();

	bool IsMemberVarStatic(LPCSTR szVarName);
	CMemberVar *GetMemberVar(LPCSTR szVarName);

	CMemberMethod *GetMethod(LPCSTR szMethodName, VecParamType &vParameter);

	CMemberMethod *GetMethod(size_t nMethodIndex) {
		ASSERT(nMethodIndex < vVirtualMethod.size());
		if (nMethodIndex >= vVirtualMethod.size())
			return NULL;
		return vVirtualMethod[nMethodIndex];
	}
	CMemberMethod *GetVirtualMethod(size_t nMethodIndex) {
		ASSERT(nMethodIndex < vVirtualMethod.size());
		if (nMethodIndex >= vVirtualMethod.size())
			return NULL;
		return vVirtualMethod[nMethodIndex];
	}

	CMemberMethod *GetVarInitMethod() { return &methodVarInit; }

	CMemberMethod *GetNoParameterConstructor();
	bool HasConstructor();

	int UpdateVirtualMethodTable(string &strMsg);

	void UpdateMemberVarAddress();

};
typedef vector<CClass *>	VecClass;

class CClassTable
{
public:
	int AddClass(LPCSTR szName);

	CClass *GetClass(LPCSTR szClassName);
	CClass *GetClass(TypeId typeId);

	bool IsClassDeclared(LPCSTR szClassName);

	bool IsSubClassOf(TypeId typeChild, TypeId typeParent);

	CMemberMethod *GetEntryPoint();
	CMemberMethod *GetClassStaticVarInitMethod() { return &m_classStaticVarInitMetod; }

	int StandardizeClassMember(string strMsg);

#ifdef _DEBUG
	void Print(class CSJVM *pVm);
#endif

	VecClass				m_vClass;
	CMemberMethod			m_classStaticVarInitMetod;

};

#include "VarDeclStack.h"

class CJObject
{
public:
	TypeId				m_typeId;
	VecInt				m_vMemberVar;

	CJObject(TypeId typeId, int nMemberVarCount) { 
		m_typeId = typeId;
		m_vMemberVar.resize(nMemberVarCount, 0);
	}
	virtual ~CJObject() { }

	int GetValue(size_t nPos) {
		ASSERT(nPos < m_vMemberVar.size());
		if (nPos >= m_vMemberVar.size())
			return 0;
		return m_vMemberVar[nPos];
	}

	void SetValue(size_t nPos, int nValue) {

		ASSERT(nPos < m_vMemberVar.size());
		if (nPos >= m_vMemberVar.size())
			return;
		m_vMemberVar[nPos] = nValue;
	}

	int *GetAddress(size_t nPos) {
		ASSERT(nPos < m_vMemberVar.size());
		if (nPos >= m_vMemberVar.size())
			return NULL;
		return &(m_vMemberVar[nPos]);
	}

};

class CStrJObject : public CJObject
{
public:
	CStrJObject(LPCTSTR szStr, bool bCopy = true);
	virtual ~CStrJObject();

	LPCTSTR GetStrValue() { return m_szValue; }

protected:
	LPCTSTR				m_szValue;
	bool				m_bCopy;

};

typedef map<int, CJObject *>	MapJObject;
typedef vector<CJObject *>	VecJObject;

class CObjectMgr
{
public:
	CObjectMgr() { 
		// Take 0 as invalid address
		m_vJObjects.push_back(NULL);
	}
	virtual ~CObjectMgr();

	int New(TypeId typeId, int nMemberVarCount);

	int NewString(LPCTSTR szString, bool bCopy = true);

	void Free(int nPos, CClassTable &clsTable);

	CJObject *Get(int nPos);

	void InitAddConstString(MapStrIndex &mapStr);

protected:
	VecJObject				m_vJObjects;
	list<int>				m_vFreeJObjs;

};

class CVMStack
{
public:
	struct Item {
		int			nValue;
		TypeId		typeId;
	};
	typedef vector<Item>		VecItems;

	void Insert(int nPos, int nValue, TypeId typeId) {
		Item i; i.nValue = nValue; i.typeId = typeId; m_vItems.insert(m_vItems.begin() + nPos, i);
	}

	void Push(int nValue, TypeId typeId) { 
		Item i; i.nValue = nValue; i.typeId = typeId; m_vItems.push_back(i); }
	void Pop() { m_vItems.pop_back(); }

	void Push(CVMStack &stack) {
		m_vItems.insert(m_vItems.end(), stack.m_vItems.begin(), stack.m_vItems.end());
	}

	void Resize(size_t s) { m_vItems.resize(s); }

	Item &Back() { ASSERT(m_vItems.size() > 0); return m_vItems.back(); }

	int GetValue(size_t nOffset)	{
		ASSERT(nOffset >= 0 && nOffset < (int)m_vItems.size());
		return m_vItems[nOffset].nValue;
	}

	Item &Get(size_t nOffset)	{
		ASSERT(nOffset >= 0 && nOffset < (int)m_vItems.size());
		return m_vItems[nOffset];
	}

	size_t size() { return m_vItems.size(); }

protected:
	VecItems			m_vItems;

};

class CSJVM
{
public:
	CSJVM(void);
	virtual ~CSJVM(void);

	void Reset();

	int CompileFiles(VecString &vFiles);
	int Compile(LPCTSTR szFile);

	int Run();

	void FinishedInitSysModule();


public:
	//
	// Interfaces for Simple JNI
	//

	int RegisterNativeAPI(LPCSTR szClassName, LPCSTR szFuncName, LPVOID funcPtr, TypeId retValue, ...);

	int CallNativeAPI(CMemberMethod *pMethod, CVMStack &stackParameter);

	LPVOID StackVarToNativeValue(int nvmStackPtr, TypeId paramType);

	CJObject *NewJObject(int nClassID);

protected:
	//
	// Methods for compiling Simple JAVA
	//

	//
	// Build symbol info and check for syntax error
	//

	// Build class name, class method, class variable symbols.
	int PreBuildSymbolInfo();
	int PreBSI_AddClassDefinition();
	int PreBSI_AddClassInformation();
	int PreBSI_AddClassMember(STreeNode *pTree, CClass *pClass);

	int BuildSymbolInfo();
	int BSI_Tree(STreeNode *pTree);
	int BSI_ClassDefinition(STreeNode *pTree);
	int BSI_FunctionDecl(STreeNode *pTree, CClass *pClass);
	int BSI_ConstructorCheck(STreeNode *pTree, CClass *pClass);
	int BSI_VariableDecl(STreeNode *pTree, CClass *pClass = NULL);
	int BSI_Expression(STreeNode *pTree);
	int BSI_ExpressionFunctionCall(STreeNode *pTree);

	int BSI_GetCalledMethod(CClass *pClass, LPCSTR szMethodName, VecParamType &vParameters, CMemberMethod **ppMethod, int nLineNo);
	int BSI_InsertClassVarInitNode(STreeNode *pTree);

	int CanCovertRightToLeftType(TypeId typeIdLeft, TypeId typeIdRight);

protected:
	//
	// Build code statement
	//
	int BuildCodeStmt();

	int BCS_ClassDefinition(STreeNode *pTree);
	int BCS_Tree(STreeNode *pTree, stmt_index &siOut);
	int BCS_FunctionDefinition(STreeNode *pTree, CClass *pClass);
	int BCS_ClassVariable(STreeNode *pTree, CClass *pClass);
	int BCS_VarDeclaration(STreeNode *pTree, stmt_index	&siOut);
	int BCS_Expression(STreeNode *pTree, stmt_index &siOut);

protected:
	//
	// Execute statement
	//
	int Execute(stmt_index siEntry);

protected:
	int ExecuteExpression(stmt_index siExpr);
	int *ExeExprGetVarAddr(stmt_index siExprVar);

	int NewJObject(StmtExpression &expr);
	void FreeJObject(int nJObject);

	int CallMethod(TypeId typeId, int nJObject, int nMethodIndex, CVMStack &stackParameter);

	int GetStaticVarValue(size_t nAddress);
	int GetJObjectMemberVarValue(int nJobject, int nMember);

	int *GetStaticVarAddress(size_t nAddress);
	int *GetJObjectMemberVarAddr(int nJobject, int nMember);

protected:

	void FormatLastError(int nLine, int nError, LPCSTR szStr, int nLineSrc);

public:
	// Get TypeId by class name, or system predefined name.
	TypeId GetTypeIdByName(LPCSTR szTypeName);

	LPCSTR GetTypeNameStrByID(TypeId id);

protected:
	// for debug
#ifdef _DEBUG
	void Print();
	friend class CClassTable;
#endif

protected:
	// All classes
	CClassTable					m_classTable;

	// Statement image
	CStmtImage					m_stmtImage;

	// Stack of virtual machine
	CVMStack					m_stack;
	size_t						m_bp;		// base pointer of current function call.
	int							m_valueMethodReturn;
	VecInt						m_vStaticVars;

	// JObject manager
	CObjectMgr					m_jobjMgr;

	bool						m_bFinishedInitSysModule;

protected:
	//
	// Compile for current file.
	//
	STreeNode					*m_syntaxTree;

	CVarDeclStack				m_varDeclStack;

	string						m_strLastErr;

	MapStrIndex					m_mapStrings;

};
