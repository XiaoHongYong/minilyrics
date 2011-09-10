#pragma once

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
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

protected:
	void UpdateShowTrackInfo();

	tstring GetMediaAttrInfo(MediaAttribute infoType);

	vector<MediaAttribute>	m_vInfoType;
	int						m_nSwitchTime;
	int						m_nCurrentItem;
	bool					m_bShowTitle;
	int						m_nIDTimerUpdateTrackInfo;

	// If CombineWith exists, all information will be combined with it.
	tstring					m_strCombineWith;

};
