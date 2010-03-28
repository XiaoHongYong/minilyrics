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

	virtual void OnKeyUp(UINT nChar, UINT nFlags);
	virtual void OnKeyDown(UINT nChar, UINT nFlags);

	virtual void OnSetFocus();
	virtual void OnKillFocus();

	void OnSize(int cx, int cy);

	void Draw(CRawGraph *pGraph);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	int GetStatus();
	void SetStatus(int nStatus);

protected:
	virtual void ButtonDownAction();
	virtual void ButtonUpAction();

	void FadeInDrawBt(CRawGraph *pGraph, CSFImage *pImage, int nAlpha);

	void SetMaxStat(int nMaxStat);
	void Destroy();

	bool IsMouseHitBt(POINT &pt);

protected:

	struct BtStatImg
	{
		tstring			strBkFile, strSelFile, strHoverFile, strDisabledFile;
		CSFImage		imgBk, imgSel, imgHover, imgDisabled;
		int				nIDCmd;
	};

	typedef vector<BtStatImg *>		V_BTSTATIMG;

	V_BTSTATIMG		m_vBtStatImg;

	int				m_xMargin, m_yMargin;
	int				m_xExtendStart, m_xExtendEnd;
	bool			m_bTile;

	int				m_nCurStatus;

	bool			m_bEnableHover;
	bool			m_bHover;
	bool			m_bLBtDown;

	// Will the cmd be triggered continuous?
	int				m_nTimerIdContinuous;
	bool			m_bContinuousCmd;
	bool			m_bContinuousBegin;

	bool			m_bFadein;
	DWORD			m_dwBeginFadeinTime;
	int				m_nTimerIdFadein;
	CSFImage		*m_pLastImage;

	BlendPixMode	m_bpm;

};

#endif // !defined(AFX_SKINNSTATUSBUTTON_H__DE43DEFF_B449_473B_8C7D_D648AFD8C519__INCLUDED_)
