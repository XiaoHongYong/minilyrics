// GdiplusGraphics.cpp: implementation of the CGdiplusGraphics class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "gfx.h"
#include "GdiplusGraphics.h"


//typedef DWORD *	ULONG_PTR;
#define iterator	Gdiplusiterator
#define list	Gdipluslist
#define map	Gdiplusmap
#include "../SDK/GDIPlus/Include/GdiPlus.h"
#undef Gdiplusiterator
#undef Gdipluslist
#undef Gdiplusmap
#include <atlbase.h>
using namespace Gdiplus;
using namespace DllExports;

ARGB MakeARGB(IN BYTE a,
                     IN BYTE r,
                     IN BYTE g,
                     IN BYTE b)
{
    return (((ARGB) (b) <<  BLUE_SHIFT) |
            ((ARGB) (g) << GREEN_SHIFT) |
            ((ARGB) (r) <<   RED_SHIFT) |
            ((ARGB) (a) << ALPHA_SHIFT));
}


class CGdiplusFont
{
public:
	CGdiplusFont()
	{
		m_font = NULL;
	}
	~CGdiplusFont()
	{
		Destroy();
	}

	bool Create(IMLGraphicsBase *pGraph, LOGFONTA &logFont)
	{
		Status		ret;

		Destroy();

		ret = CGdiplusGraphics::m_sfunGdipCreateFontFromLogfontA(pGraph->GetHandle(), &logFont, &m_font);
		if (ret == NotTrueTypeFont)
		{
			// 选择默认的TRUEType字体！
			LOGFONTA		logfontNew = logFont;
			strcpy_safe(logfontNew.lfFaceName, CountOf(logfontNew.lfFaceName), "Tahoma");
			ret = CGdiplusGraphics::m_sfunGdipCreateFontFromLogfontA(pGraph->GetHandle(), &logfontNew, &m_font);
		}

		return ret == Ok;
	}

	void Destroy()
	{
		if (m_font)
		{
			CGdiplusGraphics::m_sfunGdipDeleteFont(m_font);
			m_font = NULL;
		}
	}

	GpFont *GetHandle() { return m_font; }

protected:
	GpFont			*m_font;

};

class CGdiplusTextOut
{
public:
	void operator()(CGdiplusFont *font, LPCTSTR szClip, int nClip)
	{
		Status		ret;
		RectF		rcLayout(rect.X, rect.Y, 0.0f, 0.0f);
		RectF		rcBoundBox;

		ret = CGdiplusGraphics::m_sfunGdipDrawString(pGraph->griphics, szClip, nClip, font->GetHandle(), &rect, pGraph->stringFormat, pGraph->brush);
		if (ret != Ok)
		{
			DBGLOG1(_T("m_sfunGdipSetTextRenderingHint FAILED: %d!"), ret);
		}
		CGdiplusGraphics::m_sfunGdipMeasureString(pGraph->griphics, szClip, nClip, font->GetHandle(), &rcLayout, pGraph->stringFormat, &rcBoundBox, NULL, NULL);
		rect.X += rcBoundBox.Width;
	}

public:
	RectF	rect;
	CGdiplusGraphics	*pGraph;

};
/*
void DoDraw(RectF &rect, CGdiplusGraphics *pGraph, LOGFONTA &logFont, LPCTSTR szClip, int nClip)
{
	Status		ret;
	RectF		rcLayout(rect.X, rect.Y, 0.0f, 0.0f);
	RectF		rcBoundBox;
	GpFont*		font = NULL;

	ret = CGdiplusGraphics::m_sfunGdipCreateFontFromLogfontA(pGraph->GetHandle(), &logFont, &font);
	if (ret == NotTrueTypeFont)
	{
		// 选择默认的TRUEType字体！
		LOGFONTA		logfontNew = logFont;
		strcpy(logfontNew.lfFaceName, "Verdana");
		ret = CGdiplusGraphics::m_sfunGdipCreateFontFromLogfontA(pGraph->GetHandle(), &logfontNew, &font);
	}

	ret = CGdiplusGraphics::m_sfunGdipDrawString(pGraph->griphics, szClip, nClip, font, &rect, pGraph->stringFormat, pGraph->brush);
	if (ret != Ok)
	{
		DBGLOG1(_T("m_sfunGdipSetTextRenderingHint FAILED: %d!"), ret);
	}
	CGdiplusGraphics::m_sfunGdipMeasureString(pGraph->griphics, szClip, nClip, font, &rcLayout, pGraph->stringFormat, &rcBoundBox, NULL, NULL);
	rect.X += rcBoundBox.Width;

	if (font)
		CGdiplusGraphics::m_sfunGdipDeleteFont(font);
}*/

