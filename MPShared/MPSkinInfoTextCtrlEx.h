
#pragma once

#include "LyricShowObj.h"

class CMPSkinInfoTextCtrlEx : public CLyricShowObj
{
	UIOBJECT_CLASS_NAME_DECLARE(CLyricShowObj)
public:
	CMPSkinInfoTextCtrlEx();
	virtual ~CMPSkinInfoTextCtrlEx();

	void OnCreate();

	void Draw(CRawGraph *pGraph);

	void OnEvent(const IEvent *pEvent);

	void OnLyrDrawContextChanged();

	void OnPlayTimeChangedUpdate();

	void OnSize();

	void ReloadThemeOfSkin(bool bSavetoProfile) { }

protected:
	void AppendInfoText(const tstring &str);

	void WrapInfoText();

protected:
	CMLyrLines		m_vText;
	CMLyrLines		m_vUnWrapedText;

};

