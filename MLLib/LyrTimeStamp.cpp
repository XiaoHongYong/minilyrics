#include "stdafx.h"
#include "MLData.h"
#include "LyrTimeStamp.h"

#define HEADER_TIME_STAMP_V0	'@'
#define CHAR_NO_TIME_STAMP		'_'
#define CHAR_MAX_TIME_VALUE		'^'
#define CHAR_NEGATIVE_TIME_VALUE	'-'
#define TIME_STAMP_UNIT			100
#define MAX_DURATION			64

bool CLyrTimeStamp::IsTimeStamps(LPCTSTR szTimeStamps)
{
	return szTimeStamps[0] == HEADER_TIME_STAMP_V0;
}

const char *GetBase64Set();

void EncodeTimeStamps(int duration, tstring &timeStamps)
{
	const char *charSet = GetBase64Set();
	if (duration < 0)
	{
		timeStamps += CHAR_NEGATIVE_TIME_VALUE;
		duration = -duration;
	}
	// duration can't be too large.
	if (duration > 10 * 60 * 1000)
		duration = 6 * 1000;

	duration /= TIME_STAMP_UNIT;

	while (duration >= 0)
	{
		if (duration >= MAX_DURATION)
			timeStamps += CHAR_MAX_TIME_VALUE;
		else
			timeStamps += charSet[duration];

		duration -= MAX_DURATION;
	}
}

LPCTSTR DecodeTimeStamps(LPCTSTR szTimeStamps, int &duration)
{
	ASSERT(*szTimeStamps != CHAR_NO_TIME_STAMP);

	duration = 0;

	bool bNegative = (*szTimeStamps == CHAR_NEGATIVE_TIME_VALUE);
	if (bNegative)
		szTimeStamps++;

	while (*szTimeStamps == CHAR_MAX_TIME_VALUE)
	{
		duration += MAX_DURATION;
		szTimeStamps++;
	}

	const char *charSet = GetBase64Set();
	int i = 0;
	for (i = 0; i < MAX_DURATION; i++)
	{
		if (charSet[i] == *szTimeStamps)
		{
			duration += i;
			break;
		}
	}
	ASSERT(i < MAX_DURATION);

	duration *= TIME_STAMP_UNIT;

	if (bNegative)
		duration = -duration;

	return szTimeStamps + 1;
}

bool CLyrTimeStamp::Parse(LPCTSTR szTimeStamps, CMLData *pMLData)
{
	// Header
	if (szTimeStamps[0] != HEADER_TIME_STAMP_V0)
		return false;
	szTimeStamps++;

	// Offset time
	int nTimeOffset = 0;
	szTimeStamps = DecodeTimeStamps(szTimeStamps, nTimeOffset);

	// Time of every line
	int nTimeAboveLine = 0;
	CLyricsLines	&lines = pMLData->GetRawLyrics();
	for (CLyricsLines::iterator it = lines.begin(); it != lines.end() && *szTimeStamps;)
	{
		LyricsLine *pRow = *it;
		LyricsPiece *pFrag = pRow->vFrags[0];
		ASSERT(pRow->bLyricsLine && pFrag->IsTempBegTime());

		if (*szTimeStamps == CHAR_NO_TIME_STAMP)
		{
			szTimeStamps++;
			// Just remove this line from RawLyrics (will be kept in m_arrFileRows).
			it = lines.erase(it);
			pRow->bLyricsLine = false;
			pRow->SetContent(pFrag->szLyric, _tcslen(pFrag->szLyric));
		}
		else
		{
			int duration;
			szTimeStamps = DecodeTimeStamps(szTimeStamps, duration);
			pFrag->nBegTime = pRow->nBegTime = nTimeAboveLine + duration;
			pFrag->bTempBegTime = false;

			nTimeAboveLine = pRow->nBegTime;
			 ++it;
		}
	}

	pMLData->Properties().SetTxtLyrScrollEvents(_T(""));
	pMLData->Properties().SetOffsetTime(nTimeOffset);

	return true;
}

void CLyrTimeStamp::ToString(tstring &strTimeStamps, int nOffsetTime, CLyricsLines &lyrLines)
{
	// Header
	strTimeStamps.clear();
	strTimeStamps += HEADER_TIME_STAMP_V0;

	// Offset time
	EncodeTimeStamps(nOffsetTime, strTimeStamps);

	// Time of every line
	int nTimeAboveLine = 0;
	for (uint i = 0; i < lyrLines.size(); i++)
	{
		LyricsLine *pRow = lyrLines[i];
		if (pRow->bLyricsLine)
		{
			int duration = pRow->nBegTime - nTimeAboveLine + TIME_STAMP_UNIT / 2;
			EncodeTimeStamps(duration, strTimeStamps);
			// Round the above line.
			nTimeAboveLine += duration / TIME_STAMP_UNIT * TIME_STAMP_UNIT;
		}
		else
			strTimeStamps += CHAR_NO_TIME_STAMP;
	}
}
