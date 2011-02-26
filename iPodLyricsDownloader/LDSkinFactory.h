#pragma once

class CLDSkinFactory :public CSkinFactory
{
public:
	CLDSkinFactory(CSkinApp *pApp, STRING_ID_MAP stringIdMap[]);
	~CLDSkinFactory();

	virtual CSkinWnd *NewSkinWnd(LPCTSTR szSkinWndName, bool bMainWnd);

	CUIObject *NewUIObject(CSkinWnd *pSkin, LPCTSTR szClassName);

	virtual CSkinMenu *LoadPresetMenu(CSkinWnd *pWnd, LPCTSTR szMenu);

};
