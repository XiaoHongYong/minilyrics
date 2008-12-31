// GdiGraphics.cpp: implementation of the CGdiGraphics class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "gfx.h"
#include "GdiGraphics.h"
#include <wingdi.h>

#ifdef _WIN32_WCE
int DrawTextEx_WINCE(
			   HDC hdc,                     // handle to DC
			   LPTSTR lpchText,             // text to draw
			   int cchText,                 // length of text to draw
			   LPRECT lprc,                 // rectangle coordinates
			   UINT dwDTFormat             // formatting options
			   )
{
	if (cchText == -1)
		cchText = _tcslen(lpchText);

	ASSERT(dwDTFormat & DT_END_ELLIPSIS);
	dwDTFormat &= ~DT_END_ELLIPSIS;

	static TCHAR		szDot[] = _T("...");
	int		*vDx  = new int[cchText];
	int		nFit = 0;
	int		nRet = 0;
	int		w = lprc->right - lprc->left;
	SIZE	size;
	if (!vDx)
		return 0;

	if (GetTextExtentExPoint(hdc, lpchText, cchText, w, &nFit, vDx, &size) == 0)
		goto R_FAILED;
	ASSERT(nFit <= cchText);

	if (nFit == cchText)
		nRet = DrawText(hdc, lpchText, cchText, lprc, dwDTFormat);
	else
	{
		if (GetTextExtentPoint(hdc, szDot, 3, &size) == 0)
		{
			goto R_FAILED;
		}
		for (int i = nFit; i >= 0; i--)
		{
			if (vDx[i] + size.cx <= w)
			{
				i++;
				TCHAR		*szNewTxt;
				szNewTxt = new TCHAR[i + 3 + 1];
				if (!szNewTxt)
					goto R_FAILED;

				_tcsncpy(szNewTxt, lpchText, i);
				_tcsncpy(szNewTxt + i, szDot, 3);
				szNewTxt[i + 3] = '\0';

				// GetTextExtentPoint(hdc, lpchText, i, &size);
				nRet = DrawText(hdc, szNewTxt, i + 3, lprc, dwDTFormat);
				// GetTextExtentPoint(hdc, szNewTxt, i + 3, &size);
				delete szNewTxt;

				break;
			}
		}
	}

	delete[] vDx;
	return nRet;
R_FAILED:
	delete[] vDx;

	return 0;
}
#endif

bool CGdiFont::Create(const CMLFont &font)
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

class CGdiDrawTextClip
{
public:
	void operator()(CGdiFont *font, LPCTSTR szClip, int nClip)
	{
		if (rc.left >= rc.right)
			return;

		HFONT	hFontOld;
		SIZE	size;

		hFontOld = (HFONT)SelectObject(hdc, font->GetHandle());

		DrawText(hdc, szClip, nClip, &rc, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_TOP);
#ifdef _WINCE_
		::GetTextExtentPoint(hdc, szClip, nClip, &size);
#else
		::GetTextExtentPoint32W(hdc, szClip, nClip, &size);
#endif
		rc.left += size.cx;

		SelectObject(hdc, hFontOld);
	}

public:
	HDC		hdc;
	RECT	rc;

};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGdiGraphics::CGdiGraphics() : IMLGraphicsBase()
{
	m_GraphicsType = MGT_WIN32GDI;
	m_hFontOld = NULL;
	m_hRgnOld = NULL;
	m_nRgnOld = -1;
	m_hPenOld = NULL;

	m_hdcBk = NULL;
}

bool CGdiGraphics::Create(HDC hdc)
{
	m_hdc = hdc;

	return true;
}

void CGdiGraphics::Destroy()
{
	if (m_hFontOld)
	{
		SelectObject(m_hdc, m_hFontOld);
		m_hFontOld = NULL;
	}

	if (m_nRgnOld != -1)
	{
		if (m_nRgnOld == 0)
			SelectClipRgn(m_hdc, NULL);
		else
		{
			SelectObject(m_hdc, m_hRgnOld);
			DeleteObject(m_hRgnOld);
		}
		m_nRgnOld = -1;
	}

	if (m_hPenOld)
	{
		SelectObject(m_hdc, m_hPenOld);
		m_hPenOld = NULL;
	}

	if (m_hdcBk)
	{
		DeleteDC(m_hdcBk);
		m_hdcBk = NULL;
	}

	m_hdc = NULL;
}

CGdiGraphics::~CGdiGraphics()
{
	Destroy();
}

void CGdiGraphics::Line(int x1, int y1, int x2, int y2)
{
	MoveToEx(m_hdc, x1, y1, NULL);
	LineTo(m_hdc, x2, y2);
}

