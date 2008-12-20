// Register.h: interface for the CRegister class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGISTER_H__38BC637A_FC56_4522_A9DE_0D7E35492DF7__INCLUDED_)
#define AFX_REGISTER_H__38BC637A_FC56_4522_A9DE_0D7E35492DF7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//BOOL IsRegistered();
BOOL MLVerifyRegCode(const char *szName, const char *szRegCode);
BOOL MLVerifyRegCode(const WCHAR *szName, const WCHAR *szRegCode);

BOOL IsRegisterOK(LPCTSTR szName, LPCTSTR szRegCode);
BOOL IsRegisterOK();

////////////////////////////////////////////////////////////////////
// 新的注册码验证需要使用到的
extern char	g_szMLRegCode[64];
extern char	g_szMLRegName[64];
extern char	g_szMLRegNameNew[64];
extern int		g_nMLRegCodeVer;
// extern BOOL	g_bRegistered;

extern int		g_nRegYear;
extern int		g_nRegMonth;
extern int		g_nRegDay;

int CharToNumber(char ch);

// 验证部分由宏来代替

#define VERIFY_FAILED_EXIT()		\
{\
	CInvalidRegCodeDlg		dlg;\
	dlg.DoModal(&g_wndLyricShow);\
	PostQuitMessage(0);\
}
// MessageBox(NULL, "Invalid registration code, please visit http://"__DAMAIN_NAME"/ for more information!", "Minilyircs", MB_OK);

/*
// 留以扩展的校验部分9, 11, 13, 16
if ((CharToNumber(g_szMLRegCode[9]) & 0x3) != (CharToNumber(g_szMLRegNameNew[0]) & 0x3))
return FALSE;
if ((CharToNumber(g_szMLRegCode[9]) & 0x1C) != (CharToNumber(g_szMLRegNameNew[1]) & 0x1C))
return FALSE;
if ((CharToNumber(g_szMLRegCode[11]) & 0x3) != (CharToNumber(g_szMLRegCode[2]) & 0x3))
return FALSE;
if ((CharToNumber(g_szMLRegCode[11]) & 0x1C) != (CharToNumber(g_szMLRegCode[3]) & 0x1C))
return FALSE;
if ((CharToNumber(g_szMLRegCode[13]) & 0x3) != (CharToNumber(g_szMLRegCode[4]) & 0x3))
return FALSE;
if ((CharToNumber(g_szMLRegCode[13]) & 0x1C) != (CharToNumber(g_szMLRegCode[5]) & 0x1C))
return FALSE;
if ((CharToNumber(g_szMLRegCode[16]) & 0x3) != (CharToNumber(g_szMLRegCode[6]) & 0x3))
return FALSE;
if ((CharToNumber(g_szMLRegCode[16]) & 0x1C) != (CharToNumber(g_szMLRegCode[7]) & 0x1C))
return FALSE;

return TRUE;
*/
#define		REG_VERIFY_CODE_1		\
{\
	if (g_nMLRegCodeVer == 2 && g_LicenseType == LT_REGISTERED)\
{\
	if ((CharToNumber(g_szMLRegCode[9]) & 0x3) != (CharToNumber(g_szMLRegNameNew[0]) & 0x3))\
	VERIFY_FAILED_EXIT();\
}\
}

#define		REG_VERIFY_CODE_2		\
{\
if (g_nMLRegCodeVer == 2 && g_LicenseType == LT_REGISTERED)\
{\
if ((CharToNumber(g_szMLRegCode[9]) & 0x1C) != (CharToNumber(g_szMLRegNameNew[1]) & 0x1C))\
VERIFY_FAILED_EXIT();\
}\
}

// 新的注册码验证需要使用到的
////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_REGISTER_H__38BC637A_FC56_4522_A9DE_0D7E35492DF7__INCLUDED_)
