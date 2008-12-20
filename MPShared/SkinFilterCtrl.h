#ifndef _SKINFILTER_CTRL_H_
#define _SKINFILTER_CTRL_H_

#pragma once

class CSkinFilterCtrl : public CUIObject
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinFilterCtrl();
	~CSkinFilterCtrl();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	void EnumProperties(CUIObjProperties &listProperties);

	void Draw(CRawGraph *pGraph);

protected:
	float m_fStartPercent;

};


#endif // _SKINFILTER_CTRL_H_