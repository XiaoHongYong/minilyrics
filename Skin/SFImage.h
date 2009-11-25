#ifndef _SKIN_IMAGE_INC_
#define _SKIN_IMAGE_INC_

class CSkinFactory;

/************************************************************************

The image with --- is extendable

********---****
********---****
********---****
---------------
---------------
********---****

/************************************************************************/


class CScaleImagePainter
{
public:
	int			nSrcX, nSrcY, nSrcWidth, nSrcHeight;
	int			nSrcHorzExtendStart, nSrcHorzExtendEnd;
	int			nSrcVertExtendStart, nSrcVertExtendEnd;

public:
	template<class _DrawImageFun>
	void Blt(int nDstX, int nDstY, int nDstW, int nDstH, _DrawImageFun &blter)
	{
		// 			********---****
		// 			***A****-B-**C*
		// 			********---****
		// 			---D-----E---F-
		// 			---------------
		// 			***G****-H-**J*
		int			nDstHorzExtendStart, nDstHorzExtendEnd;
		int			nDstVertExtendStart, nDstVertExtendEnd;

		nDstHorzExtendStart = nDstX + nSrcHorzExtendStart;
		nDstHorzExtendEnd = nDstX + nDstW - (nSrcWidth - (nSrcHorzExtendEnd - nSrcX));
		nDstVertExtendStart = nDstY + nSrcVertExtendStart;
		nDstVertExtendEnd = nDstY + nDstH - (nSrcHeight - (nSrcVertExtendEnd - nSrcY));

		// Draw area A
		blter(nDstX, nDstY,
			nDstHorzExtendStart - nDstX, nSrcVertExtendStart - nDstY,
			nSrcX, nSrcY);

		// Draw area B
		if (nDstHorzExtendStart < nDstHorzExtendEnd)
		{
			TileBltT(nDstHorzExtendStart, nDstY, 
				nDstHorzExtendEnd - nDstHorzExtendStart,
				nSrcVertExtendStart - nSrcY,	// Destination cy
				nSrcHorzExtendStart, nSrcY,		// Source x, y
				nSrcHorzExtendEnd - nSrcHorzExtendStart, 
				nSrcVertExtendStart - nSrcY, blter);
		}

		// Draw area C
		blter(nDstHorzExtendEnd, nDstY,
			nDstX + nDstW - nDstHorzExtendEnd,
			nSrcVertExtendStart - nSrcY,
			nSrcHorzExtendEnd, nSrcY);

		//
		// Line 2
		//

		// Draw area D
		TileBltT(nDstX, nDstVertExtendStart,
			nDstHorzExtendStart - nDstX,
			nDstVertExtendEnd - nDstVertExtendStart,
			nSrcX, nSrcVertExtendStart,
			nSrcHorzExtendStart - nSrcX,
			nSrcVertExtendEnd - nSrcVertExtendStart, blter);

		// Draw area E
		TileBltT(nDstHorzExtendStart, nDstVertExtendStart,
			nDstHorzExtendEnd - nDstHorzExtendStart,
			nDstVertExtendEnd - nDstVertExtendStart,
			nSrcHorzExtendStart, nSrcVertExtendStart,
			nSrcHorzExtendEnd - nSrcHorzExtendStart,
			nSrcVertExtendEnd - nSrcVertExtendStart, blter);

		// Draw area F
		TileBltT(nDstHorzExtendEnd, nDstVertExtendStart,
			nDstX + nDstW - nDstHorzExtendEnd,
			nDstVertExtendEnd - nDstVertExtendStart,
			nSrcHorzExtendEnd, nSrcVertExtendStart,
			nSrcX + nSrcWidth - nSrcHorzExtendEnd,
			nSrcVertExtendEnd - nSrcVertExtendStart, blter);

		//
		// Line 3
		//

		// Draw area G
		blter(nDstX, nDstVertExtendEnd,
			nDstHorzExtendStart - nDstX, nDstY + nDstH - nSrcVertExtendEnd,
			nSrcX, nSrcVertExtendEnd);

		// Draw area H
		if (nDstHorzExtendStart < nDstHorzExtendEnd)
		{
			TileBltT(nDstHorzExtendStart, nDstVertExtendEnd, 
				nDstHorzExtendEnd - nDstHorzExtendStart,
				nDstY + nDstH - nSrcVertExtendEnd,	// Destination cy
				nSrcHorzExtendStart, nSrcVertExtendEnd,		// Source x, y
				nSrcHorzExtendEnd - nSrcHorzExtendStart, 
				nSrcVertExtendStart - nSrcY, blter);
		}

		// Draw area J
		blter(nDstHorzExtendEnd, nDstVertExtendEnd,
			nDstX + nDstW - nDstHorzExtendEnd,
			nDstY + nDstH - nSrcVertExtendEnd,
			nSrcHorzExtendEnd, nSrcVertExtendEnd);
	}

};

