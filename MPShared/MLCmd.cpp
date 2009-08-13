// MLCmd.cpp: implementation of the MLCmd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MLCmd.h"


// ID 的字符串与数值的映射表
STRING_ID_MAP g_StringIdMap[] = 
{
	{ _T("ID_WND_TITLE"), CMD_WND_TITLE, NULL },
	{ _T("ID_MINIMIZE"), CMD_MINIMIZE, _T("Minimize") },
	{ _T("ID_MAXIMIZE"), CMD_MAXIMIZE, _T("Maximize") },
	{ _T("ID_TOPMOST"), CMD_TOPMOST, _T("Toggle always on top") },
	{ _T("ID_CLICK_THROUGH"), CMD_CLICK_THROUGH, _T("Mouse click through") },
	{ _T("ID_CLOSE"), CMD_CLOSE, _T("Close") },
	{ _T("ID_QUIT"), CMD_QUIT, _T("Quit") },
	{ _T("ID_WORK_AREA"), ID_WORK_AREA, NULL },
	{ _T("ID_TOOLBAR"), ID_TOOLBAR, NULL },
	{ _T("ID_RATE_LYR"), CMD_RATE_LYR, _T("Rate this Lyrics") },
	{ _T("ID_MENU"), CMD_MENU, _T("Menu") },
	{ _T("ID_OPEN_MEDIA"), CMD_OPEN_MEDIA, _T("Open") },
	{ _T("ID_PLAY"), CMD_PLAY, _T("Play") },
	{ _T("ID_PLAYPAUSE"), CMD_PLAYPAUSE, _T("Play/Pause") },
	{ _T("ID_PAUSE"), CMD_PAUSE, _T("Pause") },
	{ _T("ID_STOP"), CMD_STOP, _T("Stop") },
	{ _T("ID_BACKWARD"), CMD_BACKWARD, _T("Backward") },
	{ _T("ID_FORWARD"), CMD_FORWARD, _T("Forward") },
	{ _T("ID_J_PREV_LINE"), CMD_J_PREV_LINE, _T("Jump to previous line of lyrics") },
	{ _T("ID_J_NEXT_LINE"), CMD_J_NEXT_LINE, _T("Jump to next line of lyrics") },

	{ _T("ID_FONT_SIZE_INC"), CMD_FONT_SIZE_INC, _T("Increase font size") },
	{ _T("ID_FONT_SIZE_DEC"), CMD_FONT_SIZE_DEC, _T("Decrease font size") },
	{ _T("ID_CLR_PREV_HUE"), CMD_CLR_PREV_HUE, _T("Previous Color") },
	{ _T("ID_CLR_NEXT_HUE"), CMD_CLR_NEXT_HUE, _T("Next Color") },

	{ _T("ID_INSERTTAG"), CMD_INSERTTAG, _T("Synchronize the current line") },
	{ _T("ID_DEL_TAG"), CMD_DEL_TAG, _T("Remove Timestamp of the current line") },
	{ _T("ID_INSERTTAG_DOWN"), CMD_INSERTTAG_DOWN, _T("Synchronize the current line and move down one line") },
	{ _T("ID_EDIT_HELP"), CMD_EDIT_HELP, _T("Editor help") },
	{ _T("ID_JUMP"), CMD_JUMP, _T("Jump to current line") },
	{ _T("ID_FORWARD_REMAIN_LINES"), CMD_FORWARD_REMAIN_LINES, _T("Forward the remaining lyrics 0.2s") },
	{ _T("ID_BACKWARD_REMAIN_LINES"), CMD_BACKWARD_REMAIN_LINES, _T("Backward the remaining lyrics 0.2s") },
	{ _T("ID_FORWARD_CUR_LINE"), CMD_FORWARD_CUR_LINE, _T("Forward the timestamp of this line 0.2s") },
	{ _T("ID_BACKWARD_CUR_LINE"), CMD_BACKWARD_CUR_LINE, _T("Backward the timestamp of this line 0.2s") },
	{ _T("ID_AUTO_FILL_LYR_INFO"), CMD_AUTO_FILL_LYR_INFO, _T("Auto fill artist, album and title info") },
	{ _T("ID_EDIT_TAG"), CMD_EDIT_TAG, _T("Lyrics Information") },
	{ _T("ID_MENU_LYR_EDIT"), CMD_MENU_LYR_EDIT, _T("More...") },
	{ _T("ID_LYR_EDITOR"), CMD_LYR_EDITOR, _T("Lyrics Editor...") },
	{ _T("ID_EXTERNAL_LYR_EDIT"), CMD_EXTERNAL_LYR_EDIT, _T("Edit with external lyrics editor...") },
	{ _T("ID_SAVE_LYR_IN_SONG_FILE"), CMD_SAVE_LYR_IN_SONG_FILE, _T("Save lyrics in MP3 file...") },

	{ _T("ID_EDIT_UNDO"), CMD_EDIT_UNDO, _T("Undo (Ctrl+Z)") },
	{ _T("ID_EDIT_REDO"), CMD_EDIT_REDO, _T("Redo (Ctrl+Y)") },
	{ _T("ID_EDIT_CUT"), CMD_EDIT_CUT, _T("Cut (Ctrl+X)") },
	{ _T("ID_EDIT_COPY"), CMD_EDIT_COPY, _T("Copy (Ctrl+C)") },
	{ _T("ID_EDIT_PASTE"), CMD_EDIT_PASTE, _T("Paste (Ctrl+V)") },
	{ _T("ID_EDIT_DELETE"), CMD_EDIT_DELETE, _T("Delete") },
	{ _T("ID_EDIT_FIND"), CMD_EDIT_FIND, _T("Find") },
	{ _T("ID_EDIT_FINDNEXT"), CMD_EDIT_FINDNEXT, _T("Find Next") },
	{ _T("ID_EDIT_REPLACE"), CMD_EDIT_REPLACE, _T("Replace") },

	{ _T("ID_NEW_LRC"), CMD_NEW_LRC, _T("New") },
	{ _T("ID_OPEN_LRC"), CMD_OPEN_LRC, _T("Open lyrics") },
	{ _T("ID_SAVE_LRC"), CMD_SAVE_LRC, _T("Save lyrics") },
	{ _T("ID_SAVE_LRC_AS"), CMD_SAVE_LRC_AS, _T("Save lyrics as") },
	{ _T("ID_ADVANCED_SAVE_LYR"), CMD_ADVANCED_SAVE_LYR, _T("Advanced save lyrics options...") },
	{ _T("ID_UPLOAD_LYR"), CMD_UPLOAD_LYR, _T("Upload Lyrics...") },

	{ _T("ID_SEARCH_LYRICS"), CMD_SEARCH_LYRICS, _T("Search Lyrics") },
	{ _T("ID_BACKWARD_LYRICS"), CMD_BACKWARD_LYRICS, _T("Lyrics Backward 0.5 Sec(Up)") },
	{ _T("ID_FORWARD_LYRICS"), CMD_FORWARD_LYRICS, _T("Lyrics Forward 0.5 Sec(Down)") },
	{ _T("ID_DISPLAY_OPT"), CMD_DISPLAY_OPT, _T("Lyrics Display Options") },
	{ _T("ID_HELP"), CMD_HELP, _T("Help and Support") },
	{ _T("ID_PREVIOUS"), CMD_PREVIOUS, _T("Previous") },
	{ _T("ID_NEXT"), CMD_NEXT, _T("Next") },

#ifdef _MPLAYER
	{ _T("ID_SEEK"), CMD_SEEK, _T("Seek") },
	{ _T("ID_VOL_INC"), CMD_VOL_INC, _T("Increase Volume") },
	{ _T("ID_VOL_DEC"), CMD_VOL_DEC, _T("Decrease Volume") },
	{ _T("ID_VOL_BT_IMG"), ID_VOL_BT_IMG, NULL },
	{ _T("ID_VOL_SWITCH"), CMD_VOL_SWITCH, _T("Switch Volume Control") },
	{ _T("ID_VOLUME"), CMD_VOLUME, _T("Volume") },
	{ _T("ID_MUTE"), CMD_MUTE, _T("Mute") },
	{ _T("ID_PLAYLIST"), ID_PLAYLIST, NULL },
	{ _T("ID_SHUFFLE"), CMD_SHUFFLE, _T("Toggle Shuffle") },
	{ _T("ID_LOOP"), CMD_LOOP, _T("Toggle Repeat") },
	{ _T("ID_EQ"), CMD_EQ, _T("Equalizer") },
	{ _T("ID_LP_SWITCH"), CMD_LP_SWITCH, _T("Show/Hide Lyrics/Playlist") },
	{ _T("ID_RATE"), CMD_RATE, _T("Rate this song") },
	{ _T("ID_TXT_TITLE"), CMD_TXT_TITLE, NULL },
	{ _T("ID_TXT_POS"), CMD_TXT_POS, NULL },
	{ _T("ID_TXT_VOL"), CMD_TXT_VOL, NULL },
	{ _T("ID_TXT_STATE"), CMD_TXT_STATE, NULL },
	{ _T("ID_TXT_INFO"), CMD_TXT_INFO, NULL },
	{ _T("ID_STEREO_STAT"), ID_STEREO_STAT, NULL },

	// playlist commands
	{ _T("ID_PL_UP"), CMD_PL_UP, _T("Up") },
	{ _T("ID_PL_DOWN"), CMD_PL_DOWN, _T("Down") },
	{ _T("ID_PL_ADD_FILE"), CMD_PL_ADD_FILE, _T("Add File") },
	{ _T("ID_PL_ADD_DIR"), CMD_PL_ADD_DIR, _T("Add Folder") },
	{ _T("ID_PL_ADD_URL"), CMD_PL_ADD_URL, _T("Add Url") },
	{ _T("ID_PL_OPEN_FILE"), CMD_PL_OPEN_FILE, _T("Open File") },
	{ _T("ID_PL_OPEN_DIR"), CMD_PL_OPEN_DIR, _T("Open Folder") },
	{ _T("ID_PL_OPEN_URL"), CMD_PL_OPEN_URL, _T("Open Url") },
	{ _T("ID_PL_DEL"), CMD_PL_DEL, _T("Delete") },
	{ _T("ID_PL_PROPERTY"), CMD_PL_PROPERTY, _T("Property") },
	{ _T("ID_PL_NEW"), CMD_PL_NEW, _T("New Playlist") },
	{ _T("ID_PL_LOAD"), CMD_PL_LOAD, _T("Load Playlist") },
	{ _T("ID_PL_SAVE"), CMD_PL_SAVE, _T("Save Playlist") },
	{ _T("ID_PL_LIST"), CMD_PL_LIST, NULL },

	{ _T("ID_TOGGLE_SCREEEN"), CMD_TOGGLE_SCREEEN, _T("Toggle Screen") },

	// Media Guide View
	{ _T("ID_MG_TREE_GUIDE"), CMD_MG_TREE_GUIDE, NULL },
	{ _T("ID_MG_MEDIA_LIST"), CMD_MG_MEDIA_LIST, NULL },

	{ _T("ID_ML_GUIDE"), CMD_ML_GUIDE, NULL },
	{ _T("ID_ML_BACK"), CMD_ML_BACK, _T("Back") },
	{ _T("ID_ML_MEDIA_LIST"), ID_ML_MEDIA_LIST, NULL },
#endif // _MPLAYER

	{ _T("ID_AD_TXT_LINK"), ID_AD_TXT_LINK, NULL },
	{ _T("ID_AD_IMG_LINK"), ID_AD_IMG_LINK, NULL },

	{ _T("ID_ALBUMART"), ID_ALBUMART, NULL },

	{ _T("CMD_TOGGLE_MP"), CMD_TOGGLE_MP, _T("Bring to front/Hide $Product$") },
	{ _T("CMD_PREFERENCES"), CMD_PREFERENCES, _T("Preferences") },
	{ _T("CMD_COPY_SONG_FILE_LYR"), CMD_COPY_SONG_FILE_LYR, _T("Copy song and lyrics to ...") },
	{ _T("CMD_RELOAD_LYR"), CMD_RELOAD_LYR, _T("Reload lyrics") },

	{ _T("CMD_FLOATING_LYRICS"), CMD_FLOATING_LYRICS, _T("Floating Lyrics") },
	{ NULL, 0, NULL },
};

