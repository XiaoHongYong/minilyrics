#ifndef _IMLGRAPHICSBASE_INC_
#define _IMLGRAPHICSBASE_INC_

class CMLFont;

#ifdef GetTextExtentPoint32
#undef GetTextExtentPoint32
#endif

#ifndef _WIN32
// the dwFlags of DrawTextEx
#define DT_TOP              0x00000000
#define DT_LEFT             0x00000000
#define DT_CENTER           0x00000001
#define DT_RIGHT            0x00000002
#define DT_VCENTER          0x00000004
#define DT_BOTTOM           0x00000008
#define DT_SINGLELINE       0x00000020
#define DT_NOCLIP           0x00000100
#define DT_END_ELLIPSIS     0x00008000
#else
#ifdef _WINCE_
#define DT_PATH_ELLIPSIS    0x00004000
#define DT_END_ELLIPSIS     0x00008000
#define DT_RTLREADING       0x00020000
#define DT_WORD_ELLIPSIS    0x00040000
#endif
#endif

#define DT_PREFIX_TEXT      0x02000000
// input string:   "A&bc&&d"
// DT_PREFIX_TEXT: "Abc&d"
//                   -
// normal:         "A&bc&&d"

enum MLGRAPHICS_TYPE
{
	MGT_UNKNOWN,
	MGT_AUTO,
	MGT_WIN32GDI,
	MGT_GDIPLUS,
	MGT_GTK2,
	MGT_MACOSX
};

class IMLGraphicsBase
{
public:
	IMLGraphicsBase()
	{
		m_font = NULL;
		m_GraphicsType = MGT_UNKNOWN;
		m_bResolveTextEncoding = true;
		m_textAlpha = 255;
	}
	virtual ~IMLGraphicsBase()
	{
	}

	virtual void Line(int x1, int y1, int x2, int y2) = 0;

	virtual void Rectangle(int x, int y, int width, int height) = 0;

	virtual void FillRect(int x, int y, int width, int height, CMLBrush *pBrush) = 0;

	byte GetTextAlpha() { return m_textAlpha; }
	void SetTextAlpha(byte alpha) { m_textAlpha = alpha; }

	virtual bool TextOut(int x, int y, LPCTSTR szText, int nLen) = 0;
	virtual bool DrawTextClip(LPCTSTR szText, int nLen, LPRECT lpRect) = 0;

	virtual bool TextOutOutlined(int x, int y, LPCTSTR szText, int nLen, const MLColor &clrText, const MLColor &clrBorder) = 0;
	virtual bool DrawTextClipOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, UINT uFormat = DT_LEFT) = 0;

	virtual bool DrawText(LPCTSTR szText, int nLen, LPRECT lpRect, UINT uFormat = DT_SINGLELINE | DT_LEFT) = 0;

	virtual bool GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize) = 0;

	virtual void SetTextColor(const MLColor &color) = 0;

	virtual void SetBkMode(bool bTransparent) = 0;

	virtual void SetFgColor(const MLColor &color) = 0;
	virtual void SetBgColor(const MLColor &color) = 0;

	virtual void SetPen(CMLPen &pen) = 0;

	virtual bool GetClipBoundBox(CRect &rc) = 0;
	virtual bool SetClipBoundBox(const CRect &rc) = 0;

	virtual void RecoverClipBox() = 0;

	virtual void DrawGraphics(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc) = 0;
	virtual void DrawGraphicsStretch(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc) = 0;

	virtual void DrawImage(CMLImage *pImage, int xdest, int ydest, int width, int height, int xsrc, int ysrc) = 0;
	virtual void DrawImageStretch(CMLImage *pImage, int xdest, int ydest, int widthdest, int heightdest, int xsrc, int ysrc, int widthsrc, int heightsrc) = 0;

	virtual void DrawGraphicsAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha) = 0;
	virtual void DrawGraphicsStretchAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha) = 0;

	virtual void DrawImageAlphaBlend(CMLImage *pImage, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha) = 0;
	virtual void DrawImageStretchAlphaBlend(CMLImage *pImage, int xdest, int ydest, int widthdest, int heightdest, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha) = 0;

	virtual void SetFont(CMLFont *font) { m_font = font; }

	virtual CMLFont *GetFont() { return m_font; }

	virtual void EnableResolveTextEncoding(bool bEnable) { m_bResolveTextEncoding = bEnable; }

	MLGRAPHICS_TYPE GetGraphicsType() { return m_GraphicsType; }

/*
	virtual HDC GetDC()
	{
		return m_hdc;
	}
	HDC				m_hdc;
*/
#ifdef _WIN32
	HDC GetHandle() { return m_hdc; }
	HDC				m_hdc;
#endif

#ifdef _LINUX_GTK2
	virtual GdkDrawable *GetHandle() { return NULL; }
#endif

protected:
	CMLFont			*m_font;
	bool			m_bResolveTextEncoding;
	MLGRAPHICS_TYPE	m_GraphicsType;
	byte			m_textAlpha;

};

#endif // _IMLGRAPHICSBASE_INC_
