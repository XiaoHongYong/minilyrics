// ImgList.cpp: implementation of the CImgList class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLSkin.h"
#include "ImgList.h"

#ifndef ILC_COLOR32
#define ILC_COLOR32		0
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImgList::CImgList()
{
}

CImgList::~CImgList()
{
	Close();
}

bool CImgList::LoadBitmap(CSkinFactory *pSkinFactory, LPCTSTR szImage, int nCx, COLORREF clrMask)
{
	return false;
}

bool CImgList::LoadBitmap(UINT nImageID, int nCx, COLORREF clrMask)
{
	return false;
}

bool CImgList::IsValid()
{
	return false;
}

void CImgList::Close()
{
}

bool CImgList::Draw(IMLGraphicsBase *pGraph, int nIndex, int x, int y, UINT uStyle)
{
	return false;
}

bool CImgList::Draw(CRawGraph *pGraph, int nIndex, int x, int y, UINT uStyle)
{
	return false;
}

bool CImgList::GetIconSize(int &nCx, int &nCy)
{
	return false;
}

int CImgList::GetIconCx()
{
	return 0;
}

int CImgList::GetIconCy()
{
	return 0;
}
