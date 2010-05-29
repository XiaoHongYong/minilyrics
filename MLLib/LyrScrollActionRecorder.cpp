#include "stdafx.h"
#include "MLData.h"
#include "LyrScrollActionRecorder.h"

//////////////////////////////////////////////////////////////////////

#define AUTO_SCROLL_LYR_HEADERV0	_T("#")
#define AUTO_SCROLL_LYR_DATA_POSV0	3

/*
FORMAT of Lyrics Scrolling Events, version 0.0:

All data are base64.

1.	VERSION   CRC        SCROLL_EVENTS   SCROLL_EVENTS  .....
(1char)  (2 chars)    (n chars)

2.  SCROLL_EVENTS = TIME_DISTANCE_WIDTH   TIME        DISTANCE
(timeWidth)  (distanceWidth)

3. TIME_DISTANCE_WIDTH = timeWidth + distanceWidth * 10;

*/

#define EMPTY_LINE_CHAR_COUNT		5

#define MAX_SPEED					((25.0 / 1000))
#define MIN_SPEED					(1.0 / 1000)
#define OPTIMIZE_TIME_BLOCK			(10 * 1000)

const char *GetBase64Set();

void IntToBase64(int n, tstring &str)
{
	str.clear();

	TCHAR		szBuff[10];
	szBuff[CountOf(szBuff) - 1] = '\0';
	int i = CountOf(szBuff) - 1;
	const char *SZ_BASE64_SET = GetBase64Set();

	do
	{
		i--;
		szBuff[i] = SZ_BASE64_SET[n % 64];
		n /= 64;
	}
	while (n > 0);

	str = szBuff + i;
}

uint32 IntFromBase64(LPCTSTR str, int nLen)
{
	uint	n = 0;
	const char *SZ_BASE64_SET = GetBase64Set();
	for (; nLen > 0; nLen--)
	{
		LPCSTR p = strchr(SZ_BASE64_SET, *str);
		ASSERT(p);
		if (p)
			n = n * 64 + (int)(p - SZ_BASE64_SET);
		str++;
	}
	return n;
}

byte CRC8(LPCTSTR str)
{
	byte crc = 0;
	while (*str)
	{
		crc += (byte)(*str);
		str++;
	}

	return crc;
}

#define MIN_DISTANCE_LINE		4
inline int CharCountToCharDistance(int nLen)
{
	if (nLen >= MIN_DISTANCE_LINE)
		return nLen;
	else
		return MIN_DISTANCE_LINE;
}

CLyrScrollActionRecorder::CLyrScrollActionRecorder()
{
	m_nLyrCharCount = -1;
	m_bHasActions = false;
}

void CLyrScrollActionRecorder::InitCreateAutoEvents(VLyrLines &lyrLines)
{
	CLyrScrollActionRecorder::Close();

	m_listEvents.push_back(ScrollTo(0, 0));
	m_nLyrCharCount = CountLyrChar(lyrLines);

	int nStartTime = 0;
	for (int i = 0; i < (int)lyrLines.size(); i++)
	{
		lyrLines[i]->nBegTime = nStartTime;
		nStartTime = MAX_TIME;
		lyrLines[i]->nEndTime = nStartTime;
	}

	/*	// Max speed test, Turn this on, and open a text file, will see how fast lyrics scrolling...
	{
	int		nCurEvent = 0;
	int		nCurDistance = 0;
	int		nLastTime = 0;

	for (int i = 0; i < (int)lyrLines.size(); i++)
	{
	PML_ROW pRow = lyrLines[i];
	ASSERT(pRow->vFrags.size() == 1);
	if (pRow->vFrags.size() != 1)
	continue;

	LPLYRIC_FRAG pFrag = pRow->vFrags[0];
	pFrag->dwAttrib |= LSA_TEMP_TIME_BEG | LSA_TEMP_TIME_END;
	pFrag->nBegTime = nLastTime;
	nCurDistance += CharCountToCharDistance(pFrag->nLen);

	pFrag->nEndTime = nCurDistance / MAX_SPEED;

	pRow->nBegTime = pFrag->nBegTime;
	nLastTime = pRow->nEndTime = pFrag->nEndTime;
	}
	}*/
}

void CLyrScrollActionRecorder::Close()
{
	m_bHasActions = false;
	m_listEvents.clear();
	m_nLyrCharCount = -1;
}

