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
	ERR_C_BREAK_CONTINUE_MUST_IN_LOOP,

	ERR_R_NO_ENTRY_POINT,
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

#define TID_EXCEPTION		CSjobjException::m_sTypeId

inline bool IsSimpleType(TypeId typeId) { return typeId < TID_NULL; }

#define JOBJ_NULL		0

#define SZ_THIS		"this"
#define	SZ_SUPER	"super"
#define SZ_NULL		"null"

#define SJNI_API			__stdcall

typedef int					jobject;

typedef vector<TypeId>		VecParamType;
typedef vector<int>			VecInt;
typedef vector<bool>		VecBool;
typedef vector<tstring>		VecString;

typedef map<tstring, int>	MapStrIndex;

struct Variable {
	int			nValue;
	TypeId		typeId;
};
typedef vector<Variable>	VecVariable;

struct StaticVar {
	bool	bInitialized;
	TypeId	typeId;
	int		nValue;
};
typedef vector<StaticVar>	VecStaticVar;

class CJObjectNewer
{
public:
	CJObjectNewer() { }
	virtual ~CJObjectNewer() { }

	virtual class CJObject *New() = 0;

};

class CMemberMethod
{
public:
	CMemberMethod() {
		nLineNo = -1;
		nVirtualIndex = -1;
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
		nLineNo = -1;
		pClassParent = NULL;
		typeId = TID_INVALID;
		bFreeJObjNewer = false;
		nTotalMemberVarCount = 0;
	}
	virtual ~CClass() {
		if (bFreeJObjNewer)
			delete pJObojNewer;
	}

public:
	int						nLineNo;
	TypeId					typeId;
	CClass					*pClassParent;
	VecMethod				vMethod;
	VecMethod				vVirtualMethod;
	CMemberMethod			methodVarInit;
	VecMemberVar			vMemberVar;
	int						nTotalMemberVarCount;
	string					strName;
	CJObjectNewer			*pJObojNewer;
	bool					bFreeJObjNewer;

public:
	void AddNativeDefaultConstructor();

	// Return the index of method, -1 for error.
	int AddNativeMethod(bool bStatic, LPCSTR szMethodName, LPVOID funcPtr, TypeId typeIdReturn, ...);

	int AddMethod(CMemberMethod *pMethod);

	int AddMemberVar(TypeId typeId, LPCSTR szVarName, bool bStatic, int nStaticAddr);

	void UpdateTotalMemberCount();

	bool IsMemberVarStatic(LPCSTR szVarName);
	CMemberVar *GetMemberVar(LPCSTR szVarName);

	CMemberMethod *GetMethod(LPCSTR szMethodName, VecParamType &vParameter);

	CMemberMethod *GetMethod(size_t nMethodIndex) {
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
	CClassTable();
	virtual ~CClassTable();

	int AddClass(LPCSTR szName, int nLineNo = -1, CJObjectNewer *pJObjNewer = NULL);

	CClass *GetClass(LPCSTR szClassName);
	CClass *GetClass(TypeId typeId);

	bool IsClassDeclared(LPCSTR szClassName);

	bool IsSubClassOf(TypeId typeChild, TypeId typeParent);

	CMemberMethod *GetEntryPoint();
	CMemberMethod *GetClassStaticVarInitMethod() { return &m_classStaticVarInitMetod; }

	int StandardizeClassMember(string strMsg);

	void UpdateJObjNewer();

#ifdef _DEBUG
	void Print(class CSJVM *pVm);
#endif

	VecClass				m_vClass;
	CMemberMethod			m_classStaticVarInitMetod;

	typedef list<CJObjectNewer*>		ListJObjNewer;

	ListJObjNewer			m_listJObjNewer;

};

#include "VarDeclStack.h"

class CJObject
{
public:
	TypeId				m_typeId;
	VecVariable			m_vMemberVar;

	CJObject(TypeId typeId, int nMemberVarCount) { 
		m_typeId = typeId;
		Variable	var;
		var.nValue = 0;
		var.typeId = TID_INVALID;
		m_vMemberVar.resize(nMemberVarCount, var);
	}
	virtual ~CJObject() { }

