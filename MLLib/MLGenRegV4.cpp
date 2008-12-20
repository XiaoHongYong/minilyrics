// MLGenRegGenV4.cpp: implementation of the MLGenRegGenV4 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <string.h>
#include "MLGenRegV4.h"

extern char	g_szMLRegCode[64];
extern char	g_szMLRegName[64];
extern char	g_szMLRegNameNew[64];
extern int		g_nMLRegCodeVer;
extern BOOL	g_bRegistered;

extern int		g_nRegYear;
extern int		g_nRegMonth;
extern int		g_nRegDay;
int		g_nLicenceMonth;

///////////////////////////////////////////////////////////////
// 公共定义部分
#define ML_REGCODE_GEN_VERSION4			4
#define ML_APPAND_NAME_LEN				10
static char _arrNumbChar[] = "3456789ABCDEFGHJKLMNPQRSTWXY";
const int MAX_REGCODE_CHARS	= strlen(_arrNumbChar);

static int CharToNumber(char ch)
{
	const char *szPos = strchr(_arrNumbChar, ch);
	if (szPos == NULL)
		return 10;// 'A'
	return int(szPos - _arrNumbChar);
}

static BYTE AvgValue(const char *szString, size_t nLen)
{
	int		nValue = 0;

	for (UINT i = 0; i < nLen; i++)
		nValue += szString[i];
	return (BYTE)(nValue / nLen);
}

///////////////////////////////////////////////////////////////
// 注册码验证部分

static int RandCodeToNumber(char chRand, char chNumber)
{
	int nRand;
	int nNumber;

	nRand = CharToNumber(chRand);
	nNumber = MAX_REGCODE_CHARS - nRand - CharToNumber(chNumber);
	ASSERT(nNumber >= 0 && nNumber < MAX_REGCODE_CHARS);

	return nNumber;
}

#define GetRegCodeVersion(szRegCode)	RandCodeToNumber(szRegCode[0], szRegCode[1])


///////////////////////////////////////////////////////////////////////////
//
// 验证部分由宏来代替
//
//#define VERIFY_FAILED_EXIT()		\
//{\
//	MessageBox(NULL, "Invalid registration code, please visit http://www.minilyrics.com/ for more infomation!", "Minilyircs", MB_OK);\
//	PostQuitMessage(0);\
//}

BOOL SimpleVerifyRegCodeV4();

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
//#define		REG_VERIFY_CODE_1		\
//{\
//	if (g_nMLRegCodeVer == 2 && g_bRegistered && g_MLRegUseTime.wDayOfWeek == 2)\
//{\
//	if ((CharToNumber(g_szMLRegCode[9]) & 0x3) != (CharToNumber(g_szMLRegNameNew[0]) & 0x3))\
//	VERIFY_FAILED_EXIT();\
//}\
//}

/*#define		REG_VERIFY_CODE_2		\
{\
if (g_nMLRegCodeVer == 2 && g_bRegistered)\
{\
if ((CharToNumber(g_szMLRegCode[9]) & 0x1C) != (CharToNumber(g_szMLRegNameNew[1]) & 0x1C))\
VERIFY_FAILED_EXIT;\
}\
}*/

