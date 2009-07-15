// RawBmpFont.cpp: implementation of the CRawBmpFont class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GfxRaw.h"
#include "RawGraph.h"
#include "RawBmpFont.h"
#include "agg_pixfmt_gray.h"
#include "agg_blur.h"

#define PATTERN_COLOR_RANGE		100
#define MEM_CLEAN_DURATION		(1000 * 60 * 2)

namespace GfxRaw
{

bool	g_bAntialiasFontEnabled = false;

static void CopyToOutlined(byte *pOut, byte *pIn, int nWidth, int nHeight, int nMarginShadow, int nOffset)
{
	byte		*p, *pOutlined;
	int			x, y;
	int			nWidthOutlined = nWidth + nMarginShadow * 2;

	if (nOffset > nMarginShadow)
		nOffset = nMarginShadow;

	int			nLeftMargin, nRightMargin;

	nLeftMargin = nMarginShadow + nOffset;
	nRightMargin = nMarginShadow - nOffset;

	p = pIn;
	pOutlined = pOut + nRightMargin * nWidthOutlined;
	for (y = 0; y < nHeight; y++)
	{
		for (x = 0; x < nLeftMargin; x++)
			pOutlined[x] = 0;
		for (x = 0; x < nRightMargin; x++)
			pOutlined[nWidthOutlined - 1 - x] = 0;
		memcpy(pOutlined + nLeftMargin, p, nWidth);
		p += nWidth;
		pOutlined += nWidthOutlined;
	}

	for (y = 0; y < nLeftMargin; y++)
	{
		pOutlined = pOut + (nHeight + nMarginShadow * 2 - y - 1) * nWidthOutlined;
		memset(pOutlined, 0, nWidthOutlined);
	}
	for (y = 0; y < nRightMargin; y++)
	{
		pOutlined = pOut + y * nWidthOutlined;
		memset(pOutlined, 0, nWidthOutlined);
	}
}

static void CopyAndOutlineBuffRGBA32(byte *pOut, byte *pIn, int nWidth, int nHeight, int nMargin)
{
	byte		*pIRow, *pORow;
	int			y, c;
	int			nWidthBytesOut = (nWidth + nMargin * 2) * G_PIX_SIZE;
	int			nWidthBytesIn = nWidth * G_PIX_SIZE;

	// CopyToOutlined(pOut, pIn, nWidth, nHeight, OUTLINED_BORDER / 2, 0);

	// outline
	pIRow = pIn;
	pORow = pOut + nMargin * nWidthBytesOut + nMargin * G_PIX_SIZE;
	for (y = 0; y < nHeight; y++)
	{
		byte		*pI = pIRow, *pO = pORow;

		for (int x = 0; x < nWidth; x++)
		{
			for (int i = 0; i < 4; i++, pI++, pO++)
			{
				c = *pI;
				if (c == 0)
					continue;
				if (c > *(pO))						*(pO) = c;
				if (c > *(pO + G_PIX_SIZE))			*(pO + G_PIX_SIZE) = c;
				if (c > *(pO - G_PIX_SIZE))			*(pO - G_PIX_SIZE) = c;
				if (c > *(pO + nWidthBytesOut))		*(pO + nWidthBytesOut) = c;
				if (c > *(pO - nWidthBytesOut))		*(pO - nWidthBytesOut) = c;
			}
		}
		pIRow += nWidthBytesIn;
		pORow += nWidthBytesOut;
	}
}

static void EraseBuffRGBA32(byte *pOut, byte *pIn, int nWidth, int nHeight, int nMargin)
{
	byte		*pIRow, *pORow;
	int			x, y;
	int			nWidthBytesOut = (nWidth + nMargin * 2) * G_PIX_SIZE;
	int			nWidthBytesIn = nWidth * G_PIX_SIZE;

	pIRow = pIn;
	pORow = pOut + nMargin * nWidthBytesOut + nMargin * G_PIX_SIZE;
	for (y = 0; y < nHeight; y++)
	{
		byte		*pI = pIRow, *pO = pORow;

		for (x = 0; x < nWidth; x++)
		{
			if (pI[G_A] == 0xFF)
			{
				pO[G_A] = pO[G_R] = pO[G_G] = pO[G_B] = 0;
			}
			pI += 4;
			pO += 4;
		}
		pIRow += nWidthBytesIn;
		pORow += nWidthBytesOut;
	}
}

static void CopyAndShadowBuffRGBA32(byte *pOut, byte *pIn, int nWidth, int nHeight, int nMarginShadow, int nAlphaShadow, int nRangeShadow)
{
	// 	if (nRangeShadow > nMarginShadow)
	// 		nRangeShadow = nMarginShadow;

	// CopyToOutlined(pOut, pIn, nWidth, nHeight, nMarginShadow, 0);
	CopyAndOutlineBuffRGBA32(pOut, pIn, nWidth, nHeight, nMarginShadow);

	agg::rendering_buffer	buff(pOut, nWidth + nMarginShadow * 2, nHeight + nMarginShadow * 2,  (nWidth + nMarginShadow * 2) * G_PIX_SIZE);
	agg::pixfmt_rgba32		pixf(buff);

	agg::stack_blur_rgba32(pixf, nRangeShadow, nRangeShadow);

	EraseBuffRGBA32(pOut, pIn, nWidth, nHeight, nMarginShadow);

	/*	byte		*p, *pOutlined;
	int			y, c;

	// Shawdow
	p = pIn;
	pOutlined = pOut+ nMargin * nWidthOutlined;
	for (y = 0; y < nHeight; y++)
	{
	for (int x = 0; x < nWidth - 1; x++)
	{
	c = p[x];
	if (c == 0)
	continue;
	if (c > p[x + 1])
	{
	// x --> shadow
	}

	if (c > p[x + nWidth])
	{
	// y --> shadow

	}
	}
	p += nWidth;
	pOutlined += nWidthOutlined;
	}*/
}


void CreateOutlinedGlyph(GfxRaw::Glyph *glyph, int nHeightBitmap, CRawBmpFont::ShadowMode shadowMode)
{
	int			nWidthOutlined;

	ASSERT(glyph->bitmapOutlined == NULL);

	nWidthOutlined = glyph->nWidthBitmap + OUTLINED_BORDER;

	glyph->bitmapOutlined = new byte[(nHeightBitmap + OUTLINED_BORDER) * glyph->WidthBytesOutlined()];
	if (!glyph->bitmapOutlined)
		return;

	memset(glyph->bitmapOutlined, 0, (nHeightBitmap + OUTLINED_BORDER) * glyph->WidthBytesOutlined());

	if (shadowMode == CRawBmpFont::SM_SHADOW)
		CopyAndShadowBuffRGBA32(glyph->bitmapOutlined, glyph->bitmap, glyph->nWidthBitmap, nHeightBitmap, OUTLINED_BORDER / 2, 255, nHeightBitmap / 8);
	else
	{
		// Outline glyph
		CopyAndOutlineBuffRGBA32(glyph->bitmapOutlined, glyph->bitmap, glyph->nWidthBitmap, nHeightBitmap, OUTLINED_BORDER / 2);

		EraseBuffRGBA32(glyph->bitmapOutlined, glyph->bitmap, glyph->nWidthBitmap, nHeightBitmap, OUTLINED_BORDER / 2);
	}
}

class CRawGlyphSetMgr
{
public:
	CRawGlyphSetMgr()
	{
	}
	virtual ~CRawGlyphSetMgr()
	{
		m_listSet.clear();
	}

