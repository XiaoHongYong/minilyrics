#ifndef _SKIN_IMAGE_INC_
#define _SKIN_IMAGE_INC_

class CSkinFactory;

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

	void ExtendXBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm = BPM_ALPHA_BLEND);
	void ExtendXBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, int nImageX, int nImageY, int nImageCx, int nImageCy, BlendPixMode bpm = BPM_ALPHA_BLEND);
	void ExtendYBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm = BPM_ALPHA_BLEND);
	void TileBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm = BPM_ALPHA_BLEND);
	void TileBltEx(CRawGraph *pGraphics, int xOrg, int yOrg, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm = BPM_ALPHA_BLEND);

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
				drawFunc(this,
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
					drawFunc(this,
						x, yDest,
						m_cx, nLeftCy,
						m_x, m_y + ySrc);
				}

				// AAA
				if (x < xDest + nWidthDest)
				{
					drawFunc(this,
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
				drawFunc(this,
					xDest, y,
					nW, m_cy,
					m_x + xSrc, m_y);
			}

			// 1111
			for (x = nTileX; x + m_cx < xDest + nWidthDest; x += m_cx)
			{
				drawFunc(this,
					x, y,
					m_cx, m_cy,
					m_x, m_y);
			}

			// 222
			if (x < xDest + nWidthDest)
			{
				drawFunc(this,
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
				drawFunc(this,
					xDest, y,
					nW, nLeftCy,
					m_x + xSrc, m_y);
			}

			// 3333
			for (x = nTileX; x + m_cx <= xDest + nWidthDest; x += m_cx)
			{
				drawFunc(this,
					x, y,
					m_cx, nLeftCy,
					m_x, m_y);
			}

			// 444
			if (x < xDest + nWidthDest)
			{
				drawFunc(this,
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
