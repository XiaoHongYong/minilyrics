#pragma once

class CDlgChooseColor
{
public:
	CDlgChooseColor();
	virtual ~CDlgChooseColor();

	int DoModal(CBaseWnd *pWndParent, const MLColor &clr);

	MLColor &GetColor() { return m_clr; }

public:
	MLColor			m_clr;

};