bool CLyrScrollActionRecorder::IsEventTag(LPCTSTR str)
{
	return _tcsncmp(str, AUTO_SCROLL_LYR_HEADERV0, _tcslen(AUTO_SCROLL_LYR_HEADERV0)) == 0;
}

tstring CLyrScrollActionRecorder::EventsDataToString()
{
	// CStrPrintf	strEvents;
	int nLastTime = 0, nLastDistance = 0;

	// If events too less, don't store these actions.
	if (m_listEvents.size() <= 6 || !m_bHasActions)
		return _T("");

	tstring strData = AUTO_SCROLL_LYR_HEADERV0 _T("AA"); // "AA"(base64) = "00"(16#)

	tstring		strTime, strDistance, strEventWidth;
	for (int i = 0; i < (int)m_listEvents.size(); i++)
	{
		IntToBase64(m_listEvents[i].nTime - nLastTime, strTime);
		IntToBase64(m_listEvents[i].distance - nLastDistance, strDistance);
		int nEventWidth = strTime.size() + strDistance.size() * 10;
		ASSERT(nEventWidth < 60);
		IntToBase64(nEventWidth, strEventWidth);

		strData.append(strEventWidth.c_str());
		strData.append(strTime.c_str());
		strData.append(strDistance.c_str());

		nLastTime = m_listEvents[i].nTime;
		nLastDistance = m_listEvents[i].distance;
	}

	byte crc = CRC8(strData.c_str() + AUTO_SCROLL_LYR_DATA_POSV0);
	IntToBase64(crc, strTime);
	for (int k = 0; k < (int)strTime.size(); k++)
		strData[AUTO_SCROLL_LYR_DATA_POSV0 - strTime.size() + k] = strTime[k];

	return strData;
}

void CLyrScrollActionRecorder::EventsDataFromString(LPCTSTR szEventsData, VLyrLines &lyrLines)
{
	InitCreateAutoEvents(lyrLines);

	if (_tcsncmp(szEventsData, AUTO_SCROLL_LYR_HEADERV0, _tcslen(AUTO_SCROLL_LYR_HEADERV0)) != 0)
		return;

	m_bHasActions = true;

	byte crc = CRC8(szEventsData + AUTO_SCROLL_LYR_DATA_POSV0);
	byte crcOrg = IntFromBase64(szEventsData + _tcslen(AUTO_SCROLL_LYR_HEADERV0), 2);
	if (crc != crcOrg)
	{
		ERRLOG0(_T("FAILED to Verify lyrics scroll event crc value."));
		return;
	}

	int		nCurTime = 0, nCurDistance = 0;
	LPCTSTR p = szEventsData + AUTO_SCROLL_LYR_DATA_POSV0;
	LPCTSTR pEnd = p + _tcslen(p);

	while (p < pEnd)
	{
		int nEventWidth = IntFromBase64(p, 1);
		p++;
		if (p >= pEnd || nEventWidth <= 0 || nEventWidth >= 60)
			break;
		byte widthTime = nEventWidth % 10;
		byte widthDistance = nEventWidth / 10;

		if (p + widthTime + widthDistance > pEnd)
			break;

		nCurTime += IntFromBase64(p, widthTime);
		p += widthTime;

		nCurDistance += IntFromBase64(p, widthDistance);
		p += widthDistance;

		AddScrollTo(lyrLines, nCurDistance, nCurTime, false);
	}

	UpdateTimeTagByEvents(lyrLines);
}

void CLyrScrollActionRecorder::ScrollToLine(VLyrLines &lyrLines, int nLine, int nTime, bool bUpdateTimeTag)
{
	m_bHasActions = true;

	AddScrollTo(lyrLines, GetDistanceOfLine(lyrLines, nLine), nTime, bUpdateTimeTag);
}

