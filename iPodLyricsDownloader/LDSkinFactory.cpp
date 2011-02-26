#include "StdAfx.h"
#include "resource.h"
#include "LDSkinFactory.h"
#include "LDSkinWnd.h"
#include "LDSkinWndMain.h"

CLDSkinFactory::CLDSkinFactory(CSkinApp *pApp, STRING_ID_MAP stringIdMap[]) :
	CSkinFactory(pApp, stringIdMap)
{
}

CLDSkinFactory::~CLDSkinFactory()
{
}

CSkinWnd *CLDSkinFactory::NewSkinWnd(LPCTSTR szSkinWndName, bool bMainWnd)
{
	if (bMainWnd)
		return new CLDSkinWndMain();
	else
		return new CLDSkinWnd();
}

CUIObject *CLDSkinFactory::NewUIObject(CSkinWnd *pSkin, LPCTSTR szClassName)
{
	CUIObject		*pObj;

	pObj = CSkinFactory::NewUIObject(pSkin, szClassName);
	if (pObj)
		return pObj;

	return NULL;
}

CSkinMenu *CLDSkinFactory::LoadPresetMenu(CSkinWnd *pWnd, LPCTSTR szMenu)
{
	CSkinMenu		*pSkinMenu = NULL;

	if (_tcsicmp(szMenu, _T("MainMenu")) == 0)
	{
		pSkinMenu = new CSkinMenu(this);
		pSkinMenu->LoadPopupMenu(IDM_POPUP, 0);
	}
	else if (_tcsicmp(szMenu, _T("MainWndMenu")) == 0)
	{
		pSkinMenu = new CSkinMenu(this);
		pSkinMenu->LoadMenu(IDM_MAIN_WND);
	}

	return pSkinMenu;
}