class CGdiplusMeasureString
{
public:
	void operator()(CGdiplusFont *font, LPCTSTR szClip, int nClip)
	{
		RectF		rcLayout(0.0f, 0.0f, 0.0f, 0.0f);
		RectF		rcBoundBox;

		CGdiplusGraphics::m_sfunGdipMeasureString(pGraph->griphics, szClip, nClip, font->GetHandle(), &rcLayout, pGraph->stringFormat, &rcBoundBox, NULL, NULL);
		cx += rcBoundBox.Width;
		if (rcBoundBox.Height > cy)
			cy = rcBoundBox.Height;
	}

public:
	REAL				cx, cy;
	CGdiplusGraphics	*pGraph;

};

typedef Status (WINGDIPAPI *FUNGdipGetStringFormatTrimming) (GpStringFormat  *format, StringTrimming *trimming);

FUNGdipCreateFromHDC					CGdiplusGraphics::m_sfunGdipCreateFromHDC = NULL;
FUNGdipCreateFontFromLogfontA			CGdiplusGraphics::m_sfunGdipCreateFontFromLogfontA = NULL;
// FUNGdipCreateFontFromLogfontW			CGdiplusGraphics::m_sfunGdipCreateFontFromLogfontW = NULL;
FUNGdipCreateFontFromDC					CGdiplusGraphics::m_sfunGdipCreateFontFromDC = NULL;
FUNGdipCreateSolidFill					CGdiplusGraphics::m_sfunGdipCreateSolidFill = NULL;
FUNGdipStringFormatGetGenericTypographic	CGdiplusGraphics::m_sfunGdipStringFormatGetGenericTypographic = NULL;
FUNGdipDeleteStringFormat				CGdiplusGraphics::m_sfunGdipDeleteStringFormat = NULL;
FUNGdipSetStringFormatFlags				CGdiplusGraphics::m_sfunGdipSetStringFormatFlags = NULL;
FUNGdipGetStringFormatFlags				CGdiplusGraphics::m_sfunGdipGetStringFormatFlags = NULL;
FUNGdipDeleteFont						CGdiplusGraphics::m_sfunGdipDeleteFont = NULL;
FUNGdipDeleteBrush						CGdiplusGraphics::m_sfunGdipDeleteBrush = NULL;
FUNGdipDeleteGraphics					CGdiplusGraphics::m_sfunGdipDeleteGraphics = NULL;
FUNGdipDrawString						CGdiplusGraphics::m_sfunGdipDrawString = NULL;
FUNGdipGetClipBoundsI					CGdiplusGraphics::m_sfunGdipGetClipBoundsI = NULL;
FUNGdipSetClipRectI						CGdiplusGraphics::m_sfunGdipSetClipRectI = NULL;
FUNGdipSetTextRenderingHint				CGdiplusGraphics::m_sfunGdipSetTextRenderingHint = NULL;
FUNGdipMeasureString					CGdiplusGraphics::m_sfunGdipMeasureString = NULL;
FUNGdipSetStringFormatAlign				CGdiplusGraphics::m_sfunGdipSetStringFormatAlign = NULL;
FUNGdipGetStringFormatAlign				CGdiplusGraphics::m_sfunGdipGetStringFormatAlign = NULL;
FUNGdipSetStringFormatLineAlign			CGdiplusGraphics::m_sfunGdipSetStringFormatLineAlign = NULL;
FUNGdipGetStringFormatLineAlign			CGdiplusGraphics::m_sfunGdipGetStringFormatLineAlign = NULL;
FUNGdipSetStringFormatTrimming			CGdiplusGraphics::m_sfunGdipSetStringFormatTrimming = NULL;
FUNGdipGetStringFormatTrimming			m_sfunGdipGetStringFormatTrimming = NULL;
FUNGdiplusStartup						CGdiplusGraphics::m_sfunGdiplusStartup = NULL;
FUNGdiplusShutdown						CGdiplusGraphics::m_sfunGdiplusShutdown = NULL;

// #ifdef _UNICODE
// #define m_sfunGdipCreateFontFromLogfont		m_sfunGdipCreateFontFromLogfontW
// #else
#define m_sfunGdipCreateFontFromLogfont		m_sfunGdipCreateFontFromLogfontA
// #endif


CGdiplusGraphics::CGdiplusGraphics() : IMLGraphicsBase()
{
	m_GraphicsType = MGT_GDIPLUS;

	m_hdcBk = NULL;

	m_hPenOld = NULL;

	// font = NULL;
	griphics = NULL;
	brush = NULL;
	stringFormat = NULL;
	bInitOK = FALSE;

	m_bRecoverClipBox = false;
}

