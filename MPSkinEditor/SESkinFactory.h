// SESkinFactory.h: interface for the CSESkinFactory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SESKINFACTORY_H__0917B794_32AC_4902_8B2F_63D193E7EB39__INCLUDED_)
#define AFX_SESKINFACTORY_H__0917B794_32AC_4902_8B2F_63D193E7EB39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSESkinFactory : public CSkinFactory  
{
public:
	CSESkinFactory();
	virtual ~CSESkinFactory();

	virtual int OpenSkin(LPCTSTR szSkin);

	virtual void Quit();

	virtual CUIObject * CreateUIObject(CSkinWnd *pSkin, LPCTSTR szClassName);

	virtual CSkinWnd *NewSkinWnd(LPCTSTR szSkinWndName, bool bMainWnd);

	virtual bool LoadMenu(CMenu **ppMenu, LPCTSTR szMenu);

	virtual void OnLanguageChanged();

	virtual CSkinFileXML *GetSkinFile() { return &m_skinFile; }

	CDynamicCmds &GetDynamicCmds() { return m_dynamicCmds; }
	CDynamicCtrls &GetDynamicCtrls() { return m_dynamicCtrls; }

};

#endif // !defined(AFX_SESKINFACTORY_H__0917B794_32AC_4902_8B2F_63D193E7EB39__INCLUDED_)
