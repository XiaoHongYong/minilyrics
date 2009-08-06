// ImgList.cpp: implementation of the CSFImgList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SFImage.h"
#include "SFImgList.h"

#ifndef ILC_COLOR32
#define ILC_COLOR32		0
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSFImgList::CSFImgList()
{
	m_nItemCx = 0;
}

CSFImgList::~CSFImgList()
{
}

bool CSFImgList::Load(CSkinFactory *pSkinFactory, LPCTSTR szImage, int nCx)
{
	Destroy();

	if (!CSFImage::LoadFromSRM(pSkinFactory, szImage))
		return false;

	m_nItemCx = nCx;

	return true;
}

bool CSFImgList::Draw(CRawGraph *pGraph, int nIndex, int x, int y, BlendPixMode bpm)
{
	return Blt(pGraph, x, y, m_nItemCx, m_cy, m_x + m_nItemCx * nIndex, m_y);
}

int CSFImgList::GetIconCx()
{
	return m_nItemCx;
}

int CSFImgList::GetIconCy()
{
	return m_cy;
}
