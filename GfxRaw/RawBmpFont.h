// RawBmpFont.h: interface for the CRawBmpFont class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_RAW_BMP_FONT_H_)
#define _RAW_BMP_FONT_H_

class CRawGraph;
class CRawImage;

namespace GfxRaw
{

void EnableAntialiasFont(bool bEnable);

#define _CLEAR_TYPE

//
// Glyph pixel format
//
#ifdef _CLEAR_TYPE
enum
{
	G_B			= PIX_B,
	G_G			= PIX_G,
	G_R			= PIX_R,
	G_A			= PIX_A,
	G_PIX_SIZE	= 4,
};
#else //  _CLEAR_TYPE
enum
{
	G_R			= 0,
	G_G			= 0,
	G_B			= 0,
	G_A			= 0,
	G_PIX_SIZE	= 1,
};
#endif //  _CLEAR_TYPE

enum
{
	MARGIN_BITMAP			= 1,
};


struct Glyph
{
	Glyph()
	{
		leftOffset = topOffset = nWidth = widthBitmap = heightBitmap = marginOutlined = 0;
		bitmap = NULL;
		bitmapOutlined = NULL;
		nLastUsedTime = GetTickCount();
		freed = false;
	}
	~Glyph()
	{
		if (bitmapOutlined)
			delete[] bitmapOutlined;
		if (bitmap)
			delete[] bitmap;
	}

	int widthOutlined() { return widthBitmap + marginOutlined; }
	int heightOutlined() { return heightBitmap + marginOutlined; }

	int WidthBytes() const { return widthBitmap * G_PIX_SIZE; }
	int WidthBytesOutlined() const { return (widthBitmap + marginOutlined) * G_PIX_SIZE; }

	StringIterator::CharType	ch;
	byte		nWidth;					// Width of char

	byte		leftOffset, topOffset;	// Left and top offset of bitmap.
	byte		widthBitmap, heightBitmap;			// Width of bitmap
	byte		marginOutlined;
	bool		freed;

	byte		*bitmap;
	byte		*bitmapOutlined;
	uint32		nLastUsedTime;
};

}; // namespace GfxRaw

#ifdef _WIN32
#include "win32/RawGlyphBuilder.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/RawGlyphBuilder.h"
#endif

#ifdef _MAC
#include "mac/RawGlyphBuilder.h"
#endif

namespace GfxRaw
{

class CRawGlyphSet : public CFontInfo
{
	OBJ_REFERENCE_DECL
public:
	CRawGlyphSet();
	virtual ~CRawGlyphSet();

	virtual bool Create(const CFontInfo &font);

	Glyph *GetGlyph(StringIterator::CharType ch);

	void ClearGlyph();

protected:
	int GetGlyphHeight() { return m_nHeight + MARGIN_BITMAP * 2; }

protected:
	friend class CRawGlyphSetMgr;
	typedef map<int, Glyph*>		MAP_GLYPH;

	MAP_GLYPH				m_mapGlyph;

	uint32					m_nLastCleanTime;

	CRawGlyphBuilder		m_rawGlyphBuilder;

};

}; // namespace GfxRaw

class CRawBmpFont
{
public:
	enum OverlayMode
	{
		OM_COLOR,
		OM_PATTERN,
		OM_DUAL_PATTERN,
		OM_PATTERN_COLOR,
	};

	enum ShadowMode
	{
		SM_NONE,
		SM_OUTLINE,
		SM_SHADOW,
	};

public:
	CRawBmpFont();
	virtual ~CRawBmpFont();

	virtual bool Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline = false);
	virtual bool Create(const CFontInfo &font);

	virtual void Destroy();

	int GetHeight() const
	{
		if (m_prawGlyphSet)
			return m_prawGlyphSet->GetHeight();
		else
			return 0;
	}

