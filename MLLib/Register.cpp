// Register.cpp: implementation of the CRegister class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Register.h"

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
	if (g_nMLRegCodeVer == 2 && g_LicenseStatus == LS_ACTIVE)\
{\
	if ((CharToNumber(g_szMLRegCode[9]) & 0x3) != (CharToNumber(g_szMLRegNameNew[0]) & 0x3))\
	VERIFY_FAILED_EXIT();\
}\
}

#define		REG_VERIFY_CODE_2		\
{\
if (g_nMLRegCodeVer == 2 && g_LicenseStatus == LS_ACTIVE)\
{\
if ((CharToNumber(g_szMLRegCode[9]) & 0x1C) != (CharToNumber(g_szMLRegNameNew[1]) & 0x1C))\
VERIFY_FAILED_EXIT();\
}\
}

int CharToNumber(char ch);

///////////////////////////////////////////////////////////////
// 注册部分代码

BYTE AvgValue(const char *szString, int nLen)
{
	int		nValue = 0;

	for (int i = 0; i < nLen; i++)
		nValue += szString[i];
	return (BYTE)(nValue / nLen);
}

void ByteToHexStr(char *szStr, BYTE byValue)
{
	char	szTemp[10];

	sprintf_s(szTemp, CountOf(szTemp), "%02X", byValue);

	szStr[0] = szTemp[0];
	szStr[1] = szTemp[1];
}

static const char N2ChTable[256] = 
{
	'L', 'G', 'T', 'G', '1', 'B', 'O', 'S', '4', 'J', '8', '8', 'H', 'C', 'O', 
	'0', '6', 'Q', 'D', 'F', 'L', 'S', 'J', 'X', 'W', 'K', 'E', '1', '9', 'C', 
	'0', 'D', 'Q', 'V', 'H', 'S', 'I', 'W', '6', 'K', 'U', '7', 'I', 'G', '3', 
	'J', 'I', 'Q', 'X', 'S', 'D', 'H', 'D', 'N', 'M', 'F', 'P', 'O', 'U', 'X', 
	'S', 'C', '1', 'Q', '0', 'Q', 'G', 'G', 'P', 'G', 'W', '5', 'G', '4', 'L', 
	'R', 'H', 'M', 'C', 'P', '7', 'R', '2', 'V', 'M', '8', '7', 'L', 'W', 'L', 
	'F', 'U', '8', '4', 'K', '5', 'E', '9', '4', 'H', 'H', '0', 'K', '8', '3', 
	'1', '5', 'K', 'E', '5', 'A', 'L', '0', 'J', 'A', 'M', 'T', 'D', '6', '1', 
	'O', 'H', 'O', 'K', '0', '7', 'E', 'U', 'L', 'W', '8', '4', 'G', '5', 'H', 
	'B', 'K', 'B', '6', 'R', 'O', 'T', 'P', 'H', 'T', 'N', 'E', 'L', 'A', '8', 
	'2', 'Q', 'R', 'P', 'U', 'B', '5', 'T', 'F', 'U', 'O', 'C', 'O', 'L', 'F', 
	'1', '6', 'I', 'E', 'A', 'O', 'E', '8', 'J', 'O', 'T', 'K', 'A', '2', 'Q', 
	'3', 'G', '3', 'K', 'N', 'A', '4', 'X', 'E', 'X', '1', 'G', 'Q', 'I', 'F', 
	'4', '1', 'K', '1', '0', '6', 'N', '5', 'A', 'D', 'B', 'L', 'V', 'M', '0', 
	'G', 'L', 'K', '8', '5', '6', '1', '2', 'V', 'D', 'A', 'I', '8', 'I', 'C', 
	'T', '9', 'B', 'K', 'A', '7', 'J', 'K', 'H', 'U', 'V', 'Q', 'A', 'E', '0', 
	'3', 'P', 'E', '9', 'S', '1', 'C', 'A', '2', 'K', 'D', 'I', 'R', '9', '0', '0'
};

