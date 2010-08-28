// DlgAdjustHue.cpp: implementation of the CDlgAdjustHue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgAdjustHue.h"
#include "SkinSeekCtrl.h"


class CPageAdjustHue : public CSkinPageHandler, public IScrollNotify
{
public:
	CPageAdjustHue() : CSkinPageHandler(_T("CID_C_ADJUST_HUE"), 
		NULL, NULL, NULL, NULL)
	{
	}

	~CPageAdjustHue()
	{
	}

	// IScrollNotify
	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
	{
		UpdateAdjustHue((float)nPos);
	}

	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
	{
		UpdateAdjustHue((float)nPos);
	}

	void UpdateAdjustHue(float hue)
	{
		g_Profile.WriteProfileInt(g_mpApp.GetSkinFactory()->GetSkinName(), _T("Hue"), (int)hue);
		g_mpApp.GetSkinFactory()->AdjustHue(hue);
	}

	void OnCreate()
	{
		CSkinPageHandler::OnCreate();

		m_hueOld = (float)(g_Profile.GetProfileInt(g_mpApp.GetSkinFactory()->GetSkinName(), _T("Hue"), 0));

		CSkinSeekCtrl *pSeek = (CSkinSeekCtrl*)GetUIObjectById(_T("CID_HUE"), CSkinSeekCtrl::ClassName());
		if (pSeek)
		{
			pSeek->SetScrollInfo(0, 360, 30, (int)m_hueOld, 1, false);
		}
	}

	void OnCancel()
	{
		UpdateAdjustHue(m_hueOld);
	}

protected:
	float					m_hueOld;

};

//////////////////////////////////////////////////////////////////////

CDlgAdjustHue::CDlgAdjustHue() : CMPSkinDialog(_T("AdjustHue.xml"))
{
	m_pageHandler.AddPage(new CPageAdjustHue());

	// m_pageHandler.SetDefaultPage(_T("CID_C_ADJUST_HUE"));
}

CDlgAdjustHue::~CDlgAdjustHue()
{

}

