// GdiplusGraphics.h: interface for the CGdiplusGraphics class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GDIPLUSGRAPHICS_H__C92B58E7_E894_4696_A643_DE12B8A99529__INCLUDED_)
#define AFX_GDIPLUSGRAPHICS_H__C92B58E7_E894_4696_A643_DE12B8A99529__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// GDI Plus·½Ê½µÄ»­Í¼
//
class CGdiplusGraphics : public IMLGraphicsBase
{
public:
	// typedef bool (*FUNTextOut)(int nXStart, int nYStart, LPCTSTR lpString, int cbString);
	CGdiplusGraphics();
	~CGdiplusGraphics();

	virtual void Line(int x1, int y1, int x2, int y2);

	virtual void Rectangle(int x, int y, int width, int height);

	virtual void FillRect(int x, int y, int width, int height, CMLBrush *pBrush);

	virtual bool TextOut(int x, int y, LPCTSTR szText, int nLen);
	virtual bool DrawTextClip(LPCTSTR szText, int nLen, LPRECT lpRect);

	virtual bool TextOutOutlined(int x, int y, LPCTSTR szText, int nLen, const MLColor &clrText, const MLColor &clrBorder);
	virtual bool DrawTextClipOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, UINT uFormat = DT_LEFT);

	virtual bool DrawText(LPCTSTR szText, int nLen, LPRECT lpRect, UINT uFormat = DT_SINGLELINE | DT_LEFT);

	virtual bool GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize);

	virtual void SetTextColor(const MLColor &color);

	virtual void SetBkMode(bool bTransparent);

	virtual void SetPen(CMLPen &pen);

	virtual void SetFgColor(const MLColor &color);
	virtual void SetBgColor(const MLColor &color);

	virtual bool GetClipBoundBox(CRect &rc);
	virtual bool SetClipBoundBox(const CRect &rc);

	virtual void RecoverClipBox();

	virtual void DrawGraphics(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc);
	virtual void DrawGraphicsStretch(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc);

	virtual void DrawImage(CMLImage *pImage, int xdest, int ydest, int width, int height, int xsrc, int ysrc);
	virtual void DrawImageStretch(CMLImage *pImage, int xdest, int ydest, int widthdest, int heightdest, int xsrc, int ysrc, int widthsrc, int heightsrc);

	virtual void DrawGraphicsAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha);
	virtual void DrawGraphicsStretchAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha);

	virtual void DrawImageAlphaBlend(CMLImage *pImage, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha);
	virtual void DrawImageStretchAlphaBlend(CMLImage *pImage, int xdest, int ydest, int widthdest, int heightdest, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha);

	virtual void SetFont(CMLFont *font);

public:
	bool Create(HDC hdc);

public:
	static bool CanSupportGdiplus();
	static bool Startup(OUT ULONG_PTR *token);
	static void Shutdown(ULONG_PTR token);

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

	bool			m_bRecoverClipBox;
	CRect			m_rcClipBoxOld;
	CFontCollection<CGdiplusFont>	m_fontCollection;

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

template<class _T>
class VarFreer
{
public:
	VarFreer(_T *var) : m_pVar(var)
	{
	}
	~VarFreer()
	{
		if (m_pVar)
			delete m_pVar;
	}

	_T		*m_pVar;

};

typedef VarFreer<IMLGraphicsBase>	MLGraphicsFreer;

#endif // !defined(AFX_GDIPLUSGRAPHICS_H__C92B58E7_E894_4696_A643_DE12B8A99529__INCLUDED_)