	CRawGlyphSet *GetGlyphSet(const CFontInfo &font)
	{
		for (LIST_SET::iterator it = m_listSet.begin(); it != m_listSet.end(); ++it)
		{
			CRawGlyphSet *pSet = *it;
			if (pSet->IsSame(font))
			{
				pSet->AddRef();
				return pSet;
			}
		}

		CRawGlyphSet	*pSet = new CRawGlyphSet;

		pSet->Create(font);
		pSet->AddRef();
		m_listSet.push_back(pSet);

		return pSet;
	}

	void RemoveRawGlyphSet(CRawGlyphSet *pRawGlyphSet)
	{
		for (LIST_SET::iterator it = m_listSet.begin(); it != m_listSet.end(); ++it)
		{
			CRawGlyphSet *pSet = *it;
			if (pSet == pRawGlyphSet)
			{
				m_listSet.erase(it);
				return;
			}
		}
	}

	void EnableAntialias(bool bEnable)
	{
#ifdef _WIN32
		if (g_bAntialiasFontEnabled == bEnable)
			return;

		g_bAntialiasFontEnabled = bEnable;

		for (LIST_SET::iterator it = m_listSet.begin(); it != m_listSet.end(); ++it)
		{
			CRawGlyphSet *pSet = *it;
			pSet->ClearGlyph();
			pSet->m_rawGlyphBuilder.Destroy();
			pSet->m_rawGlyphBuilder.Init(*pSet);
		}
#endif
	}

protected:
	typedef list<CRawGlyphSet*>	LIST_SET;

	LIST_SET				m_listSet;

};

GfxRaw::CRawGlyphSetMgr			g_rawGlyphSetMgr;


void EnableAntialiasFont(bool bEnable)
{
	g_rawGlyphSetMgr.EnableAntialias(bEnable);
}


CRawGlyphSet::CRawGlyphSet()
{
	OBJ_REFERENCE_INIT

	m_nLastCleanTime = GetTickCount();
}

OBJ_REFERENCE_IMP(CRawGlyphSet)

CRawGlyphSet::~CRawGlyphSet()
{
	ClearGlyph();

	g_rawGlyphSetMgr.RemoveRawGlyphSet(this);
}

bool CRawGlyphSet::Create(const CFontInfo &font)
{
	CFontInfo::Set(font);

	m_rawGlyphBuilder.Init(font);

	return true;
}

GfxRaw::Glyph *CRawGlyphSet::GetGlyph(StringIterator::CharType ch)
{
	MAP_GLYPH::iterator		it;
	GfxRaw::Glyph		*glyph = NULL;
	uint32		nTimeCur = GetTickCount();

	if (m_mapGlyph.size() > 256 && nTimeCur - m_nLastCleanTime >= MEM_CLEAN_DURATION)
	{
		// Clean unused glyph
		m_nLastCleanTime = nTimeCur;
		for (it = m_mapGlyph.begin(); it != m_mapGlyph.end(); ++it)
		{
			glyph = (*it).second;
			if (nTimeCur - glyph->nLastUsedTime >= MEM_CLEAN_DURATION)
			{
				if (glyph->bitmap)
				{
					delete[] glyph->bitmap;
					glyph->bitmap = NULL;
					if (glyph->bitmapOutlined)
					{
						delete[] glyph->bitmapOutlined;
						glyph->bitmapOutlined = NULL;
					}
				}
			}
		}
	}

	it = m_mapGlyph.find(ch.GetIntValue());
	if (it == m_mapGlyph.end())
	{
		glyph = m_rawGlyphBuilder.BuildGlyph(ch);
		ASSERT(glyph);
		if (!glyph)
			return NULL;
		m_mapGlyph[ch.GetIntValue()] = glyph;
	}
	else
	{
		glyph = (*it).second;
		if (glyph->bitmap == NULL)
		{
			// It must has been freed, renew one.
			Glyph		*temp = m_rawGlyphBuilder.BuildGlyph(ch);
			ASSERT(temp);
			if (!temp)
				return NULL;
			glyph->bitmap = temp->bitmap;
			temp->bitmap = NULL;
			delete temp;
		}
	}

	glyph->nLastUsedTime = nTimeCur;

	return glyph;
}

void CRawGlyphSet::ClearGlyph()
{
	for (MAP_GLYPH::iterator it = m_mapGlyph.begin(); it != m_mapGlyph.end(); ++it)
	{
		Glyph		*p = (*it).second;
		delete p;
	}
	m_mapGlyph.clear();
}

}; // namespace GfxRaw

using namespace GfxRaw;