#ifdef _WIN32_WCE

MLCmdAccKey		g_mlCmdAccKey[] = 
{
	{ CMD_PLAYPAUSE, VK_RETURN, 0, VK_RETURN, 0 },
	{ CMD_PLAYPAUSE, 0, 0, 0, 0 },
	{ CMD_STOP, 0, 0, 0 },
	{ CMD_PLAY, 0, 0, 0 },
	{ CMD_PAUSE, 0, 0, 0 },
	{ CMD_PREVIOUS, VK_LEFT, 0, VK_LEFT, 0 },
	{ CMD_NEXT, VK_RIGHT, 0, VK_RIGHT, 0 },
	{ CMD_VOL_INC, VK_UP, 0, VK_UP, 0 },
	{ CMD_VOL_DEC, VK_DOWN, 0, VK_DOWN, 0 },
	{ CMD_BACKWARD, 0, 0, 0, 0 },
	{ CMD_FORWARD, 0, 0, 0, 0 },
	{ CMD_MUTE, '#', 0, '#', 0 },
	{ CMD_BACKWARD_LYRICS, 0, 0 , 0, 0 },
	{ CMD_FORWARD_LYRICS, 0, 0, 0, 0 },
	{ CMD_TOGGLE_SCREEEN, 0, 0, 0, 0 },
	{ CMD_SHUFFLE, 0, 0, 0, 0 },
	{ CMD_LOOP, 0, 0, 0, 0 },
};

int				g_mlCmdAccKeyCount = CountOf(g_mlCmdAccKey) - 1;

#else

ML_CUSTOM_COMMAND MPCMDStrToID(LPCTSTR szID)
{
	STRING_ID_MAP		*p = g_StringIdMap;
	for (; p->szId != NULL; p++)
	{
		if (_tcsicmp(p->szId, szID) == 0)
			return (ML_CUSTOM_COMMAND)p->nId;
	}

	return (ML_CUSTOM_COMMAND)SKIN_UIOBJ_ID_UNDEFINE;
}

LPCTSTR MPCMDIDToStr(int nCmd)
{
	STRING_ID_MAP		*p = g_StringIdMap;
	for (; p->szId != NULL; p++)
	{
		if (p->nId == nCmd)
			return p->szId;
	}

	return NULL;
}

LPCTSTR MPCMDIDToToolTip(int nCmd)
{
	STRING_ID_MAP		*p = g_StringIdMap;
	for (; p->szId != NULL; p++)
	{
		if (p->nId == nCmd)
			return p->szToolTip;
	}

	return NULL;
}

#endif
