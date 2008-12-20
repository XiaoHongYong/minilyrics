
#ifndef _MINILYRICS_PLUGIN_HEADER_
#define _MINILYRICS_PLUGIN_HEADER_

#define ML_IF_VERSION			0x10	// minilyrics's interface version 1.0
#define ML_PLAYER_IF_VERSION	0x10	// player's interface version 1.0

typedef long MLPARAM;

//
// return value of plugin messages.
//
enum
{
	PR_UNIMPLEMENTED	= -1,
	PR_FAILED			= 0,
	PR_OK				= 1,
};

//
// MiniLyrics.dll supported interface
// nCommand: MLCommand
typedef long (*FUNMLService)(int nCommand, MLPARAM Param1, MLPARAM Param2);

// MiniLyrics service struct
typedef struct 
{
	UINT			uSize;
	UINT			uVersion;
	FUNMLService	funMLService;
}MLServiceEntry;


//
// Your player should supported interface
// nCommand: PlayerIFCommand
typedef long (*FUNPlayerService)(int nCommand, MLPARAM Param1, MLPARAM Param2);

// player service struct
typedef struct
{
	UINT			uSize;
	UINT			uVersion;
	FUNPlayerService	funPlayerService;
}PlayerServiceEntry;

typedef MLServiceEntry *(*FUNMiniLyricsInitEntryPoint)(PlayerServiceEntry *IfPlayer);

enum MLStarupFlags
{
	//
	// embedded minilyrics window to player window, 
	//
	ML_SUPPORT_EMBEDDED						= 1,

	// ML window can't move, resize itselft.
	ML_EMBEDDED_WND_IMMOVABLE				= (1 << 1),

	// 
	ML_EMBEDDED_SUPPORT_CLASSIC_SKINS		= (1 << 2),

};

enum MLCommand
{
	MLInit				= 0,		// param1: HINSTANCE, param2: MLStarupFlags
	MLQuit				= 1,		// param1: unused, param2: unused
	MLConfig			= 2,		// param1: window handle of config parent, param2: unused
	MLAbout				= 3,		// param1: window handle of about parent, param2: unused
	MLTitleChanged		= 4,		// param1: const char * media title: artist - title, param2, unused
	MLPlayerStateChanged= 5,		// param1: PLAYER_STATE
	MLPlayPosChanged	= 6,		// param1: media playing pos, ms.
	MLLog				= 7,		// write log, param1: log message;
	MLAddWndCloseToHWND			= 8,	// Add the wnd of minilyrics auto close to when move.param1, hWnd
	MLAddWndCloseToName			= 9,	// Add the wnd of minilyrics auto close to when move.param1, Classname, param2, wnd name

	MLGetMainWnd				= 10,	// return the main window of minilyrics
	MLNotifyPlayerActive		= 11,	// Notify to minilyrics the player window is activate, param1, true=activate, false = deactivate
	MLNotifyPlayerMinRestore	= 12,	// Notify to minilyrics the player is minimized or restored,param1: SIZE_MINIMIZED, SIZE_RESTORED

	// notify messages to minilyrics that player window is moving, so minilyrics window can "track" moving.
	MLNotifyBeforeTrackMove		= 13,	// Notify to minilyrics that the player main window is about to move, usually is WM_LBUTTONDOWN messge.
	MLNotifyTrackMove			= 14,	// WM_MOVE notify to minilyrics, Param1 = x, Param2 = y

	MLNotifySkinChanged			= 15,	// when the player's skin changed, notify to minilyrics.

	MLGetSkinMode				= 18,	// return the minilyrics skin mode. 1: player's skin, 2, MiniLyrics' skin

	MLSetWa5Bmp					= 19,	// set the winamp5's bitmaps, param1: 1, winamp5's scrollbar, param2: BITMAP HANDLE.

	MLSwitchToEmbeddedSkin		= 20,	// switch to Embedded skin. param1 = 1 switch to embedded skin mode; param1 = 0 switch to minilyrics traditional skin. 

	MLTitleChangedW				= 34,		// param1: const WCHAR * media title: artist - title, param2, unused
	MLLogW						= 37,		// write log, param1: log message(LPCWSTR);