bool CGdiplusGraphics::Create(HDC hdc)
{
	if (!m_sfunGdipCreateFromHDC)
	{
		//
		// 载入地址
		HMODULE		hModule;

		hModule = LoadGdiplusDll();
		if (!hModule)
		{
			DBGLOG0(_T("Load Gdiplus Dll FAILED!"));
			return false;
		}

		m_sfunGdipCreateFromHDC					= (FUNGdipCreateFromHDC					)GetProcAddress(hModule, "GdipCreateFromHDC");
		m_sfunGdipCreateFontFromLogfontA		= (FUNGdipCreateFontFromLogfontA			)GetProcAddress(hModule, "GdipCreateFontFromLogfontA");
		// m_sfunGdipCreateFontFromLogfontW		= (FUNGdipCreateFontFromLogfontW			)GetProcAddress(hModule, "GdipCreateFontFromLogfontW");
		m_sfunGdipCreateFontFromDC				= (FUNGdipCreateFontFromDC					)GetProcAddress(hModule, "GdipCreateFontFromDC");
		m_sfunGdipCreateSolidFill				= (FUNGdipCreateSolidFill					)GetProcAddress(hModule, "GdipCreateSolidFill");
//		m_sfunGdipStringFormatGetGenericDefault = (FUNGdipStringFormatGetGenericDefault	)GetProcAddress(hModule, "GdipStringFormatGetGenericDefault");
		m_sfunGdipStringFormatGetGenericTypographic = (FUNGdipStringFormatGetGenericTypographic	)GetProcAddress(hModule, "GdipStringFormatGetGenericTypographic");
		m_sfunGdipDeleteStringFormat			= (FUNGdipDeleteStringFormat				)GetProcAddress(hModule, "GdipDeleteStringFormat");
		m_sfunGdipSetStringFormatFlags			= (FUNGdipSetStringFormatFlags				)GetProcAddress(hModule, "GdipSetStringFormatFlags");
		m_sfunGdipGetStringFormatFlags			= (FUNGdipGetStringFormatFlags				)GetProcAddress(hModule, "GdipGetStringFormatFlags");
		m_sfunGdipDeleteFont					= (FUNGdipDeleteFont						)GetProcAddress(hModule, "GdipDeleteFont");
		m_sfunGdipDeleteBrush					= (FUNGdipDeleteBrush						)GetProcAddress(hModule, "GdipDeleteBrush");
		m_sfunGdipDeleteGraphics				= (FUNGdipDeleteGraphics					)GetProcAddress(hModule, "GdipDeleteGraphics");
		m_sfunGdipDrawString					= (FUNGdipDrawString						)GetProcAddress(hModule, "GdipDrawString");
		m_sfunGdipGetClipBoundsI				= (FUNGdipGetClipBoundsI					)GetProcAddress(hModule, "GdipGetClipBoundsI");
		m_sfunGdipSetClipRectI					= (FUNGdipSetClipRectI						)GetProcAddress(hModule, "GdipSetClipRectI");
		m_sfunGdipSetTextRenderingHint			= (FUNGdipSetTextRenderingHint				)GetProcAddress(hModule, "GdipSetTextRenderingHint");
		m_sfunGdipMeasureString					= (FUNGdipMeasureString						)GetProcAddress(hModule, "GdipMeasureString");
		m_sfunGdipSetStringFormatAlign			= (FUNGdipSetStringFormatAlign				)GetProcAddress(hModule, "GdipSetStringFormatAlign");
		m_sfunGdipGetStringFormatAlign			= (FUNGdipGetStringFormatAlign				)GetProcAddress(hModule, "GdipGetStringFormatAlign");
		m_sfunGdipSetStringFormatLineAlign		= (FUNGdipSetStringFormatLineAlign			)GetProcAddress(hModule, "GdipSetStringFormatLineAlign");
		m_sfunGdipGetStringFormatLineAlign		= (FUNGdipGetStringFormatLineAlign			)GetProcAddress(hModule, "GdipGetStringFormatLineAlign");
		m_sfunGdipSetStringFormatTrimming		= (FUNGdipSetStringFormatTrimming			)GetProcAddress(hModule, "GdipSetStringFormatTrimming");
		m_sfunGdipGetStringFormatTrimming		= (FUNGdipGetStringFormatTrimming			)GetProcAddress(hModule, "GdipGetStringFormatTrimming");
		if (!m_sfunGdipCreateFromHDC || !m_sfunGdipCreateFontFromLogfontA
			// || !m_sfunGdipCreateFontFromLogfontW
			|| !m_sfunGdipCreateFontFromDC || !m_sfunGdipCreateSolidFill
			|| !m_sfunGdipStringFormatGetGenericTypographic || !m_sfunGdipDeleteStringFormat
			|| !m_sfunGdipGetStringFormatFlags || !m_sfunGdipSetStringFormatFlags
			|| !m_sfunGdipDeleteFont || !m_sfunGdipDeleteBrush
			|| !m_sfunGdipDeleteGraphics || !m_sfunGdipSetTextRenderingHint
			|| !m_sfunGdipSetTextRenderingHint || !m_sfunGdipDrawString
			|| !m_sfunGdipGetClipBoundsI || !m_sfunGdipSetClipRectI
			|| !m_sfunGdipSetTextRenderingHint || !m_sfunGdipMeasureString
			|| !m_sfunGdipSetStringFormatAlign || !m_sfunGdipGetStringFormatAlign
			|| !m_sfunGdipSetStringFormatLineAlign || !m_sfunGdipGetStringFormatLineAlign
			|| !m_sfunGdipSetStringFormatTrimming || !m_sfunGdipGetStringFormatTrimming)
			return false;
	}

	m_hdc = hdc;

	Status		ret;

	ret = m_sfunGdipCreateFromHDC(hdc, &griphics);
	if (ret != Ok)
	{
		DBGLOG1(_T("m_sfunGdipCreateFromHDC FAILED: %d!"), ret);
		return false;
	}
	ret = m_sfunGdipStringFormatGetGenericTypographic(&stringFormat);
	if (ret != Ok)
	{
		DBGLOG1(_T("m_sfunGdipStringFormatGetGenericTypographic FAILED: %d!"), ret);
		return false;
	}
	INT	flag;
	ret = m_sfunGdipGetStringFormatFlags(stringFormat, &flag);
	flag &= ~StringFormatFlagsNoClip;
	ret = m_sfunGdipSetStringFormatFlags(stringFormat, flag | StringFormatFlagsMeasureTrailingSpaces | StringFormatFlagsNoWrap);
// 	StringTrimming	strTrimming;
// 	ret = m_sfunGdipGetStringFormatTrimming(stringFormat, &strTrimming);
	ret = m_sfunGdipSetStringFormatTrimming(stringFormat, StringTrimmingNone);
//	ret = GdipCreateFontFromDC(hdc, &font);
//	if (ret != Ok)
//		return;
//	clrText = GetTextColor(hdc);
//	ret = GdipCreateSolidFill(MakeARGB(255, GetRValue(clrText), GetGValue(clrText), GetBValue(clrText)), &brush);
//	if (ret != Ok)
//		return;
	bInitOK = TRUE;

	return true;
}