//
// version == 2
BOOL SimpleVerifyRegCodeV4()
{
	BOOL		bRet = TRUE;

	//
	// 复制g_szMLRegNameNew
	strcpy(g_szMLRegNameNew, g_szMLRegName);
	_strupr(g_szMLRegNameNew);

	strncat(g_szMLRegNameNew, g_szMLRegCode, ML_APPAND_NAME_LEN);

	// 计算年月日 2 - 7
	g_nRegYear = RandCodeToNumber(g_szMLRegCode[2], g_szMLRegCode[3]) + 2000;
	g_nRegMonth = RandCodeToNumber(g_szMLRegCode[4], g_szMLRegCode[5]);
	g_nRegDay = RandCodeToNumber(g_szMLRegCode[6], g_szMLRegCode[7]);

	// 授权月份
	g_nLicenceMonth = RandCodeToNumber(g_szMLRegCode[8], g_szMLRegCode[9]);

	// 校验码部分: 10 , 12, 14, 16, 17, 19
	if (CharToNumber(g_szMLRegCode[10]) != (CharToNumber(g_szMLRegNameNew[0]) + CharToNumber(g_szMLRegCode[11]) + CharToNumber(g_szMLRegCode[0]) + CharToNumber(g_szMLRegCode[6])) % MAX_REGCODE_CHARS)
		return FALSE;
	if (CharToNumber(g_szMLRegCode[12]) != (CharToNumber(g_szMLRegNameNew[0]) + CharToNumber(g_szMLRegCode[13]) + CharToNumber(g_szMLRegNameNew[1]) + CharToNumber(g_szMLRegCode[1]) + CharToNumber(AvgValue(g_szMLRegNameNew, strlen(g_szMLRegNameNew)))) / 5)
		return FALSE;
	if (CharToNumber(g_szMLRegCode[14]) != (CharToNumber(g_szMLRegNameNew[0]) + CharToNumber(g_szMLRegCode[15]) + CharToNumber(g_szMLRegNameNew[2]) + CharToNumber(g_szMLRegCode[2]) + CharToNumber(AvgValue(g_szMLRegNameNew, strlen(g_szMLRegNameNew))) + CharToNumber(g_szMLRegCode[11])) % MAX_REGCODE_CHARS)
		return FALSE;
	if (CharToNumber(g_szMLRegCode[16]) != (CharToNumber(g_szMLRegNameNew[0]) + CharToNumber(g_szMLRegCode[18]) + CharToNumber(g_szMLRegNameNew[3]) + CharToNumber(g_szMLRegCode[3]) + CharToNumber(AvgValue(g_szMLRegNameNew, strlen(g_szMLRegNameNew)))) / 5)
		return FALSE;
	if (CharToNumber(g_szMLRegCode[17]) != (CharToNumber(g_szMLRegNameNew[0]) + CharToNumber(g_szMLRegNameNew[8]) + CharToNumber(g_szMLRegCode[4]) * CharToNumber(AvgValue(g_szMLRegNameNew, strlen(g_szMLRegNameNew)))) % MAX_REGCODE_CHARS)
		return FALSE;
	if (CharToNumber(g_szMLRegCode[19]) != (CharToNumber(g_szMLRegNameNew[0]) + CharToNumber(g_szMLRegNameNew[9]) * CharToNumber(g_szMLRegCode[5])) % MAX_REGCODE_CHARS)
		return FALSE;

	// REG_VERIFY_CODE_1;

	return TRUE;
}