template<class _DrawImageFun>
void TileBltT(int xDest, int yDest, int nWidthDest, int nHeightDest, int xSrc, int ySrc, int cxSrcUnit, int cySrcUnit, _DrawImageFun &drawFunc)
{
	assert(cySrcUnit);
	assert(cxSrcUnit);

	if (cySrcUnit == 0 || cxSrcUnit == 0)
		return;

	int			x, y;

	//
	// 先将Y方向的绘画了
	// 1111 1111 1111 222
	// 1111 1111 1111 222
	// 3333 3333 3333 444
	for (y = yDest; y + cySrcUnit < yDest + nHeightDest; y += cySrcUnit)
	{
		// 1111
		for (x = xDest; x + cxSrcUnit <= xDest + nWidthDest; x += cxSrcUnit)
		{
			drawFunc(
				x, y,
				cxSrcUnit, cySrcUnit,
				xSrc, ySrc);
		}

		// 2222
		if (x < xDest + nWidthDest)
		{
			drawFunc(
				x, y,
				xDest + nWidthDest - x, cySrcUnit,
				xSrc, ySrc);
		}
	}

	if (y < yDest + nHeightDest)
	{
		int		nLeftCy;

		nLeftCy = yDest + nHeightDest - y;

		// 3333
		for (x = xDest; x + cxSrcUnit <= xDest + nWidthDest; x += cxSrcUnit)
		{
			drawFunc(
				x, y,
				cxSrcUnit, nLeftCy,
				xSrc, ySrc);
		}

		// 444
		if (x < xDest + nWidthDest)
		{
			drawFunc(
				x, y,
				xDest + nWidthDest - x, nLeftCy,
				xSrc, ySrc);
		}
	}
}

class CDrawImageFun
{
public:
	void operator()(int xdest, int ydest, int width, int height, int xsrc, int ysrc)
	{
		m_pImage->Blt(m_pGraph, xdest, ydest, width, height, xsrc, ysrc, m_bpm);
	}
public:
	CRawImage		*m_pImage;
	CRawGraph		*m_pGraph;
	BlendPixMode	m_bpm;

};

// CSFImage = CSkinFactoryImage
class CSFImage :
	public CRawImage
{
public:
	CSFImage(void);
	CSFImage(const CSFImage &src);
	virtual ~CSFImage(void);

public:
	virtual void Destroy();

	CSFImage &operator=(const CSFImage &src);

	bool LoadFromSRM(CSkinFactory *pskinFactory, LPCTSTR szResName);

	void ExtendXBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm = BPM_BLEND);
	void ExtendXBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, int nImageX, int nImageY, int nImageCx, int nImageCy, BlendPixMode bpm = BPM_BLEND);
	void ExtendYBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm = BPM_BLEND);
	void TileBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm = BPM_BLEND);
	void TileMaskBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, CRawImage *pImageMask, BlendPixMode bpm = BPM_BLEND);
	void TileBltEx(CRawGraph *pGraphics, int xOrg, int yOrg, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm = BPM_BLEND);

protected:

	template<class _DrawImageFun>
	void TileBltExT(CRawGraph *pGraphics, int xOrg, int yOrg, int xDest, int yDest, int nWidthDest, int nHeightDest, _DrawImageFun &drawFunc)
	{
		assert(m_cy);
		assert(m_cx);

		if (m_cy == 0 || m_cx == 0)
			return;

		int			xSrc, ySrc, x, y;
		int			nTileX, nTileY;
		int			nW;

		//
		// 先将Y方向的绘画了
		// BBB 0000 0000 0000 AAA
		// CCC 1111 1111 1111 222
		// CCC 1111 1111 1111 222
		// DDD 3333 3333 3333 444

		// BBB
		{
			int		nLeftCy;
			xSrc = (xDest - xOrg) % m_cx;
			ySrc = (yDest - yOrg) % m_cy;
			nLeftCy = min(m_cy - ySrc, nHeightDest);
			if (xSrc != 0)
			{
				nW = min(m_cx - xSrc, nWidthDest);
				drawFunc(
					xDest, yDest,
					nW, nLeftCy,
					m_x + xSrc, m_y + ySrc);
				nTileX = xDest + (m_cx - xSrc);
			}
			else
				nTileX = xDest;

			// 0000
			if (ySrc != 0)
			{
				for (x = nTileX; x + m_cx <= xDest + nWidthDest; x += m_cx)
				{
					drawFunc(
						x, yDest,
						m_cx, nLeftCy,
						m_x, m_y + ySrc);
				}

				// AAA
				if (x < xDest + nWidthDest)
				{
					drawFunc(
						x, yDest,
						xDest + nWidthDest - x, nLeftCy,
						m_x, m_y + ySrc);
				}
				nTileY = yDest + (m_cy - ySrc);
			}
			else
				nTileY = yDest;
		}

		for (y = nTileY; y + m_cy < yDest + nHeightDest; y += m_cy)
		{
			// CCCC
			if (xSrc != 0)
			{
				nW = min(m_cx - xSrc, nWidthDest);
				drawFunc(
					xDest, y,
					nW, m_cy,
					m_x + xSrc, m_y);
			}

			// 1111
			for (x = nTileX; x + m_cx < xDest + nWidthDest; x += m_cx)
			{
				drawFunc(
					x, y,
					m_cx, m_cy,
					m_x, m_y);
			}

			// 222
			if (x < xDest + nWidthDest)
			{
				drawFunc(
					x, y,
					xDest + nWidthDest - x, m_cy,
					m_x, m_y);
			}
		}

		if (y < yDest + nHeightDest)
		{
			int		nLeftCy;

			nLeftCy = yDest + nHeightDest - y;

			// DDD
			if (xSrc != 0)
			{
				nW = min(m_cx - xSrc, nWidthDest);
				drawFunc(
					xDest, y,
					nW, nLeftCy,
					m_x + xSrc, m_y);
			}

			// 3333
			for (x = nTileX; x + m_cx <= xDest + nWidthDest; x += m_cx)
			{
				drawFunc(
					x, y,
					m_cx, nLeftCy,
					m_x, m_y);
			}

			// 444
			if (x < xDest + nWidthDest)
			{
				drawFunc(
					x, y,
					xDest + nWidthDest - x, nLeftCy,
					m_x, m_y);
			}
		}
	}

	void CopyFrom(const CSFImage &src);

public:
	CSkinFactory	*m_pSkinFactory;

};

#endif // _SKIN_IMAGE_INC_
