// MediaInfoTextCtrl.h: interface for the CMediaInfoTextCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEDIAINFOTEXTCTRL_H__791B2BDE_18D2_4CFE_A932_3B3D85AF3037__INCLUDED_)
#define AFX_MEDIAINFOTEXTCTRL_H__791B2BDE_18D2_4CFE_A932_3B3D85AF3037__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SkinScrollText.h"

class CMediaInfoTextCtrl : public CSkinScrollText, public IEventHandler
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinScrollText)
public:
	CMediaInfoTextCtrl();
	virtual ~CMediaInfoTextCtrl();

	void OnCreate();

	virtual void OnEvent(const IEvent *pEvent);

	void OnTimer(int nId);

	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	void EnumProperties(CUIObjProperties &listProperties);

protected:
	void UpdateShowTrackInfo();

	vector<MediaAttribute>	m_vInfoType;
	int						m_nSwitchTime;
	int						m_nCurrentItem;
	bool					m_bShowTitle;
	int						m_nIDTimerUpdateTrackInfo;

};

#endif // !defined(AFX_MEDIAINFOTEXTCTRL_H__791B2BDE_18D2_4CFE_A932_3B3D85AF3037__INCLUDED_)
