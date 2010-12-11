
#pragma once

/*****************************************************************************************

1. Architecture
		[Player.exe + pluginX.dll]       <----->          [MiniLyrics.exe]
					  hWndPlugin							hwndML

2. Initialize
	1). Plugin		>	Register MiniLyrics.exe with HWND(hex integer, 0x1232023)
	2). MiniLyrics	>   Send M2P_QUERY_INFO, retrieve the player info.
	3).	Plugin		>	Send M2M_PLAYER_INFO, send back the player info.
	4). MiniLyrics	>   Send M2P_CONNECTED, tell Plugin, MiniLyrics connected to it.
	5). MiniLyrics	>   Send M2P_CONNECT_DONE, tell Plugin, MiniLyrics has initialized OK.

3. Quit
	 Plugin		>	M2M_DISCONNECTED
	 MiniLyrics	>	M2P_DISCONNECT

3. Communication
	After the initialization, Plugin and MiniLyrics can send message to each other free.

4. WM_COPYDATA
	wParam:					ML_MSG_VERSION
	COPYDATASTRUCT.dwData:	MessageToML or MessageToPlayer
	COPYDATASTRUCT.lpData:	Message parameter

/*****************************************************************************************/

#define ML_MSG_VERSION			0xF20F4E57

#define R_UNDETERMINED				(-1)

enum MessageToML
{
	M2M_PLAYER_INFO					= 1,		// Send PlayerInfo
	M2M_DISCONNECTED				= 2,		// Player is going to quit

	M2M_MEDIA_CHANGED				= 3,		// Send MLMediaInfo
	M2M_PLAYER_STAT_CHANGED			= 4,		// PLAYER_STATE, player status
	M2M_PLAY_POS_CHANGED			= 5,		// DWORD, new playing position, in ms.

	//
	// Optional APIs of MiniLyrics.
	//

	M2M_CONFIG						= 6,		// HWND, parent window handle of configuration window
	M2M_ABOUT						= 7,		// HWND, parent window handle of about window

	M2M_LOG							= 8,		// Zero ended WCHAR text, the log to write in MiniLyrics
	M2M_ADD_WND_CLOSE_TO			= 9,		// WndCloseToML, add the window to close to MiniLyrics
	M2M_PLAYER_ACTIVATED			= 11,		// BOOL, true=activate, false = deactivate; Player window is activated
	M2M_PLAYER_MINRESTORE			= 12,		// DWORD, SIZE_MINIMIZED, SIZE_RESTORED, Notify to MiniLyrics the player is minimized or restored.

	// notify messages to MiniLyrics tat player window is moving, so MiniLyrics window can "track" moving.
	M2M_BEFORE_TRACK_MOVE			= 13,		// HWND[], Notify to MiniLyrics that the player main window is about to move, usually is WM_LBUTTONDOWN message.
	M2M_TRACK_MOVE					= 14,		// WM_MOVE notify to MiniLyrics, Param1 = x, Param2 = y

	M2M_PLAYER_SKIN_CHANGED			= 15,		// PlayerSkinInfo, when the player's skin changed, notify to MiniLyrics.
	M2M_SWITCH_2_EMBEDDED_SKIN		= 16,		// PlayerSkinInfo, switch to Embedded skin
};

enum MessageToPlayer
{
	M2P_QUERY_INFO					= 1,		// HWND of MLService
	M2P_CONNECTED					= 2,		// HWND of MLService
	M2P_CONNECTED_DONE				= 3,		// Send MLInfo to player
	M2P_DISCONNECT					= 4,		// MiniLyrics disconnect to player.

	M2P_PLAY						= 5,		
	M2P_PAUSE						= 6,
	M2P_PLAY_PAUSE					= 7,
	M2P_STOP						= 8,
	M2P_PREV						= 9,
	M2P_NEXT						= 10,
	M2P_SEEK						= 11,	// DWORD, position, in ms

	// Optional messages

	// If ML_SUPPORT_EMBEDDED is supported, the messages below should be implemented.
	M2P_SWITCH_2_EMBEDDED_SKIN		= 15,	// Tell player, MiniLyrics is going to switch to embedded skin mode.
											// Then, player should send M2M_SWITCH_2_EMBEDDED_SKIN.
	M2P_END_EMBEDDED_SKIN			= 16,	// MiniLyrics window has ended the embedded skin mode.
};

//////////////////////////////////////////////////////////////////////////
// Message definitions send to MiniLyrics

#define LEN_SHORT_STR			64

struct PlayerSkinInfo
{
	bool		bEmbeddedSkin;
	bool		bClrThemeValid;
	WCHAR		skinPath[MAX_PATH];
	COLORREF	clrFontHighlight, clrFontLowlight, clrBackground, clrCurLineBg;
	HWND		hWndToAttach;
};

enum MLStarupFlags
{
	//
	// embedded MiniLyrics window to player window, 
	//
	ML_SUPPORT_EMBEDDED						= 1,

	// ML window can't move, resize itself.
	ML_EMBEDDED_WND_IMMOVABLE				= (1 << 1),

	ML_SUPPORT_CLASSIC_SKINS				= (1 << 2),	// For winamp 2

	ML_SUPPORT_MINIMIZE_RESTORE_WITH_PLAYER = (1 << 3),
	ML_SUPPORT_PLAY_CONTROL					= (1 << 4),
	ML_SUPPORT_SEEK_CONTROL					= (1 << 5),

};

struct PlayerInfo
{
	DWORD						startupFlags;	// Combination of MLStarupFlags
	HWND						hWndPlayer;		// The main window of player
	HWND						hWndPluginService;	// The service window of the player plugin
	WCHAR						playerName[LEN_SHORT_STR];
	WCHAR						embeddedSkinName[LEN_SHORT_STR];
	// PlayerSkinInfo				skinInfo;
};

#ifndef _PLAYER_STATE_DEFINED
#define _PLAYER_STATE_DEFINED
enum PLAYER_STATE
{
	PS_STOPED				= 1,
	PS_PLAYING				= 2,
	PS_PAUSED				= 3
};
#endif // _PLAYER_STATE_DEFINED

struct MLMediaInfo
{
	DWORD			size;		// size of this struct
	unsigned long	mediaLength;		// in ms, 0 means unknown
	WORD			offsetArtist, offsetAlbum, offsetTitle, offsetMediaUrl;
	short			hasVideo;	// Is video media? 1, has; 0, has not; -1, undeterminted.
	WCHAR			mediaInfo[1];

	LPCWSTR Artist() { ASSERT(offsetArtist < size); return mediaInfo + offsetArtist; }
	LPCWSTR Album() { ASSERT(offsetAlbum < size); return mediaInfo + offsetAlbum; }
	LPCWSTR Title() { ASSERT(offsetTitle < size); return mediaInfo + offsetTitle; }
	LPCWSTR MediaUrl() { ASSERT(offsetMediaUrl < size); return mediaInfo + offsetMediaUrl; }

};

struct WndCloseToML
{
	HWND		hWndCloseTo;
	WCHAR		wndName[LEN_SHORT_STR];
	WCHAR		className[LEN_SHORT_STR];
};

//////////////////////////////////////////////////////////////////////////
// Message definitions send to player

struct MLInfo
{
	HWND				hWndMain;			// The main window of MiniLyrics
};

// The windows class name of MiniLyrics service
#define SZ_MINILYRICS_SERVICE_CLASS_NAME		_T("MiniLyricsServiceV1")

// The window class name of player plugin service
#define SZ_PLUGIN_SERVICE_CLASS_NAME			_T("MLPluginServiceV1")