	MLDestroyNotifyWnd			= 40,		// Destroy the hidden notfication window of MiniLyrics.

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

enum SKIN_CLR_NAME
{
	SCN_FONT_HIGH = 1,
	SCN_FONT_LOW,
	SCN_NORMAL_BG,
	SCN_SEL_BG
};

enum PlayerFeature
{
	PF_MinimizeRestoreWithPlayer	= 1,
	PF_Control						= 2,		// play, pause, stop, next, prev
	PF_Seek							= 3,
};

enum PlayerIFCommand
{
	cmdPlay				= 1,
	cmdPause			= 2,
	cmdPlayPause		= 3,
	cmdStop				= 4,
	cmdPrev				= 5,
	cmdNext				= 6,
	cmdSeek				= 7,	// param1, pos, ms
	cmdOpen				= 8,

	cmdGetPos			= 10,	// param1, long * pos, ms
	cmdGetMediaLength	= 11,	// param1, long * length, ms
	cmdGetSourceMedia	= 12,	// param1, char *buffer, param2, buffer length
	cmdGetArtist		= 13,	// param1, char *buffer, param2, buffer length
	cmdGetFullTitle		= 14,	// param1, char *buffer, param2, buffer length
	cmdGetTitle			= 15,	// param1, char *buffer, param2, buffer length
	cmdGetAlbum			= 16,	// param1, char *buffer, param2, buffer length
	cmdGetPlayerState	= 17,	// return player state,1-stop, 2-play, 3-pause
	cmdGetPlayerName	= 18,	// param1, char *buffer, param2, buffer length
	cmdInit				= 19,	// init the player interface
	cmdQuit				= 20,	// quit the player interface

	cmdIsFeatureSupported = 21, // Is feature specialfied by param1 supported, PlayerFeature

	// for winamp
	cmdGetCurSkinPath	= 30,	// param1, const char *buffer, param2, buffer length
	cmdGetProxy			= 31,	// param1, const char *buffer, param2, buffer length

	cmdGetMainWnd		= 32,	// return the player's main window
	cmdHasVideo			= 34,	// if has video, return 1, if has not return 0, if not implemented, return -1

	cmdGetPlayerPlugInstance	= 35, // return the instance of player plugin

	cmdSetTimeUpdateFreq	= 39,	// set play time postion's update frequency
	// 0: slow, 1 : normal: 2: fast

	//
	// For embedded window mode.
	//
	cmdGetAttachedParentWnd	= 37,	// return the parent window to be attached.
	// Param1: (RECT*) the expected rect of the parent window

	notifySkinLoaded		= 38,	// MiniLyrics' window attached to the parent window, and skin is loaded,
	// notify the parent window to do something(resize?)

	notifyDetachedFromParentWnd	= 40,	// MiniLyrics window has been detached from parent window

	cmdGetEmbeddedSkinName	= 41,	// param1: char *buffer; param2: buffer length

	// 
	cmdGetSkinThemeColor		= 50,	// param1: color name(SKIN_CLR_NAME); param2: color value, type COLORREF


	// some players support ucs2 encoding
	// UCS2 cmd
	cmdSetUcs2			= 60,	// The player should notify the message in UCS2 encoding
	// param  : minilyrics would like receive: if param1 1, ucs2, 0: ansi
	// return : player support encoding: if return 1, ucs2, 0: ansi

	cmdGetSourceMediaW	= 62,	// param1, const WCHAR *buffer, param2, buffer length
	cmdGetFullTitleW	= 64,	// param1, const WCHAR *buffer, param2, buffer length
	cmdGetArtistW		= 65,	// param1, char *buffer, param2, buffer length
	cmdGetTitleW		= 66,	// param1, char *buffer, param2, buffer length
	cmdGetAlbumW		= 67,	// param1, char *buffer, param2, buffer length

};

//
// The plugin of the player should call minilyrics entry point init function 
// "MiniLyricsInitEntryPoint" to load minilyrics.
//

#endif	// _MINILYRICS_PLUGIN_HEADER_
