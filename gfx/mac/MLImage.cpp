#include "StdAfx.h"
#include "gfx.h"
#include "MLImage.h"

CMLImage::CMLImage(void)
{
}

CMLImage::~CMLImage(void)
{
	Destroy();
}

void CMLImage::Destroy()
{
}

bool CMLImage::Load(LPCTSTR szFile)
{
	return false;
}

bool CMLImage::Blt(IMLGraphicsBase *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc)
{
	return false;
}

bool CMLImage::StretchBlt(IMLGraphicsBase *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc)
{
	return false;
}

bool CMLImage::GetOrginalSize(int &nWidth, int &nHeight)
{
	return false;
}

void CMLImage::Attach(IMAGE_HANDLE hBmp)
{
}

void CMLImage::Detach()
{
}