	int GetValue(size_t nPos) {
		ASSERT(nPos < m_vMemberVar.size());
		if (nPos >= m_vMemberVar.size())
			return 0;
		return m_vMemberVar[nPos].nValue;
	}

	void SetValue(size_t nPos, int nValue) {

		ASSERT(nPos < m_vMemberVar.size());
		if (nPos >= m_vMemberVar.size())
			return;
		m_vMemberVar[nPos].nValue = nValue;
	}

	int *GetAddress(size_t nPos) {
		ASSERT(nPos < m_vMemberVar.size());
		if (nPos >= m_vMemberVar.size())
			return NULL;
		return &(m_vMemberVar[nPos].nValue);
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
		m_nAllocCount = 0;
	}
	virtual ~CObjectMgr();

	void Reset();

	int New(CClass *pClass);

	int NewString(LPCTSTR szString, bool bCopy = true);

	CJObject *Get(int nPos);

	void InitAddConstString(MapStrIndex &mapStr);

protected:
	friend class CSJVMGarbageCoolector;
	friend class CSjobjUnitTest;

	VecJObject				m_vJObjects;
	list<int>				m_vFreeJObjs;
	int						m_nAllocCount;

};

class CVMStack
{
public:
	void Insert(int nPos, int nValue, TypeId typeId) {
		Variable i; i.nValue = nValue; i.typeId = typeId; m_vItems.insert(m_vItems.begin() + nPos, i);
	}

	void Push(int nValue, TypeId typeId) { 
		Variable i; i.nValue = nValue; i.typeId = typeId; m_vItems.push_back(i); }
	void Pop() { m_vItems.pop_back(); }
	void PopN(size_t n) {
		ASSERT(n < m_vItems.size());
		m_vItems.resize(m_vItems.size() - n);
	}

	void Push(class CVMParameters &parameters);

	void Resize(size_t s) { m_vItems.resize(s); }

	Variable &Back() { ASSERT(m_vItems.size() > 0); return m_vItems.back(); }

	int GetValue(size_t nOffset)	{
		ASSERT(nOffset >= 0 && nOffset < (int)m_vItems.size());
		return m_vItems[nOffset].nValue;
	}

	Variable &Get(size_t nOffset)	{
		ASSERT(nOffset >= 0 && nOffset < (int)m_vItems.size());
		return m_vItems[nOffset];
	}

	size_t size() { return m_vItems.size(); }

protected:
	friend class CSJVM;
	VecVariable			m_vItems;

};

class CVMParameters
{
public:
	enum { COUNT_MAX		= 32 };

	CVMParameters() { m_nCount = 0; }

	void Push(int nValue, TypeId typeId) { 
		ASSERT(m_nCount < COUNT_MAX);
		m_vItems[m_nCount].nValue = nValue;
		m_vItems[m_nCount].typeId = typeId;
		m_nCount++;
	}

public:
	Variable			m_vItems[COUNT_MAX];
	size_t				m_nCount;

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

	int GarbageCollect();

	void FinishedInitSysModule();

public:
	//
	// Interfaces for Simple JNI
	//

	int RegisterNativeAPI(LPCSTR szClassName, LPCSTR szFuncName, LPVOID funcPtr, TypeId retValue, ...);

	CJObject *NewJObject(TypeId typeId, jobject &jobj);
	jobject NewString(LPCTSTR szString, bool bCopy) { return m_jobjMgr.NewString(szString, bCopy); }
	CJObject *GetJObject(jobject jobj) { return m_jobjMgr.Get(jobj); }

	int AddClass(LPCSTR szClassName, int nLineNo = -1, CJObjectNewer *pJObjNewer = NULL) { return m_classTable.AddClass(szClassName, nLineNo, pJObjNewer); }
	CClass *GetClass(LPCSTR szClassName) { return m_classTable.GetClass(szClassName); }
	CClassTable &GetClassTable() { return m_classTable; }

	int CallMethod(TypeId typeId, int nJObject, int nMethodIndex, CVMParameters &parameters);

	void ThrowException(LPCTSTR szException);

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
	int PreBSI_AddMemberVarDecl(STreeNode *pTree, CClass *pClass = NULL);

