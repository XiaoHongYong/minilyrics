// SrtParser.cpp: implementation of the CSrtParser class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLLib.h"
#include "HelperFun.h"
#include "LyricsParser.h"
#include "LrcParser.h"
#include "SrtParser.h"
//
//7
//00:00:18,454 --> 00:00:21,172
//对我们而言亲吻就像是开场
//
//8
//00:00:21,657 --> 00:00:26,516
//就像佛洛伊飞船出场前
//你得耐著性子先看完脱口秀

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSrtParser::CSrtParser(CMLData *pMLData) : CLyricsParser(pMLData)
{

}

CSrtParser::~CSrtParser()
{

}

// 保存为文件
int CSrtParser::SaveAsFile(LPCTSTR szFile, CHAR_ENCODING newEncoding)
{
	int			nCount;
	tstringex	strData;
	tstringex	strBuff;
	TCHAR		szTime[256];

	//8
	//00:00:21,657 --> 00:00:26,516
	//就像佛洛伊飞船出场前
	//你得耐著性子先看完脱口秀
	int			nBegOld = -1, nEndOld = -1;

	nCount = (int)m_pMLData->m_arrFileRows.size();
	for (int i = 0; i < nCount; i++)
	{
		PML_ROW		pRow;

		pRow = m_pMLData->m_arrFileRows[i];

		if (pRow && LyricsRowToText(pRow, strBuff))
		{
			if (pRow->nBegTime == nBegOld && pRow->nEndTime == nEndOld)
			{
				// 时间相同的行
				strData += strBuff;
			}
			else
			{
				int		nHour, nMinute, nSec, nMs;
				nMs = pRow->nBegTime % 1000;
				nSec = pRow->nBegTime / 1000;
				nMinute = nSec / 60;
				nSec %= 60;
				nHour = nMinute / 60;
				nMinute %= 60;
				_stprintf_s(szTime, CountOf(szTime), _T("%d\n%02d:%02d:%02d,%03d --> "), i, nHour, nMinute, nSec, nMs);
				strData += szTime;

				nMs = pRow->nEndTime % 1000;
				nSec = pRow->nEndTime / 1000;
				nMinute = nSec / 60;
				nSec %= 60;
				nHour = nMinute / 60;
				nMinute %= 60;
				_stprintf_s(szTime, CountOf(szTime), _T("%02d:%02d:%02d,%03d\n"), nHour, nMinute, nSec, nMs);
				strData += szTime;

				strData += strBuff;
			}
		}
	}

	if (!SaveStringAsFile(szFile, strData.c_str(), strData.size(), 
		newEncoding))
		return ERR_WRITE_FILE;

	return ERR_OK;
}

// //00:00:21,657
static const char * ReadTime(const char *szBeg, DWORD &dwTime)
{
	int		n;

	while (*szBeg == ' ')
		szBeg++;
	szBeg = ReadInt_t(szBeg, n);
	if (*szBeg != ':')
		return NULL;
	szBeg++;
	dwTime = n;

	while (*szBeg == ' ')
		szBeg++;
	szBeg = ReadInt_t(szBeg, n);
	if (*szBeg != ':')
		return NULL;
	szBeg++;
	dwTime = dwTime * 60 + n;

	while (*szBeg == ' ')
		szBeg++;
	szBeg = ReadInt_t(szBeg, n);
	if (*szBeg != ',')
		return NULL;
	szBeg++;
	dwTime = dwTime * 60 + n;

	while (*szBeg == ' ')
		szBeg++;
	szBeg = ReadInt_t(szBeg, n);
	dwTime = dwTime * 1000 + n;

	return szBeg;
}

// 分析歌词文件，并且存储到 pMLData 中
int CSrtParser::ParseFile(bool bUseSpecifiedEncoding, CHAR_ENCODING encoding)
{
	//8
	//00:00:21,657 --> 00:00:26,516
	//就像佛洛伊飞船出场前
	//你得耐著性子先看完脱口秀

	char		szBuff[2048 + 4];
	FILE		*fp;
	enum Parse_State
	{
		PS_NUMBER,
		PS_TIME,
		PS_TXT,
		PS_NEWLINE,
	};
	Parse_State	state;
	DWORD		dwBegTime, dwEndTime;

	fp = _tfopen(m_pMLData->GetLyricsFileName(), _T("rt"));
	if (!fp)
	{
		SetCustomErrorDesc(OSError());
		ERRLOG1(_T("Open file: %s FAILED!"), m_pMLData->GetLyricsFileName());
		return ERR_CUSTOM_ERROR;
	}

	state = PS_NUMBER;
	while (fgets(szBuff, CountOf(szBuff) - 1, fp))
	{
		const char		*szBeg;

		szBeg = szBuff;
		if (state == PS_NUMBER)
		{
			while (IsDigit(*szBeg))
				szBeg++;
			while (*szBeg == '\r' || *szBeg == '\n' || *szBeg == ' ')
				szBeg++;
			if (*szBeg != '\0')
				continue;

			state = PS_TIME;
			continue;
		}
		else if (state == PS_TIME)
		{
			szBeg = ReadTime(szBeg, dwBegTime);
			if (szBeg == NULL)
				continue;

			if (strncmp(szBeg, " --> ", 5) != 0)
				continue;
			szBeg += 5;

			szBeg = ReadTime(szBeg, dwEndTime);
			if (szBeg == NULL)
				continue;

			while (*szBeg == '\r' || *szBeg == '\n' || *szBeg == ' ')
				szBeg++;
			if (*szBeg != '\0')
				continue;

			state = PS_TXT;
			continue;
		}
		else if (state == PS_TXT)
		{
			if (*szBeg == '\r' || *szBeg == '\n')
			{
				state = PS_NUMBER;
				continue;
			}

			PLYRIC_FRAG		pFrag;
			PML_ROW			pRow;
			int				nLen;

			nLen = strlen(szBuff);
			while (szBuff[nLen - 1] == '\r' || szBuff[nLen - 1] == '\n')
				nLen--;
			szBuff[nLen] = '\0';

			pFrag = m_pMLData->NewLyricFrag(dwBegTime, dwEndTime,
				szBuff, nLen, FALSE, FALSE);
			
			pRow = m_pMLData->NewMLRow(dwBegTime, dwEndTime);
			pRow->vFrags.push_back(pFrag);

			m_pMLData->AddRowByTime(pRow);
		}
	}

	fclose(fp);

	return ERR_OK;
}

LYRICS_CONTENT_TYPE CSrtParser::GetLyrContentType()
{
	return LCT_LRC;
}

BOOL CSrtParser::LyricsRowToText(PML_ROW pRow, tstringex &strBuff)
{
	strBuff.clear();

	switch (pRow->nRowType)
	{
	case ML_ROW_UNKNOW:		// 不能识别的行
//		assert(pRow->szContent);
//		_tcscpy(szBuff, pRow->szContent);
//		_tcscat(szBuff, _T("\r\n"));
		break;
	case ML_ROW_LYRICS:		// 歌词行
		{
			int		nCount;
			nCount = (int)pRow->vFrags.size();
			for (int i = 0; i < nCount; i++)
			{
				// 去掉插入的注册信息，广告信息等
				if (IsTempLyricsFrag(pRow->vFrags[i]->dwAttrib))
				{
					strBuff.clear();
					break;
				}

				strBuff += pRow->vFrags[i]->szLyric;
			}
			strBuff +=  _T("\r\n");
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
