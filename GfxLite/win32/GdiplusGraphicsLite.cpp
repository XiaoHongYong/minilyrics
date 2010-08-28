// GdiplusGraphicsLite.cpp: implementation of the CGdiplusGraphicsLite class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../GfxLite.h"
#include "GdiplusGraphicsLite.h"


//typedef DWORD *	ULONG_PTR;
#define iterator	Gdiplusiterator
#define list	Gdipluslist
#define map	Gdiplusmap
#include "../../SDK/GDIPlus/Include/GdiPlus.h"
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

	bool Create(CMLGraphics *pGraph, LOGFONTA &logFont)
	{
		Status		ret;

		Destroy();

		ret = CGdiplusGraphicsLite::m_sfunGdipCreateFontFromLogfontA(pGraph->GetHandle(), &logFont, &m_font);
		if (ret == NotTrueTypeFont)
		{
			// 选择默认的TRUEType字体！
			LOGFONTA		logfontNew = logFont;
			strcpy_safe(logfontNew.lfFaceName, CountOf(logfontNew.lfFaceName), "Tahoma");
			ret = CGdiplusGraphicsLite::m_sfunGdipCreateFontFromLogfontA(pGraph->GetHandle(), &logfontNew, &m_font);
		}

		return ret == Ok;
	}

	void Destroy()
	{
		if (m_font)
		{
			CGdiplusGraphicsLite::m_sfunGdipDeleteFont(m_font);
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

		ret = CGdiplusGraphicsLite::m_sfunGdipDrawString(pGraph->griphics, szClip, nClip, font->GetHandle(), &rect, pGraph->stringFormat, pGraph->brush);
		if (ret != Ok)
		{
			DBGLOG1(_T("m_sfunGdipSetTextRenderingHint FAILED: %d!"), ret);
		}
		CGdiplusGraphicsLite::m_sfunGdipMeasureString(pGraph->griphics, szClip, nClip, font->GetHandle(), &rcLayout, pGraph->stringFormat, &rcBoundBox, NULL, NULL);
		rect.X += rcBoundBox.Width;
	}

public:
	RectF	rect;
	CGdiplusGraphicsLite	*pGraph;

};

class CGdiplusMeasureString
{
public:
	void operator()(CGdiplusFont *font, LPCTSTR szClip, int nClip)
	{
		RectF		rcLayout(0.0f, 0.0f, 0.0f, 0.0f);
		RectF		rcBoundBox;

		CGdiplusGraphicsLite::m_sfunGdipMeasureString(pGraph->griphics, szClip, nClip, font->GetHandle(), &rcLayout, pGraph->stringFormat, &rcBoundBox, NULL, NULL);
		cx += rcBoundBox.Width;
		if (rcBoundBox.Height > cy)
			cy = rcBoundBox.Height;
	}

public:
	REAL				cx, cy;
	CGdiplusGraphicsLite	*pGraph;

};

typedef Status (WINGDIPAPI *FUNGdipGetStringFormatTrimming) (GpStringFormat  *format, StringTrimming *trimming);

