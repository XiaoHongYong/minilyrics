// MLCmd.cpp: implementation of the MLCmd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MLCmd.h"


UIObjectIDDefinition g_uidDefinition[] = 
{
	{ _T("ID_TOPMOST"), CMD_TOPMOST, IDC_SETTOPMOST, _T("Toggle always on top") },
	{ _T("ID_CLICK_THROUGH"), CMD_CLICK_THROUGH, IDC_CLICK_THROUGH, _T("Mouse click through") },
	{ _T("ID_RATE_LYR"), CMD_RATE_LYR, IDC_RATE_LYRICS, _T("Rate these lyrics") },
	{ _T("ID_PLAY"), CMD_PLAY, IDC_PLAY, _T("Play") },
	{ _T("ID_PLAYPAUSE"), CMD_PLAYPAUSE, IDC_PLAYPAUSE, _T("Play/Pause") },
	{ _T("ID_PAUSE"), CMD_PAUSE, IDC_PAUSE, _T("Pause") },
	{ _T("ID_STOP"), CMD_STOP, IDC_STOP, _T("Stop") },
	{ _T("ID_BACKWARD"), CMD_BACKWARD, IDC_BACKWARD2SEC, _T("Backward") },
	{ _T("ID_FORWARD"), CMD_FORWARD, IDC_FORWARD2SEC, _T("Forward") },
	{ _T("ID_J_PREV_LINE"), CMD_J_PREV_LINE, 0, _T("Jump to previous line of lyrics") },
	{ _T("ID_J_NEXT_LINE"), CMD_J_NEXT_LINE, 0, _T("Jump to next line of lyrics") },

	{ _T("ID_FONT_SIZE_INC"), CMD_FONT_SIZE_INC, 0, _T("Increase font size") },
	{ _T("ID_FONT_SIZE_DEC"), CMD_FONT_SIZE_DEC, 0, _T("Decrease font size") },
	{ _T("ID_CLR_PREV_HUE"), CMD_CLR_PREV_HUE, 0, _T("Previous Color") },
	{ _T("ID_CLR_NEXT_HUE"), CMD_CLR_NEXT_HUE, 0, _T("Next Color") },
	{ _T("ID_LYR_HIGH_CLR_LIST"), CMD_LYR_HIGH_CLR_LIST, 0, _T("Highlight lyrics color") },
	{ _T("ID_MENU_STATIC_LYR"), CMD_MENU_STATIC_LYR, 0, _T("Menu") },

	{ _T("ID_LYR_SCROLL_ENABLE_RECORD"), 	CMD_LYR_SCROLL_ENABLE_RECORD, 0, _T("Enable/Disable record lyrics scrolling actions") },
	{ _T("ID_LYR_SCROLL_ENABLE_REPLAY"), 	CMD_LYR_SCROLL_ENABLE_REPLAY, 0, _T("Enable/Disable replay lyrics scrolling actions") },
	{ _T("ID_LYR_SCROLL_MENU"), 	CMD_LYR_SCROLL_MENU, 0, _T("Menu") },

	{ _T("ID_TB_LYR_SYNC"), ID_TB_LYR_SYNC, 0, NULL },
	{ _T("ID_TB_LYR_TXT"), ID_TB_LYR_TXT, 0, NULL },
	{ _T("ID_TB_LYR_EDIT"), ID_TB_LYR_EDIT, 0, NULL },

	{ _T("ID_TOGGLE_LYR_EDIT_TOOLBAR"), CMD_TOGGLE_LYR_EDIT_TOOLBAR, 0, _T("Toggle Extended Toolbar Visible") },
	{ _T("ID_INSERTTAG"), CMD_INSERTTAG, IDC_INSERT_TAG, _T("Synchronize the current line") },
	{ _T("ID_DEL_TAG"), CMD_DEL_TAG, 0, _T("Remove Timestamp of the current line") },
	{ _T("ID_INSERTTAG_DOWN"), CMD_INSERTTAG_DOWN, IDC_INSERT_TAG_AND_MOVE_DOWN, _T("Synchronize the current line and move to next line") },
	{ _T("ID_EDIT_HELP"), CMD_EDIT_HELP, 0, _T("Editor help") },
	{ _T("ID_JUMP"), CMD_JUMP, IDC_JUMP_TO_CURRENT_LINE, _T("Jump to current line") },
	{ _T("ID_FORWARD_REMAIN_LINES"), CMD_FORWARD_REMAIN_LINES, IDC_FORWARD_REMAIN_LINES, _T("Forward the remaining lyrics 0.2s") },
	{ _T("ID_BACKWARD_REMAIN_LINES"), CMD_BACKWARD_REMAIN_LINES, IDC_BACKWARD_REMAIN_LINES, _T("Backward the remaining lyrics 0.2s") },
	{ _T("ID_FORWARD_CUR_LINE"), CMD_FORWARD_CUR_LINE, IDC_FORWARD_CUR_LINE, _T("Forward the timestamp of this line 0.2s") },
	{ _T("ID_BACKWARD_CUR_LINE"), CMD_BACKWARD_CUR_LINE, IDC_BACKWARD_CUR_LINE, _T("Backward the timestamp of this line 0.2s") },
	{ _T("ID_AUTO_FILL_LYR_INFO"), CMD_AUTO_FILL_LYR_INFO, IDC_AUTO_FILL_LYRICS_INFO, _T("Auto fill artist, album and title info") },
	{ _T("ID_MENU_LYR_EDIT"), CMD_MENU_LYR_EDIT, 0, _T("More...") },
	{ _T("ID_LYR_EDITOR"), CMD_LYR_EDITOR, IDC_LYR_EDITOR, _T("Lyrics Editor...") },
	{ _T("ID_EXTERNAL_LYR_EDIT"), CMD_EXTERNAL_LYR_EDIT, IDC_EXTERNAL_LYR_EDIT, _T("Edit with external lyrics editor...") },
	{ _T("ID_SAVE_LYR_IN_SONG_FILE"), CMD_SAVE_LYR_IN_SONG_FILE, IDC_SAVE_LYRICS_TO_MP3, _T("Save lyrics in MP3 file...") },

	{ _T("ID_EDIT_UNDO"), CMD_EDIT_UNDO, IDC_EDIT_UNDO, _T("Undo (Ctrl+Z)") },
	{ _T("ID_EDIT_REDO"), CMD_EDIT_REDO, IDC_EDIT_REDO, _T("Redo (Ctrl+Y)") },
	{ _T("ID_EDIT_CUT"), CMD_EDIT_CUT, IDC_EDIT_CUT, _T("Cut (Ctrl+X)") },
	{ _T("ID_EDIT_COPY"), CMD_EDIT_COPY, IDC_EDIT_COPY, _T("Copy (Ctrl+C)") },
	{ _T("ID_EDIT_PASTE"), CMD_EDIT_PASTE, IDC_EDIT_PASTE, _T("Paste (Ctrl+V)") },
	{ _T("ID_EDIT_DELETE"), CMD_EDIT_DELETE, IDC_EDIT_DELETE, _T("Delete") },
	{ _T("ID_EDIT_FIND"), CMD_EDIT_FIND, IDC_EDIT_FIND, _T("Find") },
	{ _T("ID_EDIT_FINDNEXT"), CMD_EDIT_FINDNEXT, 0, _T("Find Next") },
	{ _T("ID_EDIT_REPLACE"), CMD_EDIT_REPLACE, IDC_EDIT_REPLACE, _T("Replace") },
	{ _T("ID_EDIT_LYR_TAG"), CMD_EDIT_LYR_TAG, IDC_EDIT_LYR_TAG, _T("Edit Lyrics Tag") },

	{ _T("CMD_REMOVE_ALL_TAG"), CMD_REMOVE_ALL_TAG, IDC_REMOVE_ALL_TAG, _T("Remove All Timestamps") },
	{ _T("CMD_REMOVE_BLANK_LINE"), CMD_REMOVE_BLANK_LINE, IDC_REMOVE_BLANK_LINE, _T("Remove All Blank Lines") },
	{ _T("CMD_TRIM_WHITESPACE"), CMD_TRIM_WHITESPACE, IDC_TRIM_SPACE, _T("Trim Leading and Trailing Whitespace of All Lines") },
	{ _T("CMD_REMOVE_UNSYNC_LINES"), CMD_REMOVE_UNSYNC_LINES, IDC_REMOVE_UNSYNC_LINE, _T("Remove All Unsynchronized Lines") },
	{ _T("CMD_CAPITALIZE_LEADING_LETTER"), CMD_CAPITALIZE_LEADING_LETTER, IDC_CAPITALIZE_LEADING_LETTER, _T("Capitalize the Leading Letter of All Lines") },
	{ _T("CMD_LYRICS_TO_LOWERCASE"), CMD_LYRICS_TO_LOWERCASE, IDC_TO_LOWER_CASE, _T("Convert All Lyrics Characters to Lower Case") },

	{ _T("ID_NEW_LRC"), CMD_NEW_LRC, IDC_NEW_LYRICS, _T("New") },
	{ _T("ID_OPEN_LRC"), CMD_OPEN_LRC, IDC_OPEN_LYRICS, _T("Open lyrics") },
	{ _T("ID_SAVE_LRC"), CMD_SAVE_LRC, IDC_SAVE_LYRICS, _T("Save lyrics") },
	{ _T("ID_SAVE_LRC_AS"), CMD_SAVE_LRC_AS, IDC_SAVE_LYRICS_AS, _T("Save lyrics as") },
	{ _T("ID_ADVANCED_SAVE_LYR"), CMD_ADVANCED_SAVE_LYR, IDC_ADVANCED_SAVE_LYR, _T("Advanced save lyrics options...") },
	{ _T("ID_UPLOAD_LYR"), CMD_UPLOAD_LYR, IDC_UPLOAD_LYRICS, _T("Upload Lyrics...") },

	{ _T("ID_SEARCH_LYRICS"), CMD_OPEN_LRC, IDC_OPEN_LYRICS, _T("Search Lyrics") },
	{ _T("ID_BACKWARD_LYRICS"), CMD_BACKWARD_LYRICS, 0, _T("Lyrics Backward 0.5 Sec (Up)") },
	{ _T("ID_FORWARD_LYRICS"), CMD_FORWARD_LYRICS, 0, _T("Lyrics Forward 0.5 Sec (Down)") },
	{ _T("ID_DISPLAY_OPT"), CMD_DISPLAY_OPT, IDC_LYR_DISPLAY_OPT, _T("Lyrics Display Options") },
	{ _T("ID_HELP"), CMD_HELP, IDC_ML_HELP, _T("Help and Support") },
	{ _T("ID_PREVIOUS"), CMD_PREVIOUS, IDC_PREV, _T("Previous Track") },
	{ _T("ID_NEXT"), CMD_NEXT, IDC_NEXT, _T("Next Track") },

	{ _T("ID_SEEK"), CMD_SEEK, 0, _T("Seek") },
	{ _T("ID_VOL_INC"), CMD_VOL_INC, 0, _T("Increase Volume") },
	{ _T("ID_VOL_DEC"), CMD_VOL_DEC, 0, _T("Decrease Volume") },
	{ _T("ID_VOLUME"), CMD_VOLUME, 0, _T("Volume") },
	{ _T("ID_MUTE"), CMD_MUTE, 0, _T("Mute") },
	{ _T("ID_PLAYLIST"), ID_PLAYLIST, 0, NULL },
	{ _T("ID_SHUFFLE"), CMD_SHUFFLE, IDC_SHUFFLE, _T("Toggle Shuffle") },
	{ _T("ID_LOOP"), CMD_LOOP, 0, _T("Toggle Repeat") },
	{ _T("ID_EQ"), CMD_EQ, IDC_EQ, _T("Equalizer") },
	{ _T("ID_RATE"), CMD_RATE, 0, _T("Rate this song") },
	{ _T("ID_TXT_TITLE"), CMD_TXT_TITLE, 0, NULL },
	{ _T("ID_TXT_POS"), CMD_TXT_POS, 0, NULL },
	{ _T("ID_TXT_VOL"), CMD_TXT_VOL, 0, NULL },
	{ _T("ID_TXT_STATE"), CMD_TXT_STATE, 0, NULL },
	{ _T("ID_TXT_INFO"), CMD_TXT_INFO, 0, NULL },
	{ _T("ID_STEREO_STAT"), ID_STEREO_STAT, 0, NULL },

#ifdef _MPLAYER
	// playlist commands
	{ _T("ID_PL_UP"), CMD_PL_UP, IDC_MOVE_UP, _T("Up") },
	{ _T("ID_PL_DOWN"), CMD_PL_DOWN, IDC_MOVE_DOWN, _T("Down") },
	{ _T("ID_PL_ADD_FILE"), CMD_PL_ADD_FILE, IDC_ADDFILE, _T("Add File") },
	{ _T("ID_PL_ADD_DIR"), CMD_PL_ADD_DIR, IDC_ADDDIR, _T("Add Folder") },
	{ _T("ID_PL_ADD_URL"), CMD_PL_ADD_URL, 0, _T("Add Url") },
	{ _T("ID_PL_OPEN_FILE"), CMD_PL_OPEN_FILE, IDC_OPENFILE, _T("Open File") },
	{ _T("ID_PL_OPEN_DIR"), CMD_PL_OPEN_DIR, IDC_OPENDIR, _T("Open Folder") },
	{ _T("ID_PL_OPEN_URL"), CMD_PL_OPEN_URL, 0, _T("Open Url") },
	{ _T("ID_PL_DEL"), CMD_PL_DEL, IDC_REMOVE, _T("Delete") },
	{ _T("ID_PL_PROPERTY"), CMD_PL_PROPERTY, IDC_MEDIA_INFO, _T("Property") },
	{ _T("ID_PL_NEW"), CMD_PL_NEW, IDC_NEW_PL, _T("New Playlist") },
	{ _T("ID_PL_LOAD"), CMD_PL_LOAD, 0, _T("Load Playlist") },
	{ _T("ID_PL_SAVE"), CMD_PL_SAVE, IDC_SAVE_PL, _T("Save Playlist") },
	{ _T("ID_PL_LIST"), CMD_PL_LIST, 0, NULL },

	{ _T("ID_TOGGLE_SCREEEN"), CMD_TOGGLE_SCREEEN, 0, _T("Toggle Screen") },
#endif

	// Media Guide View
	{ _T("ID_MG_TREE_GUIDE"), CMD_MG_TREE_GUIDE, 0, NULL },
	{ _T("ID_MG_MEDIA_LIST"), CMD_MG_MEDIA_LIST, 0, NULL },

	{ _T("ID_ML_GUIDE"), CMD_ML_GUIDE, 0, NULL },
	{ _T("ID_ML_BACK"), CMD_ML_BACK, 0, _T("Back") },
	{ _T("ID_ML_MEDIA_LIST"), ID_ML_MEDIA_LIST, 0, NULL },

	{ _T("ID_AD_TXT_LINK"), ID_AD_TXT_LINK, 0, NULL },
	{ _T("ID_AD_IMG_LINK"), ID_AD_IMG_LINK, 0, NULL },

	{ _T("ID_ALBUMART"), ID_ALBUMART, 0, NULL },

	{ _T("CMD_TOGGLE_MP"), CMD_TOGGLE_MP, 0, _T("Bring to front/Hide $Product$") },
	{ _T("CMD_PREFERENCES"), CMD_PREFERENCES, IDC_PREFERENCE, _T("Preferences") },
	{ _T("CMD_COPY_SONG_FILE_LYR"), CMD_COPY_SONG_FILE_LYR, IDC_COPYTO, _T("Copy song file and lyrics to...") },
	{ _T("CMD_RELOAD_LYR"), CMD_RELOAD_LYR, 0, _T("Reload lyrics") },

	{ _T("CMD_FLOATING_LYRICS"), CMD_FLOATING_LYRICS, IDC_FLOATING_LYRICS, _T("Floating Lyrics") },
	{ NULL, 0, 0, NULL },
};
