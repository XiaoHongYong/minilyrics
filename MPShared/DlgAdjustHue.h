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

	virtual void OnCancel();

	void OnCommand(UINT uID, UINT nNotifyCode);
	virtual BOOL OnInitDialog();

	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

	void UpdateAdjustHue(float hue);

public:
	float					m_hue;//, m_saturation, m_luminance;
	float					m_hueOld;

protected:
	CWidgetTrackBar			m_tbHue;//, m_tbSaturation, m_tbLuminance;

};

#endif // !defined(AFX_DLGADJUSTHUE_H__ABD357AF_6EC1_4AD9_A302_6CAF0CA662B0__INCLUDED_)