void TileBlt(CRawImage *pImage, CRawImage *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest)
{
	if (pImage->m_cy == 0 || pImage->m_cx == 0)
		return;

	int			x, y;

	//
	// 先将Y方向的绘画了
	// 1111 1111 1111 222
	// 1111 1111 1111 222
	// 3333 3333 3333 444
	for (y = yDest; y + pImage->m_cy < yDest + nHeightDest; y += pImage->m_cy)
	{
		// 1111
		for (x = xDest; x + pImage->m_cx <= xDest + nWidthDest; x += pImage->m_cx)
		{
			pImage->Blt(pGraphics, 
				x, y,
				pImage->m_cx, pImage->m_cy,
				pImage->m_x, pImage->m_y);
		}

		// 2222
		if (x < xDest + nWidthDest)
		{
			pImage->Blt(pGraphics, 
				x, y,
				xDest + nWidthDest - x, pImage->m_cy,
				pImage->m_x, pImage->m_y);
		}
	}

	if (y < yDest + nHeightDest)
	{
		int		nLeftCy;

		nLeftCy = yDest + nHeightDest - y;

		// 3333
		for (x = xDest; x + pImage->m_cx <= xDest + nWidthDest; x += pImage->m_cx)
		{
			pImage->Blt(pGraphics, 
				x, y,
				pImage->m_cx, nLeftCy,
				pImage->m_x, pImage->m_y);
		}

		// 444
		if (x < xDest + nWidthDest)
		{
			pImage->Blt(pGraphics, 
				x, y,
				xDest + nWidthDest - x, nLeftCy,
				pImage->m_x, pImage->m_y);
		}
	}
}
// 
// void blur_text(BYTE threshold, BYTE decay, BYTE max_depth, agg::rendering_buffer &iSrc, agg::rendering_buffer &iDst, BYTE bytes)
// {
// 
// 	long x,y,z,m;
// 	BYTE *pSrc, *pSrc2, *pSrc3, *pDst;
// 	BYTE step,n;
// 	int pivot;
// 
// 	if (max_depth<1) max_depth = 1;
// 
// 	long nmin,nmax,xmin,xmax,ymin,ymax;
// 	xmin = ymin = 0;
// 	xmax = iSrc.width();
// 	ymax = iSrc.height();
// 
// 	if (xmin==xmax || ymin==ymax) return;
// 
// 	nmin = xmin * bytes;
// 	nmax = xmax * bytes;
// 
// 	// double dbScaler = 100.0f/(ymax-ymin)/bytes;
// 
// 	for (n=0; n<bytes; n++){
// 		for (y=ymin+1;y<(ymax-1);y++)
// 		{
// 			pSrc  = iSrc.row_ptr(y);
// 			pSrc2 = iSrc.row_ptr(y+1);
// 			pSrc3 = iSrc.row_ptr(y-1);
// 			pDst  = iDst.row_ptr(y);
// 
// 			//scan left to right
// 			for (x=n+nmin /*,i=xmin*/; x<(nmax-1); x+=bytes /*,i++*/)
// 			{
// 				z=x+bytes;
// 				pivot = pSrc[z]-threshold;
// 				//find upper corner
// 				if (pSrc[x]<pivot && pSrc2[z]<pivot && pSrc3[x]>=pivot){
// 					while (z<nmax && pSrc2[z]<pSrc[x+bytes] && pSrc[x+bytes]<=pSrc[z]){
// 						z+=bytes;
// 					}
// 					m = z-x;
// 					m = (decay>1) ? ((m/bytes)/decay+1) : m/bytes;
// 					if (m>max_depth) m = max_depth;
// 					step = (BYTE)((pSrc[x+bytes]-pSrc[x])/(m+1));
// 					while (m-->1){
// 						pDst[x+m*bytes] = (BYTE)(pDst[x]+(step*(m+1)));
// 					}
// 				}
// 				//find lower corner
// 				z=x+bytes;
// 				if (pSrc[x]<pivot && pSrc3[z]<pivot && pSrc2[x]>=pivot){
// 					while (z<nmax && pSrc3[z]<pSrc[x+bytes] && pSrc[x+bytes]<=pSrc[z]){
// 						z+=bytes;
// 					}
// 					m = z-x;
// 					m = (decay>1) ? ((m/bytes)/decay+1) : m/bytes;
// 					if (m>max_depth) m = max_depth;
// 					step = (BYTE)((pSrc[x+bytes]-pSrc[x])/(m+1));
// 					while (m-->1){
// 						pDst[x+m*bytes] = (BYTE)(pDst[x]+(step*(m+1)));
// 					}
// 				}
// 			}
// 			//scan right to left
// 			for (x=nmax-1-n /*,i=(xmax-1)*/; x>0; x-=bytes /*,i--*/)
// 			{
// 				z=x-bytes;
// 				pivot = pSrc[z]-threshold;
// 				//find upper corner
// 				if (pSrc[x]<pivot && pSrc2[z]<pivot && pSrc3[x]>=pivot){
// 					while (z>n && pSrc2[z]<pSrc[x-bytes] && pSrc[x-bytes]<=pSrc[z]){
// 						z-=bytes;
// 					}
// 					m = x-z;
// 					m = (decay>1) ? ((m/bytes)/decay+1) : m/bytes;
// 					if (m>max_depth) m = max_depth;
// 					step = (BYTE)((pSrc[x-bytes]-pSrc[x])/(m+1));
// 					while (m-->1){
// 						pDst[x-m*bytes] = (BYTE)(pDst[x]+(step*(m+1)));
// 					}
// 				}
// 				//find lower corner
// 				z=x-bytes;
// 				if (pSrc[x]<pivot && pSrc3[z]<pivot && pSrc2[x]>=pivot){
// 					while (z>n && pSrc3[z]<pSrc[x-bytes] && pSrc[x-bytes]<=pSrc[z]){
// 						z-=bytes;
// 					}
// 					m = x-z;
// 					m = (decay>1) ? ((m/bytes)/decay+1) : m/bytes;
// 					if (m>max_depth) m = max_depth;
// 					step = (BYTE)((pSrc[x-bytes]-pSrc[x])/(m+1));
// 					while (m-->1){
// 						pDst[x-m*bytes] = (BYTE)(pDst[x]+(step*(m+1)));
// 					}
// 				}
// 			}
// 		}
// 	}
// }

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRawBmpFont::CRawBmpFont()
{
	m_prawGlyphSet = NULL;

	m_rcClip.SetRect(0, 0, 0, 0);

	m_overlayMode = OM_COLOR;
	m_imgPattern1 = NULL;
	m_imgPattern2 = NULL;
	m_nAlphaPattern1 = 255;
	m_nAlphaPattern2 = 255;

	m_shadowMode = SM_SHADOW;
}

CRawBmpFont::~CRawBmpFont()
{
	Destroy();
}

bool CRawBmpFont::Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline)
{
	CFontInfo	font;

	font.Set(szFaceNameLatin9, szFaceNameOthers, nHeight, nWeight, nItalic, bUnderline);

	return Create(font);
}

bool CRawBmpFont::Create(const CFontInfo &font)
{
	if (m_prawGlyphSet)
	{
		if (m_prawGlyphSet->IsSame(font))
			return true;

		m_prawGlyphSet->Release();
	}

	m_prawGlyphSet = GfxRaw::g_rawGlyphSetMgr.GetGlyphSet(font);

	return m_prawGlyphSet != NULL;
}

void CRawBmpFont::Destroy()
{
	if (m_prawGlyphSet)
	{
		m_prawGlyphSet->Release();
		m_prawGlyphSet = NULL;
	}
}