void CLyrScrollActionRecorder::AddScrollTo(VLyrLines &lyrLines, int nPos, int nTime, bool bUpdateTimeTag)
{
	ASSERT(m_listEvents.size() >= 1);
	const int RECENT_ACTION = 500;

	// Remove events conflicted with inserted item.
	// nTime + RECENT_ACTION < Event's within < nTime + RECENT_ACTION
	// m_listEvents[0] is fixed event, don't remove it.
	for (int k = 1; k < (int)m_listEvents.size(); )
	{
		if (m_listEvents[k].distance == nPos)
		{
			DBGLOG1(_T("Remove distance conflict event: position: %d"), nPos);
			m_listEvents.erase(m_listEvents.begin() + k);
			continue;
		}

		if (m_listEvents[k].nTime <= nTime - RECENT_ACTION)
		{
			k++;
			continue;
		}
		else if (m_listEvents[k].nTime >= nTime + RECENT_ACTION)
			break;
		else
		{
			DBGLOG2(_T("Remove time conflict event: %d, time: %d"), k, m_listEvents[k].nTime);
			m_listEvents.erase(m_listEvents.begin() + k);
		}
	}

	for (int k = 1; k < (int)m_listEvents.size(); )
	{
		if ((m_listEvents[k].distance <= nPos && m_listEvents[k].nTime >= nTime)
			|| (m_listEvents[k].distance >= nPos && m_listEvents[k].nTime <= nTime))
		{
			DBGLOG2(_T("Remove time or distance conflict event: %d, time: %d"), k, m_listEvents[k].nTime);
			m_listEvents.erase(m_listEvents.begin() + k);
			continue;
		}
		k++;
	}

	int i;
	for (i = 0; i < (int)m_listEvents.size(); i++)
	{
		if (nPos < m_listEvents[i].distance)
		{
			m_listEvents.insert(m_listEvents.begin() + i, ScrollTo(nPos, nTime));
			break;
		}
	}

	if (i ==  m_listEvents.size())
		m_listEvents.push_back(ScrollTo(nPos, nTime));

	if (bUpdateTimeTag)
		UpdateTimeTagByEvents(lyrLines);
}

void CLyrScrollActionRecorder::UpdateTimeTagByEvents(VLyrLines &lyrLines)
{
	if (m_nLyrCharCount == 0 || m_listEvents.empty())
		return;

	double speedlast = 0.0;
	int nTimeLast = 0;
	// Calculate the speed of every events
	for (int i = 0; i < m_listEvents.size() - 1; i++)
	{
		m_listEvents[i].speed = (m_listEvents[i + 1].distance - m_listEvents[i].distance)
			/ (double) (m_listEvents[i + 1].nTime - m_listEvents[i].nTime);

		if (i == 0)
			continue;

		if (m_listEvents[i].speed > MAX_SPEED && m_listEvents[i].distance <= EMPTY_LINE_CHAR_COUNT)
		{
			// Remove events speed too fast
			DBGLOG2(_T("Remove events too fast: speed1: %f, speed2: %f"), m_listEvents[i].speed, MAX_SPEED);
			m_listEvents.erase(m_listEvents.begin() + i);
			i -= 2;
		}
		else if (speedlast - m_listEvents[i].speed <= MIN_SPEED && speedlast - m_listEvents[i].speed >= -MIN_SPEED
			&& m_listEvents[i].nTime - nTimeLast < OPTIMIZE_TIME_BLOCK)
		{
			// Remove speed same
			DBGLOG2(_T("Remove events: speed1: %f, speed2: %f"), speedlast, m_listEvents[i].speed);
			m_listEvents.erase(m_listEvents.begin() + i);
			i -= 2;
		}
		else
		{
			nTimeLast = m_listEvents[i].nTime;
			speedlast = m_listEvents[i].speed;
		}
	}

	int		nCurEvent = 0;
	int		nCurDistance = 0;
	int		nLastTime = 0;

	for (int i = 0; i < (int)lyrLines.size(); i++)
	{
		PML_ROW pRow = lyrLines[i];
		ASSERT(pRow->vFrags.size() == 1);
		if (pRow->vFrags.size() != 1)
			continue;

		LPLYRIC_FRAG pFrag = pRow->vFrags[0];
		pFrag->dwAttrib |= LSA_TEMP_TIME_BEG | LSA_TEMP_TIME_END;
		pFrag->nBegTime = nLastTime;
		nCurDistance += CharCountToCharDistance(pFrag->nLen);

		while (nCurEvent < (int)m_listEvents.size() - 1
			&& nCurDistance >= m_listEvents[nCurEvent].distance)
			nCurEvent++;
		if (nCurEvent > 0 && m_listEvents[nCurEvent].distance >= nCurDistance)
			nCurEvent--;
		if (nCurEvent == m_listEvents.size() - 1)
		{
			// in last event, set end time as MAX_TIME.
			pFrag->nEndTime = MAX_TIME;
		}
		else
		{
			ScrollTo *pEvent = &(m_listEvents[nCurEvent]);
			ASSERT(nCurDistance >= pEvent->distance);

			if (pEvent->speed > 0)
				pFrag->nEndTime = int(pEvent->nTime + (nCurDistance - pEvent->distance) / pEvent->speed);
			else
				pFrag->nEndTime = pEvent->nTime;
		}

		pRow->nBegTime = pFrag->nBegTime;
		nLastTime = pRow->nEndTime = pFrag->nEndTime;
	}
}

