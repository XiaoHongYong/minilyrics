#ifndef _MLENCRYPT_H_
#define _MLENCRYPT_H_

#pragma once

bool MLEncriptV2(LPCTSTR szFile, LPCSTR szData1, LPCSTR szData2, stringex &strData, tstring &strErrMsg)
{
	const int		LEN_DATA_MAX = 40;
	const int		LEN_DATA1_MAX = 20;
	const int		LEN_DATA2_MAX = 19;
	const int		POS_MZ_TXT_BEG = 0x4E;
	stringex		str;
	char			szMD5[64];
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

	MD5Str(str.c_str() + 0x100, (int)str.size() - 0x100, szMD5);

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
	char			szMD5[64];
	char			szOrgData[LEN_DATA_MAX+1];

	memset(szOrgData, 0, sizeof(szOrgData));

	if (!OpenFileToBuffer(szFile, str))
		return false;

	MD5Str(str.c_str() + 0x100, (int)str.size() - 0x100, szMD5);

	strncpy_safe(szOrgData, CountOf(szOrgData), str.c_str() + POS_MZ_TXT_BEG, LEN_DATA_MAX);

	for (int i = 0; i < LEN_DATA_MAX; i++)
		szOrgData[i] = szOrgData[i] ^ szMD5[i % 32];

	strData1.append(szOrgData, LEN_DATA1_MAX);
	strData2.append(szOrgData + LEN_DATA1_MAX, LEN_DATA2_MAX);

	return true;
}



/*void EncriptV1()
{

stringex	str, strData;
TCHAR			szMD5[64];
char			strOrgData[33];

memset(strOrgData, 0, sizeof(strOrgData));
strncpy_safe(strOrgData, 16, m_strData1, 16);
strncpy_safe(strOrgData + 16, 16, m_strData2, 16);

if (!OpenFileToBuffer(m_strFile, str))
return;

MD5Str(str.c_str() + 0x100, str.size() - 0x100, szMD5);

for (int i = 0; i < 32; i++)
strOrgData[i] = strOrgData[i] ^ szMD5[i];

// 0x51 ~ 0x78
// encodeuu(strOrgData, 32, strData, true, false);
strData.append(strOrgData, 32);

SetDlgItemText(IDC_OUT, strData.c_str());

if (strData.size() > 0x78 - 0x51)
{
AfxMessageBox("Encrypt data too long!");
return;
}

FILE	*fp;

fp = fopen(m_strFile, "rb+");
if (fp)
{
fseek(fp, 0x51, SEEK_SET);
fwrite(strData.c_str(), 1, strData.size(), fp);
fclose(fp);
}
else
AfxMessageBox("Open file FAILED!");
}*/

#endif // _MLENCRYPT_H_