#include "StdAfx.h"
#include "MLSkin.h"
#include "SkinImage.h"

CSFImage::CSFImage(void)
{
	m_pSkinFactory = NULL;
}

CSFImage::CSFImage(const CSFImage &src)
{
	m_pSkinFactory = NULL;
	CopyFrom(src);
}

CSFImage::~CSFImage(void)
{
	Destroy();
}


void CSFImage::Destroy()
{
	if (m_pSkinFactory)
	{
		if (m_hImage)
		{
			m_pSkinFactory->GetResourceMgr()->FreeBitmap(m_hImage);
			m_hImage = NULL;
		}
		m_pSkinFactory = NULL;
	}
	else
	{
		CRawImage::Destroy();
	}
}


CSFImage &CSFImage::operator=(const CSFImage &src)
{
	CopyFrom(src);

	return *this;
}

void CSFImage::CopyFrom(const CSFImage &src)
{
	if (this == &src)
		return;

	Destroy();

	if (src.m_pSkinFactory)
	{
		if (src.m_hImage)
		{
			src.m_pSkinFactory->GetResourceMgr()->AddBitmapReference(src.m_hImage);
			m_hImage = src.m_hImage;
		}
	}

	m_x = src.m_x;
	m_y = src.m_y;
	m_cx = src.m_cx;
	m_cy = src.m_cy;
	m_pSkinFactory = src.m_pSkinFactory;
}

bool CSFImage::LoadFromSRM(CSkinFactory *pskinFactory, LPCTSTR szResName)
{
	PRAW_IMAGE_DATA		hImage;

	Destroy();

	ASSERT(pskinFactory);
	m_pSkinFactory = pskinFactory;

	hImage = m_pSkinFactory->GetResourceMgr()->LoadBitmap(szResName);

	if (hImage)
		Attach(hImage);
	else
		ERRLOG1(_T("Load Bitmap: %s, FAILED."), szResName);

	return hImage != NULL;
}

void CSFImage::ExtendXBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm)
{
	assert(m_cx);
	if (m_cx == 0)
		return;

	int		x;

	for (x = xDest; x + m_cx < xDest + nWidthDest; x += m_cx)
	{
		CRawImage::Blt(pGraphics, 
			x, yDest,
			m_cx, m_cy,
			m_x, m_y, bpm);
	}

	if (x < xDest + nWidthDest)
	{
		CRawImage::Blt(pGraphics, 
			x, yDest,
			xDest + nWidthDest - x, m_cy,
			m_x, m_y, bpm);
	}
}

void CSFImage::ExtendXBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, int nImageX, int nImageY, int nImageCx, int nImageCy, BlendPixMode bpm)
{
	assert(nImageCx);
	if (nImageCx == 0)
		return;

	int		x;

	for (x = xDest; x + nImageCx < xDest + nWidthDest; x += nImageCx)
	{
		CRawImage::Blt(pGraphics, 
			x, yDest,
			nImageCx, nImageCy,
			nImageX, nImageY, bpm);
	}

	if (x < xDest + nWidthDest)
	{
		CRawImage::Blt(pGraphics, 
			x, yDest,
			xDest + nWidthDest - x, nImageCy,
			nImageX, nImageY, bpm);
	}
}

void CSFImage::ExtendYBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm)
{
	assert(m_cy);
	if (m_cy == 0)
		return;

	int		y;

	for (y = yDest; y + m_cy < yDest + nHeightDest; y += m_cy)
	{
		CRawImage::Blt(pGraphics, 
			xDest, y,
			m_cx, m_cy,
			m_x, m_y, bpm);
	}

	if (y < yDest + nHeightDest)
	{
		CRawImage::Blt(pGraphics, 
			xDest, y,
			m_cx, yDest + nHeightDest - y,
			m_x, m_y, bpm);
	}
}

void CSFImage::TileBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm)
{
	CDrawImageFun	fun;

	fun.m_pImage = this;
	fun.m_pGraph = pGraphics;
	fun.m_bpm = bpm;
	TileBltT(xDest, yDest, nWidthDest, nHeightDest, m_x, m_y, m_cx, m_cy, fun);
}

class CDrawMaskImageFun
{
public:
	void operator()(int xdest, int ydest, int width, int height, int xsrc, int ysrc)
	{
		m_pImage->MaskBlt(m_pGraph, xdest, ydest, width, height, xsrc, ysrc, m_pImageMask, m_bpm);
	}
public:
	CRawImage		*m_pImage;
	CRawGraph		*m_pGraph;
	CRawImage		*m_pImageMask;
	BlendPixMode	m_bpm;

};

