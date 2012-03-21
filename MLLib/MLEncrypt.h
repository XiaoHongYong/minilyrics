#ifndef _MLENCRYPT_H_
#define _MLENCRYPT_H_

#pragma once

#define OFFSET_DATA		0x100
#define LENGTH_FILE		(1024 * (1024 * 2 + 378))

bool MLEncriptV2(LPCTSTR szFile, LPCSTR szData1, LPCSTR szData2, stringex &strData, tstring &strErrMsg)
{
	const int		LEN_DATA_MAX = 40;
	const int		LEN_DATA1_MAX = 20;
	const int		LEN_DATA2_MAX = 19;
	const int		POS_MZ_TXT_BEG = 0x4E;
	stringex		str;
	TCHAR			szMD5[64];
	char			szOrgData[LEN_DATA_MAX+1];
	const char *	SZ_MZ_TXT = "This program cannot be run in DOS mode.";

	ASSERT(strlen(szData1) <= LEN_DATA1_MAX && strlen(szData2) <= LEN_DATA2_MAX);
	if (!(strlen(szData1) <= LEN_DATA1_MAX && strlen(szData2) <= LEN_DATA2_MAX))
	{
		strErrMsg = _T("Data to encrypt is too long");
		return false;
	}

	memset(szOrgData, 0, sizeof(szOrgData));
	strncpy_safe(szOrgData, LEN_DATA1_MAX + 1, szData1, LEN_DATA1_MAX);
	strncpy_safe(szOrgData + LEN_DATA1_MAX, LEN_DATA2_MAX + 1, szData2, LEN_DATA2_MAX);

	if (!OpenFileToBuffer(szFile, str))
	{
		strErrMsg = _T("FAILED to Open file: ");
		strErrMsg += szFile;
		return false;
	}

	if (strncmp(SZ_MZ_TXT, str.c_str() + POS_MZ_TXT_BEG, strlen(SZ_MZ_TXT)) != 0)
	{
		strErrMsg = _T("NOT FOUND: This program cannot be run in DOS mode.");
		return false;
	}

	if (str.size() < OFFSET_DATA + LENGTH_FILE)
	{
		strErrMsg = _T("File length is too less than expected.");
		return false;
	}

	memset(szMD5, 0, sizeof(szMD5));
	MD5ToString(str.c_str() + OFFSET_DATA, LENGTH_FILE, szMD5);

	for (int i = 0; i < LEN_DATA_MAX; i++)
		szOrgData[i] = szOrgData[i] ^ szMD5[i % 32];

	// 0x48 ~ 0x78
	// encodeuu(strOrgData, 32, strData, true, false);
	strData.append(szOrgData, LEN_DATA_MAX);

	FILE	*fp;

	fp = _tfopen(szFile, _T("rb+"));
	if (fp)
	{
		fseek(fp, POS_MZ_TXT_BEG, SEEK_SET);
		fwrite(strData.c_str(), 1, strData.size(), fp);
		fclose(fp);
	}
	else
		strErrMsg = _T("Failed to write encrypted data.");

	return TRUE;
}

bool MLDecriptV2(LPCTSTR szFile, string &strData1, string &strData2)
{
	const int		LEN_DATA_MAX = 40;
	const int		LEN_DATA1_MAX = 20;
	const int		LEN_DATA2_MAX = 19;
	const int		POS_MZ_TXT_BEG = 0x4E;
	stringex		str;
	TCHAR			szMD5[64];
	char			szOrgData[LEN_DATA_MAX+1];

	memset(szOrgData, 0, sizeof(szOrgData));

	if (!OpenFileToBuffer(szFile, str))
		return false;

	if (str.size() < OFFSET_DATA + LENGTH_FILE)
		return false;

	memset(szMD5, 0, sizeof(szMD5));
	MD5ToString(str.c_str() + OFFSET_DATA, LENGTH_FILE, szMD5);

	strncpy_safe(szOrgData, CountOf(szOrgData), str.c_str() + POS_MZ_TXT_BEG, LEN_DATA_MAX);

	for (int i = 0; i < LEN_DATA_MAX; i++)
		szOrgData[i] = szOrgData[i] ^ szMD5[i % 32];

	strData1.append(szOrgData, LEN_DATA1_MAX);
	strData2.append(szOrgData + LEN_DATA1_MAX, LEN_DATA2_MAX);

	return true;
}

#endif // _MLENCRYPT_H_