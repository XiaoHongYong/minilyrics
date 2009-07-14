// HelperFun.cpp: implementation of the CHelperFun class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ID3v2.h"
#include "ID3Helper.h"
/*
void AppendUcs2Str(CCharBuffer &buff, wstringex &strUcs2)
{
	// Unicode
	buff += (char)0xFF;
	buff += (char)0xFE;
	buff.append((const char *)strUcs2.c_str(), (strUcs2.size() + 1) * sizeof(wchar_t));
}
*/

void AppendUcs2Str(CCharBuffer &buff, ID3v2EncType encType, LPCWSTR str, int nLen)
{
	if (encType == IET_UCS2LE_BOM)
	{
		buff += (char)0xFF;
		buff += (char)0xFE;

		buff.append((const char *)str, (nLen + 1) * sizeof(wchar_t));
	}
	else if (encType == IET_UCS2BE_NO_BOM)
	{
		while (*str)
		{
			buff += (char)((*str) >> 8);
			buff += (char)((*str) & 0xFF);
			str++;
		}
		str += (char)0;
		str += (char)0;
	}
	else if (encType == IET_UTF8)
	{
// 		buff += (char)0xEF;
// 		buff += (char)0xBB;
// 		buff += (char)0xBF;
		stringex	strUtf8;
		UCS2ToUtf8(str, nLen, strUtf8);
		buff.append(strUtf8.c_str(), strUtf8.size() + 1);
	}
}

// return the size of bytes used from buffer data
int CopyAnsiStr(string &str, const char *data, int nLen)
{
	int		nStrLen = 0;
	const char *p = data;
	while (*p && nStrLen <= nLen)
		p++, nStrLen++;
	str.append(data, nStrLen);
	return nStrLen + 1;
}

// return the size of bytes used from buffer data
int CopyAnsiStr(stringex &str, const char *data, int nLen)
{
	int		nStrLen = 0;
	const char *p = data;
	while (*p && nStrLen <= nLen)
		p++, nStrLen++;
	str.append(data, nStrLen);
	return nStrLen + 1;
}

// return the size of bytes used from buffer data
int CopyUcs2StrByBom(wstringex &str, ID3v2EncType encType, const BYTE *wtext, int len)
{
	int		lenOrg = len;

	str.resize(0);

	if (wtext[0] == 0xFF && wtext[1] == 0xFE)
	{
		ASSERT(encType == IET_UCS2LE_BOM);
		encType = IET_UCS2LE_BOM;
		wtext += 2;
		len -= 2;
	}
	else if (wtext[0] == 0xFE && wtext[1] == 0xFF)
	{
		ASSERT(encType == IET_UCS2BE_NO_BOM);
		encType = IET_UCS2BE_NO_BOM;
		wtext += 2;
		len -= 2;
	}

	if (encType == IET_UCS2LE_BOM)
	{
		str.resize(len / 2);
		int		n = wcsncpysz_safe((wchar_t *)str.c_str(), str.size() + 1, (const wchar_t*)wtext, len / 2);
		str.resize(n);

		len -= (str.size() + 1) * sizeof(wchar_t);
	}
	else if (encType == IET_UCS2BE_NO_BOM)
	{
		const BYTE *end = wtext + len;
		wchar_t		value;
		for (const BYTE *pos = wtext; pos < end; pos += 2)
		{
			value = (pos[0] << 8) | pos[1];
			if (value == '\0')
				break;
			str += value;
		}

		len -= (str.size() + 1) * sizeof(wchar_t);
	}
	else if (encType == IET_UTF8)
	{
		if (wtext[0] == 0xEF && wtext[1] == 0xBB && wtext[2] == 0xBF)
		{
			wtext += 3;
			len -= 3;
		}

		Utf8ToUCS2((const char *)wtext, -1, str);

		len -= strlen((const char *)wtext) + 1;
	}
	else
		return 0;

	return lenOrg - len;
}

