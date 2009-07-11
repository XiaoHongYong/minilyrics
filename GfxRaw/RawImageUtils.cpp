#include "stdafx.h"
#include "GfxRaw.h"
#include "RawImage.h"
#include "RawImageUtils.h"

#ifdef _WIN32

CMLImage *RawImageToMLImage(CRawImage &image)
{
	HBITMAP			hBmp = NULL;
	PRAW_IMAGE_DATA	pRawImage = image.GetHandle();
	BITMAPINFO	m_bmpInfo;
	HDC			hdcDesktop = NULL;
	int			w, h;
	uint8		*pBmpData = NULL;

	if (!pRawImage)
		return NULL;

	w = pRawImage->width;
	h = pRawImage->height;

	hdcDesktop = ::GetDC(NULL);
	ASSERT(hdcDesktop);

	m_bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
	m_bmpInfo.bmiHeader.biWidth = w;
	m_bmpInfo.bmiHeader.biHeight = h;
	m_bmpInfo.bmiHeader.biPlanes = 1;
	m_bmpInfo.bmiHeader.biBitCount = pRawImage->bitCount;
	m_bmpInfo.bmiHeader.biCompression = BI_RGB;
	m_bmpInfo.bmiHeader.biSizeImage = 0;
	m_bmpInfo.bmiHeader.biXPelsPerMeter = 0;
	m_bmpInfo.bmiHeader.biYPelsPerMeter = 0;
	m_bmpInfo.bmiHeader.biClrUsed = 0;
	m_bmpInfo.bmiHeader.biClrImportant = 0;

	hBmp = ::CreateDIBSection(
		hdcDesktop,
		&m_bmpInfo,
		DIB_RGB_COLORS,
		(void **)&(pBmpData),
		0,
		0
		);
	if (!hBmp)
	{
		::ReleaseDC(NULL, hdcDesktop);
		return NULL;
	}

	CRawImage		dst;
	RAW_IMAGE_DATA	dstRawImg;

	dstRawImg.Attach(pBmpData, w, h, pRawImage->bitCount);
	dst.Attach(&dstRawImg);

	// image.StretchBlt(&dst, 0, 0, w, h, 0, 0, pRawImage->width, pRawImage->height);
	image.Blt(&dst, 0, 0, w, h, 0, 0);
	dst.Detach();

	::ReleaseDC(NULL, hdcDesktop);

	CMLImage	*pImage;

	pImage = new CMLImage;
	pImage->Attach(hBmp);

	return pImage;

	/*	HBITMAP		hBmp;
	HDC			hdc;
	HDC			hdcDesktop = NULL;
	CRawGraph	graph;
	CGdiGraphics	gdiGraph;
	PRAW_IMAGE_DATA	pRawImage = image.GetHandle();


	if (w <= 0)
	w = pRawImage->width;
	if (h <= 0)
	h = pRawImage->height;

	hdcDesktop = ::GetDC(NULL);
	ASSERT(hdcDesktop);

	gdiGraph.Create(hdcDesktop);

	graph.Create(w, h, &gdiGraph, 24);

	if (w != pRawImage->width || h != pRawImage->height)
	graph.DrawImageStretch(&image, 0, 0, w, h, 0, 0, image.width(), image.height());
	else
	graph.DrawImage(&image, 0, 0, w, h, 0, 0);

	hdc = CreateCompatibleDC(hdcDesktop);
	hBmp = CreateBitmapIndirect()

	hBmpOld = (HBITMAP)SelectObject(hdcDesktop, hBmp);

	SelectObject(hdcDesktop, hBmpOld);

	::ReleaseDC(NULL, hdcDesktop);*/

}

#endif	// _WIN32

#ifdef _LINUX_GTK2

CMLImage *RawImageToMLImage(CRawImage &image)
{
	return NULL;
}

#endif
