// GdiplusGraphicsLite.h: interface for the CGdiplusGraphicsLite class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GdiplusGraphicsLite_H__C92B58E7_E894_4696_A643_DE12B8A99529__INCLUDED_)
#define AFX_GdiplusGraphicsLite_H__C92B58E7_E894_4696_A643_DE12B8A99529__INCLUDED_

#pragma once

#include "FontCollection.h"

#ifndef __int3264
typedef DWORD *	ULONG_PTR;
#endif

namespace Gdiplus
{
class GpFont;
class GpGraphics;
class GpSolidFill;
class GpStringFormat;
class GpBrush;
class RectF;
class Rect;
struct GdiplusStartupOutput;
struct GdiplusStartupInput;
typedef Rect GpRect;
typedef RectF GpRectF;
typedef DWORD ARGB;
enum CombineMode;
typedef float REAL;
enum TextRenderingHint;
enum Status;
enum StringAlignment;
enum StringTrimming;
};
using namespace Gdiplus;

#define WINGDIPAPI __stdcall

#define GDIPCONST const

typedef Status (WINGDIPAPI *FUNGdipCreateFromHDC)(HDC hdc, GpGraphics **graphics);
typedef Status (WINGDIPAPI *FUNGdipCreateFontFromLogfontA) ( HDC hdc, GDIPCONST LOGFONTA *logfont, GpFont **font );
typedef Status (WINGDIPAPI *FUNGdipCreateFontFromLogfontW) ( HDC hdc, GDIPCONST LOGFONTW *logfont, GpFont **font );
typedef Status (WINGDIPAPI *FUNGdipCreateFontFromDC) ( HDC hdc, GpFont **font );
typedef Status (WINGDIPAPI *FUNGdipCreateSolidFill) (ARGB color, GpSolidFill **brush);
typedef Status (WINGDIPAPI *FUNGdipStringFormatGetGenericTypographic) (GpStringFormat **format);
typedef Status (WINGDIPAPI *FUNGdipDeleteStringFormat) (GpStringFormat *format);
typedef Status (WINGDIPAPI *FUNGdipSetStringFormatFlags) (GpStringFormat *format, INT flags);
typedef Status (WINGDIPAPI *FUNGdipGetStringFormatFlags) (GDIPCONST GpStringFormat *format, INT *flags);
typedef Status (WINGDIPAPI *FUNGdipDeleteFont) (GpFont* font);
typedef Status (WINGDIPAPI *FUNGdipDeleteBrush) (GpBrush *brush);
typedef Status (WINGDIPAPI *FUNGdipDeleteGraphics) (GpGraphics *graphics);
typedef Status (WINGDIPAPI *FUNGdipDrawString) ( GpGraphics *graphics, GDIPCONST WCHAR *string, INT length, GDIPCONST GpFont *font, GDIPCONST RectF *layoutRect, GDIPCONST GpStringFormat *stringFormat, GDIPCONST GpBrush *brush );
typedef Status (WINGDIPAPI *FUNGdipGetClipBoundsI) (GpGraphics *graphics, GpRect *rect);
typedef Status (WINGDIPAPI *FUNGdipSetClipRectI) (GpGraphics *graphics, INT x, INT y, INT width, INT height, CombineMode combineMode);
typedef Status (WINGDIPAPI *FUNGdipSetTextRenderingHint) (GpGraphics *graphics, TextRenderingHint mode);
typedef Status (WINGDIPAPI *FUNGdipMeasureString) ( GpGraphics *graphics, GDIPCONST WCHAR *string, INT length, GDIPCONST GpFont *font, GDIPCONST RectF *layoutRect, GDIPCONST GpStringFormat *stringFormat, RectF *boundingBox, INT *codepointsFitted, INT *linesFilled );
typedef Status (WINGDIPAPI *FUNGdipSetStringFormatAlign) (GpStringFormat *format, StringAlignment align);
typedef Status (WINGDIPAPI *FUNGdipGetStringFormatAlign) (GDIPCONST GpStringFormat *format, StringAlignment *align);
typedef Status (WINGDIPAPI *FUNGdipSetStringFormatLineAlign) (GpStringFormat *format, StringAlignment align);
typedef Status (WINGDIPAPI *FUNGdipGetStringFormatLineAlign) (GDIPCONST GpStringFormat *format, StringAlignment *align);
typedef Status (WINGDIPAPI *FUNGdipSetStringFormatTrimming) (GpStringFormat  *format, StringTrimming trimming);
typedef Status (WINAPI *FUNGdiplusStartup) ( OUT ULONG_PTR *token, const GdiplusStartupInput *input, OUT GdiplusStartupOutput *output);
typedef VOID (WINAPI *FUNGdiplusShutdown) (ULONG_PTR token);


