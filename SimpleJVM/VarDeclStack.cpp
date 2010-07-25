#include "StdAfx.h"
#include "SJVM.h"
#include "VarDeclStack.h"


//////////////////////////////////////////////////////////////////////
//
CVarDeclStack::CVarDeclStack()
{
}

CVarDeclStack::~CVarDeclStack()
{
	for (size_t i = 0; i < m_stmtBlockStack.size(); i++)
	{
		StmtBlock	*pblock = m_stmtBlockStack[i];
		FreeStmtBlock(pblock);
	}
}

void CVarDeclStack::EnterClass(CClass *pClass)
{
	ASSERT(m_stmtBlockStack.size() == 0);
	StmtBlock	*pblock = new StmtBlock;
	pblock->m_pClass = pClass;

	m_stmtBlockStack.push_back(pblock);
}

void CVarDeclStack::EnterMethod(CMemberMethod *pMethod)
{
	ASSERT(m_stmtBlockStack.size() == 1);
	StmtBlock	*pblock = new StmtBlock;
	pblock->m_pMethod = pMethod;

	m_stmtBlockStack.push_back(pblock);
}

void CVarDeclStack::EnterBlock()
{
	StmtBlock	*pblock = new StmtBlock;

	m_stmtBlockStack.push_back(pblock);
}

void CVarDeclStack::LeaveBlock()
{
	ASSERT(!m_stmtBlockStack.empty());

	StmtBlock	*pblock = m_stmtBlockStack.back();
	m_stmtBlockStack.pop_back();
	FreeStmtBlock(pblock);
}

void CVarDeclStack::CleanStack()
{
	ASSERT(m_stmtBlockStack.empty());
	for (size_t i = 0; i < m_stmtBlockStack.size(); i++)
		FreeStmtBlock(m_stmtBlockStack[i]);
	m_stmtBlockStack.clear();
}

CClass *CVarDeclStack::GetCurClass()
{
	for (int i = (int)m_stmtBlockStack.size() - 1; i >= 0; i--)
	{
		StmtBlock	*pblock = m_stmtBlockStack[i];
		if (pblock->m_pClass)
			return pblock->m_pClass;
	}

	ASSERT(0);
	return NULL;
}

CMemberMethod *CVarDeclStack::GetCurMethod()
{
	for (int i = (int)m_stmtBlockStack.size() - 1; i >= 0; i--)
	{
		StmtBlock	*pblock = m_stmtBlockStack[i];
		if (pblock->m_pMethod)
			return pblock->m_pMethod;
	}

	ASSERT(0);
	return NULL;
}

int CVarDeclStack::AddVarDecl(LPCSTR szName, TypeId typeId, bool bStatic)
{
	if (m_stmtBlockStack.empty())
	{
		ASSERT(0 && "m_stmtBlockStack shouldn't be empty.");
		return ERR_C_UNEXPECTED_ERR;
	}

	StmtBlock	*pblock = m_stmtBlockStack.back();

	MapVarProp::iterator it = pblock->mapVarProp.find(szName);
	if (it == pblock->mapVarProp.end())
	{
		VarProperty	*pVarProperty = new VarProperty;
		pVarProperty->typeId = typeId;
		pVarProperty->bStatic = bStatic;
		if (bStatic)
			pVarProperty->nAddress = DeclareStaticVar(GetCurMethod(), szName, typeId, (int)m_stmtBlockStack.size());
		else
			pVarProperty->nAddress = (int)GetCurrentMethodLocalVarCount();
		pblock->mapVarProp[szName] = pVarProperty;
		return ERR_OK;
	}

	return ERR_C_REDEFINITION_ID;
}

void CVarDeclStack::Add_this_super_Var()
{
	// "this" is the first parameter of method call
	ASSERT(m_stmtBlockStack.size() == 2 && "this must be added when enter function declaration");

	CClass		*pClass = GetCurClass();
	ASSERT(pClass);
	if (pClass)
		AddVarDecl(SZ_THIS, pClass->typeId, false);
}

int CVarDeclStack::DeclareStaticMemberVar(CClass *pClass, LPCSTR szName, TypeId typeId)
{
	return DeclareStaticVar(pClass, szName, typeId, 0);
}

