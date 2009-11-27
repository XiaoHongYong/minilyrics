#include "StdAfx.h"
#include "SJVM.h"

#pragma warning(disable : 4996)

#define FORMATLASTERROR(p1, p2, p3)		FormatLastError(p1, p2, p3, __LINE__)


#define D_PRINT_SYNTAX_TREE


static LPTSTR dupStrT(LPCTSTR str)
{
	if (str == NULL)
		return NULL;

	TCHAR		*temp;
	size_t		nLenMax;
	nLenMax = _tcslen(str) + 1;
	temp = new TCHAR[nLenMax];

	_tcscpy(temp, str);

	return temp;
}

//////////////////////////////////////////////////////////////////////////
bool CMemberMethod::IsSameNameAndParameter(LPCSTR szMethodName, VecParamType &vParameter)
{
	if (strcmp(szMethodName, strName.c_str()) != 0)
		return false;

	if (vParameter.size() != this->vParamType.size())
		return false;

	for (size_t i = 0; i < vParamType.size(); i++)
	{
		if (vParameter[i] != this->vParamType[i])
			return false;
	}

	return true;
}

bool CMemberMethod::IsSameNameAndParameter(CMemberMethod *pMethod)
{
	return IsSameNameAndParameter(pMethod->strName.c_str(), pMethod->vParamType);
}

bool CMemberMethod::IsSame(CMemberMethod *pMethod)
{
	// Is method same? with same name, parameter and return value?

	if (typeIDRet != pMethod->typeIDRet)
		return false;

	return IsSameNameAndParameter(pMethod);
}

int CClass::AddMethod(CMemberMethod *pMethod)
{
	for (size_t i = 0; i < vMethod.size(); i++)
	{
		CMemberMethod	*p = vMethod[i];

		if (pMethod->IsSameNameAndParameter(p))
			return ERR_C_DUMPLICATE_METHOD;
	}

	vMethod.push_back(pMethod);

	return ERR_OK;
}

int CClass::AddMemberVar(TypeId typeId, LPCSTR szVarName, bool bStatic, int nStaticAddr)
{
	if (GetMemberVar(szVarName))
		return ERR_C_REDEFINITION_METHOD;

	CMemberVar	*pVar = new CMemberVar();
	pVar->strName = szVarName;
	pVar->typeId = typeId;
	pVar->bStatic = bStatic;
	if (bStatic)
		pVar->nAddr = nStaticAddr;
	else
		pVar->nAddr = -1;

	vMemberVar.push_back(pVar);

	return ERR_OK;
}

int CClass::GetTotalMemberCount()
{
	if (pClassParent)
		return pClassParent->GetTotalMemberCount() + (int)vMemberVar.size();

	return (int)vMemberVar.size();
}

CMemberVar *CClass::GetMemberVar(LPCSTR szVarName)
{
	for (size_t i = 0; i < vMemberVar.size(); i++)
	{
		if (strcmp(vMemberVar[i]->strName.c_str(), szVarName) == 0)
		{
			return vMemberVar[i];
		}
	}

	if (pClassParent)
		return pClassParent->GetMemberVar(szVarName);

	return NULL;
}

bool CClass::IsMemberVarStatic(LPCSTR szVarName)
{
	CMemberVar	*pVar = GetMemberVar(szVarName);
	if (pVar)
		return pVar->bStatic;

	return true;
}

CMemberMethod *CClass::GetMethod(LPCSTR szMethodName, VecParamType &vParameter)
{
	for (size_t i = 0; i < vVirtualMethod.size(); i++)
	{
		if (vVirtualMethod[i]->IsSameNameAndParameter(szMethodName, vParameter))
			return vVirtualMethod[i];
	}

	return NULL;
}

CMemberMethod *CClass::GetNoParameterConstructor()
{
	for (size_t i = 0; i < vMethod.size(); i++)
	{
		CMemberMethod *pMethod = vMethod[i];
		if (strcmp(pMethod->strName.c_str(), strName.c_str()) == 0
			&& pMethod->vParamType.empty())
			return pMethod;
	}

	return NULL;
}

bool CClass::HasConstructor()
{
	for (size_t i = 0; i < vMethod.size(); i++)
	{
		CMemberMethod *pMethod = vMethod[i];
		if (strcmp(pMethod->strName.c_str(), strName.c_str()) == 0)
			return true;
	}

	return false;
}

void CClass::UpdateMemberVarAddress()
{
	for (size_t k = 0; k < vMemberVar.size(); k++)
	{
		CMemberVar *pVar = vMemberVar[k];
		if (!pVar->bStatic)
		{
			if (pClassParent)
				pVar->nAddr = pClassParent->GetTotalMemberCount();
			else
				pVar->nAddr = 0;
			pVar->nAddr += (int)k;
		}
	}
}

int CClass::UpdateVirtualMethodTable(string &strMsg)
{
	if (vVirtualMethod.size())
		return ERR_OK;

	int			nRet;

	if (pClassParent)
	{
		nRet = pClassParent->UpdateVirtualMethodTable(strMsg);
		if (nRet != ERR_OK)
			return nRet;

		vVirtualMethod = pClassParent->vVirtualMethod;
	}

	for (size_t k = 0; k < vMethod.size(); k++)
	{
		CMemberMethod	*pNewMethod = vMethod[k];

		for (size_t i = 0; i < vVirtualMethod.size(); i++)
		{
			if (pNewMethod->IsSameNameAndParameter(vVirtualMethod[i]))
			{
				if (pNewMethod->typeIDRet != vVirtualMethod[i]->typeIDRet)
				{
					char		szMsg[512];
					sprintf(szMsg, "At class: %s, line: %d.", 
						strName.c_str(), pNewMethod->nLineNo);
					strMsg = szMsg;
					return ERR_C_OVERRIDE_METHOD_RET_DIFF;
				}

				vVirtualMethod[i] = pNewMethod;
				pNewMethod->nVirtualIndex = (int)i;
				pNewMethod = NULL;
				break;
			}
		}

		if (pNewMethod)
		{
			pNewMethod->nVirtualIndex = (int)vVirtualMethod.size();
			vVirtualMethod.push_back(pNewMethod);
		}
	}

	return ERR_OK;
}


//////////////////////////////////////////////////////////////////////////

int CClassTable::AddClass(LPCSTR szName)
{
	CClass *pClass;

	if (IsClassDeclared(szName))
		return ERR_C_REDEFINITION_TYPE;

	pClass = new CClass();
	pClass->strName = szName;

	m_vClass.push_back(pClass);
	pClass->typeId = (TypeId)(m_vClass.size() - 1);

	return ERR_OK;
}

CClass *CClassTable::GetClass(LPCSTR szClassName)
{
	for (size_t i = 0; i < m_vClass.size(); i++)
	{
		if (strcmp(m_vClass[i]->strName.c_str(), szClassName) == 0)
			return m_vClass[i];
	}

	return NULL;
}

CClass *CClassTable::GetClass(TypeId typeId)
{
	ASSERT(typeId >= 0 && typeId < m_vClass.size());
	ASSERT(m_vClass[typeId]->typeId == typeId);
	return m_vClass[typeId];
}

bool CClassTable::IsClassDeclared(LPCSTR szClassName)
{
	return GetClass(szClassName) != NULL;
}

bool CClassTable::IsSubClassOf(TypeId typeSub, TypeId typeParent)
{
	CClass		*pParent = GetClass(typeParent);
	if (!pParent)
		return false;

	CClass		*pSub = GetClass(typeSub);
	if (!pSub)
		return false;

	while (pSub->pClassParent)
	{
		if (pSub->pClassParent->typeId == typeParent)
			return true;
		pSub = pSub->pClassParent;
	}

	return false;
}

CMemberMethod *CClassTable::GetEntryPoint()
{
	VecParamType	vParams;

	for (size_t i = 0; i < m_vClass.size(); i++)
	{
		CClass	*pClass = m_vClass[i];
		CMemberMethod *pMethod = pClass->GetMethod("main", vParams);
		if (pMethod && pMethod->bStatic)
			return pMethod;
	}

	return NULL;
}

int CClassTable::StandardizeClassMember(string strMsg)
{
	int		nRet;

	for (size_t i = 0; i < m_vClass.size(); i++)
	{
		CClass	*pClass = m_vClass[i];

		// Add constructor
		if (!pClass->HasConstructor())
		{
			CMemberMethod *pMethod = new CMemberMethod();
			pMethod->typeIDRet = TID_VOID;
			pMethod->strName = pClass->strName;
			if (pClass->AddMethod(pMethod) != ERR_OK)
				delete pMethod;
		}

		pClass->UpdateMemberVarAddress();

		nRet = pClass->UpdateVirtualMethodTable(strMsg);
		if (nRet != ERR_OK)
			return nRet;
	}

	return ERR_OK;
}

#ifdef _DEBUG

void CClassTable::Print(class CSJVM *pVm)
{
	for (size_t i = 0; i < m_vClass.size(); i++)
	{
		CClass	*pClass = m_vClass[i];
		printf("Class: %s\n", pClass->strName.c_str());

		// Print member variable
		size_t k;
		printf("	Class member variable: %d\n", pClass->vMemberVar.size());
		for (k = 0; k < pClass->vMemberVar.size(); k++)
		{
			CMemberVar *pVar = pClass->vMemberVar[k];
			printf("		%s%s %s\n", 
				pVar->bStatic ? "static " : "",
				pVm->GetTypeNameStrByID(pVar->typeId),
				pVar->strName.c_str());
		}

		// Print class method
		printf("	Class member method: %d\n", pClass->vMethod.size());
		for (k = 0; k < pClass->vMethod.size(); k++)
		{
			CMemberMethod	*pMethod = pClass->vMethod[k];
			printf("		%s%s %s(", 
				pMethod->bStatic ? "static " : "",
				pVm->GetTypeNameStrByID(pMethod->typeIDRet),
				pMethod->strName.c_str());
			for (size_t j = 0; j < pMethod->vParamType.size(); j++)
				printf("%s, ", pVm->GetTypeNameStrByID(pMethod->vParamType[j]));
			printf(")\n");

			if (pMethod->siMethod)
			{
				int			nIndent = 0;
				pVm->m_stmtImage.PrintStmtCodeTree(pMethod->siMethod, nIndent);
			}
		}
	}
}
#endif

//////////////////////////////////////////////////////////////////////////

CStrJObject::CStrJObject(LPCTSTR szStr, bool bCopy) : CJObject(TID_STRING, 0)
{
	if (bCopy)
		m_szValue = dupStrT(szStr);
	else
		m_szValue = szStr;

	m_bCopy = bCopy;
}

CStrJObject::~CStrJObject()
{
	if (m_bCopy)
		delete [] m_szValue;
}


CObjectMgr::~CObjectMgr()
{

}

int CObjectMgr::New(TypeId typeId, int nMemberVarCount)
{
	int		nPos = 0;

	if (m_vFreeJObjs.size())
	{
		nPos = m_vFreeJObjs.front();
		m_vFreeJObjs.pop_front();
		m_vJObjects[nPos] = new CJObject(typeId, nMemberVarCount);
	}
	else
	{
		nPos = (int)m_vJObjects.size();
		m_vJObjects.push_back(new CJObject(typeId, nMemberVarCount));
	}

	return nPos;
}

int CObjectMgr::NewString(LPCTSTR szString, bool bCopy)
{
	int		nPos = 0;

	if (m_vFreeJObjs.size())
	{
		nPos = m_vFreeJObjs.front();
		m_vFreeJObjs.pop_front();
		m_vJObjects[nPos] = new CStrJObject(szString, bCopy);
	}
	else
	{
		nPos = (int)m_vJObjects.size();
		m_vJObjects.push_back(new CStrJObject(szString, bCopy));
	}

	return nPos;
}

void CObjectMgr::Free(int nPos, CClassTable &clsTable)
{
// 	ASSERT(nPos > 0 && nPos < m_vJObjects.size());
// 	if (nPos > 0 && nPos < m_vJObjects.size())
// 	{
// 		// delete m_vJObjects[nPos];
// 		m_vFreeJObjs.push_back(nPos);
// 		m_vJObjects[nPos] = NULL;
// 	}
}

CJObject *CObjectMgr::Get(int nPos)
{
	ASSERT(nPos > 0 && nPos < (int)m_vJObjects.size());
	if (nPos > 0 && nPos < (int)m_vJObjects.size())
	{
		return m_vJObjects[nPos];
	}

	return NULL;
}

