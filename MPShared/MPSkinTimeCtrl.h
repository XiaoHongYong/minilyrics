// MPSkinTimeCtrl.h: interface for the CMPSkinTimeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINTIMECTRL_H__962BB037_AA83_4BD0_B78E_41AE6BA87DE6__INCLUDED_)
#define AFX_MPSKINTIMECTRL_H__962BB037_AA83_4BD0_B78E_41AE6BA87DE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SkinPicText.h"

template<class _CSkinText>
class CMPSkinTimeCtrl : public _CSkinText  
{
	UIOBJECT_CLASS_NAME_DECLARE(_CSkinText)
public:
	CMPSkinTimeCtrl()
	{
		m_nMsgWant = UO_MSG_WANT_LBUTTON;
		m_nTimeOld = -1;
	}

	virtual ~CMPSkinTimeCtrl()
	{
		m_pSkin->UnregisterTimerObject(this);
	}

	void OnCreate()
	{
		_CSkinText::OnCreate();

		m_pSkin->RegisterTimerObject(this, 200);
	}

	bool OnLButtonUp(UINT nFlags, CPoint point)
	{
		m_bShowElapsedTime = !m_bShowElapsedTime;

		m_nTimeOld = -1;
		ResetTimeText();

		return true;
	}

	void OnTimer(int nId)
	{
		ResetTimeText();
	}

protected:
	void ResetTimeText()
	{
		int		nTime = g_Player.GetPlayPos() + 500;
		nTime /= 1000;
		if (!m_bShowElapsedTime)
			nTime = g_Player.GetMediaLength() / 1000 - nTime;

		if (nTime != m_nTimeOld)
		{
			m_nTimeOld = nTime;

			TCHAR		szTime[64];
			int			nHour, nMinute, nSec;
			tstring		strText;

			nSec = nTime % 60;
			nMinute = nTime / 60;
			nHour = nMinute / 60;
			nMinute = nMinute % 60;
			if (nHour > 0)
				_stprintf(szTime, _T("%02d:%02d:%02d"), nHour, nMinute, nSec);
			else
				_stprintf(szTime, _T("%02d:%02d"), nMinute, nSec);

			if (m_bShowElapsedTime)
				strText.resize(0);
			else
				strText = _T("-");

			strText += szTime;
			SetText(strText.c_str());

			Invalidate();
		}
	}

protected:
	bool			m_bShowElapsedTime;
	int				m_nTimeOld;

};

#endif // !defined(AFX_MPSKINTIMECTRL_H__962BB037_AA83_4BD0_B78E_41AE6BA87DE6__INCLUDED_)