VarInfo CVarDeclStack::GetVariable(LPCSTR szName)
{
	VarInfo	varInfo;

	varInfo.typeId = TID_INVALID;
	varInfo.nAddress = -1;
	varInfo.nRet = ERR_OK;
	varInfo.bStatic = false;
	varInfo.bMemberVar = false;

	if (strcmp(szName, SZ_SUPER) == 0)
	{
		CClass		*pClass = GetCurClass();
		if (pClass->pClassParent)
		{
			varInfo = GetVariable(SZ_THIS);
			varInfo.typeId = pClass->pClassParent->typeId;
			return varInfo;
		}
	}

	if (m_stmtBlockStack.empty())
		return varInfo;

	for (int i = (int)m_stmtBlockStack.size() - 1; i >= 0; i--)
	{
		StmtBlock	*pblock = m_stmtBlockStack[i];

		MapVarProp::iterator it = pblock->mapVarProp.find(szName);
		if (it == pblock->mapVarProp.end())
		{
			if (pblock->m_pClass)
			{
				// Is it a class member?
				CMemberVar		*pVar = pblock->m_pClass->GetMemberVar(szName);
				if (pVar)
				{
					// Static function can NOT access none static variable.
					ASSERT(GetCurMethod());
					if (GetCurMethod()->bStatic && !pblock->m_pClass->IsMemberVarStatic(szName))
					{
						varInfo.nRet = ERR_C_CAN_NOT_CALL_NONE_STATIC_MEMBER;
						return varInfo;
					}

					varInfo.bMemberVar = true;
					varInfo.bStatic = pVar->bStatic;
					varInfo.typeId = pVar->typeId;
					ASSERT(pVar->nAddr < 128);
					varInfo.nAddress = (char)pVar->nAddr;
					return varInfo;
				}
			}
			continue;
		}

		VarProperty	*pVarProperty = (*it).second;

		varInfo.nAddress = pVarProperty->nAddress;
		varInfo.bStatic = pVarProperty->bStatic;
		varInfo.typeId = pVarProperty->typeId;
		return varInfo;
	}

	if (strcmp(szName, SZ_THIS) == 0 || 
		strcmp(szName, SZ_SUPER) == 0)
		varInfo.nRet = ERR_C_NO_THIS_SUPER_IN_STATIC_METHOD;
	else
		varInfo.nRet = ERR_C_UNDECLARED_TYPE;

	return varInfo;
}

// For local variable, return offset
// For member variable, return member variable index.
int CVarDeclStack::GetVariableAddress(LPCSTR szName)
{
	VarInfo	info = GetVariable(szName);
	ASSERT(info.nRet == ERR_OK);
	return info.nAddress;
}

bool CVarDeclStack::IsStaticVariable(LPCSTR szName)
{
	VarInfo	info = GetVariable(szName);
	ASSERT(info.nRet == ERR_OK);
	return info.bStatic;
}

int CVarDeclStack::GetCurrentBlockSize()
{
	ASSERT(m_stmtBlockStack.size() > 0);
	if (m_stmtBlockStack.size() == 0)
		return 0;

	return (int)m_stmtBlockStack.back()->mapVarProp.size();
}

void CVarDeclStack::InitStaticVar(VecStaticVar &vStaticVar)
{
	vStaticVar.resize(m_mapStaticVars.size());

	for (MapStaticVars::iterator it = m_mapStaticVars.begin(); it != m_mapStaticVars.end(); ++it)
	{
		AddressTypeId	addr = (*it).second;
		vStaticVar[addr.nAddress].bInitialized = false;
		vStaticVar[addr.nAddress].typeId = addr.typeId;
		vStaticVar[addr.nAddress].nValue = 0;
	}
}

void CVarDeclStack::FreeStmtBlock(StmtBlock *pblock)
{
	MapVarProp::iterator	it, itEnd;

	itEnd = pblock->mapVarProp.end();
	for (it = pblock->mapVarProp.begin(); it != itEnd; ++it)
		delete (*it).second;
	pblock->mapVarProp.clear();

	delete pblock;
}

size_t CVarDeclStack::GetCurrentMethodLocalVarCount()
{
	size_t		nCount = 0;

	for (int i = (int)m_stmtBlockStack.size() - 1; i >= 0; i--)
	{
		StmtBlock	*pblock = m_stmtBlockStack[i];
		nCount += pblock->mapVarProp.size();
		if (pblock->m_pClass)
			break;
	}

	return nCount;
}

int CVarDeclStack::DeclareStaticVar(LPVOID pMethodOrClass, LPCSTR szVarName, TypeId typeId, int nStmtDeepOfMethod)
{
	char szKey[32];

	sprintf_s(szKey, CountOf(szKey), "%08X%02d", (int)(int64)pMethodOrClass, nStmtDeepOfMethod);
	string	strKey = szKey;
	strKey += szVarName;

	AddressTypeId	addr;

	MapStaticVars::iterator it = m_mapStaticVars.find(strKey);
	if (it == m_mapStaticVars.end())
	{

		addr.nAddress = (uint16)m_mapStaticVars.size();
		addr.typeId = typeId;
		m_mapStaticVars[strKey] = addr;
	}
	else
		addr = (*it).second;

	return addr.nAddress;
}