int CopyUcs2StrByBom(wstring &str, ID3v2EncType encType, const BYTE *wtext, int len)
{
	wstringex		str2;
	int				nRet;
	nRet = CopyUcs2StrByBom(str2, encType, wtext, len);
	str = str2.c_str();
	return nRet;
/*	int		lenOrg = len;

	str.resize(0);

	if (wtext[0] == 0xFE && wtext[1] == 0xFF)
	{
		encType = IET_UCS2LE_BOM;
		wtext += 2;
		len -= 2;
	}
	else if (wtext[0] == 0xFF && wtext[1] == 0xFE)
	{
		encType = IET_UCS2BE_NO_BOM;
		wtext += 2;
		len -= 2;
	}

	if (encType == IET_UCS2LE_BOM)
	{
		const BYTE *end = wtext + len;
		wchar_t		value;
		for (const BYTE *pos = wtext; pos < end; pos += 2)
		{
			value = (pos[0] << 8) | pos[1];
			if (value == '\0')
				break;
			str += value;
		}

		len -= (str.size() + 1) * sizeof(wchar_t);
	}
	else if (encType == IET_UCS2BE_NO_BOM)
	{
		str.resize(len / 2);
		int		n = wcsncpysz_safe((wchar_t *)str.c_str(), str.size() + 1, (const wchar_t*)wtext, len / 2);
		str.resize(n);

		len -= (str.size() + 1) * sizeof(wchar_t);
	}
	else if (encType == IET_UTF8)
	{
		if (wtext[0] == 0xEF && wtext[1] == 0xBB && wtext[2] == 0xBF)
		{
			wtext += 3;
			len -= 3;
		}

		wstringex	wstr;
		Utf8ToUCS2((const char *)wtext, -1, wstr);
		str = wstr.c_str();

		len -= strlen((const char *)wtext) + 1;
	}
	else
		return 0;

	return lenOrg - len;*/
}

uint32 SynchDataToUInt(BYTE *byData, int nLen)
{
	uint32	sum = 0;
	int		last;
	if (nLen > 4)
		last = 3;
	else
		last = nLen - 1;

	for(int i = 0; i <= last; i++)
		sum |= (byData[i] & 0x7f) << ((last - i) * 7);

	return sum;
}

void SynchDataFromUInt(uint32 nValue, BYTE *byData, int nLen)
{
	int		last;
	last = nLen - 1;
	for(int i = 0; i < nLen; i++)
		byData[i] = BYTE((nValue >> ((last - i) * 7)) & 0x7f);

}

uint32 ByteDataToUInt(BYTE *byData, int nLen)
{
	uint32	sum = 0;
	int		last;
	if (nLen > 4)
		last = 3;
	else
		last = nLen - 1;

	for(int i = 0; i <= last; i++)
		sum |= byData[i] << ((last - i) * 8);

	return sum;
}

void ByteDataFromUInt(uint32 nValue, BYTE *byData, int nLen)
{
	int		last;
	last = nLen - 1;
	for(int i = 0; i < nLen; i++)
		byData[i] = BYTE((nValue >> ((last - i) * 8)) & 0xff);
}

int FileDataReset(FILE *m_fp, int nOffset, int nSize, int nMask)
{
	char	buff[4096];

	memset(buff, nMask, sizeof(buff));

	if (fseek(m_fp, nOffset, SEEK_SET) != 0)
		return ERR_SEEK_FILE;

	int		nLen;

	for (int i = nSize; i > 0; i -= sizeof(buff))
	{
		if (i >= sizeof(buff))
			nLen = sizeof(buff);
		else
			nLen = i;

		if (fwrite(buff, 1, nLen, m_fp) != nLen)
		{
			return ERR_WRITE_FILE;
		}
	}

	return ERR_OK;
}

int FileDataCmp(FILE *m_fp, int nOffset, LPCVOID lpData, int nSize, int &nResult)
{
	char	buff[4096];
	char	*pData = (char *)lpData;

	if (fseek(m_fp, nOffset, SEEK_SET) != 0)
		return ERR_SEEK_FILE;

	int		nLen;

	nResult = 0;
	for (int i = nSize; i > 0; i -= sizeof(buff))
	{
		if (i >= sizeof(buff))
			nLen = sizeof(buff);
		else
			nLen = i;

		if (fread(buff, 1, nLen, m_fp) != nLen)
		{
			return ERR_READ_FILE;
		}
		nResult = memcmp(buff, pData, nLen);
		if (nResult != 0)
			return ERR_OK;
		pData += nLen;
	}

	return ERR_OK;
}

/*
//	Test function for FileDataCmp(), FileDataReset()
void TestFileDataCmp()
{
	// FileMoveEndData
	FILE		*fp;
	int			nRet;

	fp = fopen("c:\\1.txt", "r+b");
	if (!fp)
		return;

	nRet = FileDataReset(fp, 4, 1000, 0x4d);
	if (nRet != ERR_OK)
		return;

	CCharBuffer		buff;
	int				nResult;

	// buff.append("\0\0", 2);
	for (int i = 0; i < 400; i++)
		buff += 0x4d;

	nRet = FileDataCmp(fp, 100, buff.c_str(), buff.size(), nResult);
	if (nRet != ERR_OK)
		return;

	if (nResult != 0)
		AfxMessageBox("Not equal");

	fclose(fp);
}
*/

