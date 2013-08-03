// MPSkinTimeCtrl.h: interface for the CMPSkinTimeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINTIMECTRL_H__962BB037_AA83_4BD0_B78E_41AE6BA87DE6__INCLUDED_)
#define AFX_MPSKINTIMECTRL_H__962BB037_AA83_4BD0_B78E_41AE6BA87DE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SkinPicText.h"

tstring FormatTime(int nTimeSec);

template<class _CSkinText>
class CMPSkinTimeCtrl : public _CSkinText  
{
	UIOBJECT_CLASS_NAME_DECLARE(_CSkinText)
public:
	CMPSkinTimeCtrl()
	{
		_CSkinText::m_nMsgWant = UO_MSG_WANT_LBUTTON;
		m_nTimeOld = -1;
		m_bShowElapsedTime = true;
	}

	virtual ~CMPSkinTimeCtrl()
	{
		_CSkinText::m_pSkin->UnregisterTimerObject(this);
	}

	void OnCreate()
	{
		_CSkinText::OnCreate();

		_CSkinText::m_pSkin->RegisterTimerObject(this, 200);
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

			tstring		strText;

			if (!m_bShowElapsedTime)
				strText = _T("-");

			strText += FormatTime(nTime);
			_CSkinText::SetText(strText.c_str());

			_CSkinText::Invalidate();
		}
	}

protected:
	bool			m_bShowElapsedTime;
	int				m_nTimeOld;

};

#endif // !defined(AFX_MPSKINTIMECTRL_H__962BB037_AA83_4BD0_B78E_41AE6BA87DE6__INCLUDED_)