	int BuildSymbolInfo();
	int BSI_Tree(STreeNode *pTree);
	int BSI_ClassDefinition(STreeNode *pTree);
	int BSI_MemberVariable(STreeNode *pTree, CClass *pClass);
	int BSI_MethodDeclare(STreeNode *pTree, CClass *pClass);
	int BSI_ConstructorCheck(CMemberMethod *pMethod, STreeNode *pBody, CClass *pClass);
	int BSI_LocalVariable(STreeNode *pTree);
	int BSI_Expression(STreeNode *pTree);
	int BSI_ExpressionFunctionCall(STreeNode *pTree);

	int BSI_GetCalledMethod(CClass *pClass, LPCSTR szMethodName, VecParamType &vParameters, CMemberMethod **ppMethod, int nLineNo);

	int CanCovertRightToLeftType(TypeId typeIdLeft, TypeId typeIdRight);

protected:
	//
	// Build code statement
	//
	int BuildCodeStmt();

	int BCS_ClassDefinition(STreeNode *pTree);
	int BCS_Tree(STreeNode *pTree, stmt_index &siOut);
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
	StaticVar *ExeExprGetStaticVar(stmt_index siExprVar);

	int ExeNewJObject(StmtExpression &expr);

	int CallNativeAPI(CMemberMethod *pMethod, jobject thiz, CVMParameters &parameters);

	int GetStaticVarValue(size_t nAddress);
	int GetJObjectMemberVarValue(int nJobject, int nMember);

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

	class CAutoLoopState {
	public:
		CAutoLoopState(bool &bState) {
			m_pState = &bState;
			m_bOldState = bState;
			bState = true;
		}
		~CAutoLoopState() { *m_pState = m_bOldState; }

		bool		*m_pState;
		bool		m_bOldState;

	};

	friend class CSjobjUnitTest;
	friend class CSJVMGarbageCoolector;

protected:
	enum EXE_ACTION
	{
		EXE_GO_ON,
		EXE_LOOP_CONTINUE,
		EXE_LOOP_BREAK,
		EXE_FUNC_RETURN,
		EXE_EXCEPTION,
	};

	// All classes
	CClassTable					m_classTable;
	size_t						m_nSysClassCount;

	// Statement image
	CStmtImage					m_stmtImage;

	// Stack of virtual machine
	CVMStack					m_stack;
	size_t						m_bp;		// base pointer of current function call.
	int							m_valueMethodReturn;

	int							m_nCallStackDeep;

	// Static variables
	VecStaticVar				m_vStaticVars;
	size_t						m_nSysStaticVarsCount;

	EXE_ACTION					m_exeAction;
	TypeId						m_typeIdException;
	jobject						m_jobjException;

	// JObject manager
	CObjectMgr					m_jobjMgr;

	bool						m_bFinishedInitSysModule;

protected:
	//
	// Compile for current file.
	//
	STreeNode					*m_syntaxTree;

	CVarDeclStack				m_varDeclStack;

	tstring						m_strLastErr;

	MapStrIndex					m_mapStrings;
	bool						m_bsiInLoop;

};

extern CSJVM			g_sjvm;

#define DECLARE_JNI_OBJ(_Class)								\
	public:												\
	static void RegisterClassType();					\
	static LPCSTR			CLASS_NAME;					\
	static TypeId			m_sTypeId;					\
	class CObjNewer : public CJObjectNewer				\
	{													\
	public:												\
		CObjNewer() : CJObjectNewer() { }		\
		CJObject *New() {								\
			return new _Class();						\
		}												\
	};


#define IMPLENENT_JNI_OBJ(_Class, _ClassName)			\
	TypeId	_Class::m_sTypeId		= 0;				\
	LPCSTR	_Class::CLASS_NAME		= _ClassName;		\
	void _Class::RegisterClassType() {					\
	int nRet;											\
	nRet = g_sjvm.AddClass(_Class::CLASS_NAME, -1, new CObjNewer());	\
	ASSERT(nRet == ERR_OK);								\
	m_sTypeId = g_sjvm.GetTypeIdByName(_Class::CLASS_NAME);\
}

#include "SjvmLib.h"
