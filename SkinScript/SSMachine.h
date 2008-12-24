// SSMachine.h: interface for the CSSMachine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSMACHINE_H__739BEF47_8F3D_4987_8236_6420064FB738__INCLUDED_)
#define AFX_SSMACHINE_H__739BEF47_8F3D_4987_8236_6420064FB738__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SkinScript.h"
#include "SSCompiler.h"

//
// Base class API
//
// SSMC: SkinScript Machine Class
class CSSMClassAPIBase
{
public:
	CSSMClassAPIBase() { m_nClassID = INVALID_TYPE_ID; m_pMachine = NULL; }
	virtual ~CSSMClassAPIBase() { }
	virtual int RegClassAPI(CSSMachine *pMachine, CClassDeclarationTable &classDeclTable) = 0;
	virtual HHEAPDATA New(CSSMHeap &heap) = 0;
	virtual int OnCall(int nFuncNo, CSSMStack &params, int &nRetValue) { return ERR_NOT_FOUND; }

	int				m_nClassID;
	CSSMachine		*m_pMachine;

};
typedef vector<CSSMClassAPIBase*>		V_SSMCLASSAPI;

//
// Base functions API
//
// SSMF: SkinScript Machine Function
class CSSMFunctionAPIBase
{
public:
	CSSMFunctionAPIBase() { m_nMinFuncID = INVALID_FUNC_ID; m_nMaxFuncID = INVALID_FUNC_ID; }
	virtual ~CSSMFunctionAPIBase() { }

	virtual int RegFunction(CSSMachine *pMachine, CFunctionDeclTable &funcDeclTable) = 0;
	virtual int OnCall(int nFuncNo, CSSMStack &params, int &nRetValue) { return ERR_NOT_FOUND; }

	int AddFunc(CFunctionDeclTable &funcDeclTable, CFunctionDeclTable::FunctionDecl *func)
	{
		int		nFuncID;
		nFuncID = funcDeclTable.AddFunc(func);
		if (nFuncID != INVALID_FUNC_ID)
		{
			if (m_nMinFuncID == INVALID_FUNC_ID)
				m_nMinFuncID = nFuncID;
			m_nMaxFuncID = nFuncID;
		}
		return nFuncID;
	}

	int				m_nMinFuncID, m_nMaxFuncID;
	CSSMachine		*m_pMachine;

};
typedef vector<CSSMFunctionAPIBase*>		V_SSMTFUNCTIONAPI;


class CSSMachine
{
public:
	CSSMachine();
	~CSSMachine();

	int RunProcess(CSSMProcess *pProcess);

	int SysCall(int nFuncNo, CSSMStack &params, int &nRetValue);
	int SysCall(int nClassNo, int nFuncNo, CSSMStack &params, int &nRetValue);

	int NewClass(int nClassID, HHEAPDATA &hHeap, CSSMHeap &heap);

	// function table method
	CFunctionDeclTable::FunctionDecl *GetFuncByID(int nID) { return m_funcTableSys.GetFuncByID(nID); }
	CFunctionDeclTable::FunctionDecl *GetFuncByName(LPCSTR szName) { return m_funcTableSys.GetFuncByName(szName); }
	int GetFuncIDByName(LPCSTR szFuncName) { return m_funcTableSys.GetFuncIDByName(szFuncName); }

	CClassDeclarationTable::ClassDecl *GetClassByID(TYPE_NAME_ID nID) { return m_classDeclTable.GetClassByID(nID); }
	CClassDeclarationTable::ClassDecl *GetClassByName(LPCSTR szName) { return m_classDeclTable.GetClassByName(szName); }
	TYPE_NAME_ID GetClassIDByName(LPCSTR szName) { return m_classDeclTable.GetClassIDByName(szName); }

	int RegClassAPI(CSSMClassAPIBase *pClsAPI);
	int RegFunctionAPI(CSSMFunctionAPIBase *funcApi);

	CSSMProcess	*GetCurProcess() { return m_pCurProcess; }

protected:
	CFunctionDeclTable			m_funcTableSys;
	CClassDeclarationTable		m_classDeclTable;
	CSSMHeap					m_heap;
	typedef map<void *, HHEAPDATA>	MAP_HEAPITEM;

	MAP_HEAPITEM				m_mapHeapItems;
	CSSMProcess					*m_pCurProcess;

	V_SSMCLASSAPI				m_vClsAPIs;
	V_SSMTFUNCTIONAPI			m_vFunctions;

};


#endif // !defined(AFX_SSMACHINE_H__739BEF47_8F3D_4987_8236_6420064FB738__INCLUDED_)
