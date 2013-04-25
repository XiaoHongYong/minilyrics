// MPSkinInfoCtrl.h: interface for the CMPSkinInfoTextCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKININFOCTRL_H__D590EF82_5CAE_4116_A45C_370C6A8FF824__INCLUDED_)
#define AFX_MPSKININFOCTRL_H__D590EF82_5CAE_4116_A45C_370C6A8FF824__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SkinScrollText.h"

class CMPSkinInfoTextCtrl : public CSkinScrollText, public IEventHandler
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinScrollText)
public:
	CMPSkinInfoTextCtrl();
	virtual ~CMPSkinInfoTextCtrl();

	void OnCreate();

	virtual void OnEvent(const IEvent *pEvent);

	bool OnMouseMove(CPoint point);

	virtual bool OnLButtonUp(UINT nFlags, CPoint point);
	virtual bool OnLButtonDown(UINT nFlags, CPoint point);

	void OnTimer(int nId);

	void Draw(CRawGraph *pGraph);

protected:
	void UpdateShowDefaultInfo();

protected:
	int			m_nTimerIDHideInfo;
	tstring		m_strCmd;
	CCursor		m_Cursor;

};

#endif // !defined(AFX_MPSKININFOCTRL_H__D590EF82_5CAE_4116_A45C_370C6A8FF824__INCLUDED_)