void CGdiGraphics::Rectangle(int x, int y, int width, int height)
{
	::Rectangle(m_hdc, x, y, x + width, y + height);
}

void CGdiGraphics::FillRect(int x, int y, int width, int height, CMLBrush *pBrush)
{
	CRect	rc(x, y, x + width, y + height);

	::FillRect(m_hdc, &rc, pBrush->GetHandle());
}

bool CGdiGraphics::TextOut(int x, int y, LPCTSTR szText, int nLen)
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

bool CGdiGraphics::DrawTextClip(LPCTSTR szText, int nLen, LPRECT lpRect)
{
	if (m_bResolveTextEncoding)
	{
		CGdiDrawTextClip	funGdi;

		funGdi.rc = *lpRect;
		funGdi.hdc = m_hdc;
		m_fontCollection.ResovleText(this, szText, nLen, funGdi);
	}
	else
		::DrawText(m_hdc, szText, nLen, lpRect, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_TOP);
	return TRUE;
}

bool CGdiGraphics::TextOutOutlined(int x, int y, LPCTSTR szText, int nLen, const MLColor &clrText, const MLColor &clrBorder)
{
	if (m_bResolveTextEncoding)
	{
		CGdiTextOut	funGdiTextOut;

		funGdiTextOut.hdc = m_hdc;

		funGdiTextOut.y = y;

		// draw border
		::SetTextColor(m_hdc, clrBorder.get());

		funGdiTextOut.x = x - 1;
		m_fontCollection.ResovleText(this, szText, nLen, funGdiTextOut);

		funGdiTextOut.x = x + 1;
		m_fontCollection.ResovleText(this, szText, nLen, funGdiTextOut);

		funGdiTextOut.x = x;
		funGdiTextOut.y--;
		m_fontCollection.ResovleText(this, szText, nLen, funGdiTextOut);

		funGdiTextOut.x = x;
		funGdiTextOut.y += 2;
		m_fontCollection.ResovleText(this, szText, nLen, funGdiTextOut);

		// draw inner
		::SetTextColor(m_hdc, clrText.get());

		funGdiTextOut.x = x;
		funGdiTextOut.y = y;
		m_fontCollection.ResovleText(this, szText, nLen, funGdiTextOut);
	}
	else
	{
#ifdef _WIN32_WCE
		// draw border
		::SetTextColor(m_hdc, clrBorder.get());
		::ExtTextOut(m_hdc, x + 1, y, 0, NULL, szText, nLen, NULL);
		::ExtTextOut(m_hdc, x - 1, y, 0, NULL, szText, nLen, NULL);
		::ExtTextOut(m_hdc, x, y + 1, 0, NULL, szText, nLen, NULL);
		::ExtTextOut(m_hdc, x, y - 1, 0, NULL, szText, nLen, NULL);

		// draw inner
		::SetTextColor(m_hdc, clrText.get());
		::ExtTextOut(m_hdc, x, y, 0, NULL, szText, nLen, NULL);
#else
		// draw border
		::SetTextColor(m_hdc, clrBorder.get());
		::TextOut(m_hdc, x + 1, y, szText, nLen);
		::TextOut(m_hdc, x - 1, y, szText, nLen);
		::TextOut(m_hdc, x, y + 1, szText, nLen);
		::TextOut(m_hdc, x, y - 1, szText, nLen);

		// draw inner
		::SetTextColor(m_hdc, clrText.get());
		::TextOut(m_hdc, x, y, szText, nLen);
#endif
	}
	return TRUE;
}

bool CGdiGraphics::DrawTextClipOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, UINT uFormat)
{
	if (m_bResolveTextEncoding)
	{
		CGdiDrawTextClip	funGdi;

		funGdi.rc = *lpRect;
		funGdi.hdc = m_hdc;

		if (lpRect->right - lpRect->left > 1)
		{
			// draw border
			::SetTextColor(m_hdc, clrBorder.get());

			funGdi.rc.left = lpRect->left - 1;
			m_fontCollection.ResovleText(this, szText, nLen, funGdi);

			funGdi.rc.left = lpRect->left + 1;
			m_fontCollection.ResovleText(this, szText, nLen, funGdi);

			funGdi.rc.left = lpRect->left;
			funGdi.rc.top--, funGdi.rc.bottom--;
			m_fontCollection.ResovleText(this, szText, nLen, funGdi);

			funGdi.rc.left = lpRect->left;
			funGdi.rc.top += 2, funGdi.rc.bottom += 2;
			m_fontCollection.ResovleText(this, szText, nLen, funGdi);
			funGdi.rc.top--, funGdi.rc.bottom--;
		}

		// draw inner
		funGdi.rc.left = lpRect->left;
		::SetTextColor(m_hdc, clrText.get());
		m_fontCollection.ResovleText(this, szText, nLen, funGdi);
	}
	else
	{
		if (lpRect->right - lpRect->left > 1)
		{
			// draw border
			::SetTextColor(m_hdc, clrBorder.get());

			lpRect->left--;
			::DrawText(m_hdc, szText, nLen, lpRect, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_TOP);

			lpRect->left += 2;
			::DrawText(m_hdc, szText, nLen, lpRect, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_TOP);
			lpRect->left--;

			lpRect->top--, lpRect->bottom--;
			::DrawText(m_hdc, szText, nLen, lpRect, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_TOP);

			lpRect->top += 2, lpRect->bottom += 2;
			::DrawText(m_hdc, szText, nLen, lpRect, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_TOP);
			lpRect->top--, lpRect->bottom--;
		}

		// draw inner
		::SetTextColor(m_hdc, clrText.get());
		::DrawText(m_hdc, szText, nLen, lpRect, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_TOP);
	}
	return true;
}

