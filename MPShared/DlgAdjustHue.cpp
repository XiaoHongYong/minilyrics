// DlgAdjustHue.cpp: implementation of the CDlgAdjustHue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgAdjustHue.h"


class CPageAdjustHue : public CSkinContainer, public IScrollNotify
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
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
		g_Profile.WriteInt(CMPlayerAppBase::GetMPSkinFactory()->GetSkinName(), _T("Hue"), (int)hue);
		CMPlayerAppBase::GetMPSkinFactory()->AdjustHue(hue);
	}

	void OnCreate()
	{
		CSkinContainer::OnCreate();

		m_hueOld = (float)(g_Profile.GetInt(CMPlayerAppBase::GetMPSkinFactory()->GetSkinName(), _T("Hue"), 0));

		CSkinSeekCtrl *pSeek = (CSkinSeekCtrl*)GetUIObjectById(_T("CID_HUE"), CSkinSeekCtrl::ClassName());
		if (pSeek)
		{
			pSeek->SetScrollInfo(0, 360, 30, (int)m_hueOld, 1, false);
			pSeek->SetScrollNotify(this);
		}
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == GetIDByName(_T("CID_RESET")))
		{
			UpdateAdjustHue(0.0);
			CSkinSeekCtrl *pSeek = (CSkinSeekCtrl*)GetUIObjectById(GetIDByName(_T("CID_HUE")), CSkinSeekCtrl::ClassName());
			if (pSeek)
				pSeek->SetScrollPos(0);
			return true;
		}

		return CSkinContainer::OnCustomCommand(nId);
	}

	bool OnCancel()
	{
		UpdateAdjustHue(m_hueOld);
		return CSkinContainer::OnCancel();
	}

protected:
	float					m_hueOld;

};

UIOBJECT_CLASS_NAME_IMP(CPageAdjustHue, _T("Container.AdjustHue"))

//////////////////////////////////////////////////////////////////////

void ShowAdjustHueDialog(CSkinWnd *pParent)
{
	CSkinApp::GetInstance()->ShowDialog(pParent, _T("AdjustHue.xml"));
}

void RegisterAdjustHuePage(CSkinFactory *pSkinFactory)
{
	AddUIObjNewer2(pSkinFactory, CPageAdjustHue);
}