void CObjectMgr::InitAddConstString(MapStrIndex &mapStr)
{
	ASSERT(m_vJObjects.size() == 1);

	m_vJObjects.resize(mapStr.size() + 1);

	for (MapStrIndex::iterator it = mapStr.begin(); it != mapStr.end(); ++it)
	{
		ASSERT((*it).second < (int)m_vJObjects.size());
		m_vJObjects[(*it).second] = new CStrJObject((*it).first.c_str(), false);
	}
}


//////////////////////////////////////////////////////////////////////////

CSJVM::CSJVM(void)
{
	m_bFinishedInitSysModule = false;

	int			nRet;
	LPCSTR		vIDPredefined[] = { "", "void", "char", "boolean", "short", "int", "__jobject", "String" };
	int			nIDPredefined[] = { TID_INVALID, TID_VOID, TID_CHAR, TID_BOOLEAN, TID_SHORT, TID_INT, TID_NULL, TID_STRING };

	ASSERT(TID_STRING + 1 == CountOf(vIDPredefined));

	for (int i = 0; i < CountOf(vIDPredefined); i++)
	{
		nRet = m_classTable.AddClass(vIDPredefined[i]);
		ASSERT(nRet == ERR_OK);
		ASSERT(nIDPredefined[i] == GetTypeIdByName(vIDPredefined[i]));
	}

/*	nRet = m_classTable.AddClass("String");
	if (nRet == ERR_OK)
	{
		CClass	*pClass;
		pClass->AddMethod("");
	}
	*/
	m_bsiInLoop = false;
}

CSJVM::~CSJVM(void)
{
}


int yyparse();

extern STreeNode *		_syntaxTree;
extern int yyleng;
extern FILE *yyin, *yyout;

int CSJVM::Compile(LPCTSTR szFile)
{
	int		nRet;

	yyin = _tfopen(szFile, _T("rb"));
	if (!yyin)
	{
		printf("Can't open file: %s", szFile);
		return ERR_OPEN_FILE;
	}

	nRet = yyparse();
	if (nRet != 0)
		return ERR_C_PARSE;
	m_syntaxTree = _syntaxTree;
	m_bsiInLoop = false;

	nRet = PreBuildSymbolInfo();
	if (nRet != ERR_OK)
		return nRet;

	// build symbol table
	nRet = BuildSymbolInfo();
	if (nRet != ERR_OK)
		return nRet;

#ifdef D_PRINT_SYNTAX_TREE
	int		nIndent = 0;
	PrintSyntaxTree(m_syntaxTree, nIndent);
#endif

	nRet = BuildCodeStmt();
	if (nRet != ERR_OK)
		return nRet;

	m_syntaxTree = NULL;

	g_memSyntax.FreeAll();

#ifdef _DEBUG
	Print();
#endif

	return ERR_OK;
}

int CSJVM::CompileFiles(VecString &vFiles)
{
	int		nRet;
	typedef vector<STreeNode *>	VecSyntaxNodeOfFiles;

	VecSyntaxNodeOfFiles	vStnFiles;
	size_t	i;

	for (i = 0; i < vFiles.size(); i++)
	{
		LPCTSTR	szFile = vFiles[i].c_str();

		yyin = _tfopen(szFile, _T("rb"));
		if (!yyin)
		{
			printf("Can't open file: %s", szFile);
			return ERR_OPEN_FILE;
		}

		nRet = yyparse();
		if (nRet != 0)
			return ERR_C_PARSE;

		m_syntaxTree = _syntaxTree;

		nRet = PreBuildSymbolInfo();
		if (nRet != ERR_OK)
			return nRet;

		vStnFiles.push_back(_syntaxTree);
	}

	for (i = 0; i < vFiles.size(); i++)
	{
		m_syntaxTree = vStnFiles[i];

		// build symbol table
		nRet = BuildSymbolInfo();
		if (nRet != ERR_OK)
			return nRet;

		#ifdef D_PRINT_SYNTAX_TREE
			int		nIndent = 0;
			PrintSyntaxTree(m_syntaxTree, nIndent);
		#endif

		#ifdef _DEBUG
			Print();
		#endif

			nRet = BuildCodeStmt();
			if (nRet != ERR_OK)
				return nRet;

		#ifdef _DEBUG
			Print();
		#endif
	}

	m_syntaxTree = NULL;

	g_memSyntax.FreeAll();

	return ERR_OK;
}

int CSJVM::Run()
{
	//
	// Prepare for environment for run Virtual machine
	//
	m_exeAction = EXE_GO_ON;

	m_stack.Resize(0);
	m_bp = 0;
	m_vStaticVars.resize(m_varDeclStack.GetSizeOfAllStaticVar());

	m_jobjMgr.InitAddConstString(m_mapStrings);

	CMemberMethod *pMethod;

	// Execute static member variable inital statement
	pMethod = m_classTable.GetClassStaticVarInitMethod();
	Execute(pMethod->siMethod);

	pMethod = m_classTable.GetEntryPoint();
	return Execute(pMethod->siMethod);
}


void CSJVM::FinishedInitSysModule()
{
	m_bFinishedInitSysModule = true;
}

#define PAMETER_COUNT_MAX		32

/*
typedef void (* VoidFuncP0)();
typedef void (* VoidFuncP1)(int);
typedef void (* VoidFuncP2)(int, int);
typedef void (* VoidFuncP3)(int, int, int);
typedef void (* VoidFuncP4)(int, int, int, int);
typedef void (* VoidFuncP5)(int, int, int, int, int);
typedef void (* VoidFuncP6)(int, int, int, int, int, int);
typedef void (* VoidFuncP7)(int, int, int, int, int, int, int);
typedef void (* VoidFuncP8)(int, int, int, int, int, int, int, int);
typedef void (* VoidFuncP9)(int, int, int, int, int, int, int, int, int);
typedef void (* VoidFuncP10)(int, int, int, int, int, int, int, int, int, int);*/

int CSJVM::RegisterNativeAPI(LPCSTR szClassName, LPCSTR szFuncName, LPVOID funcPtr, TypeId typeIdReturn, ...)
{
	CClass	*pClass = m_classTable.GetClass(szClassName);
	if (!pClass)
		return ERR_C_UNDECLARED_TYPE;

	int				nParameterMax = -1;
	size_t			i = 0;

	for (i = 0; i < pClass->vMethod.size(); i++)
	{
		CMemberMethod	*pMethod = pClass->vMethod[i];
		if (strcmp(pMethod->strName.c_str(), szFuncName) == 0
			&& nParameterMax < (int)pMethod->vParamType.size())
			nParameterMax = (int)pMethod->vParamType.size();
	}

	if (nParameterMax == -1)
	{
		// No member method with this name was found.
		return ERR_C_UNDECLARED_METHOD;
	}

	CMemberMethod	method;
	method.strName = szFuncName;
	method.typeIDRet = typeIdReturn;

	va_list		args;

	va_start(args, typeIdReturn);

	for (i = 0; (int)i < nParameterMax; i++)
	{
		int		typeId = va_arg(args, TypeId);
		if (typeId == TID_INVALID)
			return ERR_C_UNDECLARED_METHOD;

		method.AddParam(typeId);
	}
	int		typeId = va_arg(args, TypeId);
	if (typeId != TID_INVALID)
		return ERR_C_UNDECLARED_METHOD;

	va_end(args);

	for (i = 0; i < pClass->vMethod.size(); i++)
	{
		CMemberMethod	*pMethod = pClass->vMethod[i];
		if (method.IsSame(pMethod))
		{
			pMethod->pNativeFunc = funcPtr;
			return ERR_OK;
		}
	}

	return ERR_C_UNDECLARED_METHOD;
}


int CSJVM::CallNativeAPI(CMemberMethod *pMethod, CVMStack &stackParameter)
{
	ASSERT(pMethod->pNativeFunc);
	ASSERT(stackParameter.size() <= PAMETER_COUNT_MAX);

	LPCVOID				vParameter[PAMETER_COUNT_MAX];

	for (size_t i = 0; i < stackParameter.size(); i++)
	{
		CVMStack::Item	&item = stackParameter.Get(i);
		if (item.typeId == TID_STRING)
		{
			CJObject	*pObj = m_jobjMgr.Get(item.nValue);
			if (pObj && pObj->m_typeId == TID_STRING)
			{
				// valid input
				CStrJObject	*pStrObj = (CStrJObject *)pObj;
				vParameter[i] = pStrObj->GetStrValue();
			}
			else
				vParameter[i] = "";
		}
		else
			vParameter[i] = LPVOID(item.nValue);
/*		if (IsSimpleType(item.typeId))
			vParameter[i] = LPVOID(item.nValue);
		else if (item.typeId == TID_STRING)
		{
			
		}
		else
			vParameter[i] = m_jobjMgr.Get(item.nValue);*/
	}

	int		nCount = (int)stackParameter.size() - 1;
	LPVOID	pNativeFunc = pMethod->pNativeFunc;
	int		nRet = 0;

	__asm
	{
		// Push all parameters in stack
		mov         eax,	dword ptr [nCount]
ParamStart:
		cmp         eax,	0 
		jl          Param_end
		mov         ecx,	dword ptr vParameter[eax*4] 
		push        ecx  
		sub         eax,	1 
		jmp			ParamStart 
Param_end:

		// Call function
		mov			eax,      dword ptr [pNativeFunc]
		call		eax
		mov         dword ptr [nRet],eax 
	}

	return nRet;
	// return __asm eax;
}

//////////////////////////////////////////////////////////////////////////
//
// Methods for compiling Simple JAVA
//

//
// Build symbol info and check for syntax error
//
int CSJVM::PreBuildSymbolInfo()
{
	int		nRet;

	//
	// Add class and function declaration first.
	//
	nRet = PreBSI_AddClassDefinition();
	if (nRet != ERR_OK)
	{
		printf("Build Error: %s\n", m_strLastErr.c_str());
		return nRet;
	}

	nRet = PreBSI_AddClassInformation();
	if (nRet != ERR_OK)
	{
		printf("Build Error: %s\n", m_strLastErr.c_str());
		return nRet;
	}

	return ERR_OK;
}

//
// Build symbol info and check for syntax error
//
int CSJVM::BuildSymbolInfo()
{
	int		nRet;

	//
	// Do syntax check, and fill in typeId, static variable offset, etc.
	//
	STreeNode		*pTree = m_syntaxTree;
	while (pTree)
	{
		nRet = BSI_ClassDefinition(pTree);
		if (nRet != ERR_OK)
		{
			printf("Build Error: %s\n", m_strLastErr.c_str());
			return nRet;
		}

		pTree = pTree->pNext;
	}

	return ERR_OK;
}


int CSJVM::PreBSI_AddClassDefinition()
{
	int					nRet;
	STreeNode			*pTree;

	//
	// Add class name first, create class type table.
	//
	pTree = m_syntaxTree;
	while (pTree)
	{
		if (pTree->nodeType == STreeNode::T_CLASS)
		{
			// Add class 
			nRet = m_classTable.AddClass(pTree->sClass.szClassName);
			if (nRet != ERR_OK)
			{
				FORMATLASTERROR(pTree->nLineNo, nRet, pTree->sClass.szClassName);
				return nRet;
			}
		}
		else
		{
			ASSERT(0 && "Only class node will be processed.");
		}

		pTree = pTree->pNext;
	}

	return ERR_OK;
}

