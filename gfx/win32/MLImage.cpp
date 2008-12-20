#include "StdAfx.h"
#include "gfx.h"
#include "MLImage.h"

CMLImage::CMLImage(void)
{
	m_hBmp = NULL;
	m_cx = 0;
	m_cy = 0;
}

CMLImage::~CMLImage(void)
{
	Destroy();
}

void CMLImage::Attach(HBITMAP hBmp)
{
	BITMAP	bitmap;

	GetObject(hBmp, sizeof(bitmap), &bitmap);
	m_cx = bitmap.bmWidth;
	m_cy = bitmap.bmHeight;

	m_hBmp = hBmp;
}

void CMLImage::Detach()
{
	m_hBmp = NULL;
}

void CMLImage::Destroy()
{
	if (m_hBmp)
	{
		::DeleteObject(m_hBmp);
		m_hBmp = NULL;
	}
}

bool CMLImage::Load(LPCTSTR szFile)
{
	IMAGE_HANDLE		hImage;

	Destroy();

	hImage = LoadBitmapFromFile(szFile);

	if (hImage)
		Attach(hImage);

	return hImage != NULL;
}

bool CMLImage::Blt(IMLGraphicsBase *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc)
{
	pGraphics->DrawImage(this, xDest, yDest, widthDest, heightDest, xSrc, ySrc);

	return true;
}

bool CMLImage::StretchBlt(IMLGraphicsBase *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc)
{
	pGraphics->DrawImageStretch(this, xDest, yDest, widthDest, heightDest, xSrc, ySrc, widthSrc, heightSrc);

	return true;
}

bool CMLImage::GetOrginalSize(int &nWidth, int &nHeight)
{
	BITMAP	bitmap;

	GetObject(m_hBmp, sizeof(bitmap), &bitmap);

	nWidth = m_cx = bitmap.bmWidth;
	nHeight = m_cy = bitmap.bmHeight;

	return true;
}
