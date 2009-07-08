#include "stdafx.h"
#include "GfxRaw.h"
#include "RawGraphData.h"

CRawGraphData::CRawGraphData()
{
	m_hdc = NULL;
	m_bmpMem = NULL;
	m_bmpOld = NULL;
}

CRawGraphData::~CRawGraphData()
{
	Destroy();
}

bool CRawGraphData::Create(int cx, int cy, IMLGraphicsBase *pGraph, int nBitCount)
{
	HDC		hdc = pGraph->GetHandle();

	m_bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
	m_bmpInfo.bmiHeader.biWidth = cx;
	m_bmpInfo.bmiHeader.biHeight = cy;
	m_bmpInfo.bmiHeader.biPlanes = 1;
	m_bmpInfo.bmiHeader.biBitCount = nBitCount;
	m_bmpInfo.bmiHeader.biCompression = BI_RGB;
	m_bmpInfo.bmiHeader.biSizeImage = 0;
	m_bmpInfo.bmiHeader.biXPelsPerMeter = 0;
	m_bmpInfo.bmiHeader.biYPelsPerMeter = 0;
	m_bmpInfo.bmiHeader.biClrUsed = 0;
	m_bmpInfo.bmiHeader.biClrImportant = 0;

	m_hdc = ::CreateCompatibleDC(hdc);

	m_imageData.buff = NULL;

	m_bmpMem = ::CreateDIBSection(
		m_hdc,
		&m_bmpInfo,
		DIB_RGB_COLORS,
		(void **)&(m_imageData.buff),
		0,
		0
		);

	m_imageData.Attach(m_imageData.buff, cx, cy, nBitCount);

	m_bmpOld = (HBITMAP)::SelectObject(m_hdc, m_bmpMem);

	return true;
}

void CRawGraphData::Destroy()
{
	if (m_hdc)
	{
		// Free resources 
		::SelectObject(m_hdc, m_bmpOld);
		::DeleteObject(m_bmpMem);
		::DeleteObject(m_hdc);
	}

	m_imageData.buff = NULL;
}

void CRawGraphData::DrawToGraphics(IMLGraphicsBase *pTarg, int xdest, int ydest, int width, int height, int xsrc, int ysrc)
{
	BitBlt(pTarg->GetHandle(), xdest, ydest, width, height, GetHandle(), xsrc, ysrc, SRCCOPY);
}

void CRawGraphData::DrawToGraphicsStretch(IMLGraphicsBase *pTarg, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc)
{
	StretchBlt(pTarg->GetHandle(), xdest, ydest, width, height, GetHandle(), xsrc, ysrc, widthsrc, heightsrc, SRCCOPY);
}