int FileAppendData(FILE *m_fp, int nAppend, int nMask)
{
	char	buff[4096];
	int		nOldLen, nLen;

	memset(buff, nMask, sizeof(buff));

	if (fseek(m_fp, 0, SEEK_END) != 0)
		return ERR_SEEK_FILE;

	nOldLen = ftell(m_fp);

	for (int i = nAppend; i > 0; i -= sizeof(buff))
	{
		if (i >= sizeof(buff))
			nLen = sizeof(buff);
		else
			nLen = i;

		if (fwrite(buff, 1, nLen, m_fp) != nLen)
		{
			// write error!
			filetruncate(m_fp, ftell(m_fp));

			return ERR_WRITE_FILE;
		}
	}

	return ERR_OK;
}

int FileMoveEndData(FILE *m_fp, int nOffset, int nNewOffset)
{
	CCharBuffer		buff;

	buff.reserve(1024 * 128);
	if (!buff.data())
		buff.reserve(1024);
	if (!buff.data())
		return ERR_NO_MEM;

	int			nLen, nPos, nFileLen, nNewAppend;
	int			nRet;

	if (fseek(m_fp, 0, SEEK_END) != 0)
		return ERR_SEEK_FILE;

	nFileLen = ftell(m_fp);

	if (nOffset >= nFileLen)
	{
		assert(nOffset < nFileLen);
		return ERR_FALSE;
	}

	if (nNewOffset == nOffset)
		return ERR_OK;
	else if (nNewOffset > nOffset)
	{
		nRet = FileAppendData(m_fp, nNewOffset - nOffset);
		if (nRet != ERR_OK)
			return nRet;

		int		nLastEndPos;

		nNewAppend = nNewOffset - nOffset;
		nLastEndPos = nFileLen;

		// from end to move data
		//     ---oooo
		//     ----oooo
		while (nLastEndPos > nOffset)
		{
			nPos = nLastEndPos - buff.capacity();
			if (nPos < nOffset)
				nPos = nOffset;
			nLen = nLastEndPos - nPos;

			if (fseek(m_fp, nPos, SEEK_SET) != 0)
				return ERR_SEEK_FILE;
			if (fread(buff.data(), 1, nLen, m_fp) != nLen)
				return ERR_READ_FILE;
			if (fseek(m_fp, nPos + nNewAppend, SEEK_SET) != 0)
				return ERR_SEEK_FILE;
			if (fwrite(buff.data(), 1, nLen, m_fp) != nLen)
				return ERR_WRITE_FILE;

			nLastEndPos = nPos;
		}
	}
	else
	{
		// from begin to move data
		//     ---oooo
		//     --oooo
		int		nNextPos;

		nNewAppend = nOffset - nNewOffset;
		nPos = nOffset;
		while (nPos < nFileLen)
		{
			nNextPos = nPos + buff.capacity();
			if (nNextPos > nFileLen)
				nNextPos = nFileLen;
			nLen = nNextPos - nPos;

			if (fseek(m_fp, nPos, SEEK_SET) != 0)
				return ERR_SEEK_FILE;
			if (fread(buff.data(), 1, nLen, m_fp) != nLen)
				return ERR_READ_FILE;
			if (fseek(m_fp, nPos - nNewAppend, SEEK_SET) != 0)
				return ERR_SEEK_FILE;
			if (fwrite(buff.data(), 1, nLen, m_fp) != nLen)
				return ERR_WRITE_FILE;

			nPos = nNextPos;
		}
		// clip the file.
		filetruncate(m_fp, nFileLen - nNewAppend);
	}

	return ERR_OK;
}

/*
	Test function for FileMoveEndData()
void TestFileMoveEndData()
{
	// FileMoveEndData
	FILE		*fp;
	fp = fopen("c:\\1.txt", "wb");
	if (!fp)
		return;

	for (int i = 0; i < 100; i++)
	{
		fprintf(fp, "%d", i % 10);
	}
	fclose(fp);

	fp = fopen("c:\\1.txt", "r+b");
	if (!fp)
		return;
	FileMoveEndData(fp, 90, 89);
	fclose(fp);
}
*/

bool IsUnsynchLyricsFrameID(UINT nFrameID)
{
	return nFrameID == ID3V2_2_USLT || nFrameID == ID3V2_3_USLT;
}

bool IsSynchLyricsFrameID(UINT nFrameID)
{
	return nFrameID == ID3V2_2_SYLT || nFrameID == ID3V2_3_SYLT;
}