class CGdiplusFont;

//
// GDI Plus方式的画图
//
class CGdiplusGraphicsLite : public CMLGraphics
{
public:
	// typedef bool (*FUNTextOut)(int nXStart, int nYStart, LPCTSTR lpString, int cbString);
	CGdiplusGraphicsLite();
	~CGdiplusGraphicsLite();

	virtual void Attach(HDC hdc);

	virtual bool TextOut(int x, int y, LPCTSTR szText, int nLen);

	virtual bool GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize);

	virtual void SetTextColor(const MLColor &color);

	virtual void SetBkMode(bool bTransparent);
	virtual void SetBgColor(const MLColor &color);

	virtual void SetFont(CFontInfo *font);

public:
	static bool CanSupportGdiplus();
	static bool Startup();
	static void Shutdown();

protected:
	static HMODULE LoadGdiplusDll();

public:
	// GpFont			*font;
	GpGraphics		*griphics;
	GpSolidFill		*brush;
	GpStringFormat	*stringFormat;
	BOOL			bInitOK;
	HDC				m_hdcBk;
	HPEN			m_hPenOld;

	CFontCollection<CGdiplusFont>	m_fontCollection;
	bool							m_bResolveTextEncoding;

	static FUNGdipCreateFromHDC					m_sfunGdipCreateFromHDC;
	static FUNGdipCreateFontFromLogfontA		m_sfunGdipCreateFontFromLogfontA;
	static FUNGdipCreateFontFromLogfontW		m_sfunGdipCreateFontFromLogfontW;
	static FUNGdipCreateFontFromDC				m_sfunGdipCreateFontFromDC;
	static FUNGdipCreateSolidFill				m_sfunGdipCreateSolidFill;
	static FUNGdipStringFormatGetGenericTypographic	m_sfunGdipStringFormatGetGenericTypographic;
	static FUNGdipDeleteStringFormat			m_sfunGdipDeleteStringFormat;
	static FUNGdipSetStringFormatFlags			m_sfunGdipSetStringFormatFlags;
	static FUNGdipGetStringFormatFlags			m_sfunGdipGetStringFormatFlags;
	static FUNGdipDeleteFont					m_sfunGdipDeleteFont;
	static FUNGdipDeleteBrush					m_sfunGdipDeleteBrush;
	static FUNGdipDeleteGraphics				m_sfunGdipDeleteGraphics;
	static FUNGdipDrawString					m_sfunGdipDrawString;
	static FUNGdipGetClipBoundsI				m_sfunGdipGetClipBoundsI;
	static FUNGdipSetClipRectI					m_sfunGdipSetClipRectI;
	static FUNGdipSetTextRenderingHint			m_sfunGdipSetTextRenderingHint;
	static FUNGdipMeasureString					m_sfunGdipMeasureString;
	static FUNGdipSetStringFormatAlign			m_sfunGdipSetStringFormatAlign;
	static FUNGdipGetStringFormatAlign			m_sfunGdipGetStringFormatAlign;
	static FUNGdipSetStringFormatLineAlign		m_sfunGdipSetStringFormatLineAlign;
	static FUNGdipGetStringFormatLineAlign		m_sfunGdipGetStringFormatLineAlign;
	static FUNGdipSetStringFormatTrimming		m_sfunGdipSetStringFormatTrimming;
	static FUNGdiplusStartup					m_sfunGdiplusStartup;
	static FUNGdiplusShutdown					m_sfunGdiplusShutdown;

};

#endif // !defined(AFX_GdiplusGraphicsLite_H__C92B58E7_E894_4696_A643_DE12B8A99529__INCLUDED_)
