// MPEventHost.cpp: implementation of the CMPEventHost class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MPEventsDispatcher.h"
#ifdef _MPLAYER
#include "MPVisAdapter.h"
#endif

/*

#ifndef _MPLAYER
#define MYWM_WVSLYRICS		(WM_USER + 11)
#define WVSLYRICS_GETDATA_A	1
#define WVSLYRICS_GETDATA_W	2

#define LYRICS_LEN_MAX			255
typedef struct WVS_LYRICS_DATA_A
{
	typedef struct LYRICS_ROW_A
	{
		int			nRow;					// 行数
		DWORD		dwLyricsShowAttrb;		// 该行歌词的显示属性
		float		fKaraokePos;			// 在卡拉Ok显示方式时有效，表示显示位置比例
		char		szLyrics[LYRICS_LEN_MAX];			// 歌词
	}LYRICS_ROW;
	DWORD		dwLen;			// 该结构的长度
	DWORD		dwVersion;		// 该结构的版本
	int			nCurrentRow;	// 当前歌词行
	LYRICS_ROW_A	LyricsRow[2];	// 歌词行s
}WVS_LYRICS_DATA_A;

typedef struct WVS_LYRICS_DATA_W
{
	typedef struct LYRICS_ROW_W
	{
		int			nRow;					// 行数
		DWORD		dwLyricsShowAttrb;		// 该行歌词的显示属性
		float		fKaraokePos;			// 在卡拉Ok显示方式时有效，表示显示位置比例
		WCHAR		szLyrics[LYRICS_LEN_MAX];			// 歌词
	}LYRICS_ROW_W;
	DWORD		dwLen;			// 该结构的长度
	DWORD		dwVersion;		// 该结构的版本
	int			nCurrentRow;	// 当前歌词行
	LYRICS_ROW_W	LyricsRow[2];	// 歌词行s
}WVS_LYRICS_DATA_W;
#endif

#ifndef _MPLAYER
	LRESULT OnWVSGetLyrics(WVS_LYRICS_DATA_A *pDataA, WVS_LYRICS_DATA_W *pDataW);
	LRESULT OnWVSLyrics(WPARAM wParam, LPARAM lParam);
#endif

int CMPMsgWnd::Init()
{
#if !defined(_MPLAYER) && defined(_WIN32_DESKTOP)
	// for winamp avs plugin to display lyrics.
	TCHAR		szVar[64];
	wsprintf(szVar, _T("%x"), m_hWnd);
	SetEnvironmentVariable(_T("MiniLyricsWnd"), szVar);
#endif

	return ERR_OK;
}


#ifdef _WIN32
LRESULT CMPMsgWnd::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case MPWM_QUIT:
		{
//#ifdef _MPLAYER
			PostQuitMessage(0);
//#else
//			CMPlayerAppBase::GetInstance()->Quit();
// #endif
		}
		return 0;
#ifndef _MPLAYER
	case MYWM_WVSLYRICS:
		return OnWVSLyrics(wParam, lParam);
#endif
	}

	return CBaseWnd::WndProc(message, wParam, lParam);
}

#ifndef _MPLAYER
//
// 响应WVS中的消息，取得歌词……
//
LRESULT CMPMsgWnd::OnWVSLyrics(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case WVSLYRICS_GETDATA_A:
		{
			return OnWVSGetLyrics((WVS_LYRICS_DATA_A*)lParam, NULL);
		}
		break;
	case WVSLYRICS_GETDATA_W:
		{
			return OnWVSGetLyrics(NULL, (WVS_LYRICS_DATA_W*)lParam);
		}
		break;
	default:
		return 0;
		break;
	}
	return 0;
}

//
// 响应WVS中的消息，取得歌词
//
LRESULT CMPMsgWnd::OnWVSGetLyrics(WVS_LYRICS_DATA_A *pDataA, WVS_LYRICS_DATA_W *pDataW)
{
	if (pDataA == NULL && pDataW == NULL)
		return 0;

	if ((pDataA && pDataA->dwVersion == 1) || (pDataW && pDataW->dwVersion == 1))
	{
		PML_ROW		pLyricRow;
		int			nCurSingRow;
		VLyrLines	&lyrLines = g_LyricData.GetRawLyrics();

		nCurSingRow = g_LyricData.GetCurPlayLine(lyrLines);
		if (nCurSingRow == -1)
		{
			// 没有歌词？！
			if (pDataA)
			{
				pDataA->nCurrentRow = -1;
				pDataA->LyricsRow[0].nRow = -1;
				pDataA->LyricsRow[1].nRow = -1;
			}
			else
			{
				pDataW->nCurrentRow = -1;
				pDataW->LyricsRow[0].nRow = -1;
				pDataW->LyricsRow[1].nRow = -1;
			}
			return 1;
		}
		pLyricRow = lyrLines[nCurSingRow];

		// 偶数行显示在上面，奇数行显示在下面
		int		nRow;
		tstring	strLyricsRow;

		g_LyricData.LyricsRowToText(pLyricRow, strLyricsRow);

		nRow = nCurSingRow;
		if (pDataA)
		{
			pDataA->nCurrentRow = nCurSingRow;
			pDataA->LyricsRow[nRow % 2].nRow = nRow;
			pDataA->LyricsRow[nRow % 2].fKaraokePos = 0;
			pDataA->LyricsRow[nRow % 2].dwLyricsShowAttrb = LSA_SHOW_SIMPLE;
			stringex	str;
			ConvertStr(strLyricsRow.c_str(), strLyricsRow.size(), str, g_LyricData.Properties().GetEncodingIndex());
			strcpy_safe(pDataA->LyricsRow[nRow % 2].szLyrics, LYRICS_LEN_MAX, str.c_str());
		}
		else
		{
			pDataW->nCurrentRow = nCurSingRow;
			pDataW->LyricsRow[nRow % 2].nRow = nRow;
			pDataW->LyricsRow[nRow % 2].fKaraokePos = 0;
			pDataW->LyricsRow[nRow % 2].dwLyricsShowAttrb = LSA_SHOW_SIMPLE;
			wstringex	str;
			ConvertStr(strLyricsRow.c_str(), strLyricsRow.size(), str, g_LyricData.Properties().GetEncodingIndex());
			wcscpy_safe(pDataW->LyricsRow[nRow % 2].szLyrics, LYRICS_LEN_MAX, str.c_str());
		}

		// 如果当前歌词行的显示时间超过一半，则预显示下一行歌词
		if ( (g_LyricData.GetPlayElapsedTime() - pLyricRow->nBegTime) * 2 >= pLyricRow->nEndTime - g_LyricData.GetPlayElapsedTime())
		{
			nRow = nCurSingRow + 1;
			if (nRow >= (int)lyrLines.size())
				nRow = nCurSingRow - 1;
			if (nRow >= 0)
				pLyricRow = lyrLines[nRow];
			else
				pLyricRow = NULL;
		}
		else
		{
			//
			// 还是显示上一行
			nRow = nCurSingRow - 1;
			if (nRow < 0)
				nRow = nCurSingRow + 1;

			if (nRow >= (int)lyrLines.size())
				pLyricRow = NULL;
			else
				pLyricRow = lyrLines[nRow];
		}

		if (pLyricRow)
		{
			g_LyricData.LyricsRowToText(pLyricRow, strLyricsRow);
			if (pDataA)
			{
				pDataA->LyricsRow[nRow % 2].nRow = nRow;
				pDataA->LyricsRow[nRow % 2].fKaraokePos = 0;
				pDataA->LyricsRow[nRow % 2].dwLyricsShowAttrb = LSA_SHOW_SIMPLE;
				stringex	str;
				ConvertStr(strLyricsRow.c_str(), strLyricsRow.size(), str, g_LyricData.Properties().GetEncodingIndex());
				strcpy_safe(pDataA->LyricsRow[nRow % 2].szLyrics, LYRICS_LEN_MAX, str.c_str());
			}
			else if (pDataW)
			{
				pDataW->LyricsRow[nRow % 2].nRow = nRow;
				pDataW->LyricsRow[nRow % 2].fKaraokePos = 0;
				pDataW->LyricsRow[nRow % 2].dwLyricsShowAttrb = LSA_SHOW_SIMPLE;
				wstringex	str;
				ConvertStr(strLyricsRow.c_str(), strLyricsRow.size(), str, g_LyricData.Properties().GetEncodingIndex());
				wcscpy_safe(pDataW->LyricsRow[nRow % 2].szLyrics, LYRICS_LEN_MAX, str.c_str());
			}
		}
		else
		{
			if (pDataA)
				pDataA->LyricsRow[nRow % 2].nRow = -1;
			else
				pDataW->LyricsRow[nRow % 2].nRow = -1;
		}
	}

	return 1;
}
#endif // _MPLAYER
#endif // _WIN32

*/