/*
#define VERSION			1
BOOL GetRegCode(const char *szName, const char *szDate, char *szRegCode)
{
	int			nMonth, nDay, nYear;
	WORD		wAllDays;		// 从2000－01－01开始的天，包括了该天

	char		szCodeTemp[256];
	char		szNameNew[256];
	int			i;
	BYTE		byVersion;
	BYTE		byVerRand;

	if (strlen(szName) < 3)
		return FALSE;

	strcpy(szNameNew, szName);
	strupr(szNameNew);

	srand(GetTickCount());
	memset(szCodeTemp, 0, sizeof(szCodeTemp));

	if (!GetDate(nMonth, nDay, nYear, szDate))
		return FALSE;

	wAllDays = (WORD)GetDaysFrom2000(nMonth, nDay, nYear);

	// 计算版本号: 0 - 1
	byVerRand = rand() % ('Z' - 'A' + '9' - '0' + 2);
	byVersion = VERSION + byVerRand + (AvgValue(szNameNew, strlen(szNameNew)) + szNameNew[0]) % (0x7F - ('Z' - 'A' + '9' - '0' + 2));

	// 版本号随机数: 15-15
	if (byVerRand <= 9)
		szCodeTemp[15] = '0' + byVerRand;
	else
		szCodeTemp[15] = 'A' + byVerRand - 10;
	ByteToHexStr(szCodeTemp, byVersion);

	// 计算随机数: 6 - 9
	for (i = 6; i <= 9; i++)
		szCodeTemp[i] = N2ChTable[rand() % 256];
//		szCodeTemp[i] = N2ChTable[i][rand()];

	// 计算日期部分: 2 - 5
	ByteToHexStr(&(szCodeTemp[2]), (BYTE)(AvgValue(&(szCodeTemp[6]), 4) / 3 + szCodeTemp[6] / 2 + HIWORD(wAllDays) / 2));
	ByteToHexStr(&(szCodeTemp[4]), (BYTE)(AvgValue(&(szCodeTemp[6]), 4) / 3 + szCodeTemp[7] / 2 + LOWORD(wAllDays) / 2));

	// 计算校验码部分: 10 - 14
	strcat(szNameNew, &szCodeTemp[6]);

	for (i = 10; i <= 14; i++)
	{
		szCodeTemp[i] = N2ChTable[(BYTE)(AvgValue(&(szCodeTemp[2]), 8) / 3 + szNameNew[i - 10] / 3 + szCodeTemp[6] / 3)];
//		szCodeTemp[i] = N2ChTable[i][szNameNew[i - 10] / 2 + szCodeTemp[6] / 2];
	}

	strcpy(szRegCode, szCodeTemp);

	return TRUE;
}*/

UINT String2HexUINT(char buff[])
{
	int len = strlen(buff);
	int i;
	UINT	re;
	re = 0;
	for (i = 0; i < len; i++)
	{
		if ('0' <= buff[i] && buff[i] <= '9')
			re = re * 16 + buff[i] - '0';
		else if ('a' <= buff[i] && buff[i] <= 'f')
			re = re * 16 + buff[i] - 'a' + 10;
		else if ('A' <= buff[i] && buff[i] <= 'F')
			re = re * 16 + buff[i] - 'A' + 10;
	}
	return re;
}


static int HexStrToByte(const char *str)
{
	return HexToInt(str[0]) * 16 + HexToInt(str[1]);
}

bool VerifyRegCode1(const char *szName, const char *szRegCode)
{
	char		szCodeTemp[256];
	char		szNameNew[256];
	char		szVersion[10];
	int			i;
	bool		bRet = TRUE;;
	BYTE		byVersion;
	BYTE		byVerRand;

	if (strlen(szName) < 3)
		return FALSE;

	strcpy_safe(szNameNew, CountOf(szNameNew), szName);
	_strupr(szNameNew);

	memset(szCodeTemp, 0, sizeof(szCodeTemp));
	strcpy_safe(szCodeTemp, CountOf(szCodeTemp), szRegCode);

	strncpy_safe(szVersion, CountOf(szVersion), szCodeTemp, 2);
	szVersion[2] = '\0';

	// 计算版本号: 0 - 1
	if (szCodeTemp[15] >= 'A')
		byVerRand = szCodeTemp[15] - 'A' + 10;
	else
		byVerRand = szCodeTemp[15] - '0';

	byVersion = (BYTE)String2HexUINT(szVersion) - byVerRand - (AvgValue(szNameNew, strlen(szNameNew)) + szNameNew[0]) % (0x7F - ('Z' - 'A' + '9' - '0' + 2));

	//static BOOL VerifyDate(const char *szRegCode)
	{
		int			n = HexStrToByte(szRegCode + 2);
		int			x = AvgValue(&(szRegCode[6]), 4) / 3;

		int			k = (BYTE)(x + szRegCode[6] / 2);

		k += szCodeTemp[0] + szCodeTemp[10];
		k -= szCodeTemp[2] + szCodeTemp[3];
		k += szCodeTemp[4] + szCodeTemp[7];
		int			m = 0;

		m = szCodeTemp[8];
		m += k;

		m -= szCodeTemp[0] + szCodeTemp[7];
		k = m;
		k += szCodeTemp[2];
		k -= szCodeTemp[10] + szCodeTemp[4];
		k += szCodeTemp[3];
		k -= szCodeTemp[8];

		szCodeTemp[0] += k - n;
		szCodeTemp[10] += k - n;
		szCodeTemp[6] += k - n;
		szCodeTemp[7] |= k - n;
		szCodeTemp[14] += k - n;
	}

	// 校验码部分: 10 - 15
	strcat_safe(szNameNew, CountOf(szNameNew), &szCodeTemp[6]);

	for (i = 10; i <= 14; i++)
	{
		if (CharToNumber(szCodeTemp[i]) != CharToNumber(N2ChTable[(BYTE)(AvgValue(&(szCodeTemp[2]), 8) / 3 + szNameNew[i - 10] / 3 + szCodeTemp[6] / 3)]))
			bRet = FALSE;
		// szCodeTemp[i] = N2ChTable[i][szNameNew[i - 10] / 2 + szCodeTemp[6] / 2];
	}

	return bRet;
}