FUNGdipCreateFromHDC					CGdiplusGraphicsLite::m_sfunGdipCreateFromHDC = NULL;
FUNGdipCreateFontFromLogfontA			CGdiplusGraphicsLite::m_sfunGdipCreateFontFromLogfontA = NULL;
// FUNGdipCreateFontFromLogfontW			CGdiplusGraphicsLite::m_sfunGdipCreateFontFromLogfontW = NULL;
FUNGdipCreateFontFromDC					CGdiplusGraphicsLite::m_sfunGdipCreateFontFromDC = NULL;
FUNGdipCreateSolidFill					CGdiplusGraphicsLite::m_sfunGdipCreateSolidFill = NULL;
FUNGdipStringFormatGetGenericTypographic	CGdiplusGraphicsLite::m_sfunGdipStringFormatGetGenericTypographic = NULL;
FUNGdipDeleteStringFormat				CGdiplusGraphicsLite::m_sfunGdipDeleteStringFormat = NULL;
FUNGdipSetStringFormatFlags				CGdiplusGraphicsLite::m_sfunGdipSetStringFormatFlags = NULL;
FUNGdipGetStringFormatFlags				CGdiplusGraphicsLite::m_sfunGdipGetStringFormatFlags = NULL;
FUNGdipDeleteFont						CGdiplusGraphicsLite::m_sfunGdipDeleteFont = NULL;
FUNGdipDeleteBrush						CGdiplusGraphicsLite::m_sfunGdipDeleteBrush = NULL;
FUNGdipDeleteGraphics					CGdiplusGraphicsLite::m_sfunGdipDeleteGraphics = NULL;
FUNGdipDrawString						CGdiplusGraphicsLite::m_sfunGdipDrawString = NULL;
FUNGdipGetClipBoundsI					CGdiplusGraphicsLite::m_sfunGdipGetClipBoundsI = NULL;
FUNGdipSetClipRectI						CGdiplusGraphicsLite::m_sfunGdipSetClipRectI = NULL;
FUNGdipSetTextRenderingHint				CGdiplusGraphicsLite::m_sfunGdipSetTextRenderingHint = NULL;
FUNGdipMeasureString					CGdiplusGraphicsLite::m_sfunGdipMeasureString = NULL;
FUNGdipSetStringFormatAlign				CGdiplusGraphicsLite::m_sfunGdipSetStringFormatAlign = NULL;
FUNGdipGetStringFormatAlign				CGdiplusGraphicsLite::m_sfunGdipGetStringFormatAlign = NULL;
FUNGdipSetStringFormatLineAlign			CGdiplusGraphicsLite::m_sfunGdipSetStringFormatLineAlign = NULL;
FUNGdipGetStringFormatLineAlign			CGdiplusGraphicsLite::m_sfunGdipGetStringFormatLineAlign = NULL;
FUNGdipSetStringFormatTrimming			CGdiplusGraphicsLite::m_sfunGdipSetStringFormatTrimming = NULL;
FUNGdipGetStringFormatTrimming			m_sfunGdipGetStringFormatTrimming = NULL;
FUNGdiplusStartup						CGdiplusGraphicsLite::m_sfunGdiplusStartup = NULL;
FUNGdiplusShutdown						CGdiplusGraphicsLite::m_sfunGdiplusShutdown = NULL;

// #ifdef _UNICODE
// #define m_sfunGdipCreateFontFromLogfont		m_sfunGdipCreateFontFromLogfontW
// #else
#define m_sfunGdipCreateFontFromLogfont		m_sfunGdipCreateFontFromLogfontA
// #endif

#ifdef _WIN32_DESKTOP
ULONG_PTR           g_gdiplusToken = NULL;
#endif

CGdiplusGraphicsLite::CGdiplusGraphicsLite()
{
	m_hdcBk = NULL;
	griphics = NULL;
	brush = NULL;
	stringFormat = NULL;
	bInitOK = FALSE;
	m_bResolveTextEncoding = true;
}

CGdiplusGraphicsLite::~CGdiplusGraphicsLite()
{
	if (stringFormat)
		m_sfunGdipDeleteStringFormat(stringFormat);
	if (brush)
		m_sfunGdipDeleteBrush(brush);
	if (griphics)
		m_sfunGdipDeleteGraphics(griphics);

	if (m_hdcBk)
		DeleteDC(m_hdcBk);
}

void CGdiplusGraphicsLite::Attach(HDC hdc)
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
			return;
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
			return;
	}

	m_hdc = hdc;

	Status		ret;

	ret = m_sfunGdipCreateFromHDC(hdc, &griphics);
	if (ret != Ok)
	{
		DBGLOG1(_T("m_sfunGdipCreateFromHDC FAILED: %d!"), ret);
		return;
	}
	ret = m_sfunGdipStringFormatGetGenericTypographic(&stringFormat);
	if (ret != Ok)
	{
		DBGLOG1(_T("m_sfunGdipStringFormatGetGenericTypographic FAILED: %d!"), ret);
		return;
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
}

bool CGdiplusGraphicsLite::TextOut(int x, int y, LPCTSTR szText, int nLen)
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

bool CGdiplusGraphicsLite::GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize)
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

void CGdiplusGraphicsLite::SetTextColor(const MLColor &color)
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

void CGdiplusGraphicsLite::SetBkMode(bool bTransparent)
{
	::SetBkMode(m_hdc, bTransparent ? TRANSPARENT : OPAQUE);
}

void CGdiplusGraphicsLite::SetBgColor(const MLColor &color)
{
	::SetBkColor(m_hdc, color.get());
}

void CGdiplusGraphicsLite::SetFont(CFontInfo *font)
{
	m_fontCollection.SetFont(*font);
}

bool CGdiplusGraphicsLite::CanSupportGdiplus()
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

HMODULE CGdiplusGraphicsLite::LoadGdiplusDll()
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

bool CGdiplusGraphicsLite::Startup()
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
	if (m_sfunGdiplusStartup && g_gdiplusToken == NULL)
		return m_sfunGdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL) == Ok;
	return FALSE;
}

void CGdiplusGraphicsLite::Shutdown()
{
	if (m_sfunGdiplusShutdown)
	{
		m_sfunGdiplusShutdown(g_gdiplusToken);
		g_gdiplusToken = NULL;
	}
}