int CLyrScrollActionRecorder::GetDistanceOfLine(VLyrLines &lyrLines, int nLine)
{
	int		d = 0;
	if (nLine > (int)lyrLines.size())
		nLine = (int)lyrLines.size();

	// count all the characters.
	int		nCharCount = 0;
	for (int i = 0; i < nLine; i++)
	{
		PML_ROW pRow = lyrLines[i];
		ASSERT(pRow->nRowType == ML_ROW_LYRICS);
		ASSERT(pRow->vFrags.size() == 1);
		if (pRow->vFrags.size() != 1)
			continue;
		nCharCount += CharCountToCharDistance(pRow->vFrags[0]->nLen);
	}

	return nCharCount;
}

int CLyrScrollActionRecorder::CountLyrChar(VLyrLines &lyrLines)
{
	return GetDistanceOfLine(lyrLines, lyrLines.size());
}


#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

IMPLEMENT_CPPUNIT_TEST_REG(CLyrScrollActionRecorder)

class CTestCaseLyrScrollActionRecorder : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseLyrScrollActionRecorder);
	CPPUNIT_TEST(TestAddLyrEvent);
	CPPUNIT_TEST_SUITE_END();

protected:

public:
	void setUp()
	{
	}
	void tearDown() 
	{
	}

protected:
	void TestAddLyrEvent()
	{
		/*		CLyrScrollActionRecorder	asl;
		CMLyrLines		lyrLines;

		LPCTSTR		SZ_TXT = _T("abcdefghijklmnopqrstuvw");

		for (int i = 0; i < 10; i++)
		{
		PML_ROW	pRow = new ML_ROW;
		int	nLen = rand() % 20;
		PLYRIC_FRAG pFrag = (PLYRIC_FRAG)new BYTE[sizeof(LYRIC_FRAG) + nLen * sizeof(TCHAR)];
		pFrag->nBegTime = pFrag->nEndTime = -1;
		pFrag->dwAttrib = 0;
		_tcsncpy_safe(pFrag->szLyric, nLen + 1, SZ_TXT, nLen);

		pRow->vFrags.push_back(pFrag);
		pRow->nRowType = ML_ROW_LYRICS;
		lyrLines.push_back(pRow);
		}

		asl.AddPlayLineAction(0, 5000, lyrLines);

		g_LyricData.SetPlayElapsedTime(10 * 1000);
		asl.AddActionAtCurrentPos(CLyrScrollActionRecorder::A_PAUSE, lyrLines);

		g_LyricData.SetPlayElapsedTime(12 * 1000);
		asl.AddActionAtCurrentPos(CLyrScrollActionRecorder::A_PLAY, lyrLines);

		g_LyricData.SetPlayElapsedTime(14 * 1000);
		asl.AddActionAtCurrentPos(CLyrScrollActionRecorder::A_SPEED_UP, lyrLines);

		g_LyricData.SetPlayElapsedTime(25 * 1000);
		asl.AddActionAtCurrentPos(CLyrScrollActionRecorder::A_PLAY, lyrLines);

		g_LyricData.SetPlayElapsedTime(28 * 1000);
		asl.AddActionAtCurrentPos(CLyrScrollActionRecorder::A_PLAY, lyrLines);

		/*		asl.AddPlayLineAction(3, 9000, lyrLines);
		asl.AddPlayLineAction(5, 10 * 1000, lyrLines);
		asl.AddPlayLineAction(7, 15 * 1000, lyrLines);
		asl.AddPlayLineAction(9, 25 * 1000, lyrLines);
		*/
		// CPPUNIT_ASSERT(_tcscmp(szStr, _T("abcdef1213")) == 0);
		// CPPUNIT_FAIL_T(CStrPrintf(_T("StrSplit(Set) Test, case(Can't find): %d, %s"), i, strResult[i]).c_str());
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseLyrScrollActionRecorder);


#endif // _CPPUNIT_TEST
