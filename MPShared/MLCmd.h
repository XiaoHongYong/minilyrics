// MLCmd.h: interface for the MLCmd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MLCMD_H__0F4EFB09_839A_4E3C_A964_F096D1441949__INCLUDED_)
#define AFX_MLCMD_H__0F4EFB09_839A_4E3C_A964_F096D1441949__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////////////////////////////////////////////////////////////////////////////
// Command ID of UIObject in SkinWnd:
enum ML_CUSTOM_COMMAND
{
	CMD_UNDEFINE	= UID_INVALID,

	///////////////
	// 控件的命令ID
	CMD_CLOSE,
	CMD_QUIT,
	CMD_WND_TITLE,
	CMD_MINIMIZE,
	CMD_MAXIMIZE,
	CMD_TOPMOST,
	CMD_CLICK_THROUGH,
	ID_WORK_AREA,
	CMD_RATE_LYR,
	CMD_MENU,

	ID_TOOLBAR,

	CMD_PLAYPAUSE,
	CMD_PLAY,
	CMD_PAUSE,
	CMD_STOP,
	CMD_BACKWARD,
	CMD_FORWARD,
	CMD_J_PREV_LINE,			// Jump to previous line of lyrics
	CMD_J_NEXT_LINE,			// Jump to next line of lyrics

	// Lyrics display options
	CMD_FONT_SIZE_INC,
	CMD_FONT_SIZE_DEC,
	CMD_CLR_PREV_HUE,
	CMD_CLR_NEXT_HUE,
	CMD_LYR_HIGH_CLR_LIST,
	CMD_MENU_STATIC_LYR,

	CMD_LYR_SCROLL_ENABLE_RECORD,
	CMD_LYR_SCROLL_ENABLE_REPLAY,
	CMD_LYR_SCROLL_MENU,

	// lyrics editor command
	CMD_JUMP,
	CMD_INSERTTAG,
	CMD_DEL_TAG,
	CMD_INSERTTAG_DOWN,
	CMD_FORWARD_REMAIN_LINES,		// Forward the remaining lyrics 0.2s
	CMD_BACKWARD_REMAIN_LINES,		// Backward the remaining lyrics 0.2s
	CMD_FORWARD_CUR_LINE,
	CMD_BACKWARD_CUR_LINE,
	CMD_AUTO_FILL_LYR_INFO,
	CMD_EDIT_TAG,
	CMD_NEW_LRC,
	CMD_OPEN_LRC,
	CMD_SAVE_LRC,
	CMD_SAVE_LRC_AS,
	CMD_ADVANCED_SAVE_LYR,
	CMD_UPLOAD_LYR,
	CMD_EDIT_HELP,
	CMD_MENU_LYR_EDIT,
	CMD_LYR_EDITOR,
	CMD_EXTERNAL_LYR_EDIT,
	CMD_SAVE_LYR_IN_SONG_FILE,

	// lyrics text edit command
	CMD_EDIT_UNDO,
	CMD_EDIT_REDO,
	CMD_EDIT_CUT,
	CMD_EDIT_COPY,
	CMD_EDIT_PASTE,
	CMD_EDIT_DELETE,
	CMD_EDIT_FIND,
	CMD_EDIT_FINDNEXT,
	CMD_EDIT_REPLACE,

	CMD_REMOVE_ALL_TAG,
	CMD_REMOVE_BLANK_LINE,
	CMD_TRIM_WHITESPACE,
	CMD_REMOVE_UNSYNC_LINES,
	CMD_CAPITALIZE_LEADING_LETTER,
	CMD_LYRICS_TO_LOWERCASE,

	CMD_FORWARD_LYRICS,
	CMD_BACKWARD_LYRICS,
	CMD_DISPLAY_OPT,
	CMD_HELP,
	CMD_PREVIOUS,
	CMD_NEXT,

	// Player control command/ID
	CMD_SEEK,
	CMD_VOL_INC,
	CMD_VOL_DEC,
	CMD_VOLUME,
	CMD_MUTE,
	ID_PLAYLIST,
	CMD_SHUFFLE,
	CMD_LOOP,
	CMD_EQ,
	CMD_RATE,

	CMD_TXT_TITLE,
	CMD_TXT_POS,
	CMD_TXT_VOL,
	CMD_TXT_STATE,
	CMD_TXT_INFO,

	ID_STEREO_STAT,

	// playlist command
	CMD_PL_UP,
	CMD_PL_DOWN,
	CMD_PL_ADD_FILE,
	CMD_PL_ADD_DIR,
	CMD_PL_ADD_URL,
	CMD_PL_OPEN_FILE,
	CMD_PL_OPEN_DIR,
	CMD_PL_OPEN_URL,
	CMD_PL_DEL,
	CMD_PL_PROPERTY,
	CMD_PL_NEW,
	CMD_PL_LOAD,
	CMD_PL_SAVE,
	CMD_PL_LIST,

	// pocket pc cmd
	CMD_TOGGLE_SCREEEN,		// turn screen on/off

	// Media Guide View
	CMD_MG_TREE_GUIDE,
	CMD_MG_MEDIA_LIST,

	// media library
	CMD_ML_GUIDE,
	CMD_ML_BACK,
	ID_ML_MEDIA_LIST,

	ID_AD_TXT_LINK,
	ID_AD_IMG_LINK,

	ID_ALBUMART,		// album art control

	// Skin Dialog
	CMD_OK,
	CMD_CANCEL,

	// other cmd
	CMD_TOGGLE_MP,
	CMD_PREFERENCES,

	CMD_COPY_SONG_FILE_LYR,
	CMD_RELOAD_LYR,

	CMD_FLOATING_LYRICS,
};

// ID 的字符串与数值的映射表
extern STRING_ID_MAP g_StringIdMap[];

#ifdef _WIN32_WCE

//
// Command Accerlate key for wce
//
struct MLCmdAccKey
{
	ML_CUSTOM_COMMAND		cmd;
	uint16					button;
	uint16					fsModifiers;
	uint16					buttonDef;
	uint16					fsModifiersDef;
};

extern MLCmdAccKey		g_mlCmdAccKey[];
extern int				g_mlCmdAccKeyCount;

#endif

LPCTSTR MPCMDIDToStr(int nCmd);

ML_CUSTOM_COMMAND MPCMDStrToID(LPCTSTR szID);

LPCTSTR MPCMDIDToToolTip(int nCmd);

#endif // !defined(AFX_MLCMD_H__0F4EFB09_839A_4E3C_A964_F096D1441949__INCLUDED_)
