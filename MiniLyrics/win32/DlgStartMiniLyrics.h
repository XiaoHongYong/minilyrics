
#pragma once

#include "../widget/widget.h"
#include "WidgetListCtrlEx.h"

class CDlgStartMiniLyrics : public CBaseDialog  
{
public:
	CDlgStartMiniLyrics();
	virtual ~CDlgStartMiniLyrics();

	virtual BOOL OnInitDialog();

	void OnSize(int cx, int cy);

	void OnOK();

	enum { COL_PLAYER, COL_INFORMATION };

protected:
	CWidgetListCtrl			m_listPlayers;
	HIMAGELIST				m_hImageList;
	CWndHelper				m_wndHelper;

};
