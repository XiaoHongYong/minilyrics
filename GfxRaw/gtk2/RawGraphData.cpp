#include "stdafx.h"
#include "GfxRaw.h"
#include "RawGraphData.h"

CRawGraphData::CRawGraphData()
{
	m_pGdkPixBuf = NULL;
}

CRawGraphData::~CRawGraphData()
{
	Destroy();
}

bool CRawGraphData::Create(int cx, int cy, IMLGraphicsBase *pGraph, int nBitCount)
{
	ASSERT(nBitCount == 32);

	if (!m_imageData.Create(cx, cy, nBitCount))
		return false;

	m_pGdkPixBuf = gdk_pixbuf_new_from_data(m_imageData.buff, 
		GDK_COLORSPACE_RGB,
		nBitCount == 32,
		8,
		cx,
		cy,
		m_imageData.stride,
		NULL,
		NULL);

	return m_pGdkPixBuf != NULL;
}

void CRawGraphData::Destroy()
{
	gdk_pixbuf_unref(m_pGdkPixBuf);

	m_imageData.Free();
}

void CRawGraphData::DrawToGraphics(IMLGraphicsBase *pTarg, int xdest, int ydest, int width, int height, int xsrc, int ysrc)
{
	BitBlt(pTarg->GetHandle(), xdest, ydest, width, height, GetHandle(), xsrc, ysrc, SRCCOPY);
}

void CRawGraphData::DrawToGraphicsStretch(IMLGraphicsBase *pTarg, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc)
{
	StretchBlt(pTarg->GetHandle(), xdest, ydest, width, height, GetHandle(), xsrc, ysrc, widthsrc, heightsrc, SRCCOPY);
}
