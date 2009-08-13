// SkinPicText.h: interface for the CSkinPicText class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINPICTEXT_H__4D559DA1_FA33_4D9E_83F8_42C34705D79C__INCLUDED_)
#define AFX_SKINPICTEXT_H__4D559DA1_FA33_4D9E_83F8_42C34705D79C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSkinPicText : public CUIObject  
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject);
public:
	CSkinPicText();
	virtual ~CSkinPicText();

	void Draw(CRawGraph *pGraph);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	void SetText(LPCTSTR szText);

protected:
	tstring			m_strImgText;
	CSFImage		m_imgText;
	int				m_nCharWidth;

	tstring			m_strImgChars;
	int				m_nSpaceCharPos;

	DWORD			m_dwAlignTextFlags;

	vector<int>		m_vPos;

};

#endif // !defined(AFX_SKINPICTEXT_H__4D559DA1_FA33_4D9E_83F8_42C34705D79C__INCLUDED_)