void CSFImage::TileMaskBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, CRawImage *pImageMask, BlendPixMode bpm)
{
	CDrawMaskImageFun	fun;

	fun.m_pImage = this;
	fun.m_pGraph = pGraphics;
	fun.m_pImageMask = pImageMask;
	fun.m_bpm = bpm;
	TileBltT(xDest, yDest, nWidthDest, nHeightDest, m_x, m_y, m_cx, m_cy, fun);
}

/*
void CSFImage::TileBlt(CRawGraph *pGraphics, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm)
{
	assert(m_cy);
	assert(m_cx);

	if (m_cy == 0 || m_cx == 0)
		return;

	int			x, y;

	//
	// 先将Y方向的绘画了
	// 1111 1111 1111 222
	// 1111 1111 1111 222
	// 3333 3333 3333 444
	for (y = yDest; y + m_cy < yDest + nHeightDest; y += m_cy)
	{
		// 1111
		for (x = xDest; x + m_cx <= xDest + nWidthDest; x += m_cx)
		{
			CRawImage::Blt(pGraphics, 
				x, y,
				m_cx, m_cy,
				m_x, m_y, bpm);
		}

		// 2222
		if (x < xDest + nWidthDest)
		{
			CRawImage::Blt(pGraphics, 
				x, y,
				xDest + nWidthDest - x, m_cy,
				m_x, m_y, bpm);
		}
	}

	if (y < yDest + nHeightDest)
	{
		int		nLeftCy;

		nLeftCy = yDest + nHeightDest - y;

		// 3333
		for (x = xDest; x + m_cx <= xDest + nWidthDest; x += m_cx)
		{
			CRawImage::Blt(pGraphics, 
				x, y,
				m_cx, nLeftCy,
				m_x, m_y, bpm);
		}

		// 444
		if (x < xDest + nWidthDest)
		{
			CRawImage::Blt(pGraphics, 
				x, y,
				xDest + nWidthDest - x, nLeftCy,
				m_x, m_y, bpm);
		}
	}
}
*/

/*
template<class _DrawImageFun>
void CSkinImage::TileBltExT(CRawGraph *pGraphics, int xOrg, int yOrg, int xDest, int yDest, int nWidthDest, int nHeightDest, _DrawImageFun &drawFunc)
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
}*/

void CSFImage::TileBltEx(CRawGraph *pGraphics, int xOrg, int yOrg, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm)
{
	CDrawImageFun	fun;

	fun.m_pImage = this;
	fun.m_pGraph = pGraphics;
	fun.m_bpm = bpm;
	TileBltExT(pGraphics, xOrg, yOrg, xDest, yDest, nWidthDest, nHeightDest, fun);
}

/*
void CSFImage::TileBltEx(CRawGraph *pGraphics, int xOrg, int yOrg, int xDest, int yDest, int nWidthDest, int nHeightDest)
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
			pGraphics->DrawImage(this,
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
				pGraphics->DrawImage(this,
					x, yDest,
					m_cx, nLeftCy,
					m_x, m_y + ySrc);
			}

			// AAA
			if (x < xDest + nWidthDest)
			{
				pGraphics->DrawImage(this,
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
			pGraphics->DrawImage(this,
				xDest, y,
				nW, m_cy,
				m_x + xSrc, m_y);
		}

		// 1111
		for (x = nTileX; x + m_cx < xDest + nWidthDest; x += m_cx)
		{
			pGraphics->DrawImage(this,
				x, y,
				m_cx, m_cy,
				m_x, m_y);
		}

		// 222
		if (x < xDest + nWidthDest)
		{
			pGraphics->DrawImage(this,
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
			pGraphics->DrawImage(this,
				xDest, y,
				nW, nLeftCy,
				m_x + xSrc, m_y);
		}

		// 3333
		for (x = nTileX; x + m_cx <= xDest + nWidthDest; x += m_cx)
		{
			pGraphics->DrawImage(this,
				x, y,
				m_cx, nLeftCy,
				m_x, m_y);
		}

		// 444
		if (x < xDest + nWidthDest)
		{
			pGraphics->DrawImage(this,
				x, y,
				xDest + nWidthDest - x, nLeftCy,
				m_x, m_y);
		}
	}
}*/
