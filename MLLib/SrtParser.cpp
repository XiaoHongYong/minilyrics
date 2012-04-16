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

int CSrtParser::SaveAsFile(LPCTSTR szFile, CHAR_ENCODING newEncoding)
{
	tstringex	strData;
	tstringex	strBuff;
	TCHAR		szTime[256];

	//8
	//00:00:21,657 --> 00:00:26,516
	//就像佛洛伊飞船出场前
	//你得耐著性子先看完脱口秀
	int			nBegOld = -1, nEndOld = -1;

	int index = 1;
	int nCount = (int)m_pMLData->m_arrFileLines.size();
	for (int i = 0; i < nCount; i++)
	{
		LyricsLine *pLine;

		pLine = m_pMLData->m_arrFileLines[i];

		if (pLine && LyricsLineToText(pLine, strBuff))
		{
			if (pLine->nBegTime == nBegOld && pLine->nEndTime == nEndOld)
			{
				// 时间相同的行
				strData += strBuff;
			}
			else
			{
				nBegOld = pLine->nBegTime;
				nEndOld = pLine->nEndTime;

				if (index > 1)
					strData += _T("\n");

				int		nHour, nMinute, nSec, nMs;
				nMs = pLine->nBegTime % 1000;
				nSec = pLine->nBegTime / 1000;
				nMinute = nSec / 60;
				nSec %= 60;
				nHour = nMinute / 60;
				nMinute %= 60;
				_stprintf_s(szTime, CountOf(szTime), _T("%d\n%02d:%02d:%02d,%03d --> "), index++, nHour, nMinute, nSec, nMs);
				strData += szTime;

				nMs = pLine->nEndTime % 1000;
				nSec = pLine->nEndTime / 1000;
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

	if (!SaveStringAsFile(szFile, strData.c_str(), strData.size(), newEncoding))
		return ERR_WRITE_FILE;

	return ERR_OK;
}

// //00:00:21,657
static LPCTSTR ReadTime(LPCTSTR szBeg, DWORD &dwTime)
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
	CTextFile	file;

	enum Parse_State
	{
		PS_NUMBER,
		PS_TIME,
		PS_TXT,
		PS_NEWLINE,
	};
	Parse_State	state;
	DWORD		dwBegTime, dwEndTime;

	int nRet = file.Open(m_pMLData->GetLyricsFileName(), CTextFile::OM_READ, m_pMLData->GetEncodingIndex());
	if (nRet != ERR_OK)
		nRet;

	state = PS_NUMBER;
	tstring line;
	while (file.ReadLine(line))
	{
		TrimStr(line, _T("\r\n"));

		LPCTSTR szBeg = line.c_str();
		if (state == PS_NUMBER)
		{
			while (IsDigit(*szBeg))
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

			if (_tcsncmp(szBeg, _T(" --> "), 5) != 0)
				continue;
			szBeg += 5;

			szBeg = ReadTime(szBeg, dwEndTime);
			if (szBeg == NULL)
				continue;

			if (*szBeg != '\0')
				continue;

			state = PS_TXT;
			continue;
		}
		else if (state == PS_TXT)
		{
			if (line.empty())
			{
				state = PS_NUMBER;
				continue;
			}

			LyricsLine *pLine = NewLyricsLine(dwBegTime, dwEndTime);
			pLine->AppendPiece(dwBegTime, dwEndTime,
				line.c_str(), line.size(), FALSE, FALSE);

			m_pMLData->AddLineByTime(pLine);
		}
	}

	return ERR_OK;
}

LYRICS_CONTENT_TYPE CSrtParser::GetLyrContentType()
{
	return LCT_LRC;
}

BOOL CSrtParser::LyricsLineToText(LyricsLine *pLine, tstringex &strBuff)
{
	strBuff.clear();
	if (!pLine->bLyricsLine)
		return false;

	if (pLine->IsTempLine())
		return false;

	for (int i = 0; i < (int)pLine->vFrags.size(); i++)
		strBuff += pLine->vFrags[i]->szLyric;

	strBuff +=  _T("\r\n");

	return TRUE;
}