int CSJVM::PreBSI_AddClassInformation()
{
	int					nRet;
	STreeNode			*pTree;

	//
	// Update class parent information
	//
	pTree = m_syntaxTree;
	while (pTree)
	{
		if (pTree->nodeType == STreeNode::T_CLASS)
		{
			StnClass	&sClass = pTree->sClass;
			CClass		*pClass = m_classTable.GetClass(sClass.szClassName);
			ASSERT(pClass);

			// Add parent class info
			SJType	&typeParentClass = sClass.typeParentClass;
			if (typeParentClass.szTypeName)
			{
				typeParentClass.typeId = GetTypeIdByName(typeParentClass.szTypeName);
				if (typeParentClass.typeId == TID_INVALID || IsSimpleType(typeParentClass.typeId))
				{
					FORMATLASTERROR(typeParentClass.nLineNo, ERR_C_NOT_CLASS_TYPE, typeParentClass.szTypeName);
					return ERR_C_NOT_CLASS_TYPE;
				}

				pClass->pClassParent = m_classTable.GetClass(typeParentClass.typeId);
				ASSERT(pClass->pClassParent);

				if (m_classTable.IsSubClassOf(pClass->pClassParent->typeId, pClass->typeId))
				{
					FORMATLASTERROR(typeParentClass.nLineNo, ERR_C_CLASS_HIERARCHY_INCONSISTENT, sClass.szClassName);
					return ERR_C_CLASS_HIERARCHY_INCONSISTENT;
				}
			}
		}

		pTree = pTree->pNext;
	}

	//
	// Add class member method and variable
	//
	pTree = m_syntaxTree;
	while (pTree)
	{
		if (pTree->nodeType == STreeNode::T_CLASS)
		{
			// Class node
			CClass				*pClass;
			pClass = m_classTable.GetClass(pTree->sClass.szClassName);
			ASSERT(pClass);
			if (!pClass)
			{
				FORMATLASTERROR(pTree->nLineNo, ERR_C_UNEXPECTED_ERR, pTree->sClass.szClassName);
				return ERR_C_UNEXPECTED_ERR;
			}

			m_varDeclStack.EnterClass(pClass);

			// Add class member variables and methods.
			nRet = PreBSI_AddClassMember(pTree->sClass.pClassMember, pClass);
			if (nRet != ERR_OK)
				return nRet;

			m_varDeclStack.LeaveBlock();
		}

		pTree = pTree->pNext;
	}

	m_varDeclStack.CleanStack();

	string		strMsg;

	nRet = m_classTable.StandardizeClassMember(strMsg);
	if (nRet != ERR_OK)
		return nRet;

	return ERR_OK;
}

int CSJVM::PreBSI_AddClassMember(STreeNode *pTree, CClass *pClass)
{
	ASSERT(pClass);
	int					nRet;

	//
	// Add all class member method declarations...
	//
	while (pTree)
	{
		if (pTree->nodeType == STreeNode::T_MEMBER_METHOD)
		{
			// NULL is acceptable here, just for parameter check
			m_varDeclStack.EnterMethod(NULL);

			// check function declaration, update typeId of function parameter and return value.
			nRet = BSI_FunctionDecl(pTree, pClass);
			if (nRet != ERR_OK)
				return nRet;

			m_varDeclStack.LeaveBlock();
		}
		else if (pTree->nodeType == STreeNode::T_VAR_DECLAR)
		{
			// Add declared variable
			nRet = BSI_VariableDecl(pTree, pClass);
			if (nRet  != ERR_OK)
				return nRet;
		}

		pTree = pTree->pNext;
	}

	return ERR_OK;
}

int CSJVM::BSI_Tree(STreeNode *pTree)
{
	int					nRet;

	while (pTree)
	{
		switch (pTree->nodeType)
		{
		case STreeNode::T_CLASS:
			{
				ASSERT(0 && "Class can't be declared here.");
				// class definition
				nRet = BSI_ClassDefinition(pTree);
				if (nRet != ERR_OK)
					return nRet;
			}
			break;
		case STreeNode::T_VAR_DECLAR:
			{
				nRet = BSI_VariableDecl(pTree);
				if (nRet != ERR_OK)
					return nRet;
			}
			break;
		case STreeNode::T_MEMBER_METHOD:
			{
				ASSERT(0 && "Class method can't be here.");
				/*
					Class method can only be in BSI_ClassDefinition
				*/
			}
			break;
		case STreeNode::T_VAR:
			{
				ASSERT(0 && "Var can't be here.");
			}
			break;
		case STreeNode::T_ASSIGN:
			{
				StnAssign		&assign = pTree->sAssign;
				ASSERT(assign.pVar && assign.pRightExp);
				ASSERT(assign.pVar->nodeType == STreeNode::T_EXPRESSION && assign.pVar->sExpression.type == E_VAR);
				ASSERT(assign.pRightExp->nodeType == STreeNode::T_EXPRESSION);

				nRet = BSI_Expression(assign.pVar);
				if (nRet != ERR_OK)
					return nRet;

				nRet = BSI_Expression(assign.pRightExp);
				if (nRet != ERR_OK)
					return nRet;

				nRet = CanCovertRightToLeftType(assign.pVar->sExpression.typeIdValExp, assign.pRightExp->sExpression.typeIdValExp);
				if (nRet != ERR_OK)
				{
					char		szMsg[512];
					sprintf(szMsg, "Can't convert from type '%s' to '%s'.", 
						GetTypeNameStrByID(assign.pRightExp->sExpression.typeIdValExp),
						GetTypeNameStrByID(assign.pVar->sExpression.typeIdValExp));
					FORMATLASTERROR(pTree->nLineNo, ERR_C_CONVERT_VALUE, szMsg);
					return ERR_C_CONVERT_VALUE;
				}
			}
			break;
		case STreeNode::T_IF:
			{
				StnIf		&sif = pTree->sIf;

				// Check condition
				ASSERT(sif.pCondition && sif.pCondition->nodeType == STreeNode::T_EXPRESSION);
				nRet = BSI_Expression(sif.pCondition);
				if (nRet != ERR_OK)
					return nRet;
				if (sif.pCondition->sExpression.typeIdValExp != TID_BOOLEAN)
				{
					FORMATLASTERROR(pTree->nLineNo, ERR_C_REQUIRE_LOGICAL_CONDITION, "");
					return ERR_C_REQUIRE_LOGICAL_CONDITION;
				}

				// Check true statement node
				if (sif.pTrueNode)
				{
					nRet = BSI_Tree(sif.pTrueNode);
					if (nRet != ERR_OK)
						return nRet;
				}

				if (sif.pFalseNode)
				{
					nRet = BSI_Tree(sif.pFalseNode);
					if (nRet != ERR_OK)
						return nRet;
				}
			}
			break;
		case STreeNode::T_FOR:
			{
				StnFor		&sFor = pTree->sFor;

				if (sFor.pInitalization)
				{
					nRet = BSI_Tree(sFor.pInitalization);
					if (nRet != ERR_OK)
						return nRet;
				}

				if (sFor.pTermination)
				{
					ASSERT(sFor.pTermination->nodeType == STreeNode::T_EXPRESSION);
					nRet = BSI_Expression(sFor.pTermination);
					if (nRet != ERR_OK)
						return nRet;
					if (sFor.pTermination->sExpression.typeIdValExp != TID_BOOLEAN)
					{
						FORMATLASTERROR(pTree->nLineNo, ERR_C_REQUIRE_LOGICAL_CONDITION, "");
						return ERR_C_REQUIRE_LOGICAL_CONDITION;
					}
				}

				if (sFor.pIncrement)
				{
					nRet = BSI_Tree(sFor.pIncrement);
					if (nRet != ERR_OK)
						return nRet;
				}

				// Check statement node
				ASSERT(sFor.pActionNode);
				CAutoLoopState loopState(m_bsiInLoop);
				nRet = BSI_Tree(sFor.pActionNode);
				if (nRet != ERR_OK)
					return nRet;
			}
			break;
		case STreeNode::T_WHILE:
			{
				StnWhile		&swhile = pTree->sWhile;

				// Check condition
				ASSERT(swhile.pCondition && swhile.pCondition->nodeType == STreeNode::T_EXPRESSION);
				nRet = BSI_Expression(swhile.pCondition);
				if (nRet != ERR_OK)
					return nRet;
				if (swhile.pCondition->sExpression.typeIdValExp != TID_BOOLEAN)
				{
					FORMATLASTERROR(pTree->nLineNo, ERR_C_REQUIRE_LOGICAL_CONDITION, "");
					return ERR_C_REQUIRE_LOGICAL_CONDITION;
				}

				// Check statement node
				ASSERT(swhile.pActionNode);
				CAutoLoopState loopState(m_bsiInLoop);
				nRet = BSI_Tree(swhile.pActionNode);
				if (nRet != ERR_OK)
					return nRet;
			}
			break;
		case STreeNode::T_DO_WHILE:
			{
				StnWhile		&swhile = pTree->sWhile;

				// Check condition
				ASSERT(swhile.pCondition && swhile.pCondition->nodeType == STreeNode::T_EXPRESSION);
				nRet = BSI_Expression(swhile.pCondition);
				if (nRet != ERR_OK)
					return nRet;
				if (swhile.pCondition->sExpression.typeIdValExp != TID_BOOLEAN)
				{
					FORMATLASTERROR(pTree->nLineNo, ERR_C_REQUIRE_LOGICAL_CONDITION, "");
					return ERR_C_REQUIRE_LOGICAL_CONDITION;
				}

				// Check statement node
				ASSERT(swhile.pActionNode);
				CAutoLoopState loopState(m_bsiInLoop);
				nRet = BSI_Tree(swhile.pActionNode);
				if (nRet != ERR_OK)
					return nRet;
			}
			break;
		case STreeNode::T_BREAK:
		case STreeNode::T_CONTINUE:
			if (!m_bsiInLoop)
			{
				FORMATLASTERROR(pTree->nLineNo, ERR_C_BREAK_CONTINUE_MUST_IN_LOOP, "");
				return ERR_C_BREAK_CONTINUE_MUST_IN_LOOP;
			}
			break;
		case STreeNode::T_EXPRESSION:
			{
				nRet = BSI_Expression(pTree);
				if (nRet != ERR_OK)
					return nRet;
			}
			break;
		case STreeNode::T_RETURN:
			{
				StnReturn	&sReturn = pTree->sReturn;
				if (sReturn.pExpression)
				{
					nRet = BSI_Expression(sReturn.pExpression);
					if (nRet != ERR_OK)
						return nRet;
				}
			}
			break;
		case STreeNode::T_BLOCK:
			{
				StnBlock	&sBlock = pTree->sBlock;
				if (sBlock.pChildList)
				{
					m_varDeclStack.EnterBlock();
					nRet = BSI_Tree(sBlock.pChildList);
					if (nRet != ERR_OK)
						return nRet;
					sBlock.nCountOfVarToPopup = m_varDeclStack.GetCurrentBlockSize();
					m_varDeclStack.LeaveBlock();
				}
			}
			break;
		default:
			ASSERT(0 && "Invalid node type");
			break;
		}

		pTree = pTree->pNext;
	}

	return ERR_OK;
}

int CSJVM::BSI_ClassDefinition(STreeNode *pTree)
{
	int			nRet;
	ASSERT(pTree->nodeType == STreeNode::T_CLASS);
	StnClass	&sClass = pTree->sClass;

	CClass		*pClass = m_classTable.GetClass(sClass.szClassName);
	ASSERT(pClass);
	if (!pClass)
	{
		FORMATLASTERROR(pTree->nLineNo, ERR_C_UNEXPECTED_ERR, sClass.szClassName);
		return ERR_C_UNEXPECTED_ERR;
	}

	m_varDeclStack.EnterClass(pClass);

	// Check class member methods only.
	STreeNode	*pMemberNode = sClass.pClassMember;
	while (pMemberNode)
	{
		if (pMemberNode->nodeType == STreeNode::T_MEMBER_METHOD)
		{
			// Method definition
			StnMemberMethod	&sMethod = pMemberNode->sMemberMethod;

			m_varDeclStack.EnterMethod(sMethod.pMethod);

			if (strcmp(sMethod.szMethodName, pClass->strName.c_str()) == 0)
			{
				nRet = BSI_ConstructorCheck(pMemberNode, pClass);
				if (nRet != ERR_OK)
					return nRet;
			}

			// Add parameter variable declaration.
			nRet = BSI_FunctionDecl(pMemberNode, pClass);
			if (nRet != ERR_OK)
				return nRet;

			// Check function body/statement
			nRet = BSI_Tree(sMethod.pBody);
			if (nRet != ERR_OK)
				return nRet;

			m_varDeclStack.LeaveBlock();
		}
		else
		{
			// Class member variable has been declared in PreBSI_...
			ASSERT(pMemberNode->nodeType == STreeNode::T_VAR_DECLAR);
		}

		pMemberNode = pMemberNode->pNext;
	}

	m_varDeclStack.LeaveBlock();

	return ERR_OK;
}