CGdiplusGraphics::~CGdiplusGraphics()
{
	if (stringFormat)
		m_sfunGdipDeleteStringFormat(stringFormat);
	//if (font)
	//	m_sfunGdipDeleteFont(font);
	if (brush)
		m_sfunGdipDeleteBrush(brush);
	if (griphics)
		m_sfunGdipDeleteGraphics(griphics);

	if (m_hdcBk)
		DeleteDC(m_hdcBk);
}

void CGdiplusGraphics::Line(int x1, int y1, int x2, int y2)
{
	MoveToEx(m_hdc, x1, y1, NULL);
	LineTo(m_hdc, x2, y2);
}

void CGdiplusGraphics::Rectangle(int x, int y, int width, int height)
{
	::Rectangle(m_hdc, x, y, x + width, y + height);
}

void CGdiplusGraphics::FillRect(int x, int y, int width, int height, CMLBrush *pBrush)
{
	CRect	rc(x, y, x + width, y + height);

	::FillRect(m_hdc, &rc, pBrush->GetHandle());
}

bool CGdiplusGraphics::TextOut(int x, int y, LPCTSTR szText, int nLen)
{
	Status		ret;
	RectF		rect((float)x, (float)y, (float)2000, (float)300);

	if (!bInitOK)
		return FALSE;

	ret = m_sfunGdipSetTextRenderingHint(griphics, TextRenderingHintAntiAlias);
	if (ret != Ok)
	{
		DBGLOG1(_T("m_sfunGdipSetTextRenderingHint FAILED: %d!"), ret);
		return FALSE;
	}

	if (m_bResolveTextEncoding)
	{
		CGdiplusTextOut		funGdiplus;

		funGdiplus.pGraph = this;
		funGdiplus.rect = rect;
		m_fontCollection.ResovleText(this, szText, nLen, funGdiplus);
	}
	else
	{
		CGdiplusFont	*font;
		font = m_fontCollection.GetFont(this, DEFAULT_CHARSET);
		ret = m_sfunGdipDrawString(griphics, szText, nLen, font->GetHandle(), &rect, stringFormat, brush);
	}
// 	if (ret != Ok)
// 	{
// 		DBGLOG1(_T("m_sfunGdipDrawString FAILED: %d!"), ret);
// 		return FALSE;
// 	}

	return TRUE;
}

