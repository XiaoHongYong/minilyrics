// LyricShowAgentObj.h: interface for the CLyricShowAgentObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LYRICSHOWAGENTOBJ_H__6AE48563_CB64_44CF_8578_76C673DC1F4C__INCLUDED_)
#define AFX_LYRICSHOWAGENTOBJ_H__6AE48563_CB64_44CF_8578_76C673DC1F4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LyricShowObj.h"
#include "LyricShowMultiRowObj.h"
#include "LyricShowTxtObj.h"
#include "LyricShowTwoRowObj.h"
#include "LyricShowHorzScrollObj.h"
#include "LyricShowSingleRowObj.h"
#include "LyricShowVobSub.h"
#include "../Skin/SkinLinearContainer.h"

class CLyricShowAgentObj : public CSkinLinearContainer, IEventHandler  
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinLinearContainer)
public:
	CLyricShowAgentObj();
	virtual ~CLyricShowAgentObj();
	
	virtual void OnCreate();

	// IEventHandler
	virtual void OnEvent(const IEvent *pEvent);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

public:
	static void GetLyrDispStylePropName(CSkinWnd *pSkinWnd, bool &bFloatingLyr, tstring &strLyrStylePropName);
	static void GetLyrDispStyleSettings(CSkinWnd *pSkinWnd, tstring &strLyrStyle);
	static void SetLyrDispStyleSettings(CSkinWnd *pSkinWnd, LPCTSTR szLyrStyle);

protected:
	void CreateInfoTextCtrl();

	void ChangeLyricsDisplayStyle(LPCTSTR szLyrDispalyStyle, bool bRedraw = true);

	void LoadToolbar();

protected:
	CUIObject		*m_pToolbar;
	bool			m_bEnableToolbar;
	class CMPSkinInfoTextCtrl	*m_pInfoTextCtrl;

	bool			m_bFloatingLyr;
	tstring			m_strLyrDisplayStylePropName;
	tstring			m_strLyrDisplayStyleDefault;
	tstring			m_strLyrDisplayStyleCurrent;
	CLyricShowObj	*m_pLyricsShow;
	vector<tstring>	m_vProperties;
	bool			m_bEnableStaticTextStyle;

};

#endif // !defined(AFX_LYRICSHOWAGENTOBJ_H__6AE48563_CB64_44CF_8578_76C673DC1F4C__INCLUDED_)
