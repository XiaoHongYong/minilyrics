// MLCmd.h: interface for the MLCmd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MLCMD_H__0F4EFB09_839A_4E3C_A964_F096D1441949__INCLUDED_)
#define AFX_MLCMD_H__0F4EFB09_839A_4E3C_A964_F096D1441949__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////////////////////////////////////////////////////////////////////////////
// MiniLyrics 的命令定义:
enum ML_CUSTOM_COMMAND
{
	CMD_UNDEFINE	= SKIN_UIOBJ_ID_UNDEFINE,

	///////////////
	// 控件的命令ID
	CMD_CLOSE,
	CMD_QUIT,
	CMD_WND_TITLE,
	CMD_MINIMIZE,
	CMD_MAXIMIZE,
	CMD_TOPMOST,
	ID_WORK_AREA,
	ID_WORK_AREA_DIVIDE_IMG,
	CMD_RATE_LYR,
	CMD_MENU,

	ID_TOOLBAR,

	CMD_OPEN_MEDIA,
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
	CMD_EDIT_HELP,
	CMD_MENU_LYR_EDIT,

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

	CMD_SEARCH_LYRICS,
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
	ID_VOL_BT_IMG,
	CMD_VOL_SWITCH,
	CMD_VOLUME,
	CMD_MUTE,
	ID_PLAYLIST,
	CMD_SHUFFLE,
	CMD_LOOP,
	CMD_EQ,
	CMD_LP_SWITCH,	// lyrics playlist show/hide switch
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

	// other cmd
	CMD_TOGGLE_MP,
	CMD_PREFERENCES,
};

// ID 的字符串与数值的映射表
extern STRING_ID_MAP g_StringIdMap[];

#ifdef _WIN32_WCE

//
// MiniLyrics Command Accerlate key for wce
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