bool CGdiplusGraphics::DrawTextClip(LPCTSTR szText, int nLen, LPRECT lpRect)
{
	Status		ret;

	if (!bInitOK)
		return FALSE;

	RectF	rect(Gdiplus::REAL(lpRect->left), Gdiplus::REAL(lpRect->top), Gdiplus::REAL(lpRect->right - lpRect->left), Gdiplus::REAL(lpRect->bottom - lpRect->top));

	ret = m_sfunGdipSetTextRenderingHint(griphics, TextRenderingHintAntiAlias);

	if (m_bResolveTextEncoding)
	{
		CGdiplusTextOut		funGdiplus;

		funGdiplus.pGraph = this;
		funGdiplus.rect = rect;

		m_fontCollection.ResovleText(this, szText, nLen, funGdiplus);
	}
	else
	{
		CGdiplusFont	*font;
		font = m_fontCollection.GetFont(this, DEFAULT_CHARSET);
		ret = m_sfunGdipDrawString(griphics, szText, nLen, font->GetHandle(), &rect, stringFormat, brush);
	}

	return TRUE;
}

bool CGdiplusGraphics::TextOutOutlined(int x, int y, LPCTSTR szText, int nLen, const MLColor &clrText, const MLColor &clrBorder)
{
	Status		ret;
	RectF		rect((float)x, (float)y, (float)2000, (float)300);

	if (!bInitOK)
		return FALSE;

	ret = m_sfunGdipSetTextRenderingHint(griphics, TextRenderingHintAntiAlias);
	if (ret != Ok)
	{
		DBGLOG1(_T("m_sfunGdipSetTextRenderingHint FAILED: %d!"), ret);
		return FALSE;
	}

	if (m_bResolveTextEncoding)
	{
		CGdiplusTextOut		funGdiplus;

		funGdiplus.pGraph = this;
		funGdiplus.rect = rect;

		// draw border
		SetTextColor(clrBorder);

		funGdiplus.rect.X = rect.X - 1;
		m_fontCollection.ResovleText(this, szText, nLen, funGdiplus);

		funGdiplus.rect.X = rect.X + 1;
		m_fontCollection.ResovleText(this, szText, nLen, funGdiplus);

		funGdiplus.rect.X = rect.X;
		funGdiplus.rect.Y--;
		m_fontCollection.ResovleText(this, szText, nLen, funGdiplus);

		funGdiplus.rect.X = rect.X;
		funGdiplus.rect.Y += 2;
		m_fontCollection.ResovleText(this, szText, nLen, funGdiplus);

		// draw inner
		SetTextColor(clrText);

		funGdiplus.rect.X = rect.X;
		funGdiplus.rect.Y = (Gdiplus::REAL)y;
		m_fontCollection.ResovleText(this, szText, nLen, funGdiplus);
	}
	else
	{
		CGdiplusFont	*font;
		font = m_fontCollection.GetFont(this, DEFAULT_CHARSET);

		// draw border
		SetTextColor(clrBorder);

		rect.X--;
		ret = m_sfunGdipDrawString(griphics, szText, nLen, font->GetHandle(), &rect, stringFormat, brush);

		rect.X += 2;
		ret = m_sfunGdipDrawString(griphics, szText, nLen, font->GetHandle(), &rect, stringFormat, brush);

		rect.X = (Gdiplus::REAL)x;
		rect.Y--;
		ret = m_sfunGdipDrawString(griphics, szText, nLen, font->GetHandle(), &rect, stringFormat, brush);

		rect.Y += 2;
		ret = m_sfunGdipDrawString(griphics, szText, nLen, font->GetHandle(), &rect, stringFormat, brush);

		// draw inner
		SetTextColor(clrText);

		rect.Y = (Gdiplus::REAL)y;
		ret = m_sfunGdipDrawString(griphics, szText, nLen, font->GetHandle(), &rect, stringFormat, brush);
	}

	return TRUE;
}