int CSJVM::BSI_FunctionDecl(STreeNode *pTree, CClass *pClass)
{
	int				nRet;
	StnMemberMethod	&sMethod = pTree->sMemberMethod;

	if (strcmp(sMethod.szMethodName, pClass->strName.c_str()) == 0)
	{
		// Constructor
		if (sMethod.typeRet.szTypeName)
		{
			FORMATLASTERROR(pTree->nLineNo, ERR_C_CONSTRUCTOR_NO_RETRUN_VALUE, sMethod.szMethodName);
			return ERR_C_CONSTRUCTOR_NO_RETRUN_VALUE;
		}
		sMethod.typeRet.typeId = TID_VOID;
	}
	else
	{
		// Check function return type
		if (sMethod.typeRet.szTypeName == NULL)
		{
			FORMATLASTERROR(pTree->nLineNo, ERR_C_RETRUN_TYPE_MISSING, sMethod.szMethodName);
			return ERR_C_RETRUN_TYPE_MISSING;
		}

		sMethod.typeRet.typeId = GetTypeIdByName(sMethod.typeRet.szTypeName);
		if (sMethod.typeRet.typeId == TID_INVALID)
		{
			FORMATLASTERROR(sMethod.typeRet.nLineNo, ERR_C_UNDECLARED_TYPE, sMethod.typeRet.szTypeName);
			return ERR_C_UNDECLARED_TYPE;
		}
	}

	// Add "this" as first parameters of function call
	if (sMethod.pMethod && !sMethod.pMethod->bStatic)
		m_varDeclStack.Add_this_super_Var();

	// Check function parameter declaration
	STreeNode		*pNode = sMethod.pParameterList;
	VecParamType	vParameter;
	while (pNode)
	{
		ASSERT(pNode->nodeType == STreeNode::T_VAR_DECLAR);
		nRet = BSI_VariableDecl(pNode);
		if (nRet != ERR_OK)
			return nRet;
		vParameter.push_back(pNode->sVarDeclare.type.typeId);
		pNode = pNode->pNext;
	}

	if (sMethod.pMethod == NULL)
	{
		// Add method in declaration
		CMemberMethod	*pMethod = new CMemberMethod();
		pMethod->nLineNo = pTree->nLineNo;
		pMethod->strName = sMethod.szMethodName;
		pMethod->typeIDRet = sMethod.typeRet.typeId;	// return value type
		pMethod->bStatic = sMethod.bStatic;
		pMethod->vParamType = vParameter;
		pMethod->pBodyNode = sMethod.pBody;

		nRet = pClass->AddMethod(pMethod);
		if (nRet != ERR_OK)
		{
			delete pMethod;
			FORMATLASTERROR(pTree->nLineNo, nRet, sMethod.szMethodName);
			return nRet;
		}
		sMethod.pMethod = pMethod;
	}

	return ERR_OK;
}

int CSJVM::BSI_ConstructorCheck(STreeNode *pTree, CClass *pClass)
{
	// Check whether the method called super() constructor

	StnMemberMethod	&sMethod = pTree->sMemberMethod;
	STreeNode		*pBody = sMethod.pBody;

	ASSERT(strcmp(sMethod.szMethodName, pClass->strName.c_str()) == 0);

	// Is super() constructor called already?
	if (pBody && pBody->nodeType == STreeNode::T_EXPRESSION && 
		pBody->sExpression.type == E_METHOD_CALL)
	{
		if (strcmp(pBody->sExpression.functionCall.szFuncName, SZ_SUPER) == 0)
			return ERR_OK; // Super constructor called, return.
	}

	CClass		*pParentClass = pClass->pClassParent;
	if (pParentClass)
	{
		CMemberMethod *pParentConstructor = pParentClass->GetNoParameterConstructor();
		if (!pParentConstructor)
		{
			// All super() constructor has parameter type.
			FORMATLASTERROR(pTree->nLineNo, ERR_C_CONSTRUCTOR_MUST_BE_FIRST_STMT, sMethod.szMethodName);
			return ERR_C_CONSTRUCTOR_MUST_BE_FIRST_STMT;
		}

		// Add super constructor call
		STreeNode	*pNodeConstructor = newStnExpFuncCall(pTree->nLineNo, "super", NULL, NULL);
		pNodeConstructor->pNext = sMethod.pBody;
		sMethod.pBody = pNodeConstructor;
		m_varDeclStack.GetCurMethod()->pBodyNode = sMethod.pBody;
	}
	else
	{
		// No parent constructor
		// Call CallVarInit method
		BSI_InsertClassVarInitNode(pTree);
	}

	return ERR_OK;
}

int CSJVM::BSI_VariableDecl(STreeNode *pTree, CClass *pClass)
{
	int		nRet;

	// Check variable declaration
	ASSERT(pTree->nodeType == STreeNode::T_VAR_DECLAR);
	StnVarDeclare	&varDeclare = pTree->sVarDeclare;

	// Check type name
	varDeclare.type.typeId = GetTypeIdByName(varDeclare.type.szTypeName);
	if (varDeclare.type.typeId == TID_INVALID)
	{
		// report error.
		FORMATLASTERROR(pTree->nLineNo, ERR_C_UNDECLARED_TYPE, varDeclare.type.szTypeName);
		return ERR_C_UNDECLARED_TYPE;
	}

	// Check variable name
	TypeId		typeId = varDeclare.type.typeId;
	bool		bStatic = varDeclare.bStatic;
	STreeNode	*pVarNode = varDeclare.pVarList;
	while (pVarNode)
	{
		ASSERT(pVarNode->nodeType == STreeNode::T_VAR);
		StnVar		&var = pVarNode->sVar;

		if (pClass)
		{
			// Add class member variable
			nRet = pClass->AddMemberVar(typeId, var.szVarName, bStatic, 
				bStatic ? m_varDeclStack.DeclareStaticMemberVar(pClass, var.szVarName) : -1);
			if (nRet != ERR_OK)
			{
				FORMATLASTERROR(pVarNode->nLineNo, nRet, var.szVarName);
				return nRet;
			}
			var.nAddress = pClass->GetMemberVar(var.szVarName)->nAddr;
		}
		else
		{
			// Add variable declaration in m_varDeclStack
			nRet = m_varDeclStack.AddVarDecl(var.szVarName, typeId, bStatic);
			if (nRet != ERR_OK)
			{
				FORMATLASTERROR(pVarNode->nLineNo, nRet, var.szVarName);
				return nRet;
			}
			var.nAddress = m_varDeclStack.GetVariableAddress(var.szVarName);
		}

		// Check variable assign expression.
		if (var.pExpAssign)
		{
			nRet = BSI_Expression(var.pExpAssign);
			if (nRet != ERR_OK)
				return nRet;

			// Can new object asign to left? Type check.
			nRet = CanCovertRightToLeftType(typeId, var.pExpAssign->sExpression.typeIdValExp);
			if (nRet != ERR_OK)
			{
				char		szMsg[512];
				sprintf(szMsg, "Can't convert from type '%s' to '%s'.", 
					GetTypeNameStrByID(var.pExpAssign->sExpression.typeIdValExp),
					GetTypeNameStrByID(typeId));
				FORMATLASTERROR(pVarNode->nLineNo, nRet, szMsg);
				return nRet;
			}
		}

		pVarNode = pVarNode->pNext;
	}

	return ERR_OK;
}

int CSJVM::BSI_Expression(STreeNode *pTree)
{
	int					nRet;

	ASSERT(pTree->nodeType == STreeNode::T_EXPRESSION);
	StnExpression		&exp = pTree->sExpression;

	switch (exp.type)
	{
	case E_CONST_INT:
		{
			exp.typeIdValExp = TID_INT;
			return ERR_OK;
		}
	case E_CONST_BOOLEAN:
		{
			exp.typeIdValExp = TID_BOOLEAN;
			return ERR_OK;
		}
	case E_CONST_NULL:
		{
			exp.typeIdValExp = TID_NULL;
			return ERR_OK;
		}
	case E_CONST_STRING:
		{
			exp.typeIdValExp = TID_STRING;
			return ERR_OK;
		}
	case E_NEW:
		{
			exp.typeIdValExp = GetTypeIdByName(exp.newObj.szNewType);
			VecParamType	vParameter;
			if (exp.newObj.pParameter)
			{
				// Get proper construction method
				STreeNode		*p = exp.newObj.pParameter;
				while (p)
				{
					nRet = BSI_Expression(p);
					if (nRet != ERR_OK)
						return nRet;

					vParameter.push_back(p->sExpression.typeIdValExp);
					p = p->pNext;
				}
			}

			CClass			*pClass = m_classTable.GetClass(exp.typeIdValExp);
			CMemberMethod	*pMethod = NULL;

			nRet = BSI_GetCalledMethod(pClass, exp.newObj.szNewType, vParameter, &pMethod, pTree->nLineNo);
			if (nRet != ERR_OK)
				return nRet;

			exp.newObj.nMemberIndex = pMethod->nVirtualIndex;
			return ERR_OK;
		}
	case E_VAR:
		{
			STreeNode	*pParent = exp.var.pVarParent;
			if (pParent)
			{
				// Member variable

				// Check parent type.
				ASSERT(pParent->sExpression.type == E_VAR);
				nRet = BSI_Expression(pParent);
				if (nRet != ERR_C_CLASS_TYPE_NOT_VAR &&
					nRet != ERR_OK)
					return nRet;

				CClass	*pClass = m_classTable.GetClass(pParent->sExpression.typeIdValExp);
				ASSERT(pClass);
				CMemberVar *pVar = pClass->GetMemberVar(exp.var.szVarName);
				if (!pVar)
				{
					FORMATLASTERROR(pTree->nLineNo, ERR_C_UNDECLARED_MEMBER_VAR, exp.var.szVarName);
					return ERR_C_UNDECLARED_MEMBER_VAR;
				}

				exp.typeIdValExp = pVar->typeId;
				exp.var.nAddress = pVar->nAddr;
				exp.var.bStatic = pVar->bStatic;
				if (nRet == ERR_C_CLASS_TYPE_NOT_VAR && !pVar->bStatic)
				{
					// pVar must a static member variable
					FORMATLASTERROR(pTree->nLineNo, ERR_C_CAN_NOT_CALL_NONE_STATIC_MEMBER, exp.var.szVarName);
					return ERR_C_CAN_NOT_CALL_NONE_STATIC_MEMBER;
				}
				return ERR_OK;
			}

			// Local variable?
			VarInfo varInfo = m_varDeclStack.GetVariable(exp.var.szVarName);
			if (varInfo.nRet != ERR_OK)
			{
				CClass	*pClass = NULL;
				// Is it a class type?
				pClass = m_classTable.GetClass(exp.var.szVarName);
				if (pClass)
				{
					exp.typeIdValExp = pClass->typeId;
					exp.var.nAddress = -1;
					exp.var.bStatic = true;
					FORMATLASTERROR(pTree->nLineNo, ERR_C_CLASS_TYPE_NOT_VAR, exp.var.szVarName);
					return ERR_C_CLASS_TYPE_NOT_VAR;
				}
				FORMATLASTERROR(pTree->nLineNo, ERR_C_UNDECLARED_MEMBER_VAR, exp.var.szVarName);
				return varInfo.nRet;
			}

			if (varInfo.bMemberVar && !varInfo.bStatic)
			{
				// Class member, insert "this" node.
				STreeNode	*pThisPtr = newStnExpVar(pTree->nLineNo, SZ_THIS, NULL);
				nRet = BSI_Expression(pThisPtr);
				ASSERT(nRet == ERR_OK);
				exp.var.pVarParent = pThisPtr;
			}
			exp.var.bStatic = varInfo.bStatic;
			exp.var.nAddress = varInfo.nAddress;
			exp.typeIdValExp = varInfo.typeId;
			return ERR_OK;
		}
	case E_METHOD_CALL:
		return BSI_ExpressionFunctionCall(pTree);
	default:
		break;
	}

	//
	// Check left cases for expression operator
	//

	nRet = BSI_Expression(exp.opParameter.pExpressionLeft);
	if (nRet != ERR_OK)
		return nRet;

	if (exp.opParameter.pExpressionRight)
	{
		nRet = BSI_Expression(exp.opParameter.pExpressionRight);
		if (nRet != ERR_OK)
			return nRet;
	}

	STreeNode	*pExpLeft = exp.opParameter.pExpressionLeft;
	STreeNode	*pExpRight = exp.opParameter.pExpressionRight;

	exp.typeIdValExp = pExpLeft->sExpression.typeIdValExp;

	if (exp.type == OP_EQUAL || exp.type == OP_NOT_EQUAL)
	{
		exp.typeIdValExp = TID_BOOLEAN;

		// for ==, != operator, JObject, null can be used to compare.
		if (CanCovertRightToLeftType(pExpLeft->sExpression.typeIdValExp, pExpRight->sExpression.typeIdValExp) == ERR_OK)
			return ERR_OK;

		FORMATLASTERROR(pTree->nLineNo, ERR_C_OP_TYPE_NOT_MATCH, "");
		return ERR_C_OP_TYPE_NOT_MATCH;
	}


	// TID_STRING can use OP_PLUS, OP_EQUAL, OP_NOT_EQUAL
	if (pExpLeft->sExpression.typeIdValExp == TID_STRING)
	{
		if (exp.type == OP_PLUS)
		{
			if (pExpRight->sExpression.typeIdValExp != TID_STRING)
			{
				FORMATLASTERROR(pTree->nLineNo, ERR_C_OP_TYPE_NOT_MATCH, "");
				return ERR_C_OP_TYPE_NOT_MATCH;
			}

			exp.typeIdValExp = TID_STRING;

			return ERR_OK;
		}

		FORMATLASTERROR(pTree->nLineNo, ERR_C_OP_REQUIRE_INT_TYPE, "");
		return ERR_C_OP_REQUIRE_INT_TYPE;
	}

	switch (exp.type)
	{
	// case OP_EQUAL:		// ==
	// case OP_NOT_EQUAL:	// !=
	case OP_LT_EQ:		// <=
	case OP_GT_EQ:		// >=
	case OP_LITTLE:		// <
	case OP_GREATER:		// >
		exp.typeIdValExp = TID_BOOLEAN;		// Boolean result
	case OP_PLUS:		// +
	case OP_MINUS:		// -
	case OP_MULT:		// *
	case OP_DIV:			// /
		{
			if (pExpLeft->sExpression.typeIdValExp == pExpRight->sExpression.typeIdValExp
				&& pExpRight->sExpression.typeIdValExp == TID_BOOLEAN)
				return ERR_OK;

			// Can only be target to int
			if (pExpLeft->sExpression.typeIdValExp != TID_INT)
			{
				FORMATLASTERROR(pExpLeft->nLineNo, ERR_C_OP_REQUIRE_INT_TYPE, "");
				return ERR_C_OP_REQUIRE_INT_TYPE;
			}

			if (pExpLeft->sExpression.typeIdValExp != pExpRight->sExpression.typeIdValExp)
			{
				FORMATLASTERROR(pTree->nLineNo, ERR_C_OP_TYPE_NOT_MATCH, "");
				return ERR_C_OP_TYPE_NOT_MATCH;
			}
		}
		break;
	case OP_BOOL_AND:	// &&
	case OP_BOOL_OR:		// ||
		{
			STreeNode	*pExpLeft = exp.opParameter.pExpressionLeft;
			STreeNode	*pExpRight = exp.opParameter.pExpressionRight;

			// Can only be target to int
			if (pExpLeft->sExpression.typeIdValExp != TID_BOOLEAN)
			{
				FORMATLASTERROR(pExpLeft->nLineNo, ERR_C_OP_REQUIRE_BOOL_TYPE, "");
				return ERR_C_OP_REQUIRE_BOOL_TYPE;
			}

			if (pExpLeft->sExpression.typeIdValExp != pExpRight->sExpression.typeIdValExp)
			{
				FORMATLASTERROR(pTree->nLineNo, ERR_C_OP_TYPE_NOT_MATCH, "");
				return ERR_C_OP_TYPE_NOT_MATCH;
			}
			exp.typeIdValExp = TID_BOOLEAN;
		}
		break;
	case OP_OP_INC:		// ++
	case OP_OP_DEC:		// --
		{
			STreeNode	*pExpLeft = exp.opParameter.pExpressionLeft;
			if (pExpLeft->sExpression.type != E_VAR)
			{
				FORMATLASTERROR(pTree->nLineNo, ERR_C_OP_MUST_TARGET_VAR, "");
				return ERR_C_OP_MUST_TARGET_VAR;
			}
		}
		break;
	default:
		ASSERT(0 && "Unknown expression kind");
		return ERR_C_UNEXPECTED_ERR;
	}

	return ERR_OK;
}

