// CrashHandler.h: interface for the CCrashHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CRASHRPTDLG_H__3D49A3C7_76FE_4AE9_96B6_56E9219DC44C__INCLUDED_)
#define AFX_CRASHRPTDLG_H__3D49A3C7_76FE_4AE9_96B6_56E9219DC44C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

BOOL MLCrashCallback(LPCTSTR szRpt, HMODULE hCrashMod, LPVOID lpUserData);

//////////////////////////////////////////////////////////////////////
// CCrashRptDlg
class CCrashRptDlg : public CBaseDialog
{
public:
	BOOL OnInitDialog();
	void OnOK();
	CCrashRptDlg(): CBaseDialog(IDD_CRASH_REPORT){
	}
	virtual ~CCrashRptDlg(){
	}

public:
	void UpdateListBoxHorzExtent(LPCTSTR szStrInsert);

	tstring		m_strRpt;
	tstring		m_strRptShort;

};

#endif // !defined(AFX_CRASHRPTDLG_H__3D49A3C7_76FE_4AE9_96B6_56E9219DC44C__INCLUDED_)
