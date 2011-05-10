#pragma once

class CSkinActivateCtrl : public CUIObject
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinActivateCtrl();
	~CSkinActivateCtrl();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	void Draw(CRawGraph *pGraph);

protected:

};
