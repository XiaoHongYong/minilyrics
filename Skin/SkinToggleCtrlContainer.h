#pragma once

#include "SkinContainer.h"

//
// This container will toggle sub UIObject with various effect, such as Fade in/Out
//
class CSkinToggleCtrlContainer : public CSkinContainer
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	CSkinToggleCtrlContainer(void);
	~CSkinToggleCtrlContainer(void);

	enum TimerID
	{
		T_ANIMATION	= 1
	};

	CRawGraph *GetMemGraph();

	void InvalidateUIObject(CUIObject *pObj);

	void UpdateMemGraphicsToScreen(const RECT* lpRect, CUIObject *pObjCallUpdate);

	void Draw(CRawGraph *pGraph);

	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	virtual void OnCreate();

	virtual void OnTimer(int nId);

protected:
	int					m_nActiveCtrl, m_nToggleToCtrl;
	int					m_nIDTimerAnimation;
	uint32				m_nTimeBeginAni;
	int					m_nAnimateDuration;

	CRawGraph			m_memGraph;

};
