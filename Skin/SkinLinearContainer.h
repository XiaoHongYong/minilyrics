#pragma once

class CSkinLinearContainer : public CSkinContainer  
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	CSkinLinearContainer();
	virtual ~CSkinLinearContainer();

	void RecalculateUIObjSizePos(CUIObject *pObj);

	void OnMeasureSizeByContent();
	void OnSize();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

protected:
	void OnSizeForVert();
	void OnSizeForHorz();

protected:
	bool			m_bVertical;

};