bool CRawBmpFont::TextOut(CRawGraph *pGraph, int x, int y, const MLColor &clrText, LPCTSTR szText, int nLen)
{
	return DrawText(pGraph, x, y, pGraph->Width() - x, 0, clrText, szText, nLen);
}

// graph is 32 bpp, glyph is 32 bpp
static inline void DrawGlyphRGBA32(agg::rendering_buffer &graph, int xDst, int yDst, agg::rendering_buffer &glyph, int xSrc, int ySrc, int xSrcEnd, int ySrcEnd, 
					  int nAlphaGlyph, const MLColor &clrGlyph)
{
	byte		*pDst, *pRowDst, *pSrc, *pRowSrc;
	int			x, y;
	byte		clr[3];

	clr[0] = clrGlyph.b();
	clr[1] = clrGlyph.g();
	clr[2] = clrGlyph.r();

	pRowDst = graph.row_ptr(yDst);
	pRowDst += xDst * 4;

	pRowSrc = glyph.row_ptr(ySrc);
	pRowSrc += xSrc * G_PIX_SIZE;

	// Fill with color
	for (y = ySrc; y < ySrcEnd; y++)
	{
		pDst = pRowDst;
		pSrc = pRowSrc;
		for (x = xSrc; x < xSrcEnd; x++)
		{
			if (pSrc[3] != 0)
			{
				int		a;
				for (int i = 0; i < 3; i++)
				{
					a = pSrc[i] * nAlphaGlyph / 255;
					pDst[i] = (uint8)(((clr[i] - pDst[i]) * a + (pDst[i] << 8)) >> 8);
				}
				a = pSrc[G_A] * nAlphaGlyph / 255;
				pDst[G_A] = (uint8)((a + pDst[G_A]) - ((a * pDst[G_A] + 0xFF) >> 8));
			}
			pDst += 4;
			pSrc += G_PIX_SIZE;
		}
		pRowDst += graph.stride();
		pRowSrc += glyph.stride();
	}
}

// graph is 32 bpp, glyph is 32 bpp
// pattern is 24 bpp.
inline void DrawGlyphRGBA32(agg::rendering_buffer &graph, int xDst, int yDst, agg::rendering_buffer &glyph, int xSrc, int ySrc, int xSrcEnd, int ySrcEnd,
					  int nAlphaGlyph, CRawImage *pattern, int &xPtStart)
{
	byte		*pDst, *pRowDst, *pSrc, *pRowSrc;
	byte		*pPt, *pRowPt;
	PRAW_IMAGE_DATA	imgPattern;
	int			x, y;
	int			yPtStart = 0;
	int			xPtStartNew = xPtStart;

	pRowDst = graph.row_ptr(yDst);
	pRowDst += xDst * 4;

	pRowSrc = glyph.row_ptr(ySrc);
	pRowSrc += xSrc * G_PIX_SIZE;

	imgPattern = pattern->GetHandle();
	ASSERT(imgPattern->bitCount == 24);

	yPtStart += ySrc;
	if (yPtStart >= imgPattern->height)
		yPtStart = 0;
	// xPtStart += xSrc;
	if (xPtStart >= imgPattern->width)
		xPtStart = 0;

	pRowPt = imgPattern->RowPtr(yPtStart);
	pRowPt += xPtStart * 3;

	// Fill with color
	for (y = ySrc; y < ySrcEnd; y++)
	{
		pDst = pRowDst;
		pSrc = pRowSrc;
		pPt = pRowPt;
		xPtStartNew = xPtStart;
		for (x = xSrc; x < xSrcEnd; x++)
		{
			if (pSrc[3] != 0)
			{
				int		a;
				for (int i = 0; i < 3; i++)
				{
					a = pSrc[i] * nAlphaGlyph / 255;
					pDst[i] = (uint8)(((pPt[i] - pDst[i]) * a + (pDst[i] << 8)) >> 8);
				}
				a = pSrc[G_A] * nAlphaGlyph / 255;
				pDst[G_A] = (uint8)((a + pDst[G_A]) - ((a * pDst[G_A] + 0xFF) >> 8));
// 				int		a = pSrc[0] * nAlphaGlyph / 255;
// 				pDst[0] = (uint8)(((pPt[0] - pDst[0]) * a + (pDst[0] << 8)) >> 8);
// 				pDst[1] = (uint8)(((pPt[1] - pDst[1]) * a + (pDst[1] << 8)) >> 8);
// 				pDst[2] = (uint8)(((pPt[2] - pDst[2]) * a + (pDst[2] << 8)) >> 8);
// 				pDst[3] = (uint8)((a + pDst[3]) - ((a * pDst[3] + 0xFF) >> 8));
			}
			pDst += 4;
			pSrc += G_PIX_SIZE;

			xPtStartNew++;
			if (xPtStartNew >= imgPattern->width)
			{
				xPtStartNew = 0;
				pPt = imgPattern->RowPtr(yPtStart);
			}
			else
				pPt += 3;
		}
		pRowDst += graph.stride();
		pRowSrc += glyph.stride();
		yPtStart++;
		if (yPtStart >= imgPattern->height)
		{
			yPtStart = 0;
			pRowPt = imgPattern->RowPtr(0);
			pRowPt += xPtStart * 3;
		}
		else
			pRowPt += imgPattern->stride;
	}

	xPtStart = xPtStartNew;
}

