#pragma once

class CSkinAnimationUIObj : public CUIObject
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinAnimationUIObj();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	virtual void OnAnimate(CSkinAnimation *pAnimation);

	int GetDuration() const { return m_nDuration; }

protected:
	ListObjAnimation	m_listObjAnimation;
	int					m_nDuration;

};
