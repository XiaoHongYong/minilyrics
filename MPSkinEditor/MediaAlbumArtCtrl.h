// MediaAlbumArtCtrl.h: interface for the CMediaAlbumArtCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEDIAALBUMARTCTRL_H__525B5E41_E06C_4FE2_96B8_F36CA4C6C807__INCLUDED_)
#define AFX_MEDIAALBUMARTCTRL_H__525B5E41_E06C_4FE2_96B8_F36CA4C6C807__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMediaAlbumArtCtrl : public CUIObject
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CMediaAlbumArtCtrl();
	virtual ~CMediaAlbumArtCtrl();

	void OnCreate();

	virtual bool OnLButtonUp( UINT nFlags, CPoint point );

	virtual bool OnLButtonDown(UINT nFlags, CPoint point);

	void OnTimer(int nId);

	void OnSize(int cx, int cy);

	void Draw(CRawGraph *pGraph);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	void EnumProperties(CUIObjProperties &listProperties);

protected:
	void UpdateAlbumArt();

	void CreateAlbumArtImage(PRAW_IMAGE_DATA pImgSrc);

	void ResizeAlbumArt();

	bool IsMaskImageUsed() { return m_frmMask.IsValid(); }

	tstring				m_strImgFile;
	CSFImage			m_img;

	// no album art mask image
	CSFImage			m_imgOrg;	// Save original album art for resize.

	// album art frame mask image
	tstring				m_strFrmMask;
	CSFImage			m_frmMask;
	CRect				m_rcFrameMaskPos;

	MLColor				m_clrBg;
	CMLBrush			m_brBg;

};

#endif // !defined(AFX_MEDIAALBUMARTCTRL_H__525B5E41_E06C_4FE2_96B8_F36CA4C6C807__INCLUDED_)
