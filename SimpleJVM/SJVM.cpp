#include "StdAfx.h"
#include "SJVM.h"
#include "SjvmLib.h"

/*

TODO:
	. CObjectMgr reset()
	. newStnExpVar, LPCSTR, string allocate and free check.

*/

#pragma warning(disable : 4996)

#define FORMATLASTERROR(p1, p2, p3)		FormatLastError(p1, p2, p3, __LINE__)

#define GC_TRIGGER_COUNT		1000
#define MAX_CALL_STACK_DEEP		5000

#define D_PRINT_SYNTAX_TREE

CSJVM			g_sjvm;

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

static void SJNI_API ClassDefaultConstructor(LPVOID p)
{

}

void CClass::AddNativeDefaultConstructor()
{
	CMemberMethod	*pMethod = new CMemberMethod();
	pMethod->strName = strName;
	pMethod->typeIDRet = TID_INVALID;
	pMethod->pNativeFunc = ClassDefaultConstructor;
	pMethod->bStatic = false;

	pMethod->nVirtualIndex = (int)vVirtualMethod.size();
	vVirtualMethod.push_back(pMethod);
	vMethod.push_back(pMethod);
}

int CClass::AddNativeMethod(bool bStatic, LPCSTR szMethodName, LPVOID funcPtr, TypeId typeIdReturn, ...)
{
	CMemberMethod	*pMethod = new CMemberMethod();
	pMethod->strName = szMethodName;
	pMethod->typeIDRet = typeIdReturn;
	pMethod->pNativeFunc = funcPtr;
	pMethod->bStatic = bStatic;

	va_list		args;

	va_start(args, typeIdReturn);

	for (int i = 0; i < CVMParameters::COUNT_MAX; i++)
	{
		TypeId		typeId = va_arg(args, TypeId);
		if (typeId == TID_INVALID)
			break;

		pMethod->AddParam(typeId);
	}

	va_end(args);

	vMethod.push_back(pMethod);

	if (pClassParent && vVirtualMethod.empty())
		vVirtualMethod = pClassParent->vVirtualMethod;

	for (size_t i = 0; i < vVirtualMethod.size(); i++)
	{
		if (pMethod->IsSameNameAndParameter(vVirtualMethod[i]))
		{
			if (pMethod->typeIDRet != vVirtualMethod[i]->typeIDRet)
			{
				ASSERT(0 && "Added native method override the existed method, but with different return type.");
				delete pMethod;
				return -1;
			}

			vVirtualMethod[i] = pMethod;
			pMethod->nVirtualIndex = (int)i;
			return (int)i;
		}
	}

	pMethod->nVirtualIndex = (int)vVirtualMethod.size();
	vVirtualMethod.push_back(pMethod);

	return pMethod->nVirtualIndex;
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

void CClass::UpdateTotalMemberCount()
{
	nTotalMemberVarCount = (int)vMemberVar.size();

	if (pClassParent)
	{
		pClassParent->UpdateTotalMemberCount();
		nTotalMemberVarCount += pClassParent->nTotalMemberVarCount;
	}
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
				pVar->nAddr = pClassParent->nTotalMemberVarCount;
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

CClassTable::CClassTable()
{
}

CClassTable::~CClassTable()
{
	for (size_t i = 0; i < m_vClass.size(); i++)
	{
		delete m_vClass[i];
	}
	m_vClass.clear();
}

int CClassTable::AddClass(LPCSTR szName, int nLineNo, CJObjectNewer *pJObjNewer)
{
	CClass *pClass;

	if (IsClassDeclared(szName))
		return ERR_C_REDEFINITION_TYPE;

	pClass = new CClass();
	pClass->strName = szName;
	pClass->nLineNo = nLineNo;

	m_vClass.push_back(pClass);
	pClass->typeId = (TypeId)(m_vClass.size() - 1);
	pClass->pJObojNewer = pJObjNewer;
	if (pJObjNewer)
		pClass->bFreeJObjNewer = true;

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

		pClass->UpdateTotalMemberCount();

		// Add constructor
		if (!pClass->HasConstructor())
		{
			CMemberMethod *pMethod = new CMemberMethod();
			pMethod->typeIDRet = TID_VOID;
			pMethod->nLineNo = pClass->nLineNo;
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

void CClassTable::UpdateJObjNewer()
{
	for (size_t i = 0; i < m_vClass.size(); i++)
	{
		CClass	*pClass = m_vClass[i];

		if (!pClass->pJObojNewer)
		{
			CClass	*pParent = pClass->pClassParent;
			while (pParent)
			{
				if (pParent->pJObojNewer)
				{
					pClass->pJObojNewer = pParent->pJObojNewer;
					break;
				}
				pParent = pParent->pClassParent;
			}
		}
	}
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

void CObjectMgr::Reset()
{
	m_vJObjects.resize(1);
}

int CObjectMgr::New(CClass *pClass)
{
	int			nPos = 0;
	CJObject	*pNew = NULL;

	m_nAllocCount++;
	if (m_nAllocCount > GC_TRIGGER_COUNT)
	{
		m_nAllocCount = 0;
		g_sjvm.GarbageCollect();
	}

	ASSERT(pClass->typeId != TID_STRING);

	if (pClass->pJObojNewer)
	{
		pNew = pClass->pJObojNewer->New();
		pNew->m_typeId = pClass->typeId;
		ASSERT((int)pNew->m_vMemberVar.size() <= pClass->nTotalMemberVarCount);
		pNew->m_vMemberVar.resize(pClass->nTotalMemberVarCount);
	}
	else
		pNew = new CJObject(pClass->typeId, pClass->nTotalMemberVarCount);

	CClass		*p = pClass;
	while (p)
	{
		for (size_t i = 0; i < p->vMemberVar.size(); i++)
		{
			if (!p->vMemberVar[i]->bStatic)
			{
				ASSERT(p->vMemberVar[i]->nAddr <= (int)pNew->m_vMemberVar.size());
				pNew->m_vMemberVar[p->vMemberVar[i]->nAddr].typeId = p->vMemberVar[i]->typeId;
			}
		}

		p = p->pClassParent;
	}

	if (m_vFreeJObjs.size())
	{
		nPos = m_vFreeJObjs.front();
		m_vFreeJObjs.pop_front();
		m_vJObjects[nPos] = pNew;
	}
	else
	{
		nPos = (int)m_vJObjects.size();
		m_vJObjects.push_back(pNew);
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

void CVMStack::Push(CVMParameters &parameters)
{
	for (size_t i = 0; i < parameters.m_nCount; i++)
	{
		m_vItems.push_back(parameters.m_vItems[i]);
	}
}

//////////////////////////////////////////////////////////////////////////

CSJVM::CSJVM(void)
{
	m_bFinishedInitSysModule = false;
	m_nSysClassCount = 0;

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

	InitSjvmLib();

	m_bsiInLoop = false;
	m_bp = 0;
	m_nCallStackDeep = 0;
}

CSJVM::~CSJVM(void)
{
}


int yyparse();

extern STreeNode *		_syntaxTree;
extern int yyleng;
extern FILE *yyin, *yyout;

void CSJVM::Reset()
{
	// Reset class table
	ASSERT(m_nSysClassCount <= m_classTable.m_vClass.size());
	if (m_nSysClassCount < m_classTable.m_vClass.size())
	{
		for (size_t i = m_nSysClassCount; i < m_classTable.m_vClass.size(); i++)
		{
			delete m_classTable.m_vClass[i];
		}
		m_classTable.m_vClass.resize(m_nSysClassCount);
	}

	// Reset static variables
	ASSERT(m_nSysStaticVarsCount <= m_vStaticVars.size());
	m_vStaticVars.resize(m_nSysStaticVarsCount);
	for (size_t i = 0; i < m_vStaticVars.size(); i++)
	{
		m_vStaticVars[i].bInitialized = false;
		m_vStaticVars[i].nValue = 0;
	}

	// Reset jobject manager
	m_jobjMgr.Reset();

	// Reset statement image
}

/************************************************************************

Syntax parse
1. Add class type
2. Add class member variable and method definition (PreBSI_XXX)
   Add class default constructor, update class method index
3. Check class member variable initial syntax. (BSI_XXX)
   Check class member method syntax.
   Check all constructors, and insert member variable initial statement to all constructors.

4. Generate member method code

************************************************************************/
int CSJVM::Compile(LPCTSTR szFile)
{
	int		nRet;

	g_srcLineNo = 1;
	yyin = _tfopen(szFile, _T("rb"));
	if (!yyin)
	{
		printf("Can't open file: %s", szFile);
		return ERR_OPEN_FILE;
	}

	nRet = yyparse();
	fclose(yyin);
	yyin = NULL;
	if (nRet != 0)
	{
		return ERR_C_PARSE;
	}
	m_syntaxTree = _syntaxTree;
	m_bsiInLoop = false;

	nRet = PreBuildSymbolInfo();
	if (nRet != ERR_OK)
		return nRet;

	string		strMsg;

	nRet = m_classTable.StandardizeClassMember(strMsg);
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

		g_srcLineNo = 1;
		yyin = _tfopen(szFile, _T("rb"));
		if (!yyin)
		{
			printf("Can't open file: %s", szFile);
			return ERR_OPEN_FILE;
		}

		nRet = yyparse();
		fclose(yyin);
		yyin = NULL;
		if (nRet != 0)
			return ERR_C_PARSE;

		m_syntaxTree = _syntaxTree;

		nRet = PreBuildSymbolInfo();
		if (nRet != ERR_OK)
			return nRet;

		vStnFiles.push_back(_syntaxTree);
	}

	string		strMsg;

	nRet = m_classTable.StandardizeClassMember(strMsg);
	if (nRet != ERR_OK)
		return nRet;

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
	// Prepare for environment for running Virtual machine
	//
	m_exeAction = EXE_GO_ON;
	m_typeIdException = TID_INVALID;
	m_jobjException = JOBJ_NULL;

	m_classTable.UpdateJObjNewer();

	m_stack.Resize(0);
	m_bp = 0;
	m_nCallStackDeep = 0;

	m_varDeclStack.InitStaticVar(m_vStaticVars);

	m_jobjMgr.InitAddConstString(m_mapStrings);

	CMemberMethod *pMethod;

	// Execute static member variable initial statement
	pMethod = m_classTable.GetClassStaticVarInitMethod();
	if (pMethod && pMethod->siMethod != SI_INVALID)
		Execute(pMethod->siMethod);

	pMethod = m_classTable.GetEntryPoint();
	if (pMethod && pMethod->siMethod)
		return Execute(pMethod->siMethod);
	else
		return ERR_R_NO_ENTRY_POINT;
}

class CSJVMGarbageCoolector
{
public:
	typedef vector<bool>	VecBool;
	CObjectMgr				*pObjMgr;

	CSJVMGarbageCoolector(class CObjectMgr &objMgr)
	{
		pObjMgr = &objMgr;
		vIsObjReferenced.resize(pObjMgr->m_vJObjects.size(), false);
	}

	void MarkUsed(int nJObj)
	{
		if ((size_t)nJObj >= vIsObjReferenced.size())
		{
			ASSERT(0 && "Invalid jobject variable");
			return;
		}
		if (vIsObjReferenced[nJObj])
			return;

		vIsObjReferenced[nJObj] = true;

		CJObject	*pObj = pObjMgr->m_vJObjects[nJObj];
		ASSERT(pObj);
		for (size_t i = 0; i < pObj->m_vMemberVar.size(); i++)
		{
			Variable	&var = pObj->m_vMemberVar[i];
			if (!IsSimpleType(var.typeId) && var.nValue != JOBJ_NULL)
			{
				MarkUsed(var.nValue);
			}
		}
	}

	void CollectUnused()
	{
		pObjMgr->m_vFreeJObjs.clear();
		int		nUsedEndPos;

		for (nUsedEndPos = (int)vIsObjReferenced.size() - 1; nUsedEndPos > (int)g_sjvm.m_mapStrings.size(); nUsedEndPos--)
		{
			if (vIsObjReferenced[nUsedEndPos])
				break;
		}
		nUsedEndPos++;
		if (nUsedEndPos < (int)pObjMgr->m_vJObjects.size())
		{
			pObjMgr->m_vJObjects.resize(nUsedEndPos);
			vIsObjReferenced.resize(nUsedEndPos);
		}

		// 0 ~ g_sjvm.m_mapStrings.size() is the static string.
		for (size_t i = g_sjvm.m_mapStrings.size(); i < vIsObjReferenced.size(); i++)
		{
			if (!vIsObjReferenced[i])
			{
				CJObject	*pObj = pObjMgr->m_vJObjects[i];
				if (pObj)
				{
					delete pObj;
				}
				pObjMgr->m_vFreeJObjs.push_back((int)i);
			}
		}
	}

public:
	VecBool		vIsObjReferenced;

};

int CSJVM::GarbageCollect()
{
	size_t		i;
	CSJVMGarbageCoolector	gc(m_jobjMgr);

	_tprintf(_T("run Garbage Collect\n"));

	// Add reference from stack and static variable
	for (i = 0; i < m_stack.size(); i++)
	{
		Variable	&var = m_stack.m_vItems[i];
		if (!IsSimpleType(var.typeId) && var.nValue != JOBJ_NULL)
		{
			gc.MarkUsed(var.nValue);
		}
	}

	for (i = 0; i < m_vStaticVars.size(); i++)
	{
		StaticVar	&var = m_vStaticVars[i];
		if (!IsSimpleType(var.typeId) && var.nValue != JOBJ_NULL)
		{
			gc.MarkUsed(var.nValue);
		}
	}

	gc.CollectUnused();

	return ERR_OK;
}

void CSJVM::FinishedInitSysModule()
{
	m_nSysClassCount = m_classTable.m_vClass.size();
	m_nSysStaticVarsCount = m_vStaticVars.size();

	m_bFinishedInitSysModule = true;
}

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

CJObject *CSJVM::NewJObject(TypeId typeId, jobject &jobj)
{
	jobj = JOBJ_NULL;

	CClass	*pClass = m_classTable.GetClass(typeId);
	ASSERT(pClass);
	if (!pClass)
		return NULL;

	jobj = m_jobjMgr.New(pClass);
	if (jobj == JOBJ_NULL)
		return NULL;

	return m_jobjMgr.Get(jobj);
}

int CSJVM::CallNativeAPI(CMemberMethod *pMethod, jobject thiz, CVMParameters &parameters)
{
	ASSERT(pMethod->pNativeFunc);
	ASSERT(parameters.m_nCount <= CVMParameters::COUNT_MAX);

	LPCVOID				vParameter[CVMParameters::COUNT_MAX];
	size_t				pos = 0;

	if (!pMethod->bStatic)
	{
		// Add "this" in parameter first
		vParameter[pos] = LPVOID(thiz);
		pos++;
	}

	for (size_t i = 0; i < parameters.m_nCount; i++, pos++)
	{
		Variable	&item = parameters.m_vItems[i];
		if (item.typeId == TID_STRING)
		{
			CJObject	*pObj = m_jobjMgr.Get(item.nValue);
			if (pObj && pObj->m_typeId == TID_STRING)
			{
				// valid input
				CStrJObject	*pStrObj = (CStrJObject *)pObj;
				vParameter[pos] = pStrObj->GetStrValue();
			}
			else
				vParameter[pos] = "";
		}
		else
			vParameter[pos] = LPVOID(item.nValue);
	}

	int		nCount = pos - 1;
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

	m_valueMethodReturn = nRet;

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
		_tprintf(_T("Build Error: %s\n"), m_strLastErr.c_str());
		return nRet;
	}

	nRet = PreBSI_AddClassInformation();
	if (nRet != ERR_OK)
	{
		_tprintf(_T("Build Error: %s\n"), m_strLastErr.c_str());
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
			_tprintf(_T("Build Error: %s\n"), m_strLastErr.c_str());
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
			nRet = m_classTable.AddClass(pTree->sClass.szClassName, pTree->nLineNo);
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
			nRet = BSI_MethodDeclare(pTree, pClass);
			if (nRet != ERR_OK)
				return nRet;

			m_varDeclStack.LeaveBlock();
		}
		else if (pTree->nodeType == STreeNode::T_VAR_DECLAR)
		{
			// Add declared variable
			nRet = PreBSI_AddMemberVarDecl(pTree, pClass);
			if (nRet  != ERR_OK)
				return nRet;
		}

		pTree = pTree->pNext;
	}

	return ERR_OK;
}

int CSJVM::PreBSI_AddMemberVarDecl(STreeNode *pTree, CClass *pClass)
{
	ASSERT(pClass);
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

		// Add class member variable
		nRet = pClass->AddMemberVar(typeId, var.szVarName, bStatic, 
			bStatic ? m_varDeclStack.DeclareStaticMemberVar(pClass, var.szVarName, typeId) : -1);
		if (nRet != ERR_OK)
		{
			FORMATLASTERROR(pVarNode->nLineNo, nRet, var.szVarName);
			return nRet;
		}
		var.nAddress = pClass->GetMemberVar(var.szVarName)->nAddr;

		pVarNode = pVarNode->pNext;
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
				nRet = BSI_LocalVariable(pTree);
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
		case STreeNode::T_TRY:
			{
				StnTry		&sTry = pTree->sTry;

				nRet = BSI_Tree(sTry.pTryStmt);
				if (nRet != ERR_OK)
					return nRet;

				nRet = BSI_Tree(sTry.pCatch);
				if (nRet != ERR_OK)
					return nRet;
			}
			break;
		case STreeNode::T_CATCH:
			{
				StnCatch	&sCatch = pTree->sCatch;

				m_varDeclStack.EnterBlock();

				nRet = BSI_Tree(sCatch.pException);
				if (nRet != ERR_OK)
					return nRet;

				nRet = BSI_Tree(sCatch.pCatchStmt);
				if (nRet != ERR_OK)
					return nRet;

				ASSERT(m_varDeclStack.GetCurrentBlockSize() == 1);

				m_varDeclStack.LeaveBlock();

				if (sCatch.pNextCatch)
				{
					nRet = BSI_Tree(sCatch.pNextCatch);
					if (nRet != ERR_OK)
						return nRet;
				}
			}
			break;
		case STreeNode::T_THROW:
			{
				StnThrow	&sThrow = pTree->sThrow;

				nRet = BSI_Expression(sThrow.pNewExpr);
				if (nRet != ERR_OK)
					return nRet;
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

	STreeNode	*pMemberNode = sClass.pClassMember;
	while (pMemberNode)
	{
		if (pMemberNode->nodeType == STreeNode::T_MEMBER_METHOD)
		{
			// Method definition
			StnMemberMethod	&sMethod = pMemberNode->sMemberMethod;

			m_varDeclStack.EnterMethod(sMethod.pMethod);

			// Add parameter variable declaration.
			nRet = BSI_MethodDeclare(pMemberNode, pClass);
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
			m_varDeclStack.EnterMethod(pClass->GetVarInitMethod());

			// Add "this" as first parameters of function call
			m_varDeclStack.Add_this_super_Var();

			// Class member variable has been declared in PreBSI_...
			ASSERT(pMemberNode->nodeType == STreeNode::T_VAR_DECLAR);
			nRet = BSI_MemberVariable(pMemberNode, pClass);
			if (nRet != ERR_OK)
				return nRet;

			m_varDeclStack.LeaveBlock();
		}

		pMemberNode = pMemberNode->pNext;
	}

	// Check constructor after class declaration.
	for (size_t i = 0; i < pClass->vMethod.size(); i++)
	{
		CMemberMethod *pMethod = pClass->vMethod[i];
		if (strcmp(pMethod->strName.c_str(), pClass->strName.c_str()) == 0)
		{
			if (pMethod->pBodyNode == NULL)
			{
				// This constructor was inserted by program, it may be NULL
				if (pClass->GetVarInitMethod()->pBodyNode || pClass->pClassParent)
				{
					// Insert empty body node
					newStnDefConstructorDeclare(pClass->strName.c_str(), pMethod, pMethod->nLineNo);
				}
				else
					continue;
			}

			m_varDeclStack.EnterMethod(pMethod);

			// Add "this" as first parameters of function call
			m_varDeclStack.Add_this_super_Var();

			ASSERT(pMethod->pBodyNode);
			nRet = BSI_ConstructorCheck(pMethod, pMethod->pBodyNode, pClass);
			if (nRet != ERR_OK)
				return nRet;

			m_varDeclStack.LeaveBlock();
		}
	}

	m_varDeclStack.LeaveBlock();

	return ERR_OK;
}

int CSJVM::BSI_MethodDeclare(STreeNode *pTree, CClass *pClass)
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
		nRet = BSI_LocalVariable(pNode);
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

int CSJVM::BSI_ConstructorCheck(CMemberMethod *pMethod, STreeNode *pBody, CClass *pClass)
{
	// make sure super() constructor is called, if not, insert it.
	// insert member variable initial statement, after super()

	STreeNode		*pVarInit = pClass->GetVarInitMethod()->pBodyNode;

	// Is super() constructor called already?
	if (pBody && pBody->nodeType == STreeNode::T_EXPRESSION && 
		pBody->sExpression.type == E_METHOD_CALL)
	{
		if (strcmp(pBody->sExpression.functionCall.szFuncName, SZ_SUPER) == 0)
		{
			// insert member variable initial statement, after super()
			if (pVarInit)
				InsertToSibling(pBody, newStnBlock(pBody->nLineNo, pVarInit));

			return ERR_OK; // Super constructor called, return.
		}
	}

	CClass		*pParentClass = pClass->pClassParent;
	if (pParentClass)
	{
		CMemberMethod *pParentConstructor = pParentClass->GetNoParameterConstructor();
		if (!pParentConstructor)
		{
			// All super() constructor has parameter type.
			FORMATLASTERROR(pBody->nLineNo, ERR_C_CONSTRUCTOR_MUST_BE_FIRST_STMT, pMethod->strName.c_str());
			return ERR_C_CONSTRUCTOR_MUST_BE_FIRST_STMT;
		}

		// Add super constructor call
		STreeNode	*pNodeConstructor = newStnExpFuncCall(pBody->nLineNo, SZ_SUPER, NULL, NULL);
		pNodeConstructor->pNext = pMethod->pBodyNode;
		pMethod->pBodyNode = pNodeConstructor;

		int nRet = BSI_ExpressionFunctionCall(pNodeConstructor);
		ASSERT(nRet == ERR_OK);
	}

	// insert member variable initial statement, after super()
	if (pVarInit)
	{
		InsertToSibling(pMethod->pBodyNode, newStnBlock(pBody->nLineNo, pVarInit));
	}

	return ERR_OK;
}

//
// Append member variable initial code in VarInitMethod.
//
int CSJVM::BSI_MemberVariable(STreeNode *pTree, CClass *pClass)
{
	int		nRet;

	// Check variable declaration
	ASSERT(pTree->nodeType == STreeNode::T_VAR_DECLAR);
	StnVarDeclare	&varDeclare = pTree->sVarDeclare;
	ASSERT(varDeclare.type.typeId != TID_INVALID);

	// Check variable name
	TypeId		typeId = varDeclare.type.typeId;
	bool		bStatic = varDeclare.bStatic;
	CMemberMethod	*pMethod;

	if (bStatic)
		pMethod = m_classTable.GetClassStaticVarInitMethod();
	else
		pMethod = pClass->GetVarInitMethod();
	ASSERT(pMethod);

	STreeNode	*pVarNode = varDeclare.pVarList;
	while (pVarNode)
	{
		ASSERT(pVarNode->nodeType == STreeNode::T_VAR);
		StnVar		&var = pVarNode->sVar;

		var.nAddress = m_varDeclStack.GetVariableAddress(var.szVarName);

		// Check variable assign expression.
		if (var.pExpAssign)
		{
			nRet = BSI_Expression(var.pExpAssign);
			if (nRet != ERR_OK)
				return nRet;

			// Can new object assign to left? Type check.
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

			// this.szVarName = pExpAssign
			STreeNode *pThisVar = newStnExpVar(pVarNode->nLineNo, var.szVarName, 
				newStnExpVar(pTree->nLineNo, SZ_THIS, NULL));
			STreeNode *pAssign = newStnExpAssign(pThisVar, var.pExpAssign);

			nRet = BSI_Expression(pAssign);
			if (nRet != ERR_OK)
				return nRet;

			// Add initial assign statement
			if (pMethod->pBodyNode == NULL)
				newStnDefConstructorDeclare(pClass->strName.c_str(), pMethod, pTree->nLineNo);
			AppendToSibling(pMethod->pBodyNode, pAssign);
		}

		pVarNode = pVarNode->pNext;
	}

	return ERR_OK;
}

int CSJVM::BSI_LocalVariable(STreeNode *pTree)
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

		// Add local variable declaration in m_varDeclStack
		nRet = m_varDeclStack.AddVarDecl(var.szVarName, typeId, bStatic);
		if (nRet != ERR_OK)
		{
			FORMATLASTERROR(pVarNode->nLineNo, nRet, var.szVarName);
			return nRet;
		}
		var.nAddress = m_varDeclStack.GetVariableAddress(var.szVarName);

		// Check variable assign expression.
		if (var.pExpAssign)
		{
			nRet = BSI_Expression(var.pExpAssign);
			if (nRet != ERR_OK)
				return nRet;

			// Can new object assign to left? Type check.
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
			if (exp.typeIdValExp == TID_INVALID)
			{
				FORMATLASTERROR(pTree->nLineNo, ERR_C_UNDECLARED_TYPE, exp.newObj.szNewType);
				return ERR_C_UNDECLARED_TYPE;
			}

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
	case E_DYNAMIC_CAST:
		{
			exp.typeIdValExp = GetTypeIdByName(exp.dynCast.szCastToType);
			if (exp.typeIdValExp == TID_INVALID)
			{
				FORMATLASTERROR(pTree->nLineNo, ERR_C_UNDECLARED_TYPE, exp.dynCast.szCastToType);
				return ERR_C_UNDECLARED_TYPE;
			}

			nRet = BSI_Expression(exp.dynCast.pVar);
			if (nRet != ERR_OK)
				return nRet;

			// Is castToType derived from type of pVar?
			if (!m_classTable.IsSubClassOf(exp.dynCast.pVar->sExpression.typeIdValExp, exp.typeIdValExp)
				&& !m_classTable.IsSubClassOf(exp.typeIdValExp, exp.dynCast.pVar->sExpression.typeIdValExp))
			{
				char		szMsg[512];
				sprintf(szMsg, "Can't convert from type '%s' to '%s'.", 
					GetTypeNameStrByID(exp.dynCast.pVar->sExpression.typeIdValExp),
					GetTypeNameStrByID(exp.typeIdValExp));
				FORMATLASTERROR(pTree->nLineNo, ERR_C_CONVERT_VALUE, szMsg);
				return ERR_C_CONVERT_VALUE;
			}

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
	case E_ASSIGN:
		{
			ASSERT(exp.assign.pVar && exp.assign.pRightExp);
			ASSERT(exp.assign.pVar->nodeType == STreeNode::T_EXPRESSION && exp.assign.pVar->sExpression.type == E_VAR);
			ASSERT(exp.assign.pRightExp->nodeType == STreeNode::T_EXPRESSION);

			StnExpression &expVar = exp.assign.pVar->sExpression;
			StnExpression &expRight = exp.assign.pRightExp->sExpression;

			nRet = BSI_Expression(exp.assign.pVar);
			if (nRet != ERR_OK)
				return nRet;

			nRet = BSI_Expression(exp.assign.pRightExp);
			if (nRet != ERR_OK)
				return nRet;

			nRet = CanCovertRightToLeftType(expVar.typeIdValExp, expRight.typeIdValExp);
			if (nRet != ERR_OK)
			{
				char		szMsg[512];
				sprintf(szMsg, "Can't convert from type '%s' to '%s'.", 
					GetTypeNameStrByID(expRight.typeIdValExp),
					GetTypeNameStrByID(expVar.typeIdValExp));
				FORMATLASTERROR(pTree->nLineNo, ERR_C_CONVERT_VALUE, szMsg);
				return ERR_C_CONVERT_VALUE;
			}
			exp.typeIdValExp = expVar.typeIdValExp;

			return ERR_OK;
		}
		break;
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
	case OP_GREATER:	// >
		exp.typeIdValExp = TID_BOOLEAN;		// Boolean result
	case OP_PLUS:		// +
	case OP_MINUS:		// -
	case OP_MULT:		// *
	case OP_DIV:		// /
	case OP_PLUS_ASSIGN:
	case OP_MINUS_ASSIGN:
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
	case OP_BOOL_OR:	// ||
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
			_tprintf(_T("BuildCodeStmt Error: %s\n"), m_strLastErr.c_str());
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

	// Build class member method code.
	for (size_t i = 0; i < pClass->vMethod.size(); i++)
	{
		CMemberMethod *pMethod = pClass->vMethod[i];

		// Build code statement of function body/statement
		nRet = BCS_Tree(pMethod->pBodyNode, pMethod->siMethod);
		if (nRet != ERR_OK)
			return nRet;
	}

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
			if (bStatic)
				*psiTail = m_stmtImage.newStmtExpAssignStaticVarInitial(pVarNode->nLineNo, typeId, siVarAddress, siExpression);
			else
				*psiTail = m_stmtImage.newStmtExpAssign(pVarNode->nLineNo, typeId, siVarAddress, siExpression);
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
			if (bStatic)
				*psiTail = m_stmtImage.newStmtExpAssignStaticVarInitial(pVarNode->nLineNo, typeId, siVarAddress, siExpression);
			else
				*psiTail = m_stmtImage.newStmtExpAssign(pVarNode->nLineNo, typeId, siVarAddress, siExpression);
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
	case E_DYNAMIC_CAST:
		{
			stmt_index	siVar = SI_INVALID;
			nRet = BCS_Expression(exp.dynCast.pVar, siVar);
			if (nRet != ERR_OK)
				return nRet;

			siOut = m_stmtImage.newStmtExpDynCast(pTree->nLineNo, exp.typeIdValExp, siVar);
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
	case E_ASSIGN:
		{
			ASSERT(exp.assign.pVar && exp.assign.pRightExp);
			ASSERT(exp.assign.pVar->nodeType == STreeNode::T_EXPRESSION && exp.assign.pVar->sExpression.type == E_VAR);
			ASSERT(exp.assign.pRightExp->nodeType == STreeNode::T_EXPRESSION);
			stmt_index		siVar = SI_INVALID, siRightExpr = SI_INVALID;

			nRet = BCS_Expression(exp.assign.pVar, siVar);
			if (nRet != ERR_OK)
				return nRet;
			nRet = BCS_Expression(exp.assign.pRightExp, siRightExpr);
			if (nRet != ERR_OK)
				return nRet;

			siOut = m_stmtImage.newStmtExpAssign(pTree->nLineNo, exp.typeIdValExp, siVar, siRightExpr);
			return ERR_OK;
		}
	case E_ASSIGN_STATIC_VAR_INIT:
		ASSERT(0 && "Invalid E_ASSIGN_STATIC_VAR_INIT");
		return ERR_OK;
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
		case STreeNode::T_TRY:
			{
				StnTry		&sTry = pTree->sTry;
				stmt_index	siTryStmt = SI_INVALID, siCatch = SI_INVALID;

				nRet = BCS_Tree(sTry.pTryStmt, siTryStmt);
				if (nRet != ERR_OK)
					return nRet;

				nRet = BCS_Tree(sTry.pCatch, siCatch);
				if (nRet != ERR_OK)
					return nRet;

				*psiTail = m_stmtImage.newStmtTry(pTree->nLineNo, siTryStmt, siCatch);
			}
			break;
		case STreeNode::T_CATCH:
			{
				StnCatch	&sCatch = pTree->sCatch;
				stmt_index	siCatchStmt = SI_INVALID, siNextCatch = SI_INVALID;
				TypeId		typeIdException;

				ASSERT(sCatch.pException->nodeType == STreeNode::T_VAR_DECLAR);
				typeIdException = sCatch.pException->sVarDeclare.type.typeId;

				m_varDeclStack.EnterBlock();

				nRet = BCS_Tree(sCatch.pCatchStmt, siCatchStmt);
				if (nRet != ERR_OK)
					return nRet;

				if (sCatch.pNextCatch)
				{
					nRet = BCS_Tree(sCatch.pNextCatch, siNextCatch);
					if (nRet != ERR_OK)
						return nRet;
				}

				m_varDeclStack.LeaveBlock();

				*psiTail = m_stmtImage.newStmtCatch(pTree->nLineNo, typeIdException, siCatchStmt, siNextCatch);
			}
			break;
		case STreeNode::T_THROW:
			{
				StnThrow	&sThrow = pTree->sThrow;
				stmt_index	siNewExpr = SI_INVALID;

				nRet = BCS_Expression(sThrow.pNewExpr, siNewExpr);
				if (nRet != ERR_OK)
					return nRet;

				*psiTail = m_stmtImage.newStmtThrow(pTree->nLineNo, siNewExpr);
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

// #define LOG_CODE_EXE

#ifdef LOG_CODE_EXE

// CE == CODE_EXE

static int	g_Deep = 0;

void printSpace() {
	for (int i = 0; i < g_Deep + 8; i++) printf(" ");
}

void printSpace(int nLineNo){
	printf("%3d     ", nLineNo);
	for (int i = 0; i < g_Deep; i++) printf(" ");
}

class CAutoBlock {
public:
	CAutoBlock(LPCSTR szName, int nLineNo) : m_szName(szName)
		{ printSpace(nLineNo); printf("Enter >> %s\n", szName); g_Deep++; }
	~CAutoBlock() { g_Deep--; printSpace(); printf("Leave << %s\n", m_szName); }
	LPCSTR		m_szName;
};

#define CE_AUTO_BLOCK(szName, nLineNo)		CAutoBlock	__autoBlcok(szName, nLineNo)

/*void CE_LOG(int nLineNo, LPCSTR szName)
{
	printSpace(nLineNo);
	printf("%s\n", szName); 
}*/

void CE_LOG(int nLineNo, LPCSTR szFormat, ...)
{
	va_list		args;

	printSpace(nLineNo);

	va_start(args, szFormat);
	vprintf(szFormat, args);
	va_end(args);
	printf("\n");
}

#define CE_LOG0(nLineNo, szFormat)	CE_LOG(nLineNo, szFormat)
#define CE_LOG1(nLineNo, szFormat, p1)	CE_LOG(nLineNo, szFormat, p1)
#define CE_LOG2(nLineNo, szFormat, p1, p2)	CE_LOG(nLineNo, szFormat, p1, p2)
#define CE_LOG3(nLineNo, szFormat, p1, p2, p3)	CE_LOG(nLineNo, szFormat, p1, p2, p3)


#else
#define CE_AUTO_BLOCK(szName, nLineNo)
#define CE_LOG0(nLineNo, szFormat)	
#define CE_LOG1(nLineNo, szFormat, p1)	
#define CE_LOG2(nLineNo, szFormat, p1, p2)	
#define CE_LOG3(nLineNo, szFormat, p1, p2, p3)	
#endif

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
					{
						size_t	sp = m_stack.size();

						CE_AUTO_BLOCK("{ }", ps->nLineNo);
						Execute(sBlock.siFirstChild);

						if (m_exeAction == EXE_EXCEPTION)
							break;

						m_stack.Resize(sp);
					}
				}
				break;
			case ST_TRY:
				{
					CE_AUTO_BLOCK("try", ps->nLineNo);
					StmtTry	&sTry = ps->sTry;
					size_t	sp = m_stack.size(), bp = m_bp;
					int		nCallStackDeep = m_nCallStackDeep;

					ASSERT(m_exeAction == EXE_GO_ON);
					nRet = Execute(sTry.siTryStmt);
					if (m_exeAction == EXE_EXCEPTION)
					{
						// Exception occurs, recover sp, and bp
						m_bp = bp;
						m_nCallStackDeep = nCallStackDeep;
						m_stack.Resize(sp);
						nRet = Execute(sTry.siCatch);
					}
				}
				break;
			case ST_CATCH:
				{
					CE_AUTO_BLOCK("catch", ps->nLineNo);
					StmtCatch	&sCatch = ps->sCatch;

					ASSERT(m_exeAction == EXE_EXCEPTION);
					ASSERT(m_typeIdException != TID_INVALID);
					ASSERT(m_jobjException != JOBJ_NULL);
					if (sCatch.typeIdException == m_typeIdException)
					{
						m_exeAction = EXE_GO_ON;

						// Exception variable is at top of stack, just push it.
						m_stack.Push(m_jobjException, m_typeIdException);

						nRet = Execute(sCatch.siCatchStmt);

						// Popup exception variable
						m_stack.Pop();
					}
					else if (sCatch.siNextCatch != SI_INVALID)
						nRet = Execute(sCatch.siNextCatch);
				}
				break;
			case ST_THROW:
				{
					CE_AUTO_BLOCK("throw", ps->nLineNo);
					StmtThrow	&sThrow = ps->sThrow;

					nRet = ExecuteExpression(sThrow.siNewExpr);
					ASSERT(nRet != JOBJ_NULL);
					m_exeAction = EXE_EXCEPTION;
					m_jobjException = nRet;
					if (m_jobjException != JOBJ_NULL)
					{
						CJObject *pObj = m_jobjMgr.Get(m_jobjException);
						m_typeIdException = pObj->m_typeId;
					}
				}
				break;
			case ST_IF:
				{
					CE_AUTO_BLOCK("if", ps->nLineNo);
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
					CE_AUTO_BLOCK("for", ps->nLineNo);

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
					CE_AUTO_BLOCK("while", ps->nLineNo);

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
					CE_AUTO_BLOCK("do while", ps->nLineNo);

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
			case ST_RETURN:
				{
					CE_AUTO_BLOCK("return", ps->nLineNo);

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
		CE_LOG1(ps->nLineNo, "Const: %d", expr.nConstValue);
		return expr.nConstValue;
	case E_CONST_STRING:
#ifdef LOG_CODE_EXE
		{
			CJObject	*pObj = m_jobjMgr.Get(expr.nConstValue);
			if (pObj && pObj->m_typeId == TID_STRING)
			{
				// valid input
				CStrJObject	*pStrObj = (CStrJObject *)pObj;
				CE_LOG1(ps->nLineNo, "Const String: %s", pStrObj->GetStrValue());
			}
		}
#endif
		return expr.nConstValue;
	case E_NEW:
		CE_LOG0(ps->nLineNo, "NewJObject");
		return ExeNewJObject(expr);
	case E_DYNAMIC_CAST:
		{
			CE_LOG1(ps->nLineNo, "DynamicCast: %d", expr.dynCast.siVar);
			int		nVarValue;
			nVarValue = ExecuteExpression(expr.dynCast.siVar);
			if (nVarValue != JOBJ_NULL)
			{
				CJObject	*pObj = m_jobjMgr.Get(nVarValue);
				ASSERT(pObj);
				if (pObj)
				{
					int nRet = CanCovertRightToLeftType(expr.typeId, pObj->m_typeId);
					if (nRet != ERR_OK)
					{
						TCHAR		szMsg[512];
						_stprintf(szMsg, _T("Can't convert from type '%S' to '%S'."), 
							GetTypeNameStrByID(pObj->m_typeId),
							GetTypeNameStrByID(expr.typeId));
						ThrowException(szMsg);
						return 0;
					}
				}
			}
			return nVarValue;
		}
	case E_VAR:
		{
			int		nRet;
			ASSERT(expr.var.nAddress >= 0);
			if (expr.var.bStatic)
				nRet = GetStaticVarValue(expr.var.nAddress);
			else
			{
				if (expr.var.siParentVar != SI_INVALID)
				{
					// A Jobject member varaible
					int		nParentValue;
					nParentValue = ExecuteExpression(expr.var.siParentVar);
					nRet = GetJObjectMemberVarValue(nParentValue, expr.var.nAddress);
				}
				else
				{
					// Variable is in stack
					nRet = m_stack.GetValue(m_bp + expr.var.nAddress);
				}
			}
			CE_LOG1(ps->nLineNo, "Var Value: %d", nRet);
			return nRet;
		}
	case E_METHOD_CALL:
		{
			Statement		*pStmt;
			int				nParentVarValue = 0;
			CVMParameters	parameters;

			if (!expr.functionCall.bStatic)
			{
				ASSERT(expr.functionCall.siParentVar != SI_INVALID);
				nParentVarValue = ExecuteExpression(expr.functionCall.siParentVar);

				pStmt = m_stmtImage.GetStmt(expr.functionCall.siParentVar);
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
				parameters.Push(nParamValue, pStmt->sExpression.typeId);

				CE_LOG2(ps->nLineNo, "Method Parameter: %d, type: %d", nParamValue, pStmt->sExpression.typeId);

				siParameter = m_stmtImage.GetNext(siParameter);
			}

			// Call method
			int		nRet;
			nRet = CallMethod(expr.functionCall.typeIdSpecialfiedClass, nParentVarValue, expr.functionCall.nMemberIndex, parameters);

			CE_LOG1(ps->nLineNo, "Call Method: return: %d", nRet);

			if (m_exeAction == EXE_FUNC_RETURN)
				m_exeAction = EXE_GO_ON;

			return nRet;
		}
	case E_ASSIGN:
		{
			int			*pAddress;
			int			nRet;

			nRet = ExecuteExpression(expr.assign.siExpression);
			pAddress = ExeExprGetVarAddr(expr.assign.siVarAddress);
			if (pAddress)
			{
				*pAddress = nRet;
				CE_LOG1(ps->nLineNo, " = %d", *pAddress);
			}
			return nRet;
		}
		break;
	case E_ASSIGN_STATIC_VAR_INIT:
		{
			CE_AUTO_BLOCK(" static var initial = ", ps->nLineNo);

			StaticVar	*staticVar = ExeExprGetStaticVar(expr.assign.siVarAddress);

			if (staticVar && !staticVar->bInitialized)
			{
				staticVar->nValue = ExecuteExpression(expr.assign.siExpression);
				if (staticVar)
				{
					staticVar = ExeExprGetStaticVar(expr.assign.siVarAddress);
					staticVar->bInitialized = true;
				}
				return staticVar->nValue;
			}
			return 0;
		}
		break;
	default:
		break;
	}

	int		nLeftVal = 0, nRightVal = 0;

	if (expr.expType != OP_OP_INC && expr.expType != OP_OP_DEC
		&& expr.expType != OP_PLUS_ASSIGN && expr.expType != OP_MINUS_ASSIGN)
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
			if (pInt)
			{
				int		nOld = *pInt;
				(*pInt)++;
				return nOld;
			}
		}
	case OP_OP_DEC:		// --
		{
			int		*pInt = ExeExprGetVarAddr(expr.opParameter.siExprLeft);
			if (pInt)
			{
				int		nOld = *pInt;
				(*pInt)--;
				return nOld;
			}
		}
	case OP_PLUS_ASSIGN:	// +=
		{
			int		*pInt = ExeExprGetVarAddr(expr.opParameter.siExprLeft);
			if (pInt)
			{
				int		nOld = *pInt;
				(*pInt) += ExecuteExpression(expr.opParameter.siExprRight);
				return nOld;
			}
		}
	case OP_MINUS_ASSIGN:	// -=
		{
			int		*pInt = ExeExprGetVarAddr(expr.opParameter.siExprLeft);
			if (pInt)
			{
				int		nOld = *pInt;
				(*pInt) -= ExecuteExpression(expr.opParameter.siExprRight);
				return nOld;
			}
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
	{
		ASSERT(expr.var.nAddress < (int)m_vStaticVars.size());
		if (expr.var.nAddress >= (int)m_vStaticVars.size())
			return NULL;

		// ASSERT(m_vStaticVars[expr.var.nAddress].bInitialized);
		return &(m_vStaticVars[expr.var.nAddress].nValue);
	}

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

StaticVar *CSJVM::ExeExprGetStaticVar(stmt_index siExprVar)
{
	Statement		*ps = m_stmtImage.GetStmt(siExprVar);
	ASSERT(ps->stmtType == ST_EXPRESSION);
	StmtExpression	&expr = ps->sExpression;
	ASSERT(expr.expType == E_VAR);
	ASSERT(expr.var.bStatic);

	ASSERT(expr.var.nAddress < (int)m_vStaticVars.size());
	if (expr.var.nAddress >= (int)m_vStaticVars.size())
		return NULL;

	return &(m_vStaticVars[expr.var.nAddress]);
}

int CSJVM::ExeNewJObject(StmtExpression &expr)
{
	CClass	*pClass = m_classTable.GetClass(expr.typeId);
	ASSERT(pClass);
	if (!pClass)
		return 0;

	CVMParameters	parameters;

	// New jobject
	int		nJObj = m_jobjMgr.New(pClass);

	// Execute parameter expressions
	// And push parameter in stack
	stmt_index siParameter = expr.newObj.siParameter;
	while (siParameter != SI_INVALID)
	{
		Statement *pStmt = m_stmtImage.GetStmt(siParameter);
		ASSERT(pStmt);
		int		nParamValue = ExecuteExpression(siParameter);

		// Push value in stack
		parameters.Push(nParamValue, pStmt->sExpression.typeId);

		siParameter = m_stmtImage.GetNext(siParameter);
	}

	CMemberMethod	*pMethod;

	// Execute construction method.
	pMethod = pClass->GetMethod(expr.newObj.nMemberIndex);
	ASSERT(pMethod);
	if (!pMethod)
		return 0;

	CE_LOG3(pMethod->nLineNo, "Call Constructor: type: %d, JObj: %d, Constructor Index: %d", expr.typeId, nJObj, expr.newObj.nMemberIndex);

	CallMethod(expr.typeId, nJObj, expr.newObj.nMemberIndex, parameters);

	return nJObj;
}

int CSJVM::CallMethod(TypeId typeId, int nJObject, int nMethodIndex, CVMParameters &parameters)
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

	CMemberMethod *pMethod = pClass->GetMethod(nMethodIndex);
	if (!pMethod)
		return 0;

	m_nCallStackDeep++;
	if (m_nCallStackDeep > MAX_CALL_STACK_DEEP)
		return 0;

	m_valueMethodReturn = -1;
	if (pMethod->siMethod == SI_INVALID)
	{
		// Call native member method.
		if (pMethod->pNativeFunc)
			m_valueMethodReturn = CallNativeAPI(pMethod, nJObject, parameters);
		else
			ASSERT(strcmp(pClass->strName.c_str(), pMethod->strName.c_str()) == 0);
	}
	else
	{
		m_bp = m_stack.size();

		if (!pMethod->bStatic)
		{
			// push this
			m_stack.Push(nJObject, typeId);
		}

		m_stack.Push(parameters);

		// Return value is in m_valueMethodReturn.
		Execute(pMethod->siMethod);
		if (pMethod->typeIDRet == TID_VOID)
			m_valueMethodReturn = 0;
	}

	m_nCallStackDeep--;

	// Recover bp, sp, and free object allocated in previous function of stack
	m_stack.Resize(nSp);
	m_bp = nBp;

	return m_valueMethodReturn;
}

void CSJVM::ThrowException(LPCTSTR szException)
{
	m_exeAction = EXE_EXCEPTION;
	CJObject		*pJObject;

	pJObject = NewJObject(TID_EXCEPTION, m_jobjException);
	m_typeIdException = TID_EXCEPTION;
	if (pJObject)
	{
		CSjobjException	*pObjException = (CSjobjException *)pJObject;
		pObjException->strException = szException;
	}
}

int CSJVM::GetStaticVarValue(size_t nAddress)
{
	ASSERT(nAddress < m_vStaticVars.size());
	if (nAddress >= m_vStaticVars.size())
		return 0;

	// ASSERT(m_vStaticVars[nAddress].bInitialized);

	return m_vStaticVars[nAddress].nValue;
}

int CSJVM::GetJObjectMemberVarValue(int nJobject, int nMember)
{
	CJObject	*pObj = m_jobjMgr.Get(nJobject);
	if (!pObj)
		return 0;

	return pObj->GetValue(nMember);
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


static ID_TO_STRING	__ErrID2Str[] = 
{
	{ ERR_C_UNDECLARED_ID, _T("undeclared identifier") },
	{ ERR_C_UNDECLARED_TYPE, _T("undeclared class or type") },
	{ ERR_C_UNDECLARED_METHOD, _T("undeclared class member method") },
	{ ERR_C_UNDECLARED_VAR, _T("undeclared variable") },
	{ ERR_C_UNDECLARED_MEMBER_VAR, _T("undeclared member variable") },
	{ ERR_C_PARSE, _T("Parse script file") },
	{ ERR_C_REDEFINITION_ID, _T("redefinition of identifier") },
	{ ERR_C_OP_TYPE_NOT_MATCH, _T("operand type not match") },
	{ ERR_C_OP_REQUIRE_INT_STR_TYPE, _T("operand type should be int or string") },
	{ ERR_C_OP_REQUIRE_INT_TYPE, _T("operand type should be int") },
	{ ERR_C_OP_REQUIRE_BOOL_TYPE, _T("operand type should be boolean") },
	{ ERR_C_REDEFINITION_TYPE, _T("redefinition of type") },
	{ ERR_C_REDEFINITION_METHOD, _T("redefinition of class method") },
	{ ERR_C_CONVERT_VALUE, _T("Can't convert value to another") },
	{ ERR_C_UNEXPECTED_ERR, _T("Got unexpected error") },
	{ ERR_C_FUNC_RETURN_VALUE, _T("Method return value is incorrect.") },
	{ ERR_C_NEW_WRONG_TYPE, _T("New type is incorrect.") },
	{ ERR_C_NOT_CLASS_TYPE, _T("Not a class type, it hasn't member variable or method, or can NOT be extended.") },
	{ ERR_C_CAN_NOT_CALL_NONE_STATIC_MEMBER, _T("Static method of class, can NOT call NONE static member.") },
	{ ERR_C_REQUIRE_LOGICAL_CONDITION, _T("Logical condition is required in (if, while, etc)") },
	{ ERR_C_METHOD_CALL_PARAMETER_COUNT, _T("Class method parameter count is incorrect.") },
	{ ERR_C_CLASS_TYPE_NOT_VAR, _T("It's class type, not a variable.") },
	{ ERR_C_CLASS_HIERARCHY_INCONSISTENT, _T("The hierarchy of class is inconsistent.") },
	{ ERR_C_OVERRIDE_METHOD_RET_DIFF, _T("Override method must have same return value type.") },
	{ ERR_C_DUMPLICATE_METHOD, _T("Duplicated member method.") },
	{ ERR_C_CONSTRUCTOR_MUST_BE_FIRST_STMT, _T("Constructor must be the first statement in a constructor.") },
	{ ERR_C_CONSTRUCTOR_OBJ_UNDEFINED, _T("The constructor object is undefined.") },
	{ ERR_C_CONSTRUCTOR_NO_RETRUN_VALUE, _T("The constructor should NOT have return value.") },
	{ ERR_C_RETRUN_TYPE_MISSING, _T("Return type for the method is missing.") },
	{ ERR_C_NO_THIS_SUPER_IN_STATIC_METHOD, _T("Can NOT use super in a static method.") },
	{ ERR_C_BREAK_CONTINUE_MUST_IN_LOOP, _T("Break/continue statement must be in a loop.") },

	{ ERR_R_NO_ENTRY_POINT, _T("Can NOT find the entry point.") },

	{ 0, NULL}
};

static RegErr2Str	__addErr2Str(__ErrID2Str);

void CSJVM::FormatLastError(int nLine, int nError, LPCSTR szStr, int nLineSrc)
{
	TCHAR		szBuffer[512] = _T("");
	tstringex	strW;
	ConvertStr(szStr, -1, strW);
	_stprintf(szBuffer, _T("%4d: Line: %d, %s: %s"), nLineSrc, nLine, 
		(LPCTSTR)Error2Str(nError), strW.c_str());

	m_strLastErr = szBuffer;
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
