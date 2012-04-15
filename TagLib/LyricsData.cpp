#include "StdAfx.h"
#include "LyricsData.h"


LyricsLine::LyricsLine()
{
	bAdvertise = false;
	bTempLine = false;
	bLyricsLine = true;
	szContent = NULL;
}

LyricsLine::~LyricsLine()
{
	if (szContent)
		delete szContent;

	for (int k = 0; k < (int)vFrags.size(); k++)
	{
		delete[] (BYTE*)vFrags[k];
	}
}

LyricsPiece *LyricsLine::AppendPiece(int nBegTime, int nEndTime, 
							LPCTSTR szLyrics, int nLen, bool bTempBegTime, bool bTempEndTime)
{
	LyricsPiece *pFrag;

	pFrag = (LyricsPiece *)new BYTE[sizeof(LyricsPiece) + nLen * sizeof(TCHAR)];

	pFrag->nBegTime = nBegTime;
	pFrag->nEndTime = nEndTime;
	pFrag->nDrawWidth = 0;
	_tcscpy_safe(pFrag->szLyric, nLen + 1, szLyrics);
	pFrag->nLen = nLen;
	pFrag->bTempBegTime = bTempBegTime;
	pFrag->bTempEndTime = bTempEndTime;

	vFrags.push_back(pFrag);

	return pFrag;
}

LyricsLine *NewLyricsLine(int nBegTime, int nEndTime, 
						  LPCTSTR szContent, int nLen, bool bLyricsLine)
{
	LyricsLine *pRow;

	pRow = new LyricsLine;
	pRow->nBegTime = nBegTime;
	pRow->nEndTime = nEndTime;
	pRow->bLyricsLine = bLyricsLine;

	if (szContent)
		pRow->SetContent(szContent, nLen);

	return pRow;
}

void LyricsLine::SetContent(LPCTSTR szContent, int nLen)
{
	if (this->szContent != NULL)
		delete[] this->szContent;

	this->szContent = new TCHAR[nLen + 1];
	_tcsncpysz_safe(this->szContent, nLen + 1, szContent, nLen);
}

//////////////////////////////////////////////////////////////////////////

void CLyricsLines::clear()
{
	if (m_bAutoFreeItem)
	{
		for (iterator it = begin(); it != end(); ++it)
		{
			LyricsLine *pRow = *it;
			delete pRow;
		}
	}
	vector<LyricsLine *>::clear();
}

void CLyricsLines::ClearDrawContextWidth()
{
	for (iterator it = begin(); it != end(); ++it)
	{
		LyricsLine *pRow = *it;
		for (int k = 0; k < (int)pRow->vFrags.size(); k++)
			pRow->vFrags[k]->nDrawWidth = 0;
	}
}
