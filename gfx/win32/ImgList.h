// ImgList.h: interface for the CImgList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMGLIST_H__E3016B0E_C09F_4EB7_9928_FE94DB1CB54E__INCLUDED_)
#define AFX_IMGLIST_H__E3016B0E_C09F_4EB7_9928_FE94DB1CB54E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CImgList  
{
public:
	CImgList();
	virtual ~CImgList();

	bool LoadBitmap(CSkinFactory *pSkinFactory, LPCTSTR szImage, int nCx, COLORREF clrMask = CLR_DEFAULT);

	bool LoadBitmap(UINT nImageID, int nCx, COLORREF clrMask = CLR_DEFAULT);

	bool IsValid() { return m_imageList != NULL; }
	void Close();

	bool Draw(CRawGraph *pGraph, int nIndex, int x, int y, UINT uStyle = ILD_TRANSPARENT);
	bool Draw(IMLGraphicsBase *pGraph, int nIndex, int x, int y, UINT uStyle = ILD_TRANSPARENT);

	HIMAGELIST GetHandle() { return m_imageList; }

	bool GetIconSize(int &nCx, int &nCy);

	int GetIconCx();
	int GetIconCy();

protected:
	HIMAGELIST		m_imageList;

};

#endif // !defined(AFX_IMGLIST_H__E3016B0E_C09F_4EB7_9928_FE94DB1CB54E__INCLUDED_)