int CSJVM::BSI_ExpressionFunctionCall(STreeNode *pTree)
{
	StnExpression	&exp = pTree->sExpression;
	STreeNode		*pParentVar = exp.functionCall.pVarParent;
	CClass			*pClass = NULL;
	CMemberMethod	*pMethod = NULL;
	VecParamType	vParameters;
	int				nRet;

	//
	// Build all parameter information
	//
	STreeNode	*pParam = exp.functionCall.pParameter;
	while (pParam)
	{
		ASSERT(pParam->nodeType == STreeNode::T_EXPRESSION);
		nRet = BSI_Expression(pParam);
		if (nRet != ERR_OK)
			return nRet;

		vParameters.push_back(pParam->sExpression.typeIdValExp);

		pParam = pParam->pNext;
	}

	exp.functionCall.typeIdSpecialfiedClass = TID_INVALID;
	bool	bParentIsClassName = false;

	// Check method name, and method parent variable
	if (pParentVar)
	{
		nRet = BSI_Expression(pParentVar);
		if (nRet != ERR_C_CLASS_TYPE_NOT_VAR && 
			nRet != ERR_OK)
			return nRet;

		if (nRet == ERR_C_CLASS_TYPE_NOT_VAR)
			bParentIsClassName = true;

		pClass = m_classTable.GetClass(pParentVar->sExpression.typeIdValExp);
		ASSERT(pClass);
	}
	else
	{
		pClass = m_varDeclStack.GetCurClass();
		ASSERT(pClass);
	}

	if (strcmp(exp.functionCall.szFuncName, SZ_SUPER) == 0)
	{
		//
		// Must be in construction method.
		//
		CClass	*pCurClass = m_varDeclStack.GetCurClass();
		CMemberMethod *pCurMethod = m_varDeclStack.GetCurMethod();
		if (strcmp(pCurMethod->strName.c_str(), pCurClass->strName.c_str()) != 0
			|| pParentVar || pCurMethod->pBodyNode != pTree)
		{
			FORMATLASTERROR(pTree->nLineNo, ERR_C_CONSTRUCTOR_MUST_BE_FIRST_STMT, "");
			return ERR_C_CONSTRUCTOR_MUST_BE_FIRST_STMT;
		}

		CClass *pParentClass = m_varDeclStack.GetCurClass()->pClassParent;
		if (!pParentClass)
		{
			// No parent class
			if (vParameters.empty())
			{
				// deal it as empty node.
				pTree->nodeType = STreeNode::T_EMPTY;
				return ERR_OK;
			}

			FORMATLASTERROR(pTree->nLineNo, ERR_C_CONSTRUCTOR_OBJ_UNDEFINED, "");
			return ERR_C_CONSTRUCTOR_OBJ_UNDEFINED;
		}

		nRet = BSI_GetCalledMethod(pParentClass, pParentClass->strName.c_str(), vParameters, &pMethod, pTree->nLineNo);
		if (nRet != ERR_OK)
		{
			FORMATLASTERROR(pTree->nLineNo, ERR_C_CONSTRUCTOR_OBJ_UNDEFINED, "");
			return ERR_C_CONSTRUCTOR_OBJ_UNDEFINED;
		}

		exp.functionCall.typeIdSpecialfiedClass = pParentClass->typeId;

		// Insert the class initial variable statement
		// After super() call.
		BSI_InsertClassVarInitNode(pTree);
	}
	else
	{
		nRet = BSI_GetCalledMethod(pClass, exp.functionCall.szFuncName, vParameters, &pMethod, pTree->nLineNo);
		if (nRet != ERR_OK)
			return nRet;
	}

	if (!pMethod->bStatic)
	{
		if (bParentIsClassName)
		{
			// Is call supper class member?
			if (!m_classTable.IsSubClassOf(m_varDeclStack.GetCurClass()->typeId, pClass->typeId))
			{
				// call other class method, it must a static member method
				FORMATLASTERROR(pTree->nLineNo, ERR_C_CAN_NOT_CALL_NONE_STATIC_MEMBER, exp.functionCall.szFuncName);
				return ERR_C_CAN_NOT_CALL_NONE_STATIC_MEMBER;
			}

			exp.functionCall.typeIdSpecialfiedClass = pClass->typeId;

			// Must call super/this class method.
			// Change parent as "this" pointer
			VarInfo thisInfo = m_varDeclStack.GetVariable(SZ_THIS);
			pParentVar->sExpression.var.nAddress = thisInfo.nAddress;
			pParentVar->sExpression.var.bStatic = thisInfo.bStatic;
		}
		else if (!pParentVar)
		{
			// Class member, insert "this" node.
			STreeNode	*pThisPtr = newStnExpVar(pTree->nLineNo, SZ_THIS, NULL);
			nRet = BSI_Expression(pThisPtr);
			ASSERT(nRet == ERR_OK);
			exp.functionCall.pVarParent = pThisPtr;
		}
		else if (pParentVar)
		{
			// Is "super"?
			if (strcmp(pParentVar->sExpression.var.szVarName, SZ_SUPER) == 0)
				exp.functionCall.typeIdSpecialfiedClass = pClass->typeId;
		}
	}

	exp.typeIdValExp = pMethod->typeIDRet;
	exp.functionCall.bStatic = pMethod->bStatic;
	exp.functionCall.nMethodIndex = pMethod->nVirtualIndex;
	if (pMethod->bStatic)
		exp.functionCall.typeIdSpecialfiedClass = pClass->typeId;

	return ERR_OK;
}

int CSJVM::BSI_GetCalledMethod(CClass *pClass, LPCSTR szMethodName, VecParamType &vParameters, CMemberMethod **ppMethod, int nLineNo)
{
	*ppMethod = NULL;
	// char	szMsg[512];

	for (size_t i = 0; i < pClass->vVirtualMethod.size(); i++)
	{
		CMemberMethod	*p = pClass->vVirtualMethod[i];

		if (strcmp(szMethodName, p->strName.c_str()) != 0)
			continue;
		if (p->vParamType.size() != vParameters.size())
			continue;

		size_t	k = 0;
		for (k = 0; k < vParameters.size(); k++)
		{
			if (CanCovertRightToLeftType(p->vParamType[k], vParameters[k]) != ERR_OK)
				break;
		}
		if (k == vParameters.size())
		{
			*ppMethod = p;
			return ERR_OK;
		}
	}

	FORMATLASTERROR(nLineNo, ERR_C_UNDECLARED_METHOD, szMethodName);
	return ERR_C_UNDECLARED_METHOD;
}

int CSJVM::BSI_InsertClassVarInitNode(STreeNode *pTree)
{
	ASSERT(pTree);
	CClass		*pCurClass = m_varDeclStack.GetCurClass();
	ASSERT(pCurClass);

	if (pCurClass->methodVarInit.pBodyNode)
	{
		ASSERT(!(pTree->pNext && pTree->pNext->nodeType == STreeNode::T_BLOCK && pTree->pNext->sBlock.pChildList == pCurClass->methodVarInit.pBodyNode));
		STreeNode *pVarInit = newStnBlock(pTree->nLineNo, pCurClass->methodVarInit.pBodyNode);
		pVarInit->pNext = pTree->pNext;
		pTree->pNext = pVarInit;
	}

	return ERR_OK;
}

int CSJVM::CanCovertRightToLeftType(TypeId typeIdLeft, TypeId typeIdRight)
{
	if (typeIdRight == typeIdLeft)
		return ERR_OK;

	if (m_classTable.IsSubClassOf(typeIdRight, typeIdLeft))
		return ERR_OK;

	if (typeIdRight == TID_NULL && !IsSimpleType(typeIdLeft)
		|| typeIdLeft == TID_NULL && !IsSimpleType(typeIdRight))
		return ERR_OK;

	return ERR_C_CONVERT_VALUE;
}

//////////////////////////////////////////////////////////////////////////
//
// Build code statement
//

int CSJVM::BuildCodeStmt()
{
	int					nRet;
	STreeNode		*pTree = m_syntaxTree;

	while (pTree)
	{
		ASSERT(pTree->nodeType == STreeNode::T_CLASS);
		nRet = BCS_ClassDefinition(pTree);
		if (nRet != ERR_OK)
		{
			printf("BuildCodeStmt Error: %s\n", m_strLastErr.c_str());
			return nRet;
		}
		pTree = pTree->pNext;
	}

	return ERR_OK;
}

