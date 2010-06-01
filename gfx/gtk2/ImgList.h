// ImgList.h: interface for the CImgList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMGLIST_H__E3016B0E_C09F_4EB7_9928_FE94DB1CB54E__INCLUDED_)
#define AFX_IMGLIST_H__E3016B0E_C09F_4EB7_9928_FE94DB1CB54E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifndef CLR_NONE
#define CLR_NONE                0xFFFFFFFFL
#define CLR_DEFAULT             0xFF000000L
#endif

#ifndef ILD_NORMAL
#define ILD_NORMAL              0x0000
#define ILD_TRANSPARENT         0x0001
#define ILD_MASK                0x0010
#define ILD_IMAGE               0x0020
#endif

class CImgList  
{
public:
	CImgList();
	virtual ~CImgList();

	bool LoadBitmap(CSkinFactory *pSkinFactory, LPCTSTR szImage, int nCx, COLORREF clrMask = CLR_DEFAULT);

	bool LoadBitmap(UINT nImageID, int nCx, COLORREF clrMask = CLR_DEFAULT);

	bool IsValid();
	void Close();

	bool Draw(IMLGraphicsBase *pGraph, int nIndex, int x, int y, UINT uStyle = ILD_TRANSPARENT);
	bool Draw(CRawGraph *pGraph, int nIndex, int x, int y, UINT uStyle = ILD_TRANSPARENT);

	bool GetIconSize(int &nCx, int &nCy);

	int GetIconCx();
	int GetIconCy();

protected:

};

#endif // !defined(AFX_IMGLIST_H__E3016B0E_C09F_4EB7_9928_FE94DB1CB54E__INCLUDED_)