// graph is 32 bpp, glyph is 32 bpp
// pattern is 24 bpp.
inline void DrawGlyphRGBA32(agg::rendering_buffer &graph, int xDst, int yDst, agg::rendering_buffer &glyph, int xSrc, int ySrc, int xSrcEnd, int ySrcEnd,
					  int nAlphaGlyph, CRawImage *pattern, int &xPtStart, int nAlphaPattern, const MLColor &clrPt, int nAlphaClrPt)
{
	byte		*pDst, *pRowDst, *pSrc, *pRowSrc;
	byte		*pPt, *pRowPt;
	PRAW_IMAGE_DATA	imgPattern;
	int			x, y;
	int			yPtStart = 0;
	int			xPtStartNew = xPtStart;
	byte		pclrPt[3];

	pclrPt[0] = clrPt.b();
	pclrPt[1] = clrPt.g();
	pclrPt[2] = clrPt.r();

	pRowDst = graph.row_ptr(yDst);
	pRowDst += xDst * 4;

	pRowSrc = glyph.row_ptr(ySrc);
	pRowSrc += xSrc * G_PIX_SIZE;

	imgPattern = pattern->GetHandle();
	ASSERT(imgPattern->bitCount == 24);

	yPtStart += ySrc;
	if (yPtStart >= imgPattern->height)
		yPtStart = 0;
	// xPtStart += xSrc;
	if (xPtStart >= imgPattern->width)
		xPtStart = 0;

	pRowPt = imgPattern->RowPtr(yPtStart);
	pRowPt += xPtStart * 3;

	// Fill with color
	for (y = ySrc; y < ySrcEnd; y++)
	{
		pDst = pRowDst;
		pSrc = pRowSrc;
		pPt = pRowPt;
		xPtStartNew = xPtStart;
		for (x = xSrc; x < xSrcEnd; x++)
		{
			if (pSrc[3] != 0)
			{
				int		a, c;
				for (int i = 0; i < 3; i++)
				{
					a = pSrc[i] * nAlphaGlyph / 255;
					c = (pPt[i] * nAlphaPattern + pclrPt[i] * (255 - nAlphaPattern)) / 255;
					pDst[i] = (uint8)(((c - pDst[i]) * a + (pDst[i] << 8)) >> 8);
				}
				a = pSrc[3] * nAlphaGlyph / 255;
				pDst[3] = (uint8)((a + pDst[3]) - ((a * pDst[3] + 0xFF) >> 8));
// 
// 				int	r, g, b;
// 				b = (pPt[0] * nAlphaPattern + bClrPt) / 255;
// 				g = (pPt[1] * nAlphaPattern + gClrPt) / 255;
// 				r = (pPt[2] * nAlphaPattern + rClrPt) / 255;
// 				pDst[0] = (uint8)(((b - pDst[0]) * a + (pDst[0] << 8)) >> 8);
// 				pDst[1] = (uint8)(((g - pDst[1]) * a + (pDst[1] << 8)) >> 8);
// 				pDst[2] = (uint8)(((r - pDst[2]) * a + (pDst[2] << 8)) >> 8);
// 				pDst[3] = (uint8)((a + pDst[3]) - ((a * pDst[3] + 0xFF) >> 8));
			}
			pDst += 4;
			pSrc += G_PIX_SIZE;

			xPtStartNew++;
			if (xPtStartNew >= imgPattern->width)
			{
				xPtStartNew = 0;
				pPt = imgPattern->RowPtr(yPtStart);
			}
			else
				pPt += 3;
		}
		pRowDst += graph.stride();
		pRowSrc += glyph.stride();
		yPtStart++;
		if (yPtStart >= imgPattern->height)
		{
			yPtStart = 0;
			pRowPt = imgPattern->RowPtr(0);
		}
		else
			pRowPt += imgPattern->stride;
	}

	xPtStart = xPtStartNew;
}

// graph is 32 bpp, glyph is 32 bpp
// pattern is 24 bpp.
inline void DrawGlyphRGBA32(agg::rendering_buffer &graph, int xDst, int yDst, agg::rendering_buffer &glyph, int xSrc, int ySrc, int xSrcEnd, int ySrcEnd,
					  int nAlphaGlyph, CRawImage *pattern1, int &xPt1Start, int nAlphaPt1, CRawImage *pattern2, int &xPt2Start, int nAlphaPt2)
{
	byte		*pDst, *pRowDst, *pSrc, *pRowSrc;
	byte		*pPt1, *pRowPt1;
	byte		*pPt2, *pRowPt2;
	PRAW_IMAGE_DATA	imgPattern1, imgPattern2;
	int			x, y;
	int			yPt1Start = 0, yPt2Start = 0;
	int			xPt1StartNew = xPt1Start;
	int			xPt2StartNew = xPt2Start;

	pRowDst = graph.row_ptr(yDst);
	pRowDst += xDst * 4;

	pRowSrc = glyph.row_ptr(ySrc);
	pRowSrc += xSrc * G_PIX_SIZE;

	// pattern 1
	imgPattern1 = pattern1->GetHandle();
	ASSERT(imgPattern1->bitCount == 24);
	yPt1Start += ySrc;
	if (yPt1Start >= imgPattern1->height)
		yPt1Start = 0;
	// xPt1Start += xSrc;
	if (xPt1Start >= imgPattern1->width)
		xPt1Start = 0;

	pRowPt1 = imgPattern1->RowPtr(yPt1Start);
	pRowPt1 += xPt1Start * 3;

	// pattern 2
	imgPattern2 = pattern2->GetHandle();
	ASSERT(imgPattern2->bitCount == 24);
	yPt2Start += ySrc;
	if (yPt2Start >= imgPattern2->height)
		yPt2Start = 0;
	// xPt2Start += xSrc;
	if (xPt2Start >= imgPattern2->width)
		xPt2Start = 0;

	pRowPt2 = imgPattern2->RowPtr(yPt2Start);
	pRowPt2 += xPt2Start * 3;

	// Fill with color
	for (y = ySrc; y < ySrcEnd; y++)
	{
		pDst = pRowDst;
		pSrc = pRowSrc;
		pPt1 = pRowPt1;
		pPt2 = pRowPt2;
		xPt1StartNew = xPt1Start;
		xPt2StartNew = xPt2Start;
		for (x = xSrc; x < xSrcEnd; x++)
		{
			if (pSrc[3] != 0)
			{
				int		a, c;
				for (int i = 0; i < 3; i++)
				{
					a = pSrc[i] * nAlphaGlyph / 255;
					c = (pPt1[i] * nAlphaPt1 + pPt2[i] * nAlphaPt2) / 255;
					pDst[i] = (uint8)(((c - pDst[i]) * a + (pDst[i] << 8)) >> 8);
				}
				a = pSrc[G_A] * nAlphaGlyph / 255;
				pDst[G_A] = (uint8)((a + pDst[G_A]) - ((a * pDst[G_A] + 0xFF) >> 8));
// 				int		a = pSrc[0] * nAlphaGlyph / 255;
// 				int		r, g, b;
// 				b = (pPt1[0] * nAlphaPt1 + pPt2[0] * nAlphaPt2) / 255;
// 				g = (pPt1[1] * nAlphaPt1 + pPt2[1] * nAlphaPt2) / 255;
// 				r = (pPt1[2] * nAlphaPt1 + pPt2[2] * nAlphaPt2) / 255;
// 				pDst[0] = (uint8)(((b - pDst[0]) * a + (pDst[0] << 8)) >> 8);
// 				pDst[1] = (uint8)(((g - pDst[1]) * a + (pDst[1] << 8)) >> 8);
// 				pDst[2] = (uint8)(((r - pDst[2]) * a + (pDst[2] << 8)) >> 8);
// 				pDst[3] = (uint8)((a + pDst[3]) - ((a * pDst[3] + 0xFF) >> 8));
			}
			pDst += 4;
			pSrc += G_PIX_SIZE;

			xPt1StartNew++;
			if (xPt1StartNew >= imgPattern1->width)
			{
				xPt1StartNew = 0;
				pPt1 = imgPattern1->RowPtr(yPt1Start);
			}
			else
				pPt1 += 3;

			xPt2StartNew++;
			if (xPt2StartNew >= imgPattern2->width)
			{
				xPt2StartNew = 0;
				pPt2 = imgPattern2->RowPtr(yPt2Start);
			}
			else
				pPt2 += 3;
		}
		pRowDst += graph.stride();
		pRowSrc += glyph.stride();

		yPt1Start++;
		if (yPt1Start >= imgPattern1->height)
		{
			yPt1Start = 0;
			pRowPt1 = imgPattern1->RowPtr(0);
		}
		else
			pRowPt1 += imgPattern1->stride;

		yPt2Start++;
		if (yPt2Start >= imgPattern2->height)
		{
			yPt2Start = 0;
			pRowPt2 = imgPattern2->RowPtr(0);
		}
		else
			pRowPt2 += imgPattern2->stride;
	}

	xPt1Start = xPt1StartNew;
	xPt2Start = xPt2StartNew;
}

