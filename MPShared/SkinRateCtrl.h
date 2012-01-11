// SkinRateCtrl.h: interface for the CSkinRateCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINRATECTRL_H__09781D48_35CB_4260_829A_3FE195B67612__INCLUDED_)
#define AFX_SKINRATECTRL_H__09781D48_35CB_4260_829A_3FE195B67612__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSkinRateCtrl : public CUIObject  
{
public:
	CSkinRateCtrl();
	virtual ~CSkinRateCtrl();

	LPCTSTR GetClassName();

	bool OnMouseMove(CPoint point);
	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnLButtonDown(UINT nFlags, CPoint point);
	void Draw(CRawGraph *pGraph);
	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	void SetRating(int nRating);
	int GetRating() { return m_nRating; }

public:
	static LPCTSTR ClassName() { return ms_szClassName; }
	BOOL IsKindOf(LPCTSTR szClassName);

protected:
	tstring				m_strBmpFile;
	CSFImage			m_img;
	int					m_nRateStarWidth;

	int					m_nRating;		// 0 ~ 5
	int					m_nRatingMax;

	BOOL				m_bLBtDown;

	static LPCTSTR		ms_szClassName;

};

#endif // !defined(AFX_SKINRATECTRL_H__09781D48_35CB_4260_829A_3FE195B67612__INCLUDED_)