int CSJVM::BCS_ClassDefinition(STreeNode *pTree)
{
	int			nRet;
	ASSERT(pTree->nodeType == STreeNode::T_CLASS);
	StnClass	&sClass = pTree->sClass;

	CClass	*pClass = m_classTable.GetClass(sClass.szClassName);
	ASSERT(pClass);
	if (!pClass)
	{
		nRet = ERR_C_UNEXPECTED_ERR;
		FORMATLASTERROR(pTree->nLineNo, nRet, sClass.szClassName);
		return nRet;
	}

	// Check class members.
	STreeNode	*pMemberNode = sClass.pClassMember;
	while (pMemberNode)
	{
		if (pMemberNode->nodeType == STreeNode::T_MEMBER_METHOD)
		{
			// Method definition
			nRet = BCS_FunctionDefinition(pMemberNode, pClass);
			if (nRet != ERR_OK)
				return nRet;
		}
		else
		{
			// Class member variable has been declared in PreBSI_...
			ASSERT(pMemberNode->nodeType == STreeNode::T_VAR_DECLAR);

			//
			// Add variable initial code in construction method
			//
			nRet = BCS_ClassVariable(pMemberNode, pClass);
			if (nRet != ERR_OK)
				return nRet;
		}

		pMemberNode = pMemberNode->pNext;
	}

	return ERR_OK;
}

int CSJVM::BCS_FunctionDefinition(STreeNode *pTree, CClass *pClass)
{
	StnMemberMethod	&sMethod = pTree->sMemberMethod;
	int				nRet;

	ASSERT(sMethod.pMethod);

	// Check function body/statement
	nRet = BCS_Tree(sMethod.pBody, sMethod.pMethod->siMethod);
	if (nRet != ERR_OK)
		return nRet;

	return ERR_OK;
}

int CSJVM::BCS_ClassVariable(STreeNode *pTree, CClass *pClass)
{
	int		nRet;

	// Check variable declaration
	ASSERT(pTree->nodeType == STreeNode::T_VAR_DECLAR);
	StnVarDeclare	&varDeclare = pTree->sVarDeclare;

	// Check variable id
	TypeId		typeId = varDeclare.type.typeId;
	bool		bStatic = varDeclare.bStatic;
	STreeNode	*pVarNode = varDeclare.pVarList;
	CMemberMethod	*pMethod;

	if (bStatic)
		pMethod = m_classTable.GetClassStaticVarInitMethod();
	else
		pMethod = pClass->GetVarInitMethod();
	ASSERT(pMethod);

	stmt_index		*psiTail = m_stmtImage.GetTail(&(pMethod->siMethod));

	while (pVarNode)
	{
		ASSERT(pVarNode->nodeType == STreeNode::T_VAR);
		StnVar		&var = pVarNode->sVar;

		// Check variable assign expression.
		if (var.pExpAssign)
		{
			stmt_index	siVarAddress, siExpression;

			// Var address
			siVarAddress = m_stmtImage.newStmtExpVar(pVarNode->nLineNo, bStatic, typeId, var.nAddress, TID_INVALID);

			// Value expression
			nRet = BCS_Expression(var.pExpAssign, siExpression);
			if (nRet != ERR_OK)
				return nRet;

			// new assign statement
			*psiTail = m_stmtImage.newStmtAssign(pVarNode->nLineNo, siVarAddress, siExpression);
			psiTail = m_stmtImage.GetTail(psiTail);
		}

		pVarNode = pVarNode->pNext;
	}

	return ERR_OK;
}

int CSJVM::BCS_VarDeclaration(STreeNode *pTree, stmt_index &siOut)
{
	int		nRet;

	// Check variable declaration
	ASSERT(pTree->nodeType == STreeNode::T_VAR_DECLAR);
	StnVarDeclare	&varDeclare = pTree->sVarDeclare;

	stmt_index		*psiTail = m_stmtImage.GetTail(&siOut);

	// Check variable id
	TypeId		typeId = varDeclare.type.typeId;
	bool		bStatic = varDeclare.bStatic;
	STreeNode	*pVarNode = varDeclare.pVarList;
	while (pVarNode)
	{
		ASSERT(pVarNode->nodeType == STreeNode::T_VAR);
		StnVar		&var = pVarNode->sVar;

		*psiTail = m_stmtImage.newStmtStackPush(pVarNode->nLineNo, varDeclare.type.typeId);
		psiTail = m_stmtImage.GetTail(psiTail);

		// Check variable assign expression.
		if (var.pExpAssign)
		{
			stmt_index	siVarAddress, siExpression;

			// Var address
			siVarAddress = m_stmtImage.newStmtExpVar(var.pExpAssign->nLineNo, bStatic, typeId, var.nAddress, TID_INVALID);

			// Value expression
			nRet = BCS_Expression(var.pExpAssign, siExpression);
			if (nRet != ERR_OK)
				return nRet;

			// new assign statement
			*psiTail = m_stmtImage.newStmtAssign(pVarNode->nLineNo, siVarAddress, siExpression);
			psiTail = m_stmtImage.GetTail(psiTail);
		}

		pVarNode = pVarNode->pNext;
	}

	return ERR_OK;
}

int CSJVM::BCS_Expression(STreeNode *pTree, stmt_index &siOut)
{
	int					nRet;

	ASSERT(pTree->nodeType == STreeNode::T_EXPRESSION);
	StnExpression		&exp = pTree->sExpression;

	switch (exp.type)
	{
	case E_CONST_INT:
		{
			siOut = m_stmtImage.newStmtExpConstInt(pTree->nLineNo, exp.nConstInt);
			return ERR_OK;
		}
	case E_CONST_BOOLEAN:
		{
			siOut = m_stmtImage.newStmtExpConstBool(pTree->nLineNo, exp.bConstBool);
			return ERR_OK;
		}
	case E_CONST_NULL:
		{
			siOut = m_stmtImage.newStmtExpNull(pTree->nLineNo);
			return ERR_OK;
		}
	case E_CONST_STRING:
		{
			tstring		str;

			UTF8ToTString(exp.szConstString, -1, str);
			MapStrIndex::iterator it = m_mapStrings.find(str);
			int			nIndex;
			if (it == m_mapStrings.end())
			{
				nIndex = (int)m_mapStrings.size() + 1;
				m_mapStrings[str] = nIndex;
			}
			else
				nIndex = (*it).second;

			siOut = m_stmtImage.newStmtExpConstString(pTree->nLineNo, nIndex);
			return ERR_OK;
		}
	case E_NEW:
		{
			stmt_index	siParameter = SI_INVALID;
			nRet = BCS_Tree(exp.newObj.pParameter, siParameter);
			if (nRet != ERR_OK)
				return nRet;

			siOut = m_stmtImage.newStmtExpNew(pTree->nLineNo, exp.typeIdValExp, exp.newObj.nMemberIndex, siParameter);
			return ERR_OK;
		}
	case E_VAR:
		{
			STreeNode	*pParent = exp.var.pVarParent;
			stmt_index	siParentVar = SI_INVALID;
			if (pParent && !exp.var.bStatic)
			{
				// Only non-static variable need to check parent type.
				ASSERT(pParent->sExpression.type == E_VAR);
				nRet = BCS_Expression(pParent, siParentVar);
				if (nRet != ERR_OK)
					return nRet;
			}

			siOut = m_stmtImage.newStmtExpVar(pTree->nLineNo, exp.var.bStatic, exp.typeIdValExp, exp.var.nAddress, siParentVar);
			return ERR_OK;
		}
	case E_METHOD_CALL:
		{
			stmt_index	siParentVar = SI_INVALID;
			stmt_index	siParameter = SI_INVALID;

			// Check method name, and method parent variable
			if (exp.functionCall.pVarParent && !exp.functionCall.bStatic)
			{
				// Only non-static variable need to check parent type.
				nRet = BCS_Expression(exp.functionCall.pVarParent, siParentVar);
				if (nRet != ERR_OK)
					return nRet;
			}

			nRet = BCS_Tree(exp.functionCall.pParameter, siParameter);
			if (nRet != ERR_OK)
				return nRet;

			siOut = m_stmtImage.newStmtExpFuncCall(pTree->nLineNo, exp.functionCall.bStatic, exp.typeIdValExp, exp.functionCall.typeIdSpecialfiedClass, siParentVar,
				exp.functionCall.nMethodIndex, siParameter);

			return ERR_OK;
		}
	default:
		break;
	}

	//
	// Check left cases for expression operator
	//
	stmt_index		siExpLeft = SI_INVALID, siExpRight = SI_INVALID;

	ASSERT(exp.opParameter.pExpressionLeft);
	nRet = BCS_Expression(exp.opParameter.pExpressionLeft, siExpLeft);
	if (nRet != ERR_OK)
		return nRet;

	if (exp.opParameter.pExpressionRight)
	{
		nRet = BCS_Expression(exp.opParameter.pExpressionRight, siExpRight);
		if (nRet != ERR_OK)
			return nRet;
	}

	siOut = m_stmtImage.newStmtExpOperate(pTree->nLineNo, exp.type, exp.typeIdValExp, siExpLeft, siExpRight, 
		exp.opParameter.pExpressionLeft->sExpression.typeIdValExp);

	return ERR_OK;
}

