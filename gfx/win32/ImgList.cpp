// ImgList.cpp: implementation of the CImgList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../MLSkin.h"
#include "ImgList.h"

#ifndef ILC_COLOR32
#define ILC_COLOR32		0
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImgList::CImgList()
{
	m_imageList = NULL;
}

CImgList::~CImgList()
{
	Close();
}

bool CImgList::LoadBitmap(CSkinFactory *pSkinFactory, LPCTSTR szImage, int nCx, COLORREF clrMask)
{
	ASSERT(!m_imageList);
	Close();

	if (IsEmptyString(szImage))
		return false;

	//CSFImage	image;
	TCHAR		szImageFile[MAX_PATH];
	HBITMAP		hBmp;
	BITMAP		bitmap;

	if (!pSkinFactory->GetResourceMgr()->GetResourcePathName(szImageFile, CountOf(szImageFile), szImage))
		return false;
	hBmp = LoadBitmapFromFile(szImageFile);
	if (!hBmp)
		return false;
	GetObject(hBmp, sizeof(bitmap), &bitmap);
	//if (!image.LoadFromSRM(pSkinFactory, szImage))
	//	return false;

	m_imageList = ImageList_Create(nCx, bitmap.bmHeight, ILC_MASK | ILC_COLOR32, 2, 1);
	if (!m_imageList)
		return false;

	//ImageList_AddMasked(m_imageList, image.GetHandle(), clrMask);
	ImageList_AddMasked(m_imageList, hBmp, clrMask);

	// m_imageList = ImageList_LoadImage(NULL, szImageFile, nCx, 1, clrMask, IMAGE_BITMAP, LR_LOADFROMFILE | LR_SHARED | LR_COPYRETURNORG);
	return m_imageList != NULL;
}

bool CImgList::LoadBitmap(UINT nImageID, int nCx, COLORREF clrMask)
{
	ASSERT(!m_imageList);
	Close();

	m_imageList = ImageList_LoadImage(GetAppInstance(), MAKEINTRESOURCE(nImageID), nCx, 1, clrMask, IMAGE_BITMAP, 0);
	return m_imageList != NULL;
}

void CImgList::Close()
{
	if (m_imageList)
	{
		ImageList_Destroy(m_imageList);
		m_imageList = NULL;
	}
}

bool CImgList::Draw(CRawGraph *pGraph, int nIndex, int x, int y, UINT uStyle)
{
	return ImageList_Draw(m_imageList, nIndex, pGraph->GetHandle(), x, y, uStyle) != 0;
}

bool CImgList::Draw(IMLGraphicsBase *pGraph, int nIndex, int x, int y, UINT uStyle)
{
	return ImageList_Draw(m_imageList, nIndex, pGraph->GetHandle(), x, y, uStyle) != 0;
}

bool CImgList::GetIconSize(int &nCx, int &nCy)
{
	return Tobool(ImageList_GetIconSize(m_imageList, &nCx, &nCy));
}

int CImgList::GetIconCx()
{
	int		cx, cy;
	if (ImageList_GetIconSize(m_imageList, &cx, &cy))
		return cx;
	else
		return 0;
}

int CImgList::GetIconCy()
{
	int		cx, cy;
	if (ImageList_GetIconSize(m_imageList, &cx, &cy))
		return cy;
	else
		return 0;
}
