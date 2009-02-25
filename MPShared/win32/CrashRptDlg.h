// CrashHandler.h: interface for the CCrashHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CRASHRPTDLG_H__3D49A3C7_76FE_4AE9_96B6_56E9219DC44C__INCLUDED_)
#define AFX_CRASHRPTDLG_H__3D49A3C7_76FE_4AE9_96B6_56E9219DC44C__INCLUDED_

#pragma once

#ifdef _WIN32_DESKTOP

#include "../Utils/win32/MiniDump.h"

class CMiniDumperNotify : public IMiniDumperNotify
{
public:
	virtual bool OnBeginDump(HMODULE hCrashMod, TCHAR szDumpFileToSave[], int nLen);
	virtual bool OnDumpFinished(HMODULE hCrashMod, LPCTSTR szDumpFileToSave);

};

extern CMiniDumperNotify	dumpNotify;

inline void InitMiniDumper() { CMiniDumper::Init(&dumpNotify); }

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

	tstring		m_strDumpFile;

};

#else // _WIN32_DESKTOP

inline void InitMiniDumper() { }

#endif // _WIN32_DESKTOP

#endif // !defined(AFX_CRASHRPTDLG_H__3D49A3C7_76FE_4AE9_96B6_56E9219DC44C__INCLUDED_)
