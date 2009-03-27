// RawBmpFont.h: interface for the CRawBmpFont class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__RAW_BMP_FONT_EX)
#define __RAW_BMP_FONT_EX

class CRawGraph;
class CRawImage;

#define MARGIN_Y				1
#define OUTLINED_BORDER			4

#define _CLEAR_TYPE

class CRawBmpFont : public CMLFont  
{
public:
	struct Glyph
	{
		Glyph()
		{
			nChar = 0;
			nWidth = nWidthBitmap = 0;
			bitmap = NULL;
			bitmapOutlined = NULL;
		}

		int WidthBytes() const { return nWidthBitmap * G_PIX_SIZE; }
		int WidthBytesOutlined() const { return (nWidthBitmap + OUTLINED_BORDER) * G_PIX_SIZE; }

		int			nChar;
		int16		nWidth;				// Width of char
		int16		nWidthBitmap;		// Width of bitmap
		byte		*bitmap;
		byte		*bitmapOutlined;
	};

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
	virtual bool CreateFrom(const CMLFont &font);

	virtual void Destroy();

public:
	bool TextOut(CRawGraph *pGraph, int x, int y, MLColor &clrText, LPCTSTR szText, int nLen);
	bool DrawText(CRawGraph *pGraph, int x, int y, int width, int xLeftClipOffset, MLColor &clrText, LPCTSTR szText, int nLen);
	bool DrawTextEx(CRawGraph *pGraph, int x, int y, int width, MLColor &clrText, LPCTSTR szText, int nLen, UINT uFormat);

	bool OutlinedTextOut(CRawGraph *pGraph, int x, int y, MLColor &clrText, MLColor &clrBorder, LPCTSTR szText, int nLen);
	bool OutlinedDrawText(CRawGraph *pGraph, int x, int y, int width, int xLeftClipOffset, MLColor &clrText, MLColor &clrBorder, LPCTSTR szText, int nLen);

	bool GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize);

	// void OutlineText
	void SetClipBox(RECT &rc);

	void EnableGdiplusAntialias(bool bGdiplusAntialias);

	void SetThickOutline(bool bThick);
	bool GetThickOutline() const { return m_bThickOutline; }

	bool InitSet(IMLGraphicsBase *pGraph);

	void SetOverlayPattern(CRawImage *imgPattern);
	void SetOverlayPattern(CRawImage *imgPattern1, CRawImage *imgPattern2, int nAlphaPattern1, int nAlphaPattern2);
	void SetOverlayPattern(CRawImage *imgPattern1, int nAlphaPattern1, MLColor &clrPattern2, int nAlphaPattern2);
	void UseColorOverlay();

	void SetShadowMode(ShadowMode shadowMode);

protected:
	typedef map<int, Glyph*>		MAP_GLYPH;

	Glyph *GlyphFromGraph(int nChar);
	Glyph *GetGlyph(int nChar);
	void CreateOutlinedGlyph(Glyph *glyph);

	void DestroyGlyphMap();

	void ConstructPattern();

	int GetHeightBitmap() { return m_nHeight + MARGIN_Y * 2; }

protected:
	bool					m_bGdiplusAntialias;
	IMLGraphicsBase			*m_pmemGraph;
	CRawGraph				m_memGraph;
	int						m_nWidthGraph;
	int						m_nWidthBytesGraph;

	CRawImage				*m_imgPattern1, *m_imgPattern2;
	MLColor					m_clrPattern2;
	int						m_nAlphaPattern1, m_nAlphaPattern2;
	OverlayMode				m_overlayMode;
	ShadowMode				m_shadowMode;

	MAP_GLYPH				m_mapGlyph;
	CRect					m_rcClip;

	bool					m_bThickOutline;

};

#endif // !defined(__RAW_BMP_FONT_EX)