bool CRawBmpFont::DrawTextEx(CRawGraph *pGraph, LPRECT lpRect, const MLColor &clrText, LPCTSTR szText, int nLen, UINT uFormat)
{
	ASSERT(m_prawGlyphSet);
	if (!m_prawGlyphSet)
		return false;

	if (IsFlagSet(uFormat, DT_END_ELLIPSIS))
	{
		tstring		str;
		uFormat &= ~DT_END_ELLIPSIS;

		if (ShouldDrawTextEllipsis(szText, nLen, lpRect->right - lpRect->left, str))
			return DrawTextEx(pGraph, lpRect, clrText, str.c_str(), str.size(), uFormat);
	}
	else if (IsFlagSet(uFormat, DT_PREFIX_TEXT))
	{
		tstring		str;
		int			nXPrefix, nPrefixWidth;

		uFormat &= ~DT_PREFIX_TEXT;

		if (ShouldDrawTextPrefix(szText, nLen, str, nXPrefix, nPrefixWidth))
		{
			DrawTextEx(pGraph, lpRect, clrText, str.c_str(), str.size(), uFormat);

			if (nPrefixWidth > 0)
			{
				int			x, y, width, xLeftClipOffset; //, height = lpRect->bottom - lpRect->top;

				GetDrawTextExPosition(str.c_str(), str.size(), lpRect, uFormat, x, y, width, xLeftClipOffset);
				x -= 1;
				y -= 3;
				x += nXPrefix;
				y += m_prawGlyphSet->GetHeight();
				if (x > xLeftClipOffset)
				{
					CRect	rc;
					rc.SetRect(x, y, x + nPrefixWidth, y + 1);
					pGraph->FillRect(&rc, clrText, BPM_CHANNEL_RGB | BPM_OP_COPY);
				}
			}

			return true;
		}
	}

	int			x, y, width, xLeftClipOffset; //, height = lpRect->bottom - lpRect->top;

	GetDrawTextExPosition(szText, nLen, lpRect, uFormat, x, y, width, xLeftClipOffset);

	x -= 2;
	y -= 2;

	return DrawText(pGraph, x, y, width, xLeftClipOffset, clrText, szText, nLen);
}

bool CRawBmpFont::DrawText(CRawGraph *pGraph, int x, int y, int width, int xLeftClipOffset, const MLColor &clrText, LPCTSTR szText, int nLen)
{
	byte		r = clrText.r(), g = clrText.g(), b = clrText.b();
	byte		textAlpha = pGraph->GetTextAlpha();
	int			yyStart, yyEnd;
	CRect		rcClip;
	PRAW_IMAGE_DATA pGraphRaw = pGraph->GetRawBuff();
	agg::rendering_buffer bufGraph(pGraphRaw->buff, pGraphRaw->width, pGraphRaw->height, pGraphRaw->stride);
	int			xPattern1 = 0, xPattern2 = 0;

	pGraph->GetMappedClipBoundRect(rcClip);

	if (x + xLeftClipOffset > rcClip.right)
		return true;
	if (x + xLeftClipOffset > rcClip.left)
		rcClip.left = x + xLeftClipOffset;

	if (y >= rcClip.bottom)
		return true;
	if (y + GetGlyphHeight() <= rcClip.top)
		return true;

	if (x + width < rcClip.right)
		rcClip.right = x + width;

	if (y < rcClip.top)
		yyStart = rcClip.top - y;
	else
		yyStart = 0;
	if (y + GetGlyphHeight() > rcClip.bottom)
		yyEnd = rcClip.bottom - y;
	else
		yyEnd = GetGlyphHeight();

	StringIterator	strIterator(szText);
	for (; strIterator.GetPos() < nLen; ++strIterator)
	{
		int			xxStart, xxEnd;
		// byte		*p, *pRowGlyph, *graphData, *pRowGraph;
		// int		xx, yy;
		GfxRaw::Glyph		*glyph = m_prawGlyphSet->GetGlyph(strIterator.CurChar());
		if (!glyph)
			continue;

		if (x >= rcClip.right)
			return true;

		if (x + glyph->nWidthBitmap <= rcClip.left)
		{
			x += glyph->nWidth;
			if (m_imgPattern1)
				xPattern1 = (xPattern1 + glyph->nWidthBitmap) % m_imgPattern1->width();
			if (m_imgPattern2)
				xPattern2 = (xPattern2 + glyph->nWidthBitmap) % m_imgPattern2->width();
			continue;
		}

		if (x < rcClip.left)
			xxStart = rcClip.left - x;
		else
			xxStart = 0;
		if (x + glyph->nWidthBitmap > rcClip.right)
			xxEnd = rcClip.right - x;
		else
			xxEnd = glyph->nWidthBitmap;

		if (m_imgPattern1)
			xPattern1 = (xPattern1 + xxStart) % m_imgPattern1->width();

		agg::rendering_buffer bufGlyph(glyph->bitmap, glyph->nWidthBitmap, GetGlyphHeight(), glyph->WidthBytes());

		if (m_overlayMode == OM_PATTERN)
			DrawGlyphRGBA32(bufGraph, x + xxStart, y + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, m_imgPattern1, xPattern1);
		else if (m_overlayMode == OM_COLOR)
			DrawGlyphRGBA32(bufGraph, x + xxStart, y + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, clrText);
		else if (m_overlayMode == OM_DUAL_PATTERN)
		{
			xPattern2 = (xPattern2 + xxStart) % m_imgPattern2->width();
			DrawGlyphRGBA32(bufGraph, x + xxStart, y + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, m_imgPattern1, xPattern1, m_nAlphaPattern1, m_imgPattern2, xPattern2, m_nAlphaPattern2);
		}
		else if (m_overlayMode == OM_PATTERN_COLOR)
			DrawGlyphRGBA32(bufGraph, x + xxStart, y + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, m_imgPattern1, xPattern1, m_nAlphaPattern1, m_clrPattern2, m_nAlphaPattern2);

		x += glyph->nWidth;
	}

	return true;
}