bool CGdiplusGraphics::DrawTextClipOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, UINT uFormat)
{
	Status		ret;

	if (!bInitOK)
		return FALSE;

	RectF	rect(Gdiplus::REAL(lpRect->left), Gdiplus::REAL(lpRect->top), Gdiplus::REAL(lpRect->right - lpRect->left), Gdiplus::REAL(lpRect->bottom - lpRect->top));

	ret = m_sfunGdipSetTextRenderingHint(griphics, TextRenderingHintAntiAlias);

	if (m_bResolveTextEncoding)
	{
		CGdiplusTextOut		funGdiplus;

		funGdiplus.pGraph = this;
		funGdiplus.rect = rect;

		if (lpRect->right - lpRect->left > 1)
		{
			// draw border
			SetTextColor(clrBorder);

			funGdiplus.rect.X = rect.X - 1;
			m_fontCollection.ResovleText(this, szText, nLen, funGdiplus);

			funGdiplus.rect.X = rect.X + 1;
			m_fontCollection.ResovleText(this, szText, nLen, funGdiplus);

			funGdiplus.rect.X = rect.X;
			funGdiplus.rect.Y--;
			m_fontCollection.ResovleText(this, szText, nLen, funGdiplus);

			funGdiplus.rect.X = rect.X;
			funGdiplus.rect.Y += 2;
			m_fontCollection.ResovleText(this, szText, nLen, funGdiplus);
			funGdiplus.rect.Y --;
		}

		// draw inner
		SetTextColor(clrText);
		funGdiplus.rect.X = rect.X;
		m_fontCollection.ResovleText(this, szText, nLen, funGdiplus);
	}
	else
	{
		CGdiplusFont	*font;
		font = m_fontCollection.GetFont(this, DEFAULT_CHARSET);

		if (lpRect->right - lpRect->left > 1)
		{
			// draw border
			SetTextColor(clrBorder);

			rect.X--;
			ret = m_sfunGdipDrawString(griphics, szText, nLen, font->GetHandle(), &rect, stringFormat, brush);

			rect.X += 2;
			ret = m_sfunGdipDrawString(griphics, szText, nLen, font->GetHandle(), &rect, stringFormat, brush);
			rect.X--;

			rect.Y--;
			ret = m_sfunGdipDrawString(griphics, szText, nLen, font->GetHandle(), &rect, stringFormat, brush);

			rect.Y += 2;
			ret = m_sfunGdipDrawString(griphics, szText, nLen, font->GetHandle(), &rect, stringFormat, brush);
			rect.Y--;
		}

		// draw inner
		SetTextColor(clrText);
		ret = m_sfunGdipDrawString(griphics, szText, nLen, font->GetHandle(), &rect, stringFormat, brush);
	}
	return true;
}

bool CGdiplusGraphics::DrawText(LPCTSTR szText, int nLen, LPRECT lpRect, UINT uFormat)
{
	Status		ret;

	if (!bInitOK)
		return FALSE;

	RectF	rect(Gdiplus::REAL(lpRect->left), Gdiplus::REAL(lpRect->top), Gdiplus::REAL(lpRect->right - lpRect->left), Gdiplus::REAL(lpRect->bottom - lpRect->top));


	ret = m_sfunGdipSetTextRenderingHint(griphics, TextRenderingHintAntiAlias);

	StringAlignment		lineAlign, align;
	StringTrimming		strTrimmingOld, strTrimming;

	ret = m_sfunGdipGetStringFormatAlign(stringFormat, &align);
	ret = m_sfunGdipGetStringFormatLineAlign(stringFormat, &lineAlign);
	ret = m_sfunGdipGetStringFormatTrimming(stringFormat, &strTrimmingOld);

 	if (uFormat & DT_CENTER)
 		ret = m_sfunGdipSetStringFormatAlign(stringFormat, StringAlignmentCenter);
 	if (uFormat & DT_VCENTER)
 		ret = m_sfunGdipSetStringFormatLineAlign(stringFormat, StringAlignmentCenter);
	if (uFormat & DT_END_ELLIPSIS)
		strTrimming = StringTrimmingEllipsisCharacter;
	else if (uFormat & DT_WORD_ELLIPSIS)
		strTrimming = StringTrimmingEllipsisWord;
	else if (uFormat & DT_PATH_ELLIPSIS)
		strTrimming = StringTrimmingEllipsisPath;
	else
		strTrimming = StringTrimmingNone;
	ret = m_sfunGdipSetStringFormatTrimming(stringFormat, strTrimming);

	RectF		rcLayout(rect.X, rect.Y, 0.0f, 0.0f);
	RectF		rcBoundBox;
	CGdiplusFont	*font;
	font = m_fontCollection.GetFont(this, DEFAULT_CHARSET);

#ifdef _UNICODE
	ret = CGdiplusGraphics::m_sfunGdipDrawString(griphics, szText, nLen, font->GetHandle(), &rect, stringFormat, brush);
#else
	DWORD		dwLen;
	WCHAR		szString[1024] = L"";
	dwLen = MultiByteToWideChar(CP_ACP, 0, szText, nLen, szString, CountOf(szString));
	ret = CGdiplusGraphics::m_sfunGdipDrawString(griphics, szString, dwLen, font->GetHandle(), &rect, stringFormat, brush);
#endif
	if (ret != Ok)
	{
		DBGLOG1(_T("m_sfunGdipSetTextRenderingHint FAILED: %d!"), ret);
	}

 	ret = m_sfunGdipSetStringFormatAlign(stringFormat, align);
 	ret = m_sfunGdipSetStringFormatLineAlign(stringFormat, lineAlign);
	ret = m_sfunGdipSetStringFormatTrimming(stringFormat, strTrimmingOld);

	return TRUE;
}

