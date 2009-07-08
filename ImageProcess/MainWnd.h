// MainWnd.h: interface for the CMainWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINWND_H__0640E15A_53EE_49C7_BEDB_C59C0F0C930D__INCLUDED_)
#define AFX_MAINWND_H__0640E15A_53EE_49C7_BEDB_C59C0F0C930D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "../ImageLib/RawImageData.h"
#include "../GfxRaw/GfxRaw.h"

class CMainWnd : public CBaseWnd  
{
public:
	CMainWnd();
	virtual ~CMainWnd();

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void OnCommand(UINT uID, UINT nNotifyCode);

	virtual void OnPaint(IMLGraphicsBase *pGraph);

	virtual void OnSize(int cx, int cy);

	virtual void OnRButtonUp(UINT nFlags, CPoint point);

	void OnOpenImage();

	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

public:
	CRawGraph		m_memGraph;
	CRawImage		m_rawImage, m_rawImage2, m_rawImage3;
	CMLBrush		br;
	CMLImage		m_img;

	CRawBmpFont		m_rawFont;
	CMLFont			m_font;

	bool			m_bTranslucencyLayered;

};

#endif // !defined(AFX_MAINWND_H__0640E15A_53EE_49C7_BEDB_C59C0F0C930D__INCLUDED_)