// 注册码验证部分
///////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////
//// 注册码产生部分
//
////
//// nNumber <= 'Z' - 'A' + '9' - '0' + 2
//static char NumberToChar(int nNumber)
//{
//	ASSERT(nNumber >= 0 && nNumber <= MAX_REGCODE_CHARS);
//	if (nNumber >= MAX_REGCODE_CHARS || nNumber < 0)
//		nNumber %= MAX_REGCODE_CHARS;
//	return _arrNumbChar[nNumber];
//}
//
//static void MakeRandCode(IN int nNumber, OUT char &chRand, OUT char &chNumber)
//{
//	ASSERT(nNumber >= 0 && nNumber < MAX_REGCODE_CHARS);
//
//	int		nRand = rand() % (MAX_REGCODE_CHARS - nNumber);
//	chRand = NumberToChar(nRand);
//	chNumber = NumberToChar(MAX_REGCODE_CHARS - nRand - nNumber);
//}
//
//// 24/12/2002
//static BOOL GetDate(int &nMonth, int &nDay, int &nYear, const char *szDate)
//{
//	const char		*szTemp;
//
//	szTemp = szDate;
//	nDay = atoi(szTemp);
//	nMonth = 0;
//	nYear = 0;
//
//	while ((*szTemp >= '0') && (*szTemp <= '9'))
//		szTemp ++;
//
//	if (*szTemp != '\0')
//		szTemp++;
//	nMonth = atoi(szTemp);
//
//	while ((*szTemp >= '0') && (*szTemp <= '9'))
//		szTemp ++;
//
//	if (*szTemp != '\0')
//		szTemp++;
//	nYear = atoi(szTemp);
//
//	return TRUE;
//}
//
//BOOL MLGenRegCodeV4(const char *szName, const char *szDate, char *szRegCode, int nLicenceYear)
//{
//	int			nMonth, nDay, nYear;
//
//	char		szCodeTemp[256];
//	char		szNameNew[256];
//
//	if (strlen(szName) < 1)
//		return FALSE;
//
//	strcpy(szNameNew, szName);
//	strupr(szNameNew);
//
//	static bool bInited = FALSE;
//	if (!bInited)
//	{
//		bInited = TRUE;
//		srand(GetTickCount());
//	}
//	memset(szCodeTemp, 0, sizeof(szCodeTemp));
//
//	if (!GetDate(nMonth, nDay, nYear, szDate))
//		return FALSE;
//
//	// 计算版本号: 0 - 1
//	int		nRand = rand() % (MAX_REGCODE_CHARS - ML_REGCODE_GEN_VERSION4);
//	szCodeTemp[0] = NumberToChar(nRand);
//	szCodeTemp[1] = NumberToChar(MAX_REGCODE_CHARS - nRand - ML_REGCODE_GEN_VERSION4);
//
//	// 日期部分
//	nYear %= 10;
//	MakeRandCode(nYear, szCodeTemp[2], szCodeTemp[3]);
//	MakeRandCode(nMonth, szCodeTemp[4], szCodeTemp[5]);
//	MakeRandCode(nDay, szCodeTemp[6], szCodeTemp[7]);
//
//	// 授权月份数目
//	MakeRandCode(nLicenceYear, szCodeTemp[8], szCodeTemp[9]);
//
//	strncat(szNameNew, szCodeTemp, ML_APPAND_NAME_LEN);
//
//	// 留以扩展的校验部分11, 13, 15, 18
//	szCodeTemp[11] = NumberToChar(CharToNumber(szNameNew[0]) / 2 + CharToNumber(szNameNew[3]) / 2);
//	szCodeTemp[13] = NumberToChar(CharToNumber(szCodeTemp[2]) / 2 + CharToNumber(szCodeTemp[1]) / 2);
//	szCodeTemp[15] = NumberToChar(CharToNumber(szNameNew[4]) / 3 + CharToNumber(szCodeTemp[5]) / 3 * 2);
//	szCodeTemp[18] = NumberToChar(CharToNumber(szCodeTemp[6]) / 3 + CharToNumber(szNameNew[7]) / 3 * 2);
//
//	// 校验码部分: 10 , 12, 14, 16, 17, 19
//	szCodeTemp[10] = NumberToChar(
//		(CharToNumber(szNameNew[0]) + CharToNumber(szCodeTemp[11]) + CharToNumber(szCodeTemp[0]) + CharToNumber(szCodeTemp[6])) % MAX_REGCODE_CHARS
//		);
//	szCodeTemp[12] = NumberToChar(
//		(CharToNumber(szNameNew[0]) + CharToNumber(szCodeTemp[13]) + CharToNumber(szNameNew[1]) + CharToNumber(szCodeTemp[1]) + CharToNumber(AvgValue(szNameNew, strlen(szNameNew)))) / 5
//		);
//	szCodeTemp[14] = NumberToChar(
//		(CharToNumber(szNameNew[0]) + CharToNumber(szCodeTemp[15]) + CharToNumber(szNameNew[2]) + CharToNumber(szCodeTemp[2]) + CharToNumber(AvgValue(szNameNew, strlen(szNameNew))) + CharToNumber(szCodeTemp[11])) % MAX_REGCODE_CHARS
//		);
//	szCodeTemp[16] = NumberToChar(
//		(CharToNumber(szNameNew[0]) + CharToNumber(szCodeTemp[18]) + CharToNumber(szNameNew[3]) + CharToNumber(szCodeTemp[3]) + CharToNumber(AvgValue(szNameNew, strlen(szNameNew)))) / 5
//		);
//	szCodeTemp[17] = NumberToChar(
//		(CharToNumber(szNameNew[0]) + CharToNumber(szNameNew[8]) + CharToNumber(szCodeTemp[4]) * CharToNumber(AvgValue(szNameNew, strlen(szNameNew)))) % MAX_REGCODE_CHARS
//		);
//	szCodeTemp[19] = NumberToChar(
//		(CharToNumber(szNameNew[0]) + CharToNumber(szNameNew[9]) * CharToNumber(szCodeTemp[5])) % MAX_REGCODE_CHARS
//		);
//
//	strcpy(szRegCode, szCodeTemp);
//
//	return TRUE;
//}
//
//BOOL MLVerifyRegCodeV4(const char *szName, const char *szRegCode, OUT char *szDate, OUT int &nLicenceYear)
//{
//	char		szCodeTemp[256];
//	char		szNameNew[256];
//	BOOL		bRet = TRUE;;
//
//	if (strlen(szName) < 1)
//		return FALSE;
//
//	strcpy(szNameNew, szName);
//	strupr(szNameNew);
//
//	//
//	// 将szRegCode复制到g_szMLRegCode中，并且去掉 '-'
//	const char	*szPos;
//	char	*szPos2;
//	szPos = szRegCode;
//	szPos2 = szCodeTemp;
//	while (*szPos)
//	{
//		if (*szPos != '-')
//		{
//			*szPos2 = *szPos;
//			szPos2++;
//		}
//		szPos++;
//	}
//	*szPos2 = '\0';
//
//	strncat(szNameNew, szCodeTemp, ML_APPAND_NAME_LEN);
//
//	// 计算版本号: 0 - 1
//	int	nVersion;
//	nVersion = GetRegCodeVersion(szCodeTemp);
//	if (nVersion != ML_REGCODE_GEN_VERSION4)
//		return FALSE;
//
//	// 计算年月日 2 - 7
//	int			nMonth, nDay, nYear;
//	nYear = RandCodeToNumber(szCodeTemp[2], szCodeTemp[3]) + 2000;
//	nMonth = RandCodeToNumber(szCodeTemp[4], szCodeTemp[5]);
//	nDay = RandCodeToNumber(szCodeTemp[6], szCodeTemp[7]);
//	wsprintf(szDate, "%02d/%02d/%04d", nDay, nMonth, nYear);
//
//	// 授权月份
//	nLicenceYear = RandCodeToNumber(szCodeTemp[8], szCodeTemp[9]);
//
//	// 校验码部分: 10 , 12, 14, 16, 17, 19
//	if (CharToNumber(szCodeTemp[10]) != (CharToNumber(szNameNew[0]) + CharToNumber(szCodeTemp[11]) + CharToNumber(szCodeTemp[0]) + CharToNumber(szCodeTemp[6])) % MAX_REGCODE_CHARS)
//		return FALSE;
//	if (CharToNumber(szCodeTemp[12]) != (CharToNumber(szNameNew[0]) + CharToNumber(szCodeTemp[13]) + CharToNumber(szNameNew[1]) + CharToNumber(szCodeTemp[1]) + CharToNumber(AvgValue(szNameNew, strlen(szNameNew)))) / 5)
//		return FALSE;
//	if (CharToNumber(szCodeTemp[14]) != (CharToNumber(szNameNew[0]) + CharToNumber(szCodeTemp[15]) + CharToNumber(szNameNew[2]) + CharToNumber(szCodeTemp[2]) + CharToNumber(AvgValue(szNameNew, strlen(szNameNew))) + CharToNumber(szCodeTemp[11])) % MAX_REGCODE_CHARS)
//		return FALSE;
//	if (CharToNumber(szCodeTemp[16]) != (CharToNumber(szNameNew[0]) + CharToNumber(szCodeTemp[18]) + CharToNumber(szNameNew[3]) + CharToNumber(szCodeTemp[3]) + CharToNumber(AvgValue(szNameNew, strlen(szNameNew)))) / 5)
//		return FALSE;
//	if (CharToNumber(szCodeTemp[17]) != (CharToNumber(szNameNew[0]) + CharToNumber(szNameNew[8]) + CharToNumber(szCodeTemp[4]) * CharToNumber(AvgValue(szNameNew, strlen(szNameNew)))) % MAX_REGCODE_CHARS)
//		return FALSE;
//	if (CharToNumber(szCodeTemp[19]) != (CharToNumber(szNameNew[0]) + CharToNumber(szNameNew[9]) * CharToNumber(szCodeTemp[5])) % MAX_REGCODE_CHARS)
//		return FALSE;
//
//	// 留以扩展的校验部分11, 13, 15, 18
//	if (CharToNumber(szCodeTemp[11]) != CharToNumber(szNameNew[0]) / 2 + CharToNumber(szNameNew[3]) / 2)
//		return FALSE;
//	if (CharToNumber(szCodeTemp[13]) != CharToNumber(szCodeTemp[2]) / 2 + CharToNumber(szCodeTemp[1]) / 2)
//		return FALSE;
//	if (CharToNumber(szCodeTemp[15]) != CharToNumber(szNameNew[4]) / 3 + CharToNumber(szCodeTemp[5]) / 3 * 2)
//		return FALSE;
//	if (CharToNumber(szCodeTemp[18]) != CharToNumber(szCodeTemp[6]) / 3 + CharToNumber(szNameNew[7]) / 3 * 2)
//		return FALSE;
//
//	return TRUE;
//}
//// 注册码产生部分
/////////////////////////////////////////////////////////////////
