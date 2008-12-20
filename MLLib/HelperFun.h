#ifndef _HELPER_FUNCTION_H_INCLUDED_2002_9_16_9_40
#define _HELPER_FUNCTION_H_INCLUDED_2002_9_16_9_40

bool MDStrToBin(const char *szDigest, unsigned char *digest);

int CompareVersion(LPCTSTR szVer1, LPCTSTR szVer2);
int CompareVersion(LPCTSTR szVer1, int nMajor, int nMinor, int nBuild);

int GoDie();

int MultibyteToUTF8(LPCTSTR szInput, LPTSTR szOutput, int nOutputSize);

int UTF8ToMultibyte(unsigned char* text, char *szOut, int nOutBuffLen);

// 将字符串进行异或操作
void XorString(char *szString, int nLen);

//		判断两个日期是否相等，日期的格式为：yy-mm-dd
BOOL IsDateEqual(LPCTSTR szDate1, LPCTSTR szDate2);
/*

#define CountOf(arr)		(sizeof(arr) / sizeof(arr[0]))

#define IsEmptyString(str)	(str[0] == '\0')

#define IsFlagValid(value, flag)	(((value) & (flag)) == (flag))
#define IsFlagSet(value, flag)	(((value) & (flag)) == (flag))
*/

void FilterSongFileExt(LPTSTR szFile);

BOOL AnalysisLyricsFileNameEx(TCHAR szArtist[]/* out */, TCHAR szTitle[]/* out */, LPCTSTR szFileName);

BOOL IsEncryptedLyrics(LPCTSTR szFile);

#ifdef WIN32
bool GetMLEncriptyData(stringex &strData);
#endif

#endif // _HELPER_FUNCTION_H_INCLUDED_2002_9_16_9_40