bool CGdiplusGraphics::GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize)
{
//	RectF		rcLayout(0, 0, 2000, 300);
//	RectF		rcBoundBox;
	if (!bInitOK)
	{
		pSize->cx = 0;
		pSize->cy = 0;
		return FALSE;
	}

	if (m_bResolveTextEncoding)
	{
		CGdiplusMeasureString	funGdiplus;

		funGdiplus.pGraph = this;
		funGdiplus.cx = 0;
		funGdiplus.cy = 0;

		m_fontCollection.ResovleText(this, szText, nLen, funGdiplus);

		pSize->cx = (INT)funGdiplus.cx;
		pSize->cy = (INT)funGdiplus.cy;
	}
	else
	{
		CGdiplusFont	*font;
		RectF			rcLayout(0.0f, 0.0f, 0.0f, 0.0f);
		RectF			rcBoundBox;

		font = m_fontCollection.GetFont(this, DEFAULT_CHARSET);
		if (m_sfunGdipMeasureString(griphics, szText, nLen, font->GetHandle(), &rcLayout, stringFormat, &rcBoundBox, NULL, NULL) != Ok)
			return FALSE;

		pSize->cx = (INT)rcBoundBox.Width;
		pSize->cy = (INT)rcBoundBox.Height;
	}

	return TRUE;
}

void CGdiplusGraphics::SetTextColor(const MLColor &color)
{
	Status		ret;

	if (brush)
	{
		m_sfunGdipDeleteBrush(brush);
		brush = NULL;
	}

	ret = m_sfunGdipCreateSolidFill(MakeARGB(255, GetRValue(color.get()), GetGValue(color.get()), GetBValue(color.get())), &brush);
	if (ret != Ok)
	{
		DBGLOG1(_T("m_sfunGdipCreateSolidFill FAILED: %d!"), ret);
		bInitOK = FALSE;
	}
}

void CGdiplusGraphics::SetBkMode(bool bTransparent)
{
	::SetBkMode(m_hdc, bTransparent ? TRANSPARENT : OPAQUE);
}

void CGdiplusGraphics::SetPen(CMLPen &pen)
{
	HPEN		hPenOld;
	hPenOld = (HPEN)::SelectObject(m_hdc, pen.GetHandle());
	if (!m_hPenOld)
		m_hPenOld = hPenOld;
}

void CGdiplusGraphics::SetFgColor(const MLColor &color)
{
	SetTextColor(color);
}

void CGdiplusGraphics::SetBgColor(const MLColor &color)
{
	::SetBkColor(m_hdc, color.get());
}

bool CGdiplusGraphics::GetClipBoundBox(CRect &rc)
{
	if (!bInitOK)
		return FALSE;

	Rect		rectf;
	if (m_sfunGdipGetClipBoundsI(griphics, &rectf) != Ok)
		return FALSE;

	rc.left = rectf.X;
	rc.top = rectf.Y;
	rc.right = rectf.X + rectf.Width;
	rc.bottom = rectf.Y + rectf.Height;

	return TRUE;
}

bool CGdiplusGraphics::SetClipBoundBox(CRect &rc)
{
	if (!bInitOK)
		return FALSE;

	if (m_sfunGdipSetClipRectI(griphics, rc.left, rc.top, rc.Width(), rc.Height(), CombineModeReplace) != Ok)
		return FALSE;

	m_bRecoverClipBox = true;
	m_rcClipBoxOld = rc;

	return TRUE;
}

void CGdiplusGraphics::RecoverClipBox()
{
	if (bInitOK && m_bRecoverClipBox)
	{
		m_bRecoverClipBox = false;
		m_sfunGdipSetClipRectI(griphics, m_rcClipBoxOld.left, m_rcClipBoxOld.top, m_rcClipBoxOld.Width(), m_rcClipBoxOld.Height(), CombineModeReplace);
	}
}

void CGdiplusGraphics::DrawGraphics(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc)
{
	BitBlt(GetHandle(), xdest, ydest, width, height, pSrc->GetHandle(), xsrc, ysrc, SRCCOPY);
}

void CGdiplusGraphics::DrawGraphicsStretch(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc)
{
	StretchBlt(GetHandle(), xdest, ydest, width, height, pSrc->GetHandle(), xsrc, ysrc, widthsrc, heightsrc, SRCCOPY);
}

void CGdiplusGraphics::DrawImage(CMLImage *pImage, int xdest, int ydest, int width, int height, int xsrc, int ysrc)
{
	HGDIOBJ		hOld;

	if (!m_hdcBk)
		m_hdcBk = CreateCompatibleDC(m_hdc);

	hOld = SelectObject(m_hdcBk, pImage->GetHandle());

	BitBlt(m_hdc, xdest, ydest, width, height, m_hdcBk, xsrc, ysrc, SRCCOPY);

	SelectObject(m_hdcBk, hOld);
}