public:
	bool TextOut(CRawGraph *pGraph, int x, int y, const MLColor &clrText, LPCTSTR szText, int nLen);
	bool DrawText(CRawGraph *pGraph, int x, int y, int width, int xLeftClipOffset, const MLColor &clrText, LPCTSTR szText, int nLen);
	bool DrawTextEx(CRawGraph *pGraph, LPRECT lpRect, const MLColor &clrText, LPCTSTR szText, int nLen, UINT uFormat);

	bool OutlinedTextOut(CRawGraph *pGraph, int x, int y, const MLColor &clrText, const MLColor &clrBorder, LPCTSTR szText, int nLen);
	bool OutlinedDrawText(CRawGraph *pGraph, int x, int y, int width, int xLeftClipOffset, const MLColor &clrText, const MLColor &clrBorder, LPCTSTR szText, int nLen);
	bool OutlinedDrawTextEx(CRawGraph *pGraph, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, LPCTSTR szText, int nLen, UINT uFormat);

	bool GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize)
		{	if (!m_prawGlyphSet) return false;
			pSize->cx = GetTextWidth(szText, nLen); pSize->cy = m_prawGlyphSet->GetHeight(); return true; }

	// void OutlineText
	void SetClipBox(RECT &rc);

	void SetOverlayPattern(CRawImage *imgPattern);
	void SetOverlayPattern(CRawImage *imgPattern1, CRawImage *imgPattern2, int nAlphaPattern1, int nAlphaPattern2);
	void SetOverlayPattern(CRawImage *imgPattern1, int nAlphaPattern1, const MLColor &clrPattern2, int nAlphaPattern2);
	void UseColorOverlay();

	void SetShadowMode(ShadowMode shadowMode);

protected:
	int GetGlyphHeight() { if (!m_prawGlyphSet) return 0; else return m_prawGlyphSet->GetHeight() + GfxRaw::MARGIN_BITMAP * 2; }

	inline void GetDrawTextExPosition(LPCTSTR szText, int nLen, LPRECT lpRect, UINT uFormat, int &x, int &y, int &width, int &xLeftClipOffset)
	{
		if (!m_prawGlyphSet)
			return;

		xLeftClipOffset = 0;

		if (IsFlagSet(uFormat, DT_CENTER))
		{
			// Align at center
			int		nWidthText = GetTextWidth(szText, nLen);

			x = (lpRect->right + lpRect->left - nWidthText) / 2;
		}
		else if (IsFlagSet(uFormat, DT_RIGHT))
		{
			// Align at right
			int		nWidthText = GetTextWidth(szText, nLen);

			x = lpRect->right - nWidthText;
		}
		else
			x = lpRect->left;

		if (IsFlagSet(uFormat, DT_VCENTER))
		{
			// Align at vertical center
			y = (lpRect->top + lpRect->bottom - m_prawGlyphSet->GetHeight()) / 2;
		}
		else if (IsFlagSet(uFormat, DT_BOTTOM))
		{
			// Align at bottom
			y = lpRect->bottom - m_prawGlyphSet->GetHeight();
		}
		else
			y = lpRect->top;

		if (!IsFlagSet(uFormat, DT_NOCLIP))
		{
			if (x < lpRect->left)
				xLeftClipOffset = lpRect->left - x;
			// Y is not CLIPPED yet
			// if (y < lpRect->top)
			//	yTopClipOffset = lpRect->top - y;
			width = lpRect->right - x;
		}
		else
		{
			width = GetTextWidth(szText, nLen) + m_marginOutlined;
		}
	}

	void SplitToMultiLine(LPRECT lpRect, LPCTSTR szText, int nLen, UINT uFormat, V_Str &vLines);

	bool ShouldDrawTextEllipsis(LPCTSTR szText, int nLen, int nWidthMax, tstring &strEllipsis);

	// See DT_PREFIX_TEXT declaration
	bool ShouldDrawTextPrefix(LPCTSTR szText, int nLen, tstring &strPrfix, int &nXPrefix, int &nWidthPrefix);

	int GetTextWidth(LPCTSTR szText, int nLen);

protected:
	GfxRaw::CRawGlyphSet	*m_prawGlyphSet;

	CRawImage				*m_imgPattern1, *m_imgPattern2;
	MLColor					m_clrPattern2;
	int						m_nAlphaPattern1, m_nAlphaPattern2;
	OverlayMode				m_overlayMode;
	ShadowMode				m_shadowMode;
	int						m_marginOutlined;

	CRect					m_rcClip;

};

#endif // !defined(_RAW_BMP_FONT_H_)
