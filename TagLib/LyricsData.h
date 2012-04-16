#pragma once

#define TEMP_TIME		-1

struct LyricsPiece
{
	bool				bTempBegTime, bTempEndTime;
	int					nBegTime;
	int					nEndTime;
	int16				nDrawWidth;		// The width of lyrics in Graph
	int16				nLen;			// length of szLyric
	TCHAR				szLyric[1];

	bool IsTempBegTime() { return bTempBegTime; }

};

typedef vector<LyricsPiece *>		VecLyricsPiece;

class LyricsLine
{
public:
	bool			bAdvertise;
	bool			bTempLine;
	bool			bLyricsLine;

	// For lyrics line:
	VecLyricsPiece	vFrags;
	int				nBegTime;
	int				nEndTime;

	// For Non-lyrics line:
	TCHAR			*szContent;		// content of the row

	LyricsLine();

	~LyricsLine();

	LyricsPiece *AppendPiece(int nBegTime, int nEndTime, 
		LPCTSTR szLyrics, int nLen, bool bTempBegTime, bool bTempEndTime);

	void SetContent(LPCTSTR szContent, int nLen);

	bool IsTempBegTime()
	{
		if (vFrags.size())
			return vFrags[0]->IsTempBegTime();

		return false;
	}

	bool IsAdvertise() { return bAdvertise; }

	bool IsTempLine() { return bTempLine; }
};

class CLyricsLines : public vector<LyricsLine *>
{
public:
	CLyricsLines(bool bAutoFreeItem = true) { m_bAutoFreeItem = bAutoFreeItem; }

	virtual ~CLyricsLines() { clear(); }

	void clear();

	void ClearDrawContextWidth();

protected:
	bool		m_bAutoFreeItem;

};

LyricsLine *NewLyricsLine(int nBegTime, int nEndTime, 
						  LPCTSTR szContent = NULL, int nLen = 0, bool bLyricsLine = true);

LyricsLine *DuplicateLyricsLine(LyricsLine *pLineSrc);