bool CRawBmpFont::OutlinedTextOut(CRawGraph *pGraph, int x, int y, const MLColor &clrText, const MLColor &clrBorder, LPCTSTR szText, int nLen)
{
	return OutlinedDrawText(pGraph, x, y, pGraph->Width() - x, 0, clrText, clrBorder, szText, nLen);
}


bool CRawBmpFont::OutlinedDrawTextEx(CRawGraph *pGraph, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, LPCTSTR szText, int nLen, UINT uFormat)
{
	ASSERT(m_prawGlyphSet);
	if (!m_prawGlyphSet)
		return false;

	if (IsFlagSet(uFormat, DT_END_ELLIPSIS))
	{
		tstring		str;

		uFormat &= ~DT_END_ELLIPSIS;

		if (ShouldDrawTextEllipsis(szText, nLen, lpRect->right - lpRect->left, str))
			return OutlinedDrawTextEx(pGraph, lpRect, clrText, clrBorder, str.c_str(), str.size(), uFormat);
	}
	else if (IsFlagSet(uFormat, DT_PREFIX_TEXT))
	{
		tstring		str;
		int			nXPrefix, nPrefixWidth;

		uFormat &= ~DT_PREFIX_TEXT;

		if (ShouldDrawTextPrefix(szText, nLen, str, nXPrefix, nPrefixWidth))
		{
			OutlinedDrawTextEx(pGraph, lpRect, clrText, clrBorder, str.c_str(), str.size(), uFormat);

			if (nPrefixWidth > 0)
			{
				int			x, y, width, xLeftClipOffset; //, height = lpRect->bottom - lpRect->top;

				GetDrawTextExPosition(str.c_str(), str.size(), lpRect, uFormat, x, y, width, xLeftClipOffset);
				x += 1;
				y -= 3;
				x += nXPrefix;
				y += m_prawGlyphSet->GetHeight();
				if (x > xLeftClipOffset)
				{
					CRect	rc;
					rc.SetRect(x, y, x + nPrefixWidth, y + 1);
					pGraph->FillRect(&rc, clrText, BPM_CHANNEL_RGB | BPM_OP_COPY);
				}
			}

			return true;
		}
	}


	int			x, y, width, xLeftClipOffset; //, height = lpRect->bottom - lpRect->top;

	GetDrawTextExPosition(szText, nLen, lpRect, uFormat, x, y, width, xLeftClipOffset);

	x -= 2;
	y -= 2;

	return OutlinedDrawText(pGraph, x, y, width, xLeftClipOffset, clrText, clrBorder, szText, nLen);
}

bool CRawBmpFont::OutlinedDrawText(CRawGraph *pGraph, int x, int y, int width, int xLeftClipOffset, const MLColor &clrText, const MLColor &clrBorder, LPCTSTR szText, int nLen)
{
	byte		textAlpha = pGraph->GetTextAlpha();
	int			yyStart, yyEnd;
	int			nOFHeight = GetGlyphHeight() + OUTLINED_BORDER;
	int			nOFWidth;	// OF = OutlinedFont
	CRect		rcClip;

	y -= OUTLINED_BORDER / 2;

	PRAW_IMAGE_DATA pGraphRaw = pGraph->GetRawBuff();
	agg::rendering_buffer bufGraph(pGraphRaw->buff, pGraphRaw->width, pGraphRaw->height, pGraphRaw->stride);
	int			xPattern1 = 0, xPattern2 = 0;

	pGraph->GetMappedClipBoundRect(rcClip);

	if (x + xLeftClipOffset > rcClip.right)
		return true;
	if (x + xLeftClipOffset > rcClip.left)
		rcClip.left = x + xLeftClipOffset;

	if (y >= rcClip.bottom)
		return true;
	if (y + nOFHeight <= rcClip.top)
		return true;

	if (x + width < rcClip.right)
		rcClip.right = x + width;

	if (x + OUTLINED_BORDER / 2 < rcClip.left)
	{
		if (m_imgPattern1)
			xPattern1 = (rcClip.left - x - OUTLINED_BORDER / 2) % m_imgPattern1->width();
		if (m_imgPattern2)
			xPattern2 = (rcClip.left - x - OUTLINED_BORDER / 2) % m_imgPattern2->width();
	}

	StringIterator	strIterator(szText);
	for (; strIterator.GetPos() < nLen; ++strIterator)
	{
		int			xxStart, xxEnd;
		// int			xx, yy;
		// byte		*p, *pRowGlyph, *graphData, *pRowGraph;
		GfxRaw::Glyph		*glyph = m_prawGlyphSet->GetGlyph(strIterator.CurChar());
		if (!glyph)
			continue;

		if (x >= rcClip.right)
			return true;

		nOFWidth = glyph->nWidthBitmap + OUTLINED_BORDER;

		if (x + nOFWidth <= rcClip.left)
		{
			x += glyph->nWidth;
			continue;
		}

		if (!glyph->bitmapOutlined)
			CreateOutlinedGlyph(glyph, GetGlyphHeight(), m_shadowMode);

		if (glyph->bitmapOutlined)
		{
			//
			// Draw outlined border
			//
			if (y < rcClip.top)
				yyStart = rcClip.top - y;
			else
				yyStart = 0;
			if (y + nOFHeight > rcClip.bottom)
				yyEnd = rcClip.bottom - y;
			else
				yyEnd = nOFHeight;

			if (x < rcClip.left)
				xxStart = rcClip.left - x;
			else
				xxStart = 0;
			if (x + nOFWidth > rcClip.right)
				xxEnd = rcClip.right - x;
			else
				xxEnd = nOFWidth;

			agg::rendering_buffer bufGlyph(glyph->bitmapOutlined, nOFWidth, nOFHeight, glyph->WidthBytesOutlined());
			DrawGlyphRGBA32(bufGraph, x + xxStart, y + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, clrBorder);
		}

		if (glyph->bitmap)
		{
			//
			// Draw inner text
			//
			if (y + OUTLINED_BORDER / 2 < rcClip.top)
				yyStart = rcClip.top - (y + OUTLINED_BORDER / 2);
			else
				yyStart = 0;
			if (y + OUTLINED_BORDER / 2 + GetGlyphHeight() > rcClip.bottom)
				yyEnd = rcClip.bottom - (y + OUTLINED_BORDER / 2);
			else
				yyEnd = GetGlyphHeight();

			if (x + OUTLINED_BORDER / 2 < rcClip.left)
				xxStart = rcClip.left - (x + OUTLINED_BORDER / 2);
			else
				xxStart = 0;
			if (x + OUTLINED_BORDER / 2 + glyph->nWidthBitmap > rcClip.right)
				xxEnd = rcClip.right - (x + OUTLINED_BORDER / 2);
			else
				xxEnd = glyph->nWidthBitmap;

			agg::rendering_buffer bufGlyph(glyph->bitmap, glyph->nWidthBitmap, GetGlyphHeight(), glyph->WidthBytes());

			if (m_overlayMode == OM_PATTERN)
				DrawGlyphRGBA32(bufGraph, x + OUTLINED_BORDER / 2 + xxStart, y + OUTLINED_BORDER / 2 + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, m_imgPattern1, xPattern1);
			else if (m_overlayMode == OM_COLOR)
				DrawGlyphRGBA32(bufGraph, x + OUTLINED_BORDER / 2 + xxStart, y + OUTLINED_BORDER / 2 + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, clrText);
			else if (m_overlayMode == OM_DUAL_PATTERN)
				DrawGlyphRGBA32(bufGraph, x + OUTLINED_BORDER / 2 + xxStart, y + OUTLINED_BORDER / 2 + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, m_imgPattern1, xPattern1, m_nAlphaPattern1, m_imgPattern2, xPattern2, m_nAlphaPattern2);
			else if (m_overlayMode == OM_PATTERN_COLOR)
				DrawGlyphRGBA32(bufGraph, x + OUTLINED_BORDER / 2 + xxStart, y + OUTLINED_BORDER / 2 + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, m_imgPattern1, xPattern1, m_nAlphaPattern1, m_clrPattern2, m_nAlphaPattern2);
		}

		x += glyph->nWidth;
	}
	return true;
}

