// SkinTextButton.h: interface for the CSkinTextButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINTEXTBUTTON_H__86F68037_E86D_4927_8B88_47034765E7D0__INCLUDED_)
#define AFX_SKINTEXTBUTTON_H__86F68037_E86D_4927_8B88_47034765E7D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSkinTextButton : public CSkinButton
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinButton)
public:
	CSkinTextButton();
	virtual ~CSkinTextButton();

	void Draw(CRawGraph *pGraph);

	virtual void OnCreate();

	virtual bool OnMenuKey(UINT nChar, UINT nFlags);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	virtual void SetText(LPCTSTR szText);

	virtual void OnLanguageChanged();

protected:
	DWORD				m_dwAlignTextFlags;
	bool				m_bResizeToContent;
	CSFImage			m_imgContent;

	int					m_chMenuKey;	// Menu Key: Alt + VK_X to execute it.

	int					m_nTextLeftMargin, m_nTextRightMargin;

	CSkinFontProperty	m_font;

};

#endif // !defined(AFX_SKINTEXTBUTTON_H__86F68037_E86D_4927_8B88_47034765E7D0__INCLUDED_)