bool CGdiGraphics::DrawText(LPCTSTR szText, int nLen, LPRECT lpRect, UINT uFormat)
{
#ifdef _WIN32_WCE
	if (uFormat & DT_END_ELLIPSIS)
		DrawTextEx_WINCE(m_hdc, (LPTSTR)szText, nLen, lpRect, uFormat | DT_NOPREFIX);
	else
		::DrawText(m_hdc, (LPTSTR)szText, nLen, lpRect, uFormat | DT_NOPREFIX);
#else
	::DrawTextEx(m_hdc, (LPTSTR)szText, nLen, lpRect, uFormat | DT_NOPREFIX, NULL);
#endif
	return TRUE;
}

bool CGdiGraphics::GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize)
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

void CGdiGraphics::SetFgColor(const MLColor &color)
{
	::SetTextColor(m_hdc, color.get());
}

void CGdiGraphics::SetBgColor(const MLColor &color)
{
	::SetBkColor(m_hdc, color.get());
}

void CGdiGraphics::SetPen(CMLPen &pen)
{
	HPEN		hPenOld;
	hPenOld = (HPEN)::SelectObject(m_hdc, pen.GetHandle());
	if (!m_hPenOld)
		m_hPenOld = hPenOld;
}

bool CGdiGraphics::GetClipBoundBox(CRect &rc)
{
	int		nRet;
	nRet = ::GetClipBox(m_hdc, &rc);

	return nRet != ERROR && nRet != NULLREGION;
}

bool CGdiGraphics::SetClipBoundBox(const CRect &rc)
{
	HRGN		hRgn;

	hRgn = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);

	if (m_nRgnOld == -1)
	{
		m_hRgnOld = CreateRectRgn(0, 0, 1, 1);
		m_nRgnOld = GetClipRgn(m_hdc, m_hRgnOld);	// save old
		if (m_nRgnOld != 1)
		{
			DeleteObject(m_hRgnOld);
			m_hRgnOld = NULL;
		}
	}

	SelectClipRgn(m_hdc, hRgn);	// select new

	DeleteObject(hRgn);			// free

	return TRUE;
}

void CGdiGraphics::RecoverClipBox()
{
	if (m_nRgnOld != -1)
	{
		if (m_nRgnOld == 0)
			SelectClipRgn(m_hdc, NULL);
		else
		{
			SelectClipRgn(m_hdc, m_hRgnOld);
 			DeleteObject(m_hRgnOld);
 			m_hRgnOld = NULL;
		}
		m_nRgnOld = -1;
	}
}

void CGdiGraphics::DrawGraphics(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc)
{
	BitBlt(GetHandle(), xdest, ydest, width, height, pSrc->GetHandle(), xsrc, ysrc, SRCCOPY);
}

void CGdiGraphics::DrawGraphicsStretch(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc)
{
	StretchBlt(GetHandle(), xdest, ydest, width, height, pSrc->GetHandle(), xsrc, ysrc, widthsrc, heightsrc, SRCCOPY);
}

void CGdiGraphics::DrawImage(CMLImage *pImage, int xdest, int ydest, int width, int height, int xsrc, int ysrc)
{
	HGDIOBJ		hOld;

	if (!m_hdcBk)
		m_hdcBk = CreateCompatibleDC(m_hdc);

	hOld = SelectObject(m_hdcBk, pImage->GetHandle());

	BitBlt(m_hdc, xdest, ydest, width, height, m_hdcBk, xsrc, ysrc, SRCCOPY);

	SelectObject(m_hdcBk, hOld);
}

