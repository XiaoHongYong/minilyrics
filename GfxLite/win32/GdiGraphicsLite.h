// GdiGraphics.h: interface for the CGdiGraphicsLite class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GDIGRAPHICS_H__459134AA_966B_474C_B99D_61232BA41BDF__INCLUDED_)
#define AFX_GDIGRAPHICS_H__459134AA_966B_474C_B99D_61232BA41BDF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FontCollection.h"

class CGdiFont
{
public:
	CGdiFont()
	{
		m_hFont = NULL;
	}
	~CGdiFont()
	{
		Destroy();
	}

	bool Create(CMLGraphics *pGraph, LOGFONTA &logFont)
	{
		Destroy();

		m_hFont = CreateFontIndirectA(&logFont);

		return m_hFont != NULL;
	}

	bool Create(const CFontInfo &font);

	void Destroy()
	{
		if (m_hFont)
		{
			DeleteObject(m_hFont);
			m_hFont = NULL;
		}
	}

	HFONT GetHandle() { return m_hFont; }

	bool IsValid() const { return m_hFont != NULL; }

protected:
	HFONT			m_hFont;

};

class CGdiGraphicsLite : public CMLGraphics
{
public:
	CGdiGraphicsLite();
	~CGdiGraphicsLite();

	virtual HDC Detach();

	virtual void SetFont(CFontInfo *font);

	virtual bool TextOut(int x, int y, LPCTSTR szText, int nLen);
	virtual bool GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize);

	virtual void SetTextColor(const MLColor &color);

	virtual void SetBgColor(const MLColor &color);

	virtual void EnableResolveTextEncoding(bool bEnable);

protected:
	void SelectFont(HFONT hFont)
	{
		HFONT		hFontOld;

		hFontOld = (HFONT)SelectObject(m_hdc, hFont);
		if (!m_hFontOld)
			m_hFontOld = hFontOld;
	}

protected:
	CFontCollection<CGdiFont>		m_fontCollection;
	HFONT							m_hFontOld;
	bool							m_bResolveTextEncoding;

};

#endif // !defined(AFX_GDIGRAPHICS_H__459134AA_966B_474C_B99D_61232BA41BDF__INCLUDED_)
