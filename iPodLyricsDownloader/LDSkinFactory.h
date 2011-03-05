#pragma once

class CLDSkinFactory :public CSkinFactory
{
public:
	CLDSkinFactory(CSkinApp *pApp, UIObjectIDDefinition uidDefinition[]);
	~CLDSkinFactory();

	virtual CSkinWnd *NewSkinWnd(LPCTSTR szSkinWndName, bool bMainWnd);

	CUIObject *NewUIObject(CSkinWnd *pSkin, CSkinContainer *pContainer, LPCTSTR szClassName);

	virtual CSkinMenu *LoadPresetMenu(CSkinWnd *pWnd, LPCTSTR szMenu);

};
