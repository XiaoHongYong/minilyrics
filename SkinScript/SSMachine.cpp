// SSMachine.cpp: implementation of the CSSMachine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSCompiler.h"
#include "SSMachine.h"

class CSSMClassEventAdapter
{
public:
	int				m_nClassID;
	string			m_strClassName;

	HHEAPDATA		m_hHeapCls;
	CSSMProcess		*m_pProcess;
	CClassDeclarationTable::ClassDecl	*m_pclassDecl;

	CSSMClassEventAdapter()
	{
		m_pclassDecl = NULL;
		m_pProcess = NULL;
		m_nClassID = INVALID_TYPE_ID;
		m_hHeapCls = NULL;
	}

	virtual void OnDispatchEvent(LPCSTR szEventName, CSSMStack *params)
	{
		int		nFuncID = m_pclassDecl->m_methodTable.GetFuncIDByName(szEventName);
		if (nFuncID == -1)
			return;

		params->InsertFront((int)m_hHeapCls);

		m_pProcess->CallFun(m_nClassID, nFuncID, params);
	}

	int Set(CSSMProcess *pProcess, LPCSTR szClassName, HHEAPDATA hHeapCls)
	{
		m_pProcess = pProcess;
		m_strClassName = szClassName;
		m_nClassID = m_pProcess->GetClassIDByName(szClassName);
		ASSERT(m_nClassID != INVALID_TYPE_ID);
		if (m_nClassID == INVALID_TYPE_ID)
			return ERR_NOT_FOUND;

		m_hHeapCls = hHeapCls;
		m_pclassDecl = m_pProcess->GetClassByID(m_nClassID);
		ASSERT(m_pclassDecl);

		return ERR_OK;
	}

};

class CSSMPlayerEventAdapter : public CSSMClassEventAdapter
{
public:
	void OnSkinWndClose()
	{
		CSSMStack	params;
		params.Push((int)"skinwndname");
		OnDispatchEvent("OnSkinWndClose", &params);
	}

};

class CSSMCAPIString : public CSSMClassAPIBase
{
public:
	int RegClassAPI(CSSMachine *pMachine, CClassDeclarationTable &classDeclTable)
	{
		CFunctionDeclTable::FunctionDecl	*func;
		CClassDeclarationTable::ClassDecl	*cls;

		m_pMachine = pMachine;

		cls = new CClassDeclarationTable::ClassDecl("CString");

		func = new CFunctionDeclTable::FunctionDecl("c_str", g_typeIDString);
		m_c_str = cls->m_methodTable.AddFunc(func);

		func = new CFunctionDeclTable::FunctionDecl("assign", g_typeIDVoid);
		func->AddParam(g_typeIDString);
		m_assign = cls->m_methodTable.AddFunc(func);

		m_nClassID = classDeclTable.AddClass(cls);

		return ERR_OK;
	}

	virtual HHEAPDATA New(CSSMHeap &heap)
	{
		HHEAPDATA hHeap = heap.New(1);
		hHeap->nData[1] = (int)new string;
		return hHeap;
	}

	virtual int OnCall(int nFuncNo, CSSMStack &params, int &nRetValue)
	{
		HHEAPDATA		hHeap = (HHEAPDATA) params.GetValue(0);
		string			*str = (string*)hHeap->nData[1];

		if (!str)
			return ERR_R_FUNC_PARAM;

		if (nFuncNo == m_c_str)
		{
			nRetValue = (int)str->c_str();
		}
		else if (nFuncNo == m_assign)
		{
			LPCSTR	szValue = (LPCSTR)params.GetValue(1);
			ASSERT(szValue);
			*str = szValue;
		}
		else
			return ERR_NOT_FOUND;

		return ERR_OK;
	}

	int			m_c_str;
	int			m_assign;

};

