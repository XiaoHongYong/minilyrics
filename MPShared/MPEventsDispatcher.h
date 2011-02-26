// MPEventHost.h: interface for the CMPEventHost class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPEVENTHOST_H__AE092490_ADF2_4326_9066_B35EE136A3DD__INCLUDED_)
#define AFX_MPEVENTHOST_H__AE092490_ADF2_4326_9066_B35EE136A3DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../MPlayerLib/IMPlayer.h"
#include "MPlayerAppBase.h"

class CMPVisAdapter;

// section name
#define SZ_SECT_LYR_DISPLAY			_T("LyrDispaly")
#define SZ_SECT_FLOATING_LYR		_T("FloatingLyr")
#define SZ_SECT_PLAYER				_T("Player")
#define SZ_SECT_LYR_DL				_T("Download")

#ifdef _MPLAYER
#define SZ_SECT_UI			_T("ZikiPlayer")
#else
#define SZ_SECT_UI			_T("MiniLyrics")
#endif

enum MLEventType
{
	ET_INVALID = 0,
	ET_NULL = 0,

	ET_PLAYER_STATUS_CHANGED,
	ET_PLAYER_SEEK,
	ET_PLAYER_CUR_MEDIA_CHANGED,
	ET_PLAYER_CUR_MEDIA_INFO_CHANGED,
	ET_PLAYER_CUR_PLAYLIST_CHANGED,
	ET_PLAYER_SETTING_CHANGED,
	ET_PLAYER_EQ_SETTING_CHANGED,
	ET_PLAYER_PLAY_HALT_ERROR,
	ET_PLAYER_POS_UPDATE,

	ET_LYRICS_DISPLAY_SETTINGS,
	ET_LYRICS_FLOATING_SETTINGS,
	ET_LYRICS_DRAW_UPDATE,
	ET_LYRICS_CHANGED,
	ET_LYRICS_ON_SAVE_EDIT,
	ET_LYRICS_EDITOR_RELOAD_TAG,

	ET_LYRICS_SEARCH_END,
	ET_DOWNLOAD_END,

	ET_UI_SETTINGS_CHANGED,
	ET_UI_INFO_TEXT,
	ET_UI_LONG_ERROR_TEXT,

	ET_VIS_DRAW_UPDATE,

	COUNT_OF_EVENT_TYPE,
};

struct CEventPlayerStatusChanged : public IEvent
{
	PLAYER_STATE		status;
};

struct CEventPlayerSettingChanged : public IEvent
{
	IMPEvent::MP_SETTING_TYPE		settingType;
	long				nValue;
};

struct CEventPlaylistChanged : public IEvent
{
	IMPEvent::MP_PLAYLIST_CHANGE_ACTION		action;
	long				nIndex, nIndexOld;
};

struct CEventPlayerEQChanged : public IEvent
{
	EQualizer			eqlalizer;
};

class CEventLyricsEditorCmd : public IEvent
{
public:
	UINT				cmd;
};

class CEventDownloadEnd : public IEvent
{
public:
	enum DOWNLOAD_TYPE
	{
		DT_DL_LRC_FAILED,
		DT_DL_CHECK_NEW_VERSION_OK,
	};

	DOWNLOAD_TYPE		downloadType;
	class CDownloadTask	*pTask;

};

class CEventVisDrawUpdate : public IEvent
{
public:
	VisParam			*pVisParam;
};

class ISkinCmdHandler
{
public:
	ISkinCmdHandler() { m_pSkinWnd = NULL; }
	virtual ~ISkinCmdHandler() { }

	virtual void Init(CSkinWnd *pSkinWnd) { m_pSkinWnd = pSkinWnd; }

	// if the command id is processed, return true.
	virtual bool OnCommand(UINT nID) = 0;
	virtual bool OnCustomCommand(int nId) = 0;
	virtual bool OnUIObjNotify(IUIObjNotify *pNotify) = 0;

	// IUICheckStatus method
	virtual bool GetChecked(UINT nID, bool &bChecked) { return false; }
	virtual bool GetRadioChecked(vector<UINT> &vIDs, UINT &nIDChecked) { return false; }

protected:
	CSkinWnd		*m_pSkinWnd;

};

class CMPEventsDispatcher : public CEventsDispatcher
{
public:
	virtual void Quit();

	virtual void RegisterHanlder(EventType eventType, IEventHandler *pHandler);
	virtual void UnregisterHanlder(EventType eventType, IEventHandler *pHandler);

protected:
#ifndef _MPLAYER
	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);
#endif

protected:
#ifdef _MPLAYER
	CMPVisAdapter			*m_pVisAdapter;
#endif

};

class CMPlayerSettings
{
public:
	// szValue: p1="v1" p2="v2"
	//	or	    v1
	static void SetLyricsDisplaySettings(LPCTSTR szSettingName, LPCTSTR szValue, bool bNotify = true)
	{
		SetSettings(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, szSettingName, szValue, bNotify);
	}

	// szValue: p1="v1" p2="v2"
	//	or	    v1
	static void SetLyricsDisplaySettings(LPCTSTR szSettingName, int nValue, bool bNotify = true)
	{
		SetSettings(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, szSettingName, nValue, bNotify);
	}

	//
	// Player UI
	//
	static void GetPlayerUISettings(LPCTSTR szSettingName, tstring &strValue)
	{
		GetSettings(SZ_SECT_UI, szSettingName, strValue);
	}

	// szValue: p1="v1" p2="v2"
	//	or	    v1
	static void SetPlayerUISettings(LPCTSTR szSettingName, LPCTSTR szValue, bool bNotify = true)
	{
		SetSettings(ET_UI_SETTINGS_CHANGED, SZ_SECT_UI, szSettingName, szValue, bNotify);
	}

	// szValue: p1="v1" p2="v2"
	//	or	    v1
	static void SetPlayerUISettings(LPCTSTR szSettingName, int nValue, bool bNotify = true)
	{
		SetSettings(ET_UI_SETTINGS_CHANGED, SZ_SECT_UI, szSettingName, nValue, bNotify);
	}

	static void GetSettings(LPCTSTR szSectionName, LPCTSTR szSettingName, tstring &strValue)
	{
		strValue = g_Profile.GetProfileString(szSectionName, szSettingName, _T(""));
	}

	static int GetSettings(LPCTSTR szSectionName, LPCTSTR szSettingName, int nValueDef)
	{
		return g_Profile.GetProfileInt(szSectionName, szSettingName, nValueDef);
	}

	// szValue: p1="v1" p2="v2"
	//	or	    v1
	static void SetSettings(EventType eventType, LPCTSTR szSectionName, LPCTSTR szSettingName, LPCTSTR szValue, bool bNotify = true);

	static void SetSettings(EventType eventType, LPCTSTR szSectionName, LPCTSTR szSettingName, int nValue, bool bNotify = true);

};

#endif // !defined(AFX_MPEVENTHOST_H__AE092490_ADF2_4326_9066_B35EE136A3DD__INCLUDED_)
