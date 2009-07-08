#ifndef _RAW_GRAPH_DATA_H_
#define _RAW_GRAPH_DATA_H_

#pragma once

class CRawGraphData
{
public:
	CRawGraphData();
	virtual ~CRawGraphData();

	virtual bool Create(int cx, int cy, IMLGraphicsBase *pGraph, int nBitCount = 32);

	virtual void Destroy();

	int Width() const { return m_imageData.width; }
	int Height() const { return m_imageData.height; }

	RAW_IMAGE_DATA *GetRawBuff() { return &m_imageData; }
	uint8 *GetBuff() { return m_imageData.buff; }

	bool IsValid() { return m_imageData.buff != NULL; }

	virtual void DrawToGraphics(IMLGraphicsBase *pTarg, int xdest, int ydest, int width, int height, int xsrc, int ysrc);
	virtual void DrawToGraphicsStretch(IMLGraphicsBase *pTarg, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc);

// 	virtual void DrawGraphicsAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha);
// 	virtual void DrawGraphicsStretchAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha);

public:
	// For win32 only
	HDC GetHandle() const { return m_hdc; }

protected:
	HDC				m_hdc;
	RAW_IMAGE_DATA	m_imageData;
	HBITMAP			m_bmpMem;
	HBITMAP			m_bmpOld;

	BITMAPINFO		m_bmpInfo;

};

#endif // _RAW_GRAPH_DATA_H_
