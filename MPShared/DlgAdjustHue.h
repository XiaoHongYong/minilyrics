// DlgAdjustHue.h: interface for the CDlgAdjustHue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLGADJUSTHUE_H__ABD357AF_6EC1_4AD9_A302_6CAF0CA662B0__INCLUDED_)
#define AFX_DLGADJUSTHUE_H__ABD357AF_6EC1_4AD9_A302_6CAF0CA662B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CWidgetTrackBar : public CWidgetCtrlBase
{

public:

	void SetRange(int nMin, int nMax);
	void SetLineSize(int nLine);
	void SetPageSize(int nPage);
	void SetTickFreq(int nFreq);

	void SetPos(int nPos);
	int GetPos();
	void SetTick();

};

class CDlgAdjustHue : public CBaseDialog  
{
public:
	CDlgAdjustHue();
	virtual ~CDlgAdjustHue();

	int DoModal(CBaseWnd *pWndParent, EventType eventType, LPCTSTR szSectName)
	{
		m_bAdjustSkinHue = false;
		m_eventType = eventType;
		m_strSectName = szSectName;
		return CBaseDialog::DoModal(pWndParent);
	}

	int DoModal(CBaseWnd *pWndParent)
	{
		m_bAdjustSkinHue = true;
		return CBaseDialog::DoModal(pWndParent);
	}

	virtual void OnCancel();

	void OnCommand(UINT uID, UINT nNotifyCode);
	virtual BOOL OnInitDialog();

	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

	void UpdateAdjustHue(float hue);

public:
	float					m_hue;//, m_saturation, m_luminance;
	float					m_hueOld;

protected:
	bool					m_bAdjustSkinHue;	// false: adjust lyrics hue

	// used to adjust lyrics hue
	EventType				m_eventType;
	tstring					m_strSectName;

	CWidgetTrackBar			m_tbHue;//, m_tbSaturation, m_tbLuminance;

};

#endif // !defined(AFX_DLGADJUSTHUE_H__ABD357AF_6EC1_4AD9_A302_6CAF0CA662B0__INCLUDED_)