class CSSMFAPIDebugPrint : public CSSMFunctionAPIBase
{
public:
	virtual int RegFunction(CSSMachine *pMachine, CFunctionDeclTable &funcDeclTable)
	{
		CFunctionDeclTable::FunctionDecl	*func;

		m_pMachine = pMachine;

		func = new CFunctionDeclTable::FunctionDecl("print", g_typeIDVoid);
		func->AddParam(g_typeIDInt);
		m_print = AddFunc(funcDeclTable, func);

		func = new CFunctionDeclTable::FunctionDecl("prints", g_typeIDVoid);
		func->AddParam(g_typeIDString);
		m_prints = AddFunc(funcDeclTable, func);

		return ERR_OK;
	}

	virtual int OnCall(int nFuncNo, CSSMStack &params, int &nRetValue)
	{
		if (nFuncNo == m_print)
		{
			ASSERT(params.size() == 1);
			printf("print: %d\n", params.GetValue(0));
		}
		else if (nFuncNo == m_prints)
		{
			ASSERT(params.size() == 1);
			printf("prints: %s\n", params.GetValue(0));
		}
		else
			return ERR_NOT_FOUND;

		return ERR_OK;
	}

	int			m_print, m_prints;

};


class CSSMFAPIString: public CSSMFunctionAPIBase
{
public:
	virtual int RegFunction(CSSMachine *pMachine, CFunctionDeclTable &funcDeclTable)
	{
		CFunctionDeclTable::FunctionDecl	*func;

		m_pMachine = pMachine;

		func = new CFunctionDeclTable::FunctionDecl("strcmp", g_typeIDInt);
		func->AddParam(g_typeIDString);
		func->AddParam(g_typeIDString);
		m_strcmp = AddFunc(funcDeclTable, func);

		func = new CFunctionDeclTable::FunctionDecl("stricmp", g_typeIDInt);
		func->AddParam(g_typeIDString);
		func->AddParam(g_typeIDString);
		m_stricmp = AddFunc(funcDeclTable, func);

		return ERR_OK;
	}

	virtual int OnCall(int nFuncNo, CSSMStack &params, int &nRetValue)
	{
		if (nFuncNo == m_strcmp)
		{
			ASSERT(params.size() == 2);
			nRetValue = strcmp((LPCSTR)params.GetValue(0), (LPCSTR)params.GetValue(1));
		}
		else if (nFuncNo == m_stricmp)
		{
			ASSERT(params.size() == 2);
			nRetValue = stricmp((LPCSTR)params.GetValue(0), (LPCSTR)params.GetValue(1));
		}
		else
			return ERR_NOT_FOUND;

		return ERR_OK;
	}

	int			m_strcmp, m_stricmp;

};

class CSSMFAPISystem : public CSSMFunctionAPIBase
{
public:
	virtual int RegFunction(CSSMachine *pMachine, CFunctionDeclTable &funcDeclTable)
	{
		CFunctionDeclTable::FunctionDecl	*func;

		m_pMachine = pMachine;

		func = new CFunctionDeclTable::FunctionDecl("RegisterSkinSysHandler", g_typeIDVoid);
		func->AddParam(g_typeIDAny);
		func->AddParam(g_typeIDString);
		m_RegisterSkinSysHandler = AddFunc(funcDeclTable, func);

		return ERR_OK;
	}

	virtual int OnCall(int nFuncNo, CSSMStack &params, int &nRetValue)
	{
		if (nFuncNo == m_RegisterSkinSysHandler)
		{
			ASSERT(params.size() == 2);
			LPCSTR			szClassName = (LPCSTR)params.GetValue(1);
			HHEAPDATA		hHeapData = (HHEAPDATA)params.GetValue(0);
			m_pPlayerEventAdpter = new CSSMPlayerEventAdapter;
			m_pPlayerEventAdpter->Set(m_pMachine->GetCurProcess(), szClassName, hHeapData);

			m_pPlayerEventAdpter->OnSkinWndClose();
		}
		else
			return ERR_NOT_FOUND;

		return ERR_OK;
	}

	int			m_RegisterSkinSysHandler, m_prints;

