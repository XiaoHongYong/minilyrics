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

	void SetPos(int nPos);
	int GetPos();

};

class CMainWnd;

class CDlgAdjustHue : public CBaseDialog  
{
public:
	CDlgAdjustHue();
	virtual ~CDlgAdjustHue();

	virtual void OnOK();

	void OnCommand(UINT uID, UINT nNotifyCode);
	virtual BOOL OnInitDialog();


	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);
	
public:
	float					m_hue, m_saturation, m_luminance;

	CRawImage				m_rawImgOrg;
	CMainWnd				*m_pMainWnd;

protected:
	CWidgetTrackBar			m_tbHue, m_tbSaturation, m_tbLuminance;

};

#endif // !defined(AFX_DLGADJUSTHUE_H__ABD357AF_6EC1_4AD9_A302_6CAF0CA662B0__INCLUDED_)
