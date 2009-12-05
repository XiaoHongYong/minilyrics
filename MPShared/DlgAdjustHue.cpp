// DlgAdjustHue.cpp: implementation of the CDlgAdjustHue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgAdjustHue.h"

void CWidgetTrackBar::SetRange(int nMin, int nMax)
{
	SendMessage(m_hWnd, TBM_SETRANGE, TRUE, MAKELONG(nMin, nMax));
}

void CWidgetTrackBar::SetLineSize(int nLine)
{
	SendMessage(m_hWnd, TBM_SETLINESIZE, 0, nLine);
}

void CWidgetTrackBar::SetPageSize(int nPage)
{
	SendMessage(m_hWnd, TBM_SETPAGESIZE, 0, nPage);
}

void CWidgetTrackBar::SetTickFreq(int nFreq)
{
	SendMessage(m_hWnd, TBM_SETTICFREQ, nFreq, 0);
}

void CWidgetTrackBar::SetPos(int nPos)
{
	SendMessage(m_hWnd, TBM_SETPOS, TRUE, nPos);
}

int CWidgetTrackBar::GetPos()
{
	return SendMessage(m_hWnd, TBM_GETPOS, 0, 0);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgAdjustHue::CDlgAdjustHue() : CBaseDialog(IDD_ADJUST_HUE)
{
}

CDlgAdjustHue::~CDlgAdjustHue()
{

}

void CDlgAdjustHue::OnCancel()
{
// 	m_hue = m_tbHue.GetPos();
// 	if (m_hue < 0)
// 		m_hue += 360;
// 
// 	m_saturation = m_tbSaturation.GetPos();
// 	if (m_saturation > 0)
// 		m_saturation /= 100.0;
// 	else
// 		m_saturation = -(100 + m_saturation) / 100.0;
// 
// 	m_luminance = m_tbLuminance.GetPos();
// 	if (m_luminance > 0)
// 		m_luminance /= 100.0;
// 	else
// 		m_luminance = -(100 + m_luminance) / 100.0;
	UpdateAdjustHue(m_hueOld);

	CBaseDialog::OnCancel();
}

void CDlgAdjustHue::OnCommand(UINT uID, UINT nNotifyCode)
{
	if (uID == IDC_RESET)
	{
		m_tbHue.SetPos(0);
		UpdateAdjustHue(0.0);
	}
	CBaseDialog::OnCommand(uID, nNotifyCode);
}

BOOL CDlgAdjustHue::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	m_tbHue.Attach(this, IDC_HUE);
	m_tbHue.SetRange(0, 360);

	COLORREF	clr = RGB(255, 254, 253);
	AdjustColorHue(clr, 1);

	m_tbHue.SetLineSize(10);
	m_tbHue.SetPageSize(40);
	m_tbHue.SetTickFreq(0);

	m_hueOld = (float)(g_Profile.GetProfileInt(g_mpApp.GetSkinFactory()->GetSkinName(), _T("Hue"), 0));
	m_tbHue.SetPos((int)m_hueOld);

// 	m_tbSaturation.Attach(this, IDC_SATURATION);
// 	m_tbLuminance.Attach(this, IDC_LUMINANCE);
// 	m_tbLuminance.SetRange(-100, 100);
// 	m_tbLuminance.SetPos(0);
// 	m_tbSaturation.SetRange(-100, 100);
// 	m_tbLuminance.SetPos(0);

	return TRUE;
}

LRESULT CDlgAdjustHue::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_hWnd);
	switch (message)
	{
	case WM_HSCROLL:
		{
			// CSkinVScrollBarOSStyle
			m_hue = (float)m_tbHue.GetPos();
			UpdateAdjustHue(m_hue);
		}
		break;
	default:
		return CBaseDialog::WndProc(message, wParam, lParam);
	}
	return 0;
}

void CDlgAdjustHue::UpdateAdjustHue(float hue)
{
	// SetDlgItemInt(IDC_E_HUE, (int)m_hue, false);

	g_Profile.WriteProfileInt(g_mpApp.GetSkinFactory()->GetSkinName(), _T("Hue"), (int)hue);
	g_mpApp.GetSkinFactory()->AdjustHue(hue);
}
