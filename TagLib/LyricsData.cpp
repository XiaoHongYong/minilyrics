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
	LyricsPiece *pPiece;

	pPiece = (LyricsPiece *)new BYTE[sizeof(LyricsPiece) + nLen * sizeof(TCHAR)];

	pPiece->nBegTime = nBegTime;
	pPiece->nEndTime = nEndTime;
	pPiece->nDrawWidth = 0;
	_tcscpy_safe(pPiece->szLyric, nLen + 1, szLyrics);
	pPiece->nLen = nLen;
	pPiece->bTempBegTime = bTempBegTime;
	pPiece->bTempEndTime = bTempEndTime;

	vFrags.push_back(pPiece);

	return pPiece;
}

void LyricsLine::SetContent(LPCTSTR szContent, int nLen)
{
	if (this->szContent != NULL)
		delete[] this->szContent;

	this->szContent = new TCHAR[nLen + 1];
	_tcsncpysz_safe(this->szContent, nLen + 1, szContent, nLen);
}

LyricsLine *NewLyricsLine(int nBegTime, int nEndTime, 
						  LPCTSTR szContent, int nLen, bool bLyricsLine)
{
	LyricsLine *pLine;

	pLine = new LyricsLine;
	pLine->nBegTime = nBegTime;
	pLine->nEndTime = nEndTime;
	pLine->bLyricsLine = bLyricsLine;

	if (szContent)
		pLine->SetContent(szContent, nLen);

	return pLine;
}

LyricsLine *DuplicateLyricsLine(LyricsLine *pLineSrc)
{
	LyricsLine *pLine = new LyricsLine;
	pLine->nBegTime = pLineSrc->nBegTime;
	pLine->nEndTime = pLineSrc->nEndTime;
	pLine->bLyricsLine = pLineSrc->bLyricsLine;
	pLine->bTempLine = pLineSrc->bTempLine;
	pLine->bAdvertise = pLineSrc->bAdvertise;

	if (pLineSrc->szContent)
	{
		int		n = _tcslen(pLineSrc->szContent) + 1;
		pLine->szContent = new TCHAR[n];
		_tcscpy_safe(pLine->szContent, n, pLineSrc->szContent);
	}

	for (int i = 0; i < (int)pLineSrc->vFrags.size(); i++)
	{
		LyricsPiece *pPieceSrc = pLineSrc->vFrags[i];

		int len = sizeof(LyricsPiece) + pPieceSrc->nLen * sizeof(TCHAR);
		LyricsPiece *pPiece = (LyricsPiece *)new BYTE[len];
		memcpy(pPiece, pPieceSrc, len);

		pLine->vFrags.push_back(pPiece);
	}

	return pLine;
}

//////////////////////////////////////////////////////////////////////////

void CLyricsLines::clear()
{
	if (m_bAutoFreeItem)
	{
		for (iterator it = begin(); it != end(); ++it)
		{
			LyricsLine *pLine = *it;
			delete pLine;
		}
	}
	vector<LyricsLine *>::clear();
}

void CLyricsLines::ClearDrawContextWidth()
{
	for (iterator it = begin(); it != end(); ++it)
	{
		LyricsLine *pLine = *it;
		for (int k = 0; k < (int)pLine->vFrags.size(); k++)
			pLine->vFrags[k]->nDrawWidth = 0;
	}
}

int CLyricsLines::GetCountWithSameTimeStamps(int nLine)
{
	if (nLine < 0 || nLine >= size())
		return 0;

	LyricsLine *pLine = at(nLine);
	for (int i = nLine; i < size(); i++)
	{
		LyricsLine *p = at(i);
		if (pLine->nBegTime != p->nBegTime || pLine->nEndTime != p->nEndTime)
			return i - nLine;
	}

	return size() - nLine;
}
