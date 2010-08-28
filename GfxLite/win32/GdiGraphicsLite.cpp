// GdiGraphicsLite.cpp: implementation of the CGdiGraphicsLite class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "../GfxLite.h"
#include "GdiGraphicsLite.h"
#include <wingdi.h>

#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY	5
#endif

BYTE		g_byFontQuality = (GetOperationSystemType() >= OPS_WINXP) ? CLEARTYPE_QUALITY : ANTIALIASED_QUALITY;

bool CGdiFont::Create(const CFontInfo &font)
{
	LOGFONT logFont = {25, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, g_byFontQuality, DEFAULT_PITCH | FF_ROMAN, _T("Tahoma")};

	logFont.lfHeight = font.GetHeight();
	logFont.lfWeight = font.GetWeight();
	logFont.lfItalic = font.GetItalic();
	logFont.lfUnderline = font.IsUnderline();
	_tcscpy_safe(logFont.lfFaceName, CountOf(logFont.lfFaceName), font.GetName());

	m_hFont = CreateFontIndirect(&logFont);

	return m_hFont != NULL;
}


class CGdiTextOut
{
public:
	void operator()(CGdiFont *font, LPCTSTR szClip, int nClip)
	{
		HFONT	hFontOld;

		hFontOld = (HFONT)SelectObject(hdc, font->GetHandle());

#ifdef _WINCE_
		::ExtTextOut(hdc, x, y, 0, NULL, szClip, nClip, NULL);
		::GetTextExtentPoint(hdc, szClip, nClip, &size);
#else
		::TextOut(hdc, x, y, szClip, nClip);
		::GetTextExtentPoint32W(hdc, szClip, nClip, &size);
#endif
		x += size.cx;

		SelectObject(hdc, hFontOld);
	}

public:
	int		x, y;
	HDC		hdc;
	SIZE	size;

};

class CGdiGetTextExtentPoint32
{
public:
	void operator()(CGdiFont *font, LPCTSTR szClip, int nClip)
	{
		HFONT	hFontOld;
		SIZE	size;

		hFontOld = (HFONT)SelectObject(hdc, font->GetHandle());

#ifdef _WINCE_
		::GetTextExtentPoint(hdc, szClip, nClip, &size);
#else
		::GetTextExtentPoint32W(hdc, szClip, nClip, &size);
#endif
		pSize->cx += size.cx;
		if (size.cy > pSize->cy)
			pSize->cy = size.cy;

		SelectObject(hdc, hFontOld);
	}

public:
	HDC		hdc;
	LPSIZE	pSize;

};

//////////////////////////////////////////////////////////////////////
CGdiGraphicsLite::CGdiGraphicsLite()
{
	m_hFontOld = NULL;
	m_bResolveTextEncoding = true;
}

CGdiGraphicsLite::~CGdiGraphicsLite()
{
	Detach();
}

HDC CGdiGraphicsLite::Detach()
{
	if (m_hFontOld)
	{
		SelectObject(m_hdc, m_hFontOld);
		m_hFontOld = NULL;
	}

	return CMLGraphics::Detach();
}

void CGdiGraphicsLite::SetFont(CFontInfo *font)
{
	m_fontCollection.SetFont(*font);

	if (!m_bResolveTextEncoding)
	{
		CGdiFont	*pGdiFont = m_fontCollection.GetFont(this, DEFAULT_CHARSET);
		SelectFont(pGdiFont->GetHandle());
	}
}

bool CGdiGraphicsLite::TextOut(int x, int y, LPCTSTR szText, int nLen)
{
	if (m_bResolveTextEncoding)
	{
		CGdiTextOut	funGdiTextOut;

		funGdiTextOut.x = x;
		funGdiTextOut.y = y;
		funGdiTextOut.hdc = m_hdc;
		m_fontCollection.ResovleText(this, szText, nLen, funGdiTextOut);
	}
	else
	{
#ifdef _WINCE_
		::ExtTextOut(m_hdc, x, y, 0, NULL, szText, nLen, NULL);
#else
		::TextOut(m_hdc, x, y, szText, nLen);
#endif
	}
	return TRUE;
}

bool CGdiGraphicsLite::GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize)
{
	if (m_bResolveTextEncoding)
	{
		CGdiGetTextExtentPoint32	funGdi;

		pSize->cx = 0;
		pSize->cy = 0;
		funGdi.pSize = pSize;
		funGdi.hdc = m_hdc;
		m_fontCollection.ResovleText(this, szText, nLen, funGdi);
	}
	else
	{
#ifdef _WINCE_
		::GetTextExtentPoint(m_hdc, szText, nLen, pSize);
#else
		::GetTextExtentPoint32W(m_hdc, szText, nLen, pSize);
#endif
	}
	return TRUE;
}

void CGdiGraphicsLite::SetTextColor(const MLColor &color)
{
	::SetTextColor(m_hdc, color.get());
}

void CGdiGraphicsLite::SetBgColor(const MLColor &color)
{
	::SetBkColor(m_hdc, color.get());
}

void CGdiGraphicsLite::EnableResolveTextEncoding(bool bEnable)
{
	if (!m_bResolveTextEncoding)
	{
		CGdiFont	*pGdiFont = m_fontCollection.GetFont(this, DEFAULT_CHARSET);
		SelectFont(pGdiFont->GetHandle());
	}
}