//////////////////////////////////////////////////////////////////////////

void CMPEventsDispatcher::Quit()
{
	CEventsDispatcher::Quit();

#ifdef _MPLAYER
	if (m_pVisAdapter)
	{
		g_Player.UnregisterVis(m_pVisAdapter);
		m_pVisAdapter->Release();
		m_pVisAdapter = NULL;
	}
#endif
}

void CMPEventsDispatcher::RegisterHandler(EventType eventType, IEventHandler *pHandler)
{
	CEventsDispatcher::RegisterHandler(eventType, pHandler);

#ifdef _MPLAYER
	if (eventType == ET_VIS_DRAW_UPDATE)
	{
		CMutexAutolock		lock(m_mutex);
		LIST_EVENTHANDLER		&listHandler = m_vListEventHandler[eventType];

		// register vis ?
		if (listHandler.size() == 1)
		{
			ASSERT(m_pVisAdapter == NULL);
			m_pVisAdapter = new CMPVisAdapter;
			m_pVisAdapter->AddRef();
			g_Player.RegisterVis(m_pVisAdapter);
		}
	}
#endif
}

void CMPEventsDispatcher::UnRegisterHandler(EventType eventType, IEventHandler *pHandler)
{
	CEventsDispatcher::UnRegisterHandler(eventType, pHandler);

#ifdef _MPLAYER
	if (eventType == ET_VIS_DRAW_UPDATE)
	{
		CMutexAutolock		lock(m_mutex);
		LIST_EVENTHANDLER	&listHandler = m_vListEventHandler[eventType];

		// unregister vis ?
		if (listHandler.size() == 0)
		{
			ASSERT(m_pVisAdapter != NULL);
			g_Player.UnregisterVis(m_pVisAdapter);
			m_pVisAdapter->Release();
			m_pVisAdapter = NULL;
		}
	}
#endif
}

// szValue: p1="v1" p2="v2"
//	or	    v1
void CMPlayerSettings::SetSettings(EventType eventType, LPCTSTR szSectionName, LPCTSTR szSettingName, LPCTSTR szValue, bool bNotify)
{
	// save setting
	g_Profile.WriteProfileString(szSectionName, szSettingName, szValue);

	if (!bNotify)
		return;

	// dispatch setting event
	IEvent	*pEvt  = new IEvent;

	pEvt->eventType = eventType;
	pEvt->strName = szSettingName;
	pEvt->strValue = szValue;

	CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(pEvt);
}

void CMPlayerSettings::SetSettings(EventType eventType, LPCTSTR szSectionName, LPCTSTR szSettingName, int nValue, bool bNotify)
{
	// save setting
	g_Profile.WriteProfileInt(szSectionName, szSettingName, nValue);

	if (!bNotify)
		return;

	// dispatch setting event
	IEvent	*pEvt  = new IEvent;

	pEvt->eventType = eventType;
	pEvt->strName = szSettingName;
	pEvt->strValue = StringFromInt(nValue);

	CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(pEvt);
}
