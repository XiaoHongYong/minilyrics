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
int CSncParser::SaveAsFile(LPCTSTR szFile, FILE_ENCODING fileEncoding)
{
	int			nCount;
	tstringex	strData;
	tstringex	strBuff;

	nCount = (int)m_pMLData->m_arrFileRows.size();
	for (int i = 0; i < nCount; i++)
	{
		PML_ROW		pRow;

		pRow = m_pMLData->m_arrFileRows[i];

		if (pRow)
		{
			// save row.
			if (LyricsRowToText(pRow, strBuff))
			{
				strData += strBuff;
			}
		}
	}

	if (!SaveStringAsFile(szFile, strData.c_str(), strData.size(), FE_ANSI, m_pMLData->GetEncodingIndex()))
		return ERR_WRITE_FILE;

	return ERR_OK;
}

//	snc 文件的格式：
//		⑩00010450⑿
//          MMSS
//		我躲在车里
BOOL CSncParser::LyricsRowToText(PML_ROW pRow, tstringex &strBuff)
{
	strBuff.clear();

	switch (pRow->nRowType)
	{
	case ML_ROW_UNKNOW:		// 不能识别的行
		// lstrcpy(szBuff, pRow->szContent);
		// lstrcat(szBuff, "\r\n");
		break;
	case ML_ROW_NULL:		// 空行
		// lstrcat(szBuff, "\r\n");
		break;
	case ML_ROW_LYRICS:		// 歌词行
		{
			int		nMin, nSec, n10Ms;
			int		nMs;
			int		nLen;

			nMs = pRow->nBegTime + m_pMLData->GetOffsetTime();
			n10Ms = (nMs / 10) % 100;
			nSec = (nMs / 1000) % 60;
			nMin = nMs / (1000 * 60);
			// _stprintf(szBuff, _T("⑩%04d%02d%02d⑿\r\n"), nMin, nSec, n10Ms);
			strBuff.reserve(100);
			nLen = _stprintf_s(strBuff.data(), strBuff.capacity(), _T("⑩%04d%02d%02d⑿\r\n"), nMin, nSec, n10Ms);
			strBuff.setSize(nLen);

			int			nCount;
			tstringex	strLyrics;

			nCount = (int)pRow->vFrags.size();
			for (int i = 0; i < nCount; i++)
			{
				// 去掉插入的注册信息，广告信息等
				if (IsTempLyricsFrag(pRow->vFrags[i]->dwAttrib))
				{
					strBuff.clear();
					return TRUE;
				}

				strLyrics += pRow->vFrags[i]->szLyric;
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
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

// 分析歌词文件，并且存储到 pMLData 中
int CSncParser::Parse(bool bForceUseEncoding)
{
	return ERR_FALSE;
}