void CGdiplusGraphics::DrawImageStretch(CMLImage *pImage, int xdest, int ydest, int widthdest, int heightdest, int xsrc, int ysrc, int widthsrc, int heightsrc)
{
	HGDIOBJ		hOld;

	if (!m_hdcBk)
		m_hdcBk = CreateCompatibleDC(m_hdc);

	hOld = SelectObject(m_hdcBk, pImage->GetHandle());

	StretchBlt(m_hdc, xdest, ydest, widthdest, heightdest, m_hdcBk, xsrc, ysrc, widthsrc, heightsrc, SRCCOPY);

	SelectObject(m_hdcBk, hOld);
}

void CGdiplusGraphics::DrawGraphicsAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha)
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

void CGdiplusGraphics::DrawGraphicsStretchAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha)
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

void CGdiplusGraphics::DrawImageAlphaBlend(CMLImage *pImage, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha)
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

//	DBGLOG1(_T("%s"), CStrPrintf(_T("AlphaBlend: %X, %d, %d, %d, %d, %X, %d, %d, %d, %d"), m_hdc, xdest, ydest, width, height, m_hdcBk, xsrc, ysrc, width, height).c_str());
/*
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

void CGdiplusGraphics::DrawImageStretchAlphaBlend(CMLImage *pImage, int xdest, int ydest, int widthdest, int heightdest, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha)
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

void CGdiplusGraphics::SetFont(CMLFont *font)
{
	IMLGraphicsBase::SetFont(font);

	m_fontCollection.SetFont(*font);
}

/*

bool CGdiplusGraphics::SetFont(HFONT hFont)
{
	assert(hFont);
	if (!bInitOK)
		return FALSE;

	Status		ret;
	LOGFONTA	logfont;

	if (GetObjectA(hFont, sizeof(logfont), &logfont) == 0)
	{
		DBGLOG1(_T("Get Object FAILED: %d!"), GetLastError());
		return FALSE;
	}

	if (font)
	{
		m_sfunGdipDeleteFont(font);
		font = NULL;
	}

	ret = m_sfunGdipCreateFontFromLogfontA(m_hdc, &logfont, &font);
	if (ret == NotTrueTypeFont)
	{
		DBGLOG0(_T("NotTrueTypeFont, use true type font!"));
		strcpy(logfont.lfFaceName, "Verdana");
		ret = m_sfunGdipCreateFontFromLogfontA(m_hdc, &logfont, &font);
	}
	if (ret != Ok)
	{
		DBGLOG1(_T("m_sfunGdipCreateFontFromLogfontA FAILED: %d!"), ret);
		return FALSE;
	}

	return TRUE;
}
*/

bool CGdiplusGraphics::CanSupportGdiplus()
{
	int		nRet;
	nRet = GetOperationSystemType();
	if (nRet == OPS_WIN95 || nRet == OPS_WINNT4)
		return FALSE;

	HMODULE		hModule;

	hModule = LoadGdiplusDll();
	if (!hModule)
		return FALSE;

	FreeLibrary(hModule);
	return TRUE;
}

HMODULE CGdiplusGraphics::LoadGdiplusDll()
{
	HMODULE		hModule;

	hModule = LoadLibrary(_T("Gdiplus.dll"));
	if (!hModule)
	{
		TCHAR	szDll[MAX_PATH];

		GetWorkingFolder(szDll);
		_tcscat_safe(szDll, CountOf(szDll), _T("Gdiplus.dll"));
		hModule = LoadLibrary(szDll);
	}
	return hModule;
}


bool CGdiplusGraphics::Startup(OUT ULONG_PTR *token)
{
	GdiplusStartupInput gdiplusStartupInput;

	if (!m_sfunGdiplusStartup || !m_sfunGdiplusShutdown)
	{
		HMODULE		hModule;
		hModule = LoadGdiplusDll();
		if (!hModule)
			return FALSE;
		m_sfunGdiplusStartup = (FUNGdiplusStartup)GetProcAddress(hModule, "GdiplusStartup");
		m_sfunGdiplusShutdown = (FUNGdiplusShutdown)GetProcAddress(hModule, "GdiplusShutdown");
	}

	// Initialize GDI+.
	if (m_sfunGdiplusStartup)
		return m_sfunGdiplusStartup(token, &gdiplusStartupInput, NULL) == Ok;
	return FALSE;
}

void CGdiplusGraphics::Shutdown(ULONG_PTR token)
{
	if (m_sfunGdiplusShutdown)
		m_sfunGdiplusShutdown(token);
}