void CGdiGraphics::DrawImageStretch(CMLImage *pImage, int xdest, int ydest, int widthdest, int heightdest, int xsrc, int ysrc, int widthsrc, int heightsrc)
{
	HGDIOBJ		hOld;

	if (!m_hdcBk)
		m_hdcBk = CreateCompatibleDC(m_hdc);

	hOld = SelectObject(m_hdcBk, pImage->GetHandle());

	StretchBlt(m_hdc, xdest, ydest, widthdest, heightdest, m_hdcBk, xsrc, ysrc, widthsrc, heightsrc, SRCCOPY);

	SelectObject(m_hdcBk, hOld);
}

void CGdiGraphics::DrawGraphicsAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha)
{
	BLENDFUNCTION	blend;

	if (bPerPixelAlpha)
		blend.AlphaFormat = AC_SRC_ALPHA;
	else
		blend.AlphaFormat = 0;
	blend.BlendFlags = 0;
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = nAlpha;

	WinAlphaBlend(GetHandle(), xdest, ydest, width, height, pSrc->GetHandle(), xsrc, ysrc, width, height, blend);
}

void CGdiGraphics::DrawGraphicsStretchAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha)
{
	BLENDFUNCTION	blend;

	if (bPerPixelAlpha)
		blend.AlphaFormat = AC_SRC_ALPHA;
	else
		blend.AlphaFormat = 0;
	blend.BlendFlags = 0;
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = nAlpha;

	WinAlphaBlend(GetHandle(), xdest, ydest, width, height, pSrc->GetHandle(), xsrc, ysrc, widthsrc, heightsrc, blend);
}

void CGdiGraphics::DrawImageAlphaBlend(CMLImage *pImage, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha)
{
	HGDIOBJ			hOld;
	BLENDFUNCTION	blend;

	if (bPerPixelAlpha)
		blend.AlphaFormat = AC_SRC_ALPHA;
	else
		blend.AlphaFormat = 0;
	blend.BlendFlags = 0;
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = nAlpha;

	if (!m_hdcBk)
		m_hdcBk = CreateCompatibleDC(m_hdc);

	hOld = SelectObject(m_hdcBk, pImage->GetHandle());

/*	DBGLOG1(_T("%s"), CStrPrintf(_T("AlphaBlend: %X, %d, %d, %d, %d, %X, %d, %d, %d, %d"), m_hdc, xdest, ydest, width, height, m_hdcBk, xsrc, ysrc, width, height).c_str());

	if (xdest < 0)
		xdest = 0;
	if (xsrc < 0)
		xsrc = 0;*/

	if (!WinAlphaBlend(m_hdc, xdest, ydest, width, height, m_hdcBk, xsrc, ysrc, width, height, blend))
	{
//		ERRLOG1(_T("AlphaBlend Error: %s"), CWin32Error().Description());
	}
	else
//		DBGLOG0(_T("----------Successfully"));

	SelectObject(m_hdcBk, hOld);
}

void CGdiGraphics::DrawImageStretchAlphaBlend(CMLImage *pImage, int xdest, int ydest, int widthdest, int heightdest, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha)
{
	HGDIOBJ			hOld;
	BLENDFUNCTION	blend;

	if (bPerPixelAlpha)
		blend.AlphaFormat = AC_SRC_ALPHA;
	else
		blend.AlphaFormat = 0;
	blend.BlendFlags = 0;
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = nAlpha;

	if (!m_hdcBk)
		m_hdcBk = CreateCompatibleDC(m_hdc);

	hOld = SelectObject(m_hdcBk, pImage->GetHandle());

	WinAlphaBlend(m_hdc, xdest, ydest, widthdest, heightdest, m_hdcBk, xsrc, ysrc, widthsrc, heightsrc, blend);

	SelectObject(m_hdcBk, hOld);
}

//
//bool CGdiGraphics::SetFont(HFONT hFont)
//{
//	HFONT		hFontOld;
//
//	hFontOld = (HFONT)SelectObject(m_hdc, hFont);
//	if (!m_hFontOld)
//		m_hFontOld = hFontOld;
//
//	return TRUE;
//}

void CGdiGraphics::SetFont(CMLFont *font)
{
	IMLGraphicsBase::SetFont(font);

	m_fontCollection.SetFont(*font);

	if (!m_bResolveTextEncoding)
	{
		CGdiFont	*pGdiFont = m_fontCollection.GetFont(this, DEFAULT_CHARSET);
		SelectFont(pGdiFont->GetHandle());
	}
}

void CGdiGraphics::EnableResolveTextEncoding(bool bEnable)
{
	if (!m_bResolveTextEncoding)
	{
		CGdiFont	*pGdiFont = m_fontCollection.GetFont(this, DEFAULT_CHARSET);
		SelectFont(pGdiFont->GetHandle());
	}
}