void CRawBmpFont::SetOverlayPattern(CRawImage *imgPattern)
{
	ASSERT(imgPattern);
	m_imgPattern1 = imgPattern;
	m_overlayMode = OM_PATTERN;
	m_imgPattern2 = NULL;
}

void CRawBmpFont::SetOverlayPattern(CRawImage *imgPattern1, CRawImage *imgPattern2, int nAlphaPattern1, int nAlphaPattern2)
{
	ASSERT(imgPattern1);
	ASSERT(imgPattern2);
	m_imgPattern1 = imgPattern1;
	m_imgPattern2 = imgPattern2;
	m_nAlphaPattern1 = nAlphaPattern1;
	m_nAlphaPattern2 = nAlphaPattern2;
	m_overlayMode = OM_DUAL_PATTERN;
}

void CRawBmpFont::SetOverlayPattern(CRawImage *imgPattern1, int nAlphaPattern1, const MLColor &clrPattern2, int nAlphaPattern2)
{
	ASSERT(imgPattern1);
	m_imgPattern1 = imgPattern1;
	m_clrPattern2 = clrPattern2;
	m_nAlphaPattern1 = nAlphaPattern1;
	m_nAlphaPattern2 = nAlphaPattern2;
	m_overlayMode = OM_PATTERN_COLOR;
	m_imgPattern2 = NULL;
}

void CRawBmpFont::UseColorOverlay()
{
	m_imgPattern1 = NULL;
	m_imgPattern2 = NULL;
	m_overlayMode = OM_COLOR;
}

bool CRawBmpFont::ShouldDrawTextEllipsis(LPCTSTR szText, int nLen, int nWidthMax, tstring &strEllipsis)
{
	int		w = 0;
	GfxRaw::Glyph	*glyph;

	StringIterator	strIterator(szText);
	for (; strIterator.GetPos() < nLen; ++strIterator)
	{
		glyph = m_prawGlyphSet->GetGlyph(strIterator.CurChar());
		if (!glyph)
			continue;

		w += glyph->nWidth;
		if (w > nWidthMax)
			break;
	}

	if (strIterator.GetPos() >= nLen)
		return false;

	tstring		str;
	int			wMax;
	nLen = strIterator.GetPos();

	glyph = m_prawGlyphSet->GetGlyph((int)'.');
	if (!glyph)
		return false;
	wMax = nWidthMax - glyph->nWidth * 3;

	w = 0;
	for (strIterator.SetPos(0); strIterator.GetPos() < nLen; ++strIterator)
	{
		glyph = m_prawGlyphSet->GetGlyph(strIterator.CurChar());
		if (!glyph)
			continue;

		w += glyph->nWidth;
		if (w > wMax)
			break;

		str += (LPCTSTR)glyph->ch;
	}
	str += _T("...");

	return true;
}


bool CRawBmpFont::ShouldDrawTextPrefix(LPCTSTR szText, int nLen, tstring &strPrfix, int &nXPrefix, int &nWidthPrefix)
{
	GfxRaw::Glyph	*glyph;

	nXPrefix = 0;
	nWidthPrefix = 0;

	StringIterator	strIterator(szText);
	for (; strIterator.GetPos() < nLen; ++strIterator)
	{
		glyph = m_prawGlyphSet->GetGlyph(strIterator.CurChar());
		if (!glyph)
			continue;

		if (glyph->ch == StringIterator::CharType('&'))
		{
			break;
		}

		nXPrefix += glyph->nWidth;
	}
	if (strIterator.GetPos() >= nLen)
		return false;

	strPrfix = szText;
	strPrfix.erase(strIterator.GetPos(), 1);

	if (strIterator.GetPos() < (int)strPrfix.size())
	{
		StringIterator strIteratorNew(strPrfix.c_str());
		strIteratorNew.SetPos(strIterator.GetPos());

		glyph = m_prawGlyphSet->GetGlyph(strIteratorNew.CurChar());
		if (glyph)
			nWidthPrefix = glyph->nWidth;
	}

	return true;
}


int CRawBmpFont::GetTextWidth(LPCTSTR szText, int nLen)
{
	int		n = 0;

	for (int i = 0; i < nLen; i++)
	{
		GfxRaw::Glyph		*glyph = m_prawGlyphSet->GetGlyph((unsigned short)szText[i]);
		if (!glyph)
			continue;

		n += glyph->nWidth;
	}

	return n;
}