int CSJVM::BCS_Tree(STreeNode *pTree, stmt_index &siOut)
{
	int					nRet;
	stmt_index			*psiTail = m_stmtImage.GetTail(&siOut);

	while (pTree)
	{
		switch (pTree->nodeType)
		{
		case STreeNode::T_CLASS:
			{
				ASSERT(0 && "Class can't be declared here.");
				// class definition
			}
			break;
		case STreeNode::T_VAR_DECLAR:
			{
				nRet = BCS_VarDeclaration(pTree, *psiTail);
				if (nRet != ERR_OK)
					return nRet;
			}
			break;
		case STreeNode::T_MEMBER_METHOD:
			{
				ASSERT(0 && "Class method can't be here.");
				/*
				Class method can only be in BSI_ClassDefinition
				*/
			}
			break;
		case STreeNode::T_VAR:
			{
				ASSERT(0 && "Var can't be here.");
			}
			break;
		case STreeNode::T_ASSIGN:
			{
				StnAssign		&assign = pTree->sAssign;
				ASSERT(assign.pVar && assign.pRightExp);
				ASSERT(assign.pVar->nodeType == STreeNode::T_EXPRESSION && assign.pVar->sExpression.type == E_VAR);
				ASSERT(assign.pRightExp->nodeType == STreeNode::T_EXPRESSION);
				stmt_index		siVar = SI_INVALID, siRightExpr = SI_INVALID;

				nRet = BCS_Expression(assign.pVar, siVar);
				if (nRet != ERR_OK)
					return nRet;
				nRet = BCS_Expression(assign.pRightExp, siRightExpr);
				if (nRet != ERR_OK)
					return nRet;

				*psiTail = m_stmtImage.newStmtAssign(pTree->nLineNo, siVar, siRightExpr);
			}
			break;
		case STreeNode::T_IF:
			{
				StnIf		&sif = pTree->sIf;
				stmt_index	siCondition = SI_INVALID, siTrueStmt = SI_INVALID, siFalseStmt = SI_INVALID;

				// Check condition
				ASSERT(sif.pCondition && sif.pCondition->nodeType == STreeNode::T_EXPRESSION);
				nRet = BCS_Expression(sif.pCondition, siCondition);
				if (nRet != ERR_OK)
					return nRet;

				// Check true statement node
				if (sif.pTrueNode)
				{
					nRet = BCS_Tree(sif.pTrueNode, siTrueStmt);
					if (nRet != ERR_OK)
						return nRet;
				}

				if (sif.pFalseNode)
				{
					nRet = BCS_Tree(sif.pFalseNode, siFalseStmt);
					if (nRet != ERR_OK)
						return nRet;
				}

				*psiTail = m_stmtImage.newStmtIf(pTree->nLineNo, siCondition, siTrueStmt, siFalseStmt);
			}
			break;
		case STreeNode::T_FOR:
			{
				StnFor		&sFor = pTree->sFor;
				stmt_index	siInitalization = SI_INVALID, siTermination = SI_INVALID;
				stmt_index	siIncrement = SI_INVALID, siStmt = SI_INVALID;

				if (sFor.pInitalization)
				{
					nRet = BCS_Tree(sFor.pInitalization, siInitalization);
					if (nRet != ERR_OK)
						return nRet;
				}

				if (sFor.pTermination)
				{
					ASSERT(sFor.pTermination->nodeType == STreeNode::T_EXPRESSION);
					nRet = BCS_Expression(sFor.pTermination, siTermination);
					if (nRet != ERR_OK)
						return nRet;
					if (sFor.pTermination->sExpression.typeIdValExp != TID_BOOLEAN)
					{
						FORMATLASTERROR(pTree->nLineNo, ERR_C_REQUIRE_LOGICAL_CONDITION, "");
						return ERR_C_REQUIRE_LOGICAL_CONDITION;
					}
				}

				if (sFor.pIncrement)
				{
					nRet = BCS_Tree(sFor.pIncrement, siIncrement);
					if (nRet != ERR_OK)
						return nRet;
				}

				// Check statement node
				ASSERT(sFor.pActionNode);
				nRet = BCS_Tree(sFor.pActionNode, siStmt);
				if (nRet != ERR_OK)
					return nRet;

				*psiTail = m_stmtImage.newStmtFor(pTree->nLineNo, siInitalization, siTermination, siIncrement, siStmt);
			}
			break;
		case STreeNode::T_WHILE:
			{
				StnWhile		&swhile = pTree->sWhile;
				stmt_index		siCondition = SI_INVALID, siActionStmt = SI_INVALID;

				// Check condition
				ASSERT(swhile.pCondition && swhile.pCondition->nodeType == STreeNode::T_EXPRESSION);
				nRet = BCS_Expression(swhile.pCondition, siCondition);
				if (nRet != ERR_OK)
					return nRet;

				// Check statement node
				ASSERT(swhile.pActionNode);
				nRet = BCS_Tree(swhile.pActionNode, siActionStmt);
				if (nRet != ERR_OK)
					return nRet;

				*psiTail = m_stmtImage.newStmtWhile(pTree->nLineNo, siCondition, siActionStmt);
			}
			break;
		case STreeNode::T_BREAK:
			*psiTail = m_stmtImage.newStmtBreak(pTree->nLineNo);
			break;
		case STreeNode::T_CONTINUE:
			*psiTail = m_stmtImage.newStmtContinue(pTree->nLineNo);
			break;
		case STreeNode::T_DO_WHILE:
			{
				StnWhile		&swhile = pTree->sWhile;
				stmt_index		siCondition = SI_INVALID, siActionStmt = SI_INVALID;

				// Check condition
				ASSERT(swhile.pCondition && swhile.pCondition->nodeType == STreeNode::T_EXPRESSION);
				nRet = BCS_Expression(swhile.pCondition, siCondition);
				if (nRet != ERR_OK)
					return nRet;

				// Check statement node
				ASSERT(swhile.pActionNode);
				nRet = BCS_Tree(swhile.pActionNode, siActionStmt);
				if (nRet != ERR_OK)
					return nRet;

				*psiTail = m_stmtImage.newStmtWhile(pTree->nLineNo, siCondition, siActionStmt);
			}
			break;
		case STreeNode::T_EXPRESSION:
			{
				nRet = BCS_Expression(pTree, *psiTail);
				if (nRet != ERR_OK)
					return nRet;
			}
			break;
		case STreeNode::T_RETURN:
			{
				StnReturn	&sReturn = pTree->sReturn;
				stmt_index	siExpression = SI_INVALID;
				if (sReturn.pExpression)
				{
					nRet = BCS_Expression(sReturn.pExpression, siExpression);
					if (nRet != ERR_OK)
						return nRet;
				}

				*psiTail = m_stmtImage.newStmtReturn(pTree->nLineNo, siExpression);
			}
			break;
		case STreeNode::T_BLOCK:
			{
				StnBlock	&sBlock = pTree->sBlock;
				stmt_index	siChildBlock = SI_INVALID;
				if (sBlock.pChildList)
				{
					nRet = BCS_Tree(sBlock.pChildList, siChildBlock);
					if (nRet != ERR_OK)
						return nRet;
				}

				*psiTail = m_stmtImage.newStmtBlock(pTree->nLineNo, siChildBlock);
				psiTail = m_stmtImage.GetTail(psiTail);

				if (sBlock.nCountOfVarToPopup > 0)
					*psiTail = m_stmtImage.newStmtStackPop(pTree->nLineNo, sBlock.nCountOfVarToPopup);

				psiTail = m_stmtImage.GetTail(psiTail);
			}
			break;
		default:
			ASSERT(0 && "Invalid nodetype");
			break;
		}

		pTree = pTree->pNext;
		psiTail = m_stmtImage.GetTail(psiTail);
	}

	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////

//
// Execute statement
//

int CSJVM::Execute(stmt_index siEntry)
{
	Statement	*ps = m_stmtImage.GetStmt(siEntry);
	int			nRet;
	stmt_index	si = siEntry;

	while (ps)
	{
		switch (ps->stmtType)
		{
			case ST_BLOCK:
				{
					StmtBlock	&sBlock = ps->sBlock;
					if (sBlock.siFirstChild != SI_INVALID)
						Execute(sBlock.siFirstChild);
				}
				break;
			case ST_IF:
				{
					StmtIf	&sIf = ps->sIf;
					nRet = ExecuteExpression(sIf.siCondition);
					if (nRet > 0)
					{
						if (sIf.siTrueStmt != SI_INVALID)
							nRet = Execute(sIf.siTrueStmt);
					}
					else
					{
						if (sIf.siFalseStmt != SI_INVALID)
							nRet = Execute(sIf.siFalseStmt);
					}
				}
				break;
			case ST_FOR:
				{
					StmtFor	sFor = ps->sFor;

					if (sFor.siInitalization != SI_INVALID)
						Execute(sFor.siInitalization);
					while (sFor.siTermination != SI_INVALID ? ExecuteExpression(sFor.siTermination) : true)
					{
						if (sFor.siStmt != SI_INVALID)
							Execute(sFor.siStmt);

						if (m_exeAction == EXE_GO_ON)
							;// Do nothing, for speed up, put it on first check
						else if (m_exeAction == EXE_LOOP_BREAK)
						{
							m_exeAction = EXE_GO_ON;
							break;
						}
						else
						{
							ASSERT(m_exeAction == EXE_LOOP_CONTINUE);
							m_exeAction = EXE_GO_ON;
						}

						if (sFor.siIncrement != SI_INVALID)
							Execute(sFor.siIncrement);
					}
				}
				break;
			case ST_WHILE:
				{
					StmtWhile	sWhile = ps->sWhile;
					while (ExecuteExpression(sWhile.siCondition))
					{
						if (sWhile.siStmt != SI_INVALID)
							Execute(sWhile.siStmt);

						if (m_exeAction == EXE_GO_ON)
							;// Do nothing, for speed up, put it on first check
						else if (m_exeAction == EXE_LOOP_BREAK)
						{
							m_exeAction = EXE_GO_ON;
							break;
						}
						else
						{
							ASSERT(m_exeAction == EXE_LOOP_CONTINUE);
							m_exeAction = EXE_GO_ON;
						}
					}
				}
				break;
			case ST_DO_WHILE:
				{
					StmtWhile	sWhile = ps->sWhile;
					do {
						if (sWhile.siStmt != SI_INVALID)
							Execute(sWhile.siStmt);

						if (m_exeAction == EXE_GO_ON)
							;// Do nothing, for speed up, put it on first check
						else if (m_exeAction == EXE_LOOP_BREAK)
						{
							m_exeAction = EXE_GO_ON;
							break;
						}
						else
						{
							ASSERT(m_exeAction == EXE_LOOP_CONTINUE);
							m_exeAction = EXE_GO_ON;
						}
					}
					while (ExecuteExpression(sWhile.siCondition));
				}
				break;
			case ST_BREAK:
				ASSERT(m_exeAction == EXE_GO_ON);
				m_exeAction = EXE_LOOP_BREAK;
				break;
			case ST_CONTINUE:
				ASSERT(m_exeAction == EXE_GO_ON);
				m_exeAction = EXE_LOOP_CONTINUE;
				break;
			case ST_ASSIGN:
				{
					StmtAssign	&sAssign = ps->sAssign;
					nRet = ExecuteExpression(sAssign.siExpression);
					int			*pAddress;
					pAddress = ExeExprGetVarAddr(sAssign.siVarAddress);

					*pAddress = nRet;
				}
				break;
			case ST_RETURN:
				{
					StmtReturn	&sReturn = ps->sRetrun;
					if (sReturn.siExpression != SI_INVALID)
						m_valueMethodReturn = ExecuteExpression(sReturn.siExpression);
					else
						m_valueMethodReturn = 0;

					ASSERT(m_exeAction == EXE_GO_ON);
					m_exeAction = EXE_FUNC_RETURN;
				}
				break;
			case ST_PUSH_STACK:
				{
					StmtStackPush	&sStackPush = ps->sStackPush;
					m_stack.Push(0, sStackPush.typeIdVar);
				}
				break;
			case ST_POP_STACK:
				{
					StmtStackPop	sStackPop = ps->sStackPop;
					for (int i = 0; i < sStackPop.nCountOfVarToPopup; i++)
					{
						CVMStack::Item &item = m_stack.Back();
						if (!IsSimpleType(item.typeId))
							m_jobjMgr.Free(item.nValue, m_classTable);
						m_stack.Pop();
					}
				}
				break;
			case ST_EXPRESSION:
				{
					ExecuteExpression(si);
				}
				break;
			default:
				ASSERT(0 && "Invalid statement type");
				return 0;
		}

		if (ps->siNextStmt == SI_INVALID || m_exeAction != EXE_GO_ON)
			break;
		si = ps->siNextStmt;
		ps = m_stmtImage.GetStmt(ps->siNextStmt);
	}
	return 0;
}

int CSJVM::ExecuteExpression(stmt_index siExpr)
{
	ASSERT(siExpr != SI_INVALID);
	Statement		*ps = m_stmtImage.GetStmt(siExpr);
	ASSERT(ps->stmtType == ST_EXPRESSION);
	StmtExpression	&expr = ps->sExpression;

	switch (expr.expType)
	{
	case E_CONST:
		return expr.nConstValue;
	case E_CONST_STRING:
		return expr.nConstValue;
	case E_NEW:
		return NewJObject(expr);
	case E_VAR:
		{
			if (expr.var.bStatic)
				return GetStaticVarValue(expr.var.nAddress);

			if (expr.var.siParentVar != SI_INVALID)
			{
				// A Jobject member varaible
				int		nParentValue;
				nParentValue = ExecuteExpression(expr.var.siParentVar);
				return GetJObjectMemberVarValue(nParentValue, expr.var.nAddress);
			}

			// Variable is in stack
			return m_stack.GetValue(m_bp + expr.var.nAddress);
		}
	case E_METHOD_CALL:
		{
			Statement		*pStmt;
			int				nParentVarValue = 0;
			CVMStack		stackParameter;

			if (!expr.functionCall.bStatic)
			{
				ASSERT(expr.functionCall.siParentVar != SI_INVALID);
				nParentVarValue = ExecuteExpression(expr.functionCall.siParentVar);
				pStmt = m_stmtImage.GetStmt(expr.functionCall.siParentVar);

				// Push "this" pointer in stack first.
				stackParameter.Push(nParentVarValue, pStmt->sExpression.typeId);
			}

			// Execute parameter expressions
			stmt_index		siParameter = expr.functionCall.siParameter;

			// Push parameter in stack
			while (siParameter != SI_INVALID)
			{
				pStmt = m_stmtImage.GetStmt(siParameter);
				ASSERT(pStmt);
				int		nParamValue = ExecuteExpression(siParameter);

				// Push value in stack
				stackParameter.Push(nParamValue, pStmt->sExpression.typeId);

				siParameter = m_stmtImage.GetNext(siParameter);
			}

			// Call method
			int		nRet;
			nRet = CallMethod(expr.functionCall.typeIdSpecialfiedClass, nParentVarValue, expr.functionCall.nMemberIndex, stackParameter);

			m_exeAction = EXE_GO_ON;

			return nRet;
		}
	default:
		break;
	}

	int		nLeftVal = 0, nRightVal = 0;

	if (expr.expType != OP_OP_INC && expr.expType != OP_OP_DEC)
	{
		nLeftVal = ExecuteExpression(expr.opParameter.siExprLeft);
		nRightVal = ExecuteExpression(expr.opParameter.siExprRight);
	}

	//
	// TID_STRING can use OP_PLUS, OP_EQUAL, OP_NOT_EQUAL
	//
	if (expr.opParameter.typeIdOperand == TID_STRING)
	{
		CStrJObject	*pLeft, *pRight;
		CJObject	*pJObj;

		if (nLeftVal == JOBJ_NULL || nRightVal == JOBJ_NULL)
		{
			if (expr.expType == OP_EQUAL)
				return nLeftVal == nRightVal;
			else if (expr.expType == OP_NOT_EQUAL)
				return nLeftVal != nRightVal;
		}

		pJObj = m_jobjMgr.Get(nLeftVal);
		if (!pJObj || pJObj->m_typeId != TID_STRING)
		{
			ASSERT(0 && "Invalid String JObject.");
			return 0;
		}
		pLeft = (CStrJObject*)pJObj;

		pJObj = m_jobjMgr.Get(nRightVal);
		if (!pJObj || pJObj->m_typeId != TID_STRING)
		{
			ASSERT(0 && "Invalid String JObject.");
			return 0;
		}
		pRight = (CStrJObject*)pJObj;

		if (expr.expType == OP_EQUAL)
			return _tcscmp(pLeft->GetStrValue(), pRight->GetStrValue()) == 0;
		else if (expr.expType == OP_NOT_EQUAL)
			return _tcscmp(pLeft->GetStrValue(), pRight->GetStrValue()) != 0;
		else if (expr.expType == OP_PLUS)
		{
			tstring		str = pLeft->GetStrValue();
			str += pRight->GetStrValue();
			return m_jobjMgr.NewString(str.c_str());
		}
		else
			ASSERT(0 && "Not supported operator for string");
	}

	switch (expr.expType)
	{
	case OP_LT_EQ:		// <=
		return nLeftVal <= nRightVal;
	case OP_GT_EQ:		// >=
		return nLeftVal == nRightVal;
	case OP_LITTLE:		// <
		return nLeftVal < nRightVal;
	case OP_GREATER:	// >
		return nLeftVal > nRightVal;
	case OP_EQUAL:		// ==
		return nLeftVal == nRightVal;
	case OP_NOT_EQUAL:	// !=
		return nLeftVal != nRightVal;
	case OP_BOOL_AND:	// &&
		return nLeftVal && nRightVal;
	case OP_BOOL_OR:	// ||
		return nLeftVal || nRightVal;
	case OP_PLUS:		// +
		return nLeftVal + nRightVal;
	case OP_MINUS:		// -
		return nLeftVal - nRightVal;
	case OP_MULT:		// *
		return nLeftVal * nRightVal;
	case OP_DIV:		// /
		return nLeftVal / nRightVal;
	case OP_OP_INC:		// ++
		{
			int		*pInt = ExeExprGetVarAddr(expr.opParameter.siExprLeft);
			int		nOld = *pInt;
			(*pInt)++;
			return nOld;
		}
	case OP_OP_DEC:		// --
		{
			int		*pInt = ExeExprGetVarAddr(expr.opParameter.siExprLeft);
			int		nOld = *pInt;
			(*pInt)--;
			return nOld;
		}
	default:
		ASSERT(0 && "Undefined expression type.");
	}

	return 0;
}

int *CSJVM::ExeExprGetVarAddr(stmt_index siExprVar)
{
	Statement		*ps = m_stmtImage.GetStmt(siExprVar);
	ASSERT(ps->stmtType == ST_EXPRESSION);
	StmtExpression	&expr = ps->sExpression;
	ASSERT(expr.expType == E_VAR);

	if (expr.var.bStatic)
		return GetStaticVarAddress(expr.var.nAddress);

	if (expr.var.siParentVar)
	{
		// A Jobject member varaible
		int		nParentValue;
		nParentValue = ExecuteExpression(expr.var.siParentVar);
		return GetJObjectMemberVarAddr(nParentValue, expr.var.nAddress);
	}

	// Variable is in stack
	return &(m_stack.Get(m_bp + expr.var.nAddress).nValue);
}


int CSJVM::NewJObject(StmtExpression &expr)
{
	CClass	*pClass = m_classTable.GetClass(expr.typeId);
	ASSERT(pClass);
	if (!pClass)
		return 0;

	CVMStack		stackParameter;

	// Execute parameter expressions
	// And push parameter in stack
	stmt_index siParameter = expr.newObj.siParameter;
	while (siParameter != SI_INVALID)
	{
		Statement *pStmt = m_stmtImage.GetStmt(siParameter);
		ASSERT(pStmt);
		int		nParamValue = ExecuteExpression(siParameter);

		// Push value in stack
		stackParameter.Push(nParamValue, pStmt->sExpression.typeId);

		siParameter = m_stmtImage.GetNext(siParameter);
	}

	// Execute construction method.
	CMemberMethod	*pMethod = pClass->GetMethod(expr.newObj.nMemberIndex);
	ASSERT(pMethod);
	if (!pMethod)
		return 0;

	int		nJObj = m_jobjMgr.New(expr.typeId, pClass->GetTotalMemberCount());

	stackParameter.Insert(0, nJObj, expr.typeId);

	CallMethod(expr.typeId, nJObj, expr.newObj.nMemberIndex, stackParameter);

	return nJObj;
}

void CSJVM::FreeJObject(int nJObject)
{
	m_jobjMgr.Free(nJObject, m_classTable);
}

int CSJVM::CallMethod(TypeId typeId, int nJObject, int nMethodIndex, CVMStack &stackParameter)
{
	// Save bp, sp
	size_t				nBp, nSp;
	nBp = m_bp;
	nSp = m_stack.size();

	if (typeId == TID_INVALID)
	{
		CJObject	*pObj = m_jobjMgr.Get(nJObject);
		ASSERT(pObj);
		if (!pObj)
			return 0;
		typeId = pObj->m_typeId;
	}

	CClass *pClass = m_classTable.GetClass(typeId);
	if (!pClass)
		return 0;

	CMemberMethod *pMethod = pClass->GetVirtualMethod(nMethodIndex);
	if (!pMethod)
		return 0;

	int		nRet;
	if (pMethod->siMethod == SI_INVALID)
	{
		// Call native member method.
		ASSERT(pMethod->pNativeFunc);
		if (pMethod->pNativeFunc)
			nRet = CallNativeAPI(pMethod, stackParameter);
	}
	else
	{
		m_bp = m_stack.size();

		m_stack.Push(stackParameter);

		// Return value is in m_valueMethodReturn.
		Execute(pMethod->siMethod);
		if (pMethod->typeIDRet == TID_VOID)
			m_valueMethodReturn = 0;
		else
			nRet = m_valueMethodReturn;
	}

	// Recover bp, sp, and free object allocated in previous function of stack
	if (m_stack.size() != nSp)
	{
		for (size_t i = nSp; i < m_stack.size(); i++)
		{
			CVMStack::Item	&item = m_stack.Get(i);
			if (item.nValue != 0 && !IsSimpleType(item.typeId))
				FreeJObject(item.nValue);
		}
		m_stack.Resize(nSp);
	}
	m_bp = nBp;

	m_valueMethodReturn = nRet;

	return nRet;
}

int CSJVM::GetStaticVarValue(size_t nAddress)
{
	ASSERT(nAddress < m_vStaticVars.size());
	if (nAddress >= m_vStaticVars.size())
		return 0;

	return m_vStaticVars[nAddress];
}

int CSJVM::GetJObjectMemberVarValue(int nJobject, int nMember)
{
	CJObject	*pObj = m_jobjMgr.Get(nJobject);
	if (!pObj)
		return 0;

	return pObj->GetValue(nMember);
}

int *CSJVM::GetStaticVarAddress(size_t nAddress)
{
	ASSERT(nAddress < m_vStaticVars.size());
	if (nAddress >= m_vStaticVars.size())
		return 0;

	return &(m_vStaticVars[nAddress]);
}

int *CSJVM::GetJObjectMemberVarAddr(int nJobject, int nMember)
{
	CJObject	*pObj = m_jobjMgr.Get(nJobject);
	if (!pObj)
		return 0;

	return pObj->GetAddress(nMember);
}

//////////////////////////////////////////////////////////////////////////

void PrintSapces(int nIndent);

//////////////////////////////////////////////////////////////////////////

void FormatLastError(string &strOutput, int nLine, int nError, LPCSTR szStr, int nLineSrc)
{
	string str;
	switch (nError)
	{
	case ERR_C_UNDECLARED_ID:	str = "undeclared identifier"; break;
	case ERR_C_UNDECLARED_TYPE:	str = "undeclared class or type"; break;
	case ERR_C_UNDECLARED_METHOD:	str = "undeclared class member method"; break;
	case ERR_C_UNDECLARED_VAR:	str = "undeclared variable"; break;
	case ERR_C_UNDECLARED_MEMBER_VAR:	str = "undeclared member variable"; break;
	case ERR_C_PARSE:	str = "Parse script file"; break;
	case ERR_C_REDEFINITION_ID:	str = "redefinition of identifier"; break;
	case ERR_C_OP_TYPE_NOT_MATCH:	str = "operand type not match"; break;
	case ERR_C_OP_REQUIRE_INT_STR_TYPE:	str = "operand type should be int or string"; break;
	case ERR_C_OP_REQUIRE_INT_TYPE:	str = "operand type should be int"; break;
	case ERR_C_OP_REQUIRE_BOOL_TYPE:	str = "operand type should be boolean"; break;
	case ERR_C_REDEFINITION_TYPE:	str = "redefinition of type"; break;
	case ERR_C_REDEFINITION_METHOD:	str = "redefinition of class method"; break;
	case ERR_C_CONVERT_VALUE:	str = "Can't convert value to another"; break;
	case ERR_C_UNEXPECTED_ERR:	str = "Got unexpected error"; break;
	case ERR_C_FUNC_RETURN_VALUE:	str = "Method return value is incorrect."; break;
	case ERR_C_NEW_WRONG_TYPE:	str = "New type is incorrect."; break;
	case ERR_C_NOT_CLASS_TYPE:	str = "Not a class type, it hasn't member variable or method, or can NOT be extended."; break;
	case ERR_C_CAN_NOT_CALL_NONE_STATIC_MEMBER:	str = "Static method of class, can NOT call NONE static member."; break;
	case ERR_C_REQUIRE_LOGICAL_CONDITION:	str = "Logical condition is required in (if, while, etc)"; break;
	case ERR_C_METHOD_CALL_PARAMETER_COUNT:	str = "Class method parameter count is incorrect."; break;
	case ERR_C_CLASS_TYPE_NOT_VAR:	str = "It's class type, not a variable."; break;
	case ERR_C_CLASS_HIERARCHY_INCONSISTENT:	str = "The hierarchy of class is inconsistent."; break;
	case ERR_C_OVERRIDE_METHOD_RET_DIFF:	str = "Override method must have same return value type."; break;
	case ERR_C_DUMPLICATE_METHOD:	str = "Duplicated member method."; break;
	case ERR_C_CONSTRUCTOR_MUST_BE_FIRST_STMT:	str = "Constructor must be the first statement in a constructor."; break;
	case ERR_C_CONSTRUCTOR_OBJ_UNDEFINED:	str = "The constructor object is undefined."; break;
	case ERR_C_CONSTRUCTOR_NO_RETRUN_VALUE:	str = "The constructor should NOT have return value."; break;
	case ERR_C_RETRUN_TYPE_MISSING:	str = "Return type for the method is missing."; break;
	case ERR_C_NO_THIS_SUPER_IN_STATIC_METHOD:	str = "Can NOT use super in a static method."; break;
	case ERR_C_BREAK_CONTINUE_MUST_IN_LOOP:	str = "Break/continue statement must be in a loop."; break;

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

void CSJVM::FormatLastError(int nLine, int nError, LPCSTR szStr, int nLineSrc)
{
	::FormatLastError(m_strLastErr, nLine, nError, szStr, nLineSrc);
}

// Get TypeId by class name, or system predefined name.
TypeId CSJVM::GetTypeIdByName(LPCSTR szTypeName)
{
	CClass *pClass = m_classTable.GetClass(szTypeName);
	if (pClass)
		return pClass->typeId;
	return TID_INVALID;
}

LPCSTR CSJVM::GetTypeNameStrByID(TypeId id)
{
	CClass *pClass = m_classTable.GetClass(id);
	if (pClass)
		return pClass->strName.c_str();

	return "";
}

#ifdef _DEBUG
void CSJVM::Print()
{
	m_classTable.Print(this);
}
#endif