///////////////////////////////////////////////////////////////////////////
// 迷你歌词新的注册码验证

char	g_szMLRegCode[64];
char	g_szMLRegName[64];
char	g_szMLRegNameNew[64];
int		g_nMLRegCodeVer = 1;
// BOOL	g_bRegistered = FALSE;

int		g_nRegYear = 02;
int		g_nRegMonth = 12;
int		g_nRegDay = 12;

///////////////////////////////////////////////////////////////
// 公共定义部分
#define ML_REGCODE_GEN_VERSION2			2
#define MAX_REGCODE_CHARS	('Z' - 'A' + '9' - '0')
static char _arrNumbChar[] = "123456789ABCDEFGHIJKLMNPQRSTUVWXYZ";

int CharToNumber(char ch)
{
	const char *szPos = strchr(_arrNumbChar, ch);
	if (szPos == NULL)
		return 10;// 'A'
	return int(szPos - _arrNumbChar);
}

BYTE AvgValue(const char *szString, size_t nLen)
{
	int		nValue = 0;

	for (UINT i = 0; i < nLen; i++)
		nValue += szString[i];
	return (BYTE)(nValue / nLen);
}

///////////////////////////////////////////////////////////////
// 注册码验证部分

int RandCodeToNumber(char chRand, char chNumber)
{
	int nRand;
	int nNumber;

	nRand = CharToNumber(chRand);
	nNumber = MAX_REGCODE_CHARS - nRand - CharToNumber(chNumber);
	assert(nNumber >= 0 && nNumber < MAX_REGCODE_CHARS);

	return nNumber;
}

#define GetRegCodeVersion(szRegCode)	RandCodeToNumber(szRegCode[0], szRegCode[1])


///////////////////////////////////////////////////////////////////////////
//
// 验证部分由宏来代替
/*
#define VERIFY_FAILED_EXIT()		\
{\
	CInvalidRegCodeDlg		dlg;\
	dlg.DoModal(&g_wndLyricShow);\
	PostQuitMessage(0);\
}
// MessageBox(NULL, "Invalid registration code, please visit http://www.viewlyrics.com/ for more information!", "Minilyircs", MB_OK);


// 留以扩展的校验部分9, 11, 13, 16
if (CharToNumber(g_szMLRegCode[14]) != (CharToNumber(g_szMLRegNameNew[0]) + CharToNumber(g_szMLRegCode[16]) + CharToNumber(g_szMLRegNameNew[3]) + CharToNumber(g_szMLRegCode[3]) + CharToNumber(AvgValue(g_szMLRegNameNew, strlen(g_szMLRegNameNew)))) / 5)
return FALSE;
if ((CharToNumber(g_szMLRegCode[9]) & 0x3) != (CharToNumber(g_szMLRegNameNew[0]) & 0x3))
return FALSE;
if ((CharToNumber(g_szMLRegCode[9]) & 0x1C) != (CharToNumber(g_szMLRegNameNew[1]) & 0x1C))
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
/*#define		REG_VERIFY_CODE_1()		\
{\
	if (g_nMLRegCodeVer == 2 && g_MiniLyricsMain.m_bRegistered && g_MLRegUseTime.wDayOfWeek == 2)\
{\
	if ((CharToNumber(g_szMLRegCode[9]) & 0x3) != (CharToNumber(g_szMLRegNameNew[0]) & 0x3))\
	VERIFY_FAILED_EXIT();\
}\
}

/*#define		REG_VERIFY_CODE_2		\
{\
if (g_nMLRegCodeVer == 2 && g_MiniLyricsMain.m_bRegistered)\
{\
if ((CharToNumber(g_szMLRegCode[9]) & 0x1C) != (CharToNumber(g_szMLRegNameNew[1]) & 0x1C))\
VERIFY_FAILED_EXIT;\
}\
}*/

bool SimpleVerifyRegCode2();

