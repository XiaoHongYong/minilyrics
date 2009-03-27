// DlgAdjustHue.cpp: implementation of the CDlgAdjustHue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "MainWnd.h"
#include "DlgAdjustHue.h"
#include "math.h"

void CWidgetTrackBar::SetRange(int nMin, int nMax)
{
	SendMessage(m_hWnd, TBM_SETRANGE, TRUE, MAKELONG(nMin, nMax));
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

void CDlgAdjustHue::OnOK()
{

	CBaseDialog::OnOK();
}

void CDlgAdjustHue::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);
}

const int	MAX_LEVEL = 100;

float		g_nLog[MAX_LEVEL];

BOOL CDlgAdjustHue::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	m_tbHue.Attach(this, IDC_HUE);
	m_tbSaturation.Attach(this, IDC_SATURATION);
	m_tbLuminance.Attach(this, IDC_LUMINANCE);

	m_tbHue.SetRange(-180, 180);
	m_tbHue.SetPos(0);
	m_tbLuminance.SetRange(-100, 100);
	m_tbLuminance.SetPos(0);
	m_tbSaturation.SetRange(-100, 100);
	m_tbLuminance.SetPos(0);

	for (int i = 0; i < MAX_LEVEL; i++)
	{
		g_nLog[i] = log((float)i);
	}


	return TRUE;
}

void AdjustImageHueTest(PRAW_IMAGE_DATA pImage, float hueOffset, float saturationRatio, float luminanceRatio);

LRESULT CDlgAdjustHue::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_hWnd);
	switch (message)
	{
	case WM_HSCROLL:
		{
			m_hue = m_tbHue.GetPos();
			if (m_hue < 0)
				m_hue += 360;

			m_saturation = m_tbSaturation.GetPos();
			m_saturation = log(m_saturation);


			if (m_saturation > 0)
				m_saturation /= 100.0;
			else
				m_saturation = -(100 + m_saturation) / 100.0;

			m_luminance = m_tbLuminance.GetPos();
			if (m_luminance > 0)
				m_luminance /= 100.0;
			else
				m_luminance = -(100 + m_luminance) / 100.0;

			m_rawImgOrg.Blt(&m_pMainWnd->m_rawImage, 0, 0, BPM_COPY);

			AdjustImageHueTest(m_pMainWnd->m_rawImage.GetHandle(), m_hue, m_saturation, m_luminance);
			m_pMainWnd->InvalidateRect();
		}
		break;
	default:
		return CBaseDialog::WndProc(message, wParam, lParam);
	}
	return 0;
}