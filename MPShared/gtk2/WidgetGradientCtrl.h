#pragma once

class CWidgetGradientCtrl : public CWidgetCtrlBase
{
public:
	enum	{ COUNT = 3 };

	CWidgetGradientCtrl();
	~CWidgetGradientCtrl();

	void SetColor(MLColor clr[COUNT]);
	void GetColor(MLColor clr[COUNT]);

protected:
	CRawPen			m_pen;
	MLColor			m_vColors[COUNT];

};