//
// 验证迷你歌词的注册码
bool MLVerifyRegCodeV7Before(const char *szName, const char *szRegCode)
{
	int		nLen;

	nLen = (int)strlen(szRegCode);
	if (nLen >= 64)
		return FALSE;
	if (strlen(szName) >= 48)
		return FALSE;

	//
	// 将szRegCode复制到g_szMLRegCode中，并且去掉 '-'
	strcpy_safe(g_szMLRegCode, CountOf(g_szMLRegCode), szRegCode);
	strdel(g_szMLRegCode, '-');

	//
	// 复制注册名
	strcpy_safe(g_szMLRegName, CountOf(g_szMLRegName), szName);

	nLen = (int)strlen(g_szMLRegCode);
	if (nLen == 16)
	{
#ifndef _WIN32_WCE
		// 最初版本的迷你歌词注册码
		if (strlen(szName) < 3)
			return FALSE;

		g_nMLRegCodeVer = 1;
		return VerifyRegCode1(szName, szRegCode);
#endif
	}
	else if (nLen == 18)
	{
		// 2.0
		if (strlen(szName) < 1)
			return FALSE;

		// 计算版本号: (0 - 1)位
		g_nMLRegCodeVer = GetRegCodeVersion(g_szMLRegCode);
		if (g_nMLRegCodeVer == ML_REGCODE_GEN_VERSION2)
		{
			return SimpleVerifyRegCode2();
		}
	}

	return FALSE;
}

bool MLVerifyRegCodeV7Before(const WCHAR *szName, const WCHAR *szRegCode)
{
	stringex		strName, strRegCode;

	ConvertStr(szName, -1, strName);
	ConvertStr(szRegCode, -1, strRegCode);

	return MLVerifyRegCodeV7Before(strName.c_str(), strRegCode.c_str());
}

//
// version == 2
bool SimpleVerifyRegCode2()
{
	//
	// 复制g_szMLRegNameNew
	strcpy_safe(g_szMLRegNameNew, CountOf(g_szMLRegNameNew), g_szMLRegName);
	_strupr(g_szMLRegNameNew);
	strncat(g_szMLRegNameNew, g_szMLRegCode, 8);

	// 计算年月日 2 - 7
	g_nRegYear = RandCodeToNumber(g_szMLRegCode[2], g_szMLRegCode[3]) + 2000;
	g_nRegMonth = RandCodeToNumber(g_szMLRegCode[4], g_szMLRegCode[5]);
	g_nRegDay = RandCodeToNumber(g_szMLRegCode[6], g_szMLRegCode[7]);

	// 校验码部分: 8 , 10, 12, 14, 15, 17, 18
	if (CharToNumber(g_szMLRegCode[8]) != (CharToNumber(g_szMLRegNameNew[0]) + CharToNumber(g_szMLRegCode[9]) + CharToNumber(g_szMLRegCode[0]) + CharToNumber(g_szMLRegCode[6])) % MAX_REGCODE_CHARS)
		return FALSE;
	if (CharToNumber(g_szMLRegCode[10]) != (CharToNumber(g_szMLRegNameNew[0]) + CharToNumber(g_szMLRegCode[11]) + CharToNumber(g_szMLRegNameNew[1]) + CharToNumber(g_szMLRegCode[1]) + CharToNumber(AvgValue(g_szMLRegNameNew, strlen(g_szMLRegNameNew)))) / 5)
		return FALSE;
	if (CharToNumber(g_szMLRegCode[12]) != (CharToNumber(g_szMLRegNameNew[0]) + CharToNumber(g_szMLRegCode[13]) + CharToNumber(g_szMLRegNameNew[2]) + CharToNumber(g_szMLRegCode[2]) + CharToNumber(AvgValue(g_szMLRegNameNew, strlen(g_szMLRegNameNew))) + CharToNumber(g_szMLRegCode[10])) % MAX_REGCODE_CHARS)
		return FALSE;
	if (CharToNumber(g_szMLRegCode[15]) != (CharToNumber(g_szMLRegNameNew[0]) + CharToNumber(g_szMLRegNameNew[4]) + CharToNumber(g_szMLRegCode[4]) * CharToNumber(AvgValue(g_szMLRegNameNew, strlen(g_szMLRegNameNew)))) % MAX_REGCODE_CHARS)
		return FALSE;
	if (CharToNumber(g_szMLRegCode[17]) != (CharToNumber(g_szMLRegNameNew[0]) + CharToNumber(g_szMLRegNameNew[5]) * CharToNumber(g_szMLRegCode[5])) % MAX_REGCODE_CHARS)
		return FALSE;

	if ((CharToNumber(g_szMLRegCode[11]) & 0x3) != (CharToNumber(g_szMLRegCode[2]) & 0x3))
		return FALSE;

	return TRUE;
}

// 注册码验证部分
///////////////////////////////////////////////////////////////
