// SkinNStatusButton.h: interface for the CSkinNStatusButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINNSTATUSBUTTON_H__DE43DEFF_B449_473B_8C7D_D648AFD8C519__INCLUDED_)
#define AFX_SKINNSTATUSBUTTON_H__DE43DEFF_B449_473B_8C7D_D648AFD8C519__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UIObject.h"

class CSkinNStatusButton : public CUIObject  
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinNStatusButton();
	virtual ~CSkinNStatusButton();

	void OnTimer(int nId);

	bool OnMouseMove(UINT nFlags, CPoint point);
	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnLButtonDown(UINT nFlags, CPoint point);
	void Draw(CRawGraph *pGraph);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	void EnumProperties(CUIObjProperties &listProperties);

	int GetStatus();
	void SetStatus(int nStatus);

protected:
	void SetMaxStat(int nMaxStat);
	void Destroy();

protected:

	struct BtStatImg
	{
		tstring			strBkFile, strSelFile, strHoverFile, strDisabledFile;
		CSFImage		imgBk, imgSel, imgHover, imgDisabled;
		int				nIDCmd;
	};

	typedef vector<BtStatImg *>		V_BTSTATIMG;

	V_BTSTATIMG		m_vBtStatImg;

	int				m_nCurStatus;

	bool			m_bEnableHover;
	bool			m_bHover;
	bool			m_bLBtDown;

	// Will the cmd be triggered continuous?
	bool			m_bContinuousCmd;
	bool			m_bContinuousBegin;

	BlendPixMode	m_bpm;

};

#endif // !defined(AFX_SKINNSTATUSBUTTON_H__DE43DEFF_B449_473B_8C7D_D648AFD8C519__INCLUDED_)
