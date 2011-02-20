// SkinTxtLink.h: interface for the CSkinTxtLink class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINTXTLINK_H__2EE3C6D0_0A0A_4EFD_9BBA_AEE8D767D5BF__INCLUDED_)
#define AFX_SKINTXTLINK_H__2EE3C6D0_0A0A_4EFD_9BBA_AEE8D767D5BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSkinTxtLink : public CUIObject  
{
public:
	CSkinTxtLink();
	virtual ~CSkinTxtLink();

	LPCTSTR GetClassName();
	void Draw(CRawGraph *pGraph);
	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	void OnCreate();

	bool OnLButtonUp(UINT nFlags, CPoint point);

	bool OnLButtonDown(UINT nFlags, CPoint point);

	bool OnMouseMove(UINT nFlags, CPoint point);

public:
	static LPCTSTR ClassName() { return ms_szClassName; }
	static LPCTSTR		ms_szClassName;

protected:
	tstring			m_strLink;
	DWORD			m_dwAlignText;
	CSkinFontProperty	m_font;
	CCursor			m_Cursor;

};

#endif // !defined(AFX_SKINTXTLINK_H__2EE3C6D0_0A0A_4EFD_9BBA_AEE8D767D5BF__INCLUDED_)
