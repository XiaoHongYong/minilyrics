// SncParser.cpp: implementation of the CSncParser class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLLib.h"
#include "SncParser.h"
#include "HelperFun.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSncParser::CSncParser(CMLData *pMLData) : CLyricsParser(pMLData)
{

}

CSncParser::~CSncParser()
{

}

// COMMENTS:
//	保存为*.snc文件
//	snc 文件的格式：
//		⑩00003210⑿
//		我躲在车里
int CSncParser::SaveAsFile(LPCTSTR szFile, CHAR_ENCODING newEncoding)
{
	int			nCount;
	tstringex	strData;
	tstringex	strBuff;

	nCount = (int)m_pMLData->m_arrFileLines.size();
	for (int i = 0; i < nCount; i++)
	{
		LyricsLine *pLine;

		pLine = m_pMLData->m_arrFileLines[i];

		if (pLine)
		{
			// save row.
			if (LyricsLineToText(pLine, strBuff))
			{
				strData += strBuff;
			}
		}
	}

	if (!SaveStringAsFile(szFile, strData.c_str(), strData.size(), newEncoding))
		return ERR_WRITE_FILE;

	return ERR_OK;
}

//	snc 文件的格式：
//		⑩00010450⑿
//          MMSS
//		我躲在车里
BOOL CSncParser::LyricsLineToText(LyricsLine *pLine, tstringex &strBuff)
{
	strBuff.clear();

	if (!pLine->bLyricsLine)
		return false;

	if (pLine->IsTempLine())
		return false;

	int		nMin, nSec, n10Ms;
	int		nMs;
	int		nLen;

	nMs = pLine->nBegTime + m_pMLData->GetOffsetTime();
	n10Ms = (nMs / 10) % 100;
	nSec = (nMs / 1000) % 60;
	nMin = nMs / (1000 * 60);
	// _stprintf(szBuff, _T("⑩%04d%02d%02d⑿\r\n"), nMin, nSec, n10Ms);
	strBuff.reserve(100);
	nLen = _stprintf_s(strBuff.data(), strBuff.capacity(), _T("⑩%04d%02d%02d⑿\r\n"), nMin, nSec, n10Ms);
	strBuff.setSize(nLen);

	int			nCount;
	tstringex	strLyrics;

	nCount = (int)pLine->vFrags.size();
	for (int i = 0; i < nCount; i++)
	{
		strLyrics += pLine->vFrags[i]->szLyric;
	}
	strLyrics.trim();

	// 如果本行歌词为空，则不保存本行
	if (strLyrics.empty())
		strBuff.clear();
	else
	{
		strBuff += strLyrics;
		strBuff += _T("\r\n");
	}

	return TRUE;
}
