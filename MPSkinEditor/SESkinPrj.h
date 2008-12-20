// SESkinPrj.h: interface for the CSESkinPrj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SESKINPRJ_H__D74E01C4_62F2_4695_89E1_D6942F4801DC__INCLUDED_)
#define AFX_SESKINPRJ_H__D74E01C4_62F2_4695_89E1_D6942F4801DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SESkinFactory.h"

extern class CSESkinPrj			g_skinPrj;
extern class CSESkinFactory		g_skinFactory;

extern class CSEUIAdapter		g_seUIAdapter;

class CSEUIAdapter
{
public:
	CSEUIAdapter();
	virtual ~CSEUIAdapter();

	void SetMousePointerInfo(int x, int y);

protected:

};

class ISESkinDataItem
{
public:
	virtual void OnSave() = 0;

};

class CSESkinPrj  
{
public:
	CSESkinPrj();
	virtual ~CSESkinPrj();

	int Open(LPCTSTR szSkinFile);

	int Save();

	void Close();

	bool IsOpened() { return true; }
	bool IsModified();

	void OnCreateSkinDataItem(ISESkinDataItem *pSkinDataItem);
	void OnDestroySkinDataItem(ISESkinDataItem *pSkinDataItem);

	CSESkinFactory *GetSkinFactory() { return &g_skinFactory; }

protected:
	// CSkinFactory		m_skinFactory;
	typedef vector<ISESkinDataItem *>		V_SKIN_DATA_ITEM;

	V_SKIN_DATA_ITEM	m_vSkinDataItems;

};

#endif // !defined(AFX_SESKINPRJ_H__D74E01C4_62F2_4695_89E1_D6942F4801DC__INCLUDED_)
