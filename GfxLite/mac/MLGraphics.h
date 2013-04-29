// MLGraphics.h: interface for the CMLGraphics class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GTKGRAPHICS_H__D13EC141_707E_4988_B61C_99F775A880A6__INCLUDED_)
#define AFX_GTKGRAPHICS_H__D13EC141_707E_4988_B61C_99F775A880A6__INCLUDED_

#pragma once


#ifndef WIN32
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#define GetRValue(rgb)      ((BYTE)(rgb))
#define GetGValue(rgb)      ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      ((BYTE)((rgb)>>16))
#endif

typedef struct CGContext *CGContextRef;

class MLColor;
class CFontInfo;

class CMLGraphics
{
public:
	CMLGraphics();
	virtual ~CMLGraphics();
	
public:
	virtual void SetFont(CFontInfo *font);
	virtual bool TextOut(int x, int y, LPCTSTR szText, int nLen);
	virtual bool GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize);
	virtual void SetTextColor(const MLColor &color);
	virtual void SetBgColor(const MLColor &color);

	// For MAC only
	CGContextRef GetHandle() const { return m_context; }
	void Attach(CGContextRef context);
	CGContextRef Detach();

protected:
	CGContextRef	m_context;
	CFontInfo		m_fontInfo;

};

#endif // !defined(AFX_GTKGRAPHICS_H__D13EC141_707E_4988_B61C_99F775A880A6__INCLUDED_)