	CSSMPlayerEventAdapter	*m_pPlayerEventAdpter;

};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern TYPE_NAME_ID			g_typeIDString, g_typeIDInt, g_typeIDVoid;

CSSMachine::CSSMachine()
{
	m_pCurProcess = NULL;

	RegFunctionAPI(new CSSMFAPIDebugPrint);
	RegFunctionAPI(new CSSMFAPISystem);
	RegFunctionAPI(new CSSMFAPIString);

	RegClassAPI(new CSSMCAPIString);
}

CSSMachine::~CSSMachine()
{
	int		i;

	for (i = 0; i < m_vClsAPIs.size(); i++)
		delete m_vClsAPIs[i];

	for (i = 0; i < m_vFunctions.size(); i++)
		delete m_vFunctions[i];
}

int CSSMachine::RunProcess(CSSMProcess *pProcess)
{
	int		nRet;

	m_pCurProcess = pProcess;

	pProcess->CallFun(0, NULL);

	nRet = pProcess->GetFuncIDByName("main");
	if (nRet != -1)
	{
		nRet = pProcess->CallFun(nRet, NULL);
		return nRet;
	}

	return ERR_NOT_FOUND;
}

int CSSMachine::SysCall(int nFuncNo, CSSMStack &params, int &nRetValue)
{
	if (nFuncNo <= INVALID_FUNC_ID || nFuncNo >= m_funcTableSys.GetFuncCount())
		return ERR_NOT_FOUND;

	for (int i = 0; i < m_vFunctions.size(); i++)
	{
		CSSMFunctionAPIBase	*func = m_vFunctions[i];
		if (nFuncNo >= func->m_nMinFuncID && nFuncNo <= func->m_nMaxFuncID)
		{
			return func->OnCall(nFuncNo, params, nRetValue);
		}
	}

	return ERR_NOT_FOUND;
}

int CSSMachine::SysCall(int nClassNo, int nFuncNo, CSSMStack &params, int &nRetValue)
{
	ASSERT(params.size() >= 1);
	if (params.size() < 1)
		return ERR_R_FUNC_PARAM;

	if (nClassNo <= INVALID_TYPE_ID || nClassNo >= m_classDeclTable.GetClassCount())
		return ERR_NOT_FOUND;

	HHEAPDATA		hHeap = (HHEAPDATA) params.GetValue(0);

	for (int i = 0; i < m_vClsAPIs.size(); i++)
	{
		CSSMClassAPIBase		*pclsApi = m_vClsAPIs[i];
		if (pclsApi->m_nClassID == nClassNo)
		{
			return pclsApi->OnCall(nFuncNo, params, nRetValue);
		}
	}
	return ERR_NOT_FOUND;
}

int CSSMachine::NewClass(int nClassID, HHEAPDATA &hHeap, CSSMHeap &heap)
{
	for (int i = 0; i < m_vClsAPIs.size(); i++)
	{
		CSSMClassAPIBase		*pclsApi = m_vClsAPIs[i];

		if (pclsApi->m_nClassID == nClassID)
		{
			hHeap = pclsApi->New(heap);

			return ERR_OK;
		}
	}

	return ERR_NOT_FOUND;
}

int CSSMachine::RegClassAPI(CSSMClassAPIBase *pClsAPI)
{
	int		nRet;

	nRet = pClsAPI->RegClassAPI(this, m_classDeclTable);
	if (nRet != ERR_OK)
	{
		delete pClsAPI;
		return nRet;
	}

	m_vClsAPIs.push_back(pClsAPI);

	return ERR_OK;
}

int CSSMachine::RegFunctionAPI(CSSMFunctionAPIBase *funcApi)
{
	int		nRet;

	nRet = funcApi->RegFunction(this, m_funcTableSys);
	if (nRet != ERR_OK)
	{
		delete funcApi;
		return nRet;
	}

	m_vFunctions.push_back(funcApi);

	return ERR_OK;
}
