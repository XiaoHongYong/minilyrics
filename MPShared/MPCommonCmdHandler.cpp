// MPCommonCmdHandler.cpp: implementation of the CMPCommonCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MLCmd.h"
#include "MPCommonCmdHandler.h"
#include "MPlayerApp.h"
#include "LyricShowMultiRowObj.h"
#include "LyricShowTxtObj.h"
#include "LyricShowTwoRowObj.h"
#include "LyricShowHorzScrollObj.h"
#include "LyricShowSingleRowObj.h"
#include "LyricShowVobSub.h"
#include "CharsetEncoding.h"
#include "SkinSeekCtrl.h"
#include "SkinRateCtrl.h"
#include "DlgSearchLyrics.h"
#include "LyricsDownSelDlg.h"
#include "DlgSelFileEncoding.h"
#include "DlgSelMbcsEncoding.h"
#include "../widget/win32/FileOpenDlg.h"
#include "RegistrationDlg.h"
#include "DlgSelFileEncoding.h"
#include "DlgSelMbcsEncoding.h"
#include "DlgEmbeddedLyricsLoad.h"
#include "DlgEmbeddedLyricsSave.h"
#include "DlgEmbeddedLyricsRemove.h"
#include "LrcUploadDlg.h"
#include "LoginDlg.h"
#include "VersionUpdate.h"
#include "PreferenceDlg.h"
#include "DlgAbout.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "TagInfoDlg.h"
#include "DlgCopySongLyrics.h"
#include "MPMsgWnd.h"
#include "DlgRateLyrics.h"
#include "DlgAdjustHue.h"

#ifdef _MPLAYER
#include "MPHelper.h"
#endif

#ifdef _WIN32_DESKTOP
#include "win32/MPFloatingLyrWnd.h"
#endif

bool		g_bInModalDlLrcSelDlg;

const int SET_SPEED_SPAN = 500;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPCommonCmdHandler::CMPCommonCmdHandler(bool bFloatingLyr)
{
	if (bFloatingLyr)
	{
		m_strSectName = SZ_SECT_FLOATING_LYR;
		m_etDispSettings = ET_LYRICS_FLOATING_SETTINGS;
	}
	else
	{
		m_strSectName = SZ_SECT_LYR_DISPLAY;
		m_etDispSettings = ET_LYRICS_DISPLAY_SETTINGS;
	}
}

CMPCommonCmdHandler::~CMPCommonCmdHandler()
{

}

// if the command id is processed, return true.
bool CMPCommonCmdHandler::OnCommand(UINT nID)
{
	switch (nID)
	{
	case IDC_PREV:
		g_Player.Prev();
		break;
	case IDC_PLAY:
		g_Player.Play();
		break;
	case IDC_PAUSE:
		g_Player.Pause();
		break;
	case IDC_PLAYPAUSE:
		g_Player.PlayPause();
		break;
	case IDC_STOP:
		g_Player.Stop();
		break;
	case IDC_NEXT:
		g_Player.Next();
		break;
#ifdef _MPLAYER
	case IDC_SHUFFLE:
		g_Player.SetShuffle(!g_Player.IsShuffle());
		break;
	case IDC_REPEAT_OFF:
		g_Player.SetLoop(MP_LOOP_OFF);
		break;
	case IDC_REPEAT_ALL:
		g_Player.SetLoop(MP_LOOP_ALL);
		break;
	case IDC_REPEAT_TRACK:
		g_Player.SetLoop(MP_LOOP_TRACK);
		break;
	case IDC_OPENFILE:
		OnSongOpenFileCmd(m_pSkinWnd, true);
		break;
	case IDC_OPENDIR:
		OnSongOpenDirCmd(m_pSkinWnd, true);
		break;
	case IDC_ADDFILE:
		OnSongOpenFileCmd(m_pSkinWnd, false);
		break;
	case IDC_ADDDIR:
		OnSongOpenDirCmd(m_pSkinWnd, false);
		break;
#endif
	case IDC_OPEN_LYRICS:
	case IDC_DOWNLOAD_CURRENT_LYRICS:
		//
		// 歌词下载
		//
		{
			if (!g_bInModalDlLrcSelDlg)
			{
				g_bInModalDlLrcSelDlg = TRUE;
				CDlgSearchLyrics		LyricsDownSelDlg;

				LyricsDownSelDlg.DoModal(m_pSkinWnd);
				g_bInModalDlLrcSelDlg = FALSE;
			}
		}
		break;
	case IDC_SAVE_LYRICS:
		OnCustomCommand(CMD_SAVE_LRC);
		break;
	case IDC_SAVE_LYRICS_AS:
		OnCustomCommand(CMD_SAVE_LRC_AS);
		break;
	case IDC_FORWARD_LYRICS:
		OnCustomCommand(CMD_FORWARD_LYRICS);
		break;
	case IDC_BACKWARD_LYRICS:
		OnCustomCommand(CMD_BACKWARD_LYRICS);
		break;
	case IDC_LOAD_LYRICS_FROM_MP3:
		{
			OnCommandOpenLyricsInMp3();
		}
		break;
	case IDC_SAVE_LYRICS_TO_MP3:
		{
			OnCommandSaveLyricsToMp3();
		}
		break;
	case IDC_REMOVE_LRC_IN_MP3:
		{
			OnCommandRemoveLyricsInMp3();
		}
		break;
	case IDC_UPDATE_LYRICS:
		g_mpApp.ResearchLyrics();
		break;

	case IDC_LYR_DISPLAY_OPT:
		if (m_etDispSettings == ET_LYRICS_FLOATING_SETTINGS)
			g_wndPreference.Create(m_pSkinWnd, PAGE_FLOATING_LYR_THEME);
		else
			g_wndPreference.Create(m_pSkinWnd, PAGE_LYR_THEME);
		break;
	case IDC_LOAD_SKIN_THEME:
		CMPlayerSettings::SetSettings(m_etDispSettings, m_strSectName.c_str(), _T("ReloadSkinDefClrTheme"), TRUE);
		break;
	case IDC_ALIGN_LYRICS_LEFT:
	case IDC_ALIGN_LYRICS_CENTER:
	case IDC_ALIGN_LYRICS_RIGHT:
		{
			tstring		strAlign;

			if (nID == IDC_ALIGN_LYRICS_LEFT)
				strAlign = _T("left");
			else if (nID == IDC_ALIGN_LYRICS_CENTER)
				strAlign = _T("center");
			else if (nID == IDC_ALIGN_LYRICS_RIGHT)
				strAlign = _T("right");
			else
				ASSERT(0 && _T("Invalid ID."));

			CMPlayerSettings::SetSettings(m_etDispSettings, m_strSectName.c_str(), _T("LyrAlign"), strAlign.c_str());
		}
		break;

	case IDC_LDO_KARAOKE:
		CMPlayerSettings::SetSettings(m_etDispSettings, m_strSectName.c_str(), _T("Karaoke"), 
			!g_Profile.GetProfileBool(m_strSectName.c_str(), _T("Karaoke"), false));
		break;
	case IDC_LDO_NORMAL:
	case IDC_LDO_FADE_IN:
	case IDC_LDO_FADEOUT_BG:
		{
			tstring		strDrawOptions;

			if (nID == IDC_LDO_NORMAL)
				strDrawOptions = _T("normal");
			else if (nID == IDC_LDO_FADE_IN)
				strDrawOptions = _T("fadein");
			else if (nID == IDC_LDO_FADEOUT_BG)
				strDrawOptions = _T("fadeout");
			else
				return false;

			CMPlayerSettings::SetSettings(m_etDispSettings, m_strSectName.c_str(), _T("LyrDrawOpt"), strDrawOptions.c_str());
		}
		break;
	case IDC_ENABLE_BK_IMAGE:
		{
			bool bUseBgImg = g_Profile.GetProfileBool(m_strSectName.c_str(), _T("UseBgImage"), false);
			CMPlayerSettings::SetSettings(m_etDispSettings, m_strSectName.c_str(), _T("UseBgImage"), !bUseBgImg);
		}
		break;
	case IDC_SEL_BKIMAGE:
		OnCommandSetBkImg();
		break;
#ifdef _WIN32_DESKTOP
	case IDC_ANTIAlIAS:
		{
			//
			// 反锯齿
			tstring		str;
			bool		bAntialias;
			CMPlayerSettings::GetPlayerUISettings(_T("Antialias"), str);
			bAntialias = !IsTRUE(str.c_str());

			if (bAntialias)//  && !CGdiplusGraphics::CanSupportGdiplus())
			{
				int		nRet;
				nRet = GetOperationSystemType();
				if (nRet == OPS_WIN95 || nRet == OPS_WINNT4)
				{
					m_pSkinWnd->MessageOut(TOLOCAL(_T("Sorry, Microsoft windows 95 and windows NT4 does not support Antialiasing!")), MB_ICONINFORMATION | MB_YESNO);
					break;
				}

				if (!CGdiplusGraphics::CanSupportGdiplus())
				{
					//
					// 告知用户需要Gdiplus支持！
					if (m_pSkinWnd->MessageOut(TOLOCAL(_T("Antialiasing needs Gdiplus.dll, do you want to download it?")), MB_ICONINFORMATION | MB_YESNO) == IDYES)
					{
						OpenUrl(m_pSkinWnd, GetStrName(SN_HTTP_GDIPLUS));
						return TRUE;
					}
				}
			}
			CMLGraphicsFactory::SetGraphicsType(bAntialias ? MGT_GDIPLUS : MGT_WIN32GDI);

			CMPlayerSettings::SetPlayerUISettings(_T("Antialias"), bAntialias);
		}
		break;
	case IDC_FLOATING_LYRICS:
		{
			g_Profile.WriteProfileInt(_T("FloatingLyr"), !g_wndFloatingLyr.IsValid());
			if (!g_wndFloatingLyr.IsValid())
				g_wndFloatingLyr.Create();
			else
				g_wndFloatingLyr.Destroy();
		}
		break;
	case IDC_CLICK_THROUGH_FLOATING_LYR:
		if (g_wndFloatingLyr.IsValid())
			g_wndFloatingLyr.OnCommand(IDC_CLICK_THROUGH, 0);
		break;
#endif
	case IDC_ML_HELP:
		// 显示帮助
		OnCustomCommand(CMD_HELP);
		break;
	case IDC_WEBHOME:
		OpenUrl(m_pSkinWnd, GetStrName(SN_HTTP_DOMAIN));
		break;
	case IDC_WEBFORUM:
		OpenUrl(m_pSkinWnd, GetStrName(SN_HTTP_BBS));
		break;
	case IDC_EMAIL:
		// feed back
		{
			tstring		str;

			str = GetStrName(SN_EMAIL);
			str += _T("?subject=feedback");

			OpenUrl(m_pSkinWnd, str.c_str());
		}
		break;
	case IDC_PROMOTE:
			OpenUrl(m_pSkinWnd, GetStrName(SN_HTTP_PROMOTE));
		break;
	case IDC_REGISTER:
		ShowRegisterNotice(m_pSkinWnd, TRUE);
		break;
	case IDC_MINIMIZE:
#if !defined(_MPLAYER) && defined(_WIN32_DESKTOP)
		if (::GetParent(g_mpApp.GetMainWnd()->GetHandle()))
			break;
#endif
		g_mpApp.GetSkinFactory()->MinizeAll();
		break;
	case IDC_UPLOAD_LYRICS:
		{
			// 上传歌词
			CLrcUploadDlg	dlg;

			dlg.DoModal(m_pSkinWnd);
		}
		break;
	case IDC_LOGIN:
		{
			CLoginDlg		dlg;
			dlg.DoModal(m_pSkinWnd);
		}
		break;
	case IDC_APPLY_ACCOUNT:
		{
			OpenUrl(m_pSkinWnd, GetStrName(SN_HTTP_SIGNUP));
		}
		break;
	case IDC_LOGIN_VIA_IE:
		{
			OpenUrl(m_pSkinWnd, GetStrName(SN_HTTP_LOGIN));
		}
		break;
	case IDC_VIEW_AS_TEXT:
		{
			// 转换尾纯文本，并使用用记事本打开
			TCHAR		szTempTxtFile[MAX_PATH];

			GetAppDataDir(szTempTxtFile);
			_tcscat(szTempTxtFile, _T("TempTxtLyrics.txt"));
			g_LyricData.SaveToTextFile(szTempTxtFile, true);

			// 用记事本打开
			Execute(m_pSkinWnd, szTempTxtFile, NULL);
		}
		break;
	case IDC_COPYTO:
		{
			CDlgCopySongLyrics	dlg;

			dlg.m_strSong = g_Player.GetSrcMedia();
			dlg.m_strLyrics = g_LyricData.GetLyricsFileName();
			dlg.DoModal(m_pSkinWnd);
		}
		break;
	case IDC_CHECK_UPDATE:
		{
			CVersionUpdate	verUpdate;
			verUpdate.CheckNewVersionManual(&g_LyricsDownloader);
		}
		break;
	case IDC_EDIT_LYRICS:
		//
		// 调用外部的编辑器编辑歌词
		if (g_LyricData.GetLyricsSourceType() != LST_FILE)
		{
			m_pSkinWnd->MessageOut(TOLOCAL(_T("Embedded lyrics can not be edited with outside editors.")));
			break;
		}

		if (g_LyricData.IsOK())
		{
			if (!g_mpApp.OnLyricsChangingSavePrompt())
				break;

			tstring		strEditor;
			GetNotepadEditor(strEditor);
			Execute(m_pSkinWnd, 
				CMLProfile::GetDir(SZ_SECT_UI, _T("LyricsEditor"), strEditor.c_str()).c_str(),
				g_LyricData.GetLyricsFileName());
		}
		break;
	case IDC_RATE_LYRICS:
		OnCustomCommand(CMD_RATE_LYR);
		break;
#ifdef _WIN32_WCE
	case IDC_MEDIALIB:
		g_mpApp.GetSkinFactory()->OpenOrCloseSkinWnd(false, _SZ_SKINWND_CLASS_NAME, _T("Media Library"), _T("medialib"));
		break;
	case IDC_PLAYLIST:
		g_mpApp.GetSkinFactory()->OpenOrCloseSkinWnd(false, _SZ_SKINWND_CLASS_NAME, _T("Playlist"), _T("Playlist"));
		break;
	case IDC_LYRICS_VIEWER:
		g_mpApp.GetSkinFactory()->OpenOrCloseSkinWnd(false, _SZ_SKINWND_CLASS_NAME, _T("Lyrics"), _T("lyrics"));
		break;
#endif
	case IDC_OPTIONS:
	case IDC_PREFERENCE:
		g_wndPreference.Create(m_pSkinWnd, PAGE_SETUP);
		break;
	case IDC_ADJUST_HUE:
		{
			if (!g_mpApp.GetSkinFactory()->GetResourceMgr()->CanAdjustHue())
			{
				m_pSkinWnd->MessageOut(TOLOCAL(_T("Sorry, this skin doesn't support this feature.")));
				break;
			}

			CDlgAdjustHue	dlg;
			dlg.DoModal(m_pSkinWnd);
		}
		break;
	case IDC_ADJUST_LYR_HUE:
		{
			CDlgAdjustHue	dlg;
			dlg.DoModal(m_pSkinWnd, m_etDispSettings, m_strSectName.c_str());
		}
		break;
	case IDC_ABOUT:
		{
			CDlgAbout		dlg;
			dlg.DoModal(m_pSkinWnd);
		}
		break;
	case IDC_EXIT:
		g_mpMsgWnd.PostQuitMPMessage();
		break;
	case IDC_EDIT_LRC_TAG:
		OnCustomCommand(CMD_EDIT_TAG);
		break;
		
	case IDC_EDIT_UNDO:
		OnCustomCommand(CMD_EDIT_UNDO);
		break;
	case IDC_EDIT_REDO:
		OnCustomCommand(CMD_EDIT_REDO);
		break;
	case IDC_EDIT_CUT:
		OnCustomCommand(CMD_EDIT_CUT);
		break;
	case IDC_EDIT_COPY:
		OnCustomCommand(CMD_EDIT_COPY);
		break;
	case IDC_EDIT_PASTE:
		OnCustomCommand(CMD_EDIT_PASTE);
		break;
	case IDC_EDIT_DELETE:
		OnCustomCommand(CMD_EDIT_DELETE);
		break;
	case IDC_AUTO_FILL_LYRICS_INFO:
		OnCustomCommand(CMD_AUTO_FILL_LYR_INFO);
		break;
	case IDC_REMOVE_ALL_TAG:
		OnCustomCommand(CMD_REMOVE_ALL_TAG);
		break;
	case IDC_REMOVE_BLANK_LINE:
		OnCustomCommand(CMD_REMOVE_BLANK_LINE);
		break;
	case IDC_TRIM_SPACE:
		OnCustomCommand(CMD_TRIM_WHITESPACE);
		break;
	case IDC_REMOVE_UNSYNC_LINE:
		OnCustomCommand(CMD_REMOVE_UNSYNC_LINES);
		break;
	case IDC_CAPITALIZE_LEADING_LETTER:
		OnCustomCommand(CMD_CAPITALIZE_LEADING_LETTER);
		break;
	case IDC_TO_LOWER_CASE:
		OnCustomCommand(CMD_LYRICS_TO_LOWERCASE);
		break;
	case IDC_FORWARD_CUR_LINE:
		OnCustomCommand(CMD_FORWARD_CUR_LINE);
		break;
	case IDC_BACKWARD_CUR_LINE:
		OnCustomCommand(CMD_BACKWARD_CUR_LINE);
		break;
	case IDC_FORWARD_REMAIN_LINES:
		OnCustomCommand(CMD_FORWARD_REMAIN_LINES);
		break;
	case IDC_BACKWARD_REMAIN_LINES:
		OnCustomCommand(CMD_BACKWARD_REMAIN_LINES);
		break;
	case IDC_CLOSE:
		OnCustomCommand(CMD_CLOSE);
		break;
	case IDC_BR_ALBUM_ART:
		{
			tstring				strAlbumArtFileName;

			FileGetPath(g_Player.GetSrcMedia(), strAlbumArtFileName);
			if (!IsDirExist(strAlbumArtFileName.c_str()))
				break;

			if (IsEmptyString(g_Player.GetAlbum()))
				strAlbumArtFileName += _T("Folder");
			else
				strAlbumArtFileName += g_Player.GetAlbum();

			CFileOpenDlg		dlg(TOLOCAL(_T("Browse album art picture file")), _T(""), 
				_T("All Picture Files\0*.jpg;*.gif;*.bmp;*.png\0JPEG (*.jpg)\0*.jpg\0Bitmap files (*.bmp)\0*.bmp\0PNG (*.png)\0*.png\0\0"), 0);

			if (dlg.DoModal(m_pSkinWnd) != IDOK)
				break;

			strAlbumArtFileName += FileGetExt(dlg.GetOpenFile());

			if (_tcsicmp(strAlbumArtFileName.c_str(), dlg.GetOpenFile()) == 0)
				break;

			CopyFile(dlg.GetOpenFile(), strAlbumArtFileName.c_str(), false);

			g_eventDispather.DispatchSyncEvent(ET_PLAYER_CUR_MEDIA_INFO_CHANGED);
		}
		break;
	default:
		if (OnCommandCharEncoding(nID))
			break;
		return false;
	}

	return true;
}

bool CMPCommonCmdHandler::OnCustomCommand(UINT nID)
{
	switch (nID)
	{
#ifndef _WIN32_WCE
	case CMD_MINIMIZE:
		m_pSkinWnd->ShowWindow(SW_SHOWMINIMIZED);
		break;
	case CMD_MAXIMIZE:
		if (m_pSkinWnd->IsZoomed())
			m_pSkinWnd->ShowWindow(SW_RESTORE);
		else
			m_pSkinWnd->ShowWindow(SW_SHOWMAXIMIZED);
		break;
	case CMD_TOGGLE_MP:
		{
/*			if (IsIconic() || ::GetForegroundWindow() != m_hWnd)
				Activate();
			else
				Minimize(true);*/
		}
		break;
	case CMD_PREFERENCES:
		g_wndPreference.Create(m_pSkinWnd, PAGE_SETUP);
		break;
	case CMD_FLOATING_LYRICS:
		OnCommand(IDC_FLOATING_LYRICS);
		break;
#endif
	case CMD_EDIT_HELP:
		MLHtmlHelper(m_pSkinWnd, _T("help.chm::/how-to-editlyrics.htm"));
		break;
	case CMD_HELP:
		MLHtmlHelper(m_pSkinWnd, _T("help.chm::/getstarted.htm#downlyrics"));
		break;
	case CMD_PAUSE:
		if (g_Player.GetPlayerState() == PS_PLAYING)
			g_Player.Pause();
		else if (g_Player.GetPlayerState() == PS_PAUSED)
			g_Player.Play();
		break;
	case CMD_PLAY:
		g_Player.Play();
		break;
	case CMD_PLAYPAUSE:
		g_Player.PlayPause();
		break;
	case CMD_STOP:
		g_Player.Stop();
		break;
	case CMD_NEXT:
		g_Player.Next();
		break;
	case CMD_PREVIOUS:
		g_Player.Prev();
		break;
	case CMD_SEEK:
		{
			CSkinSeekCtrl	*pCtrl = (CSkinSeekCtrl*)m_pSkinWnd->GetUIObjectById(CMD_SEEK, CSkinSeekCtrl::ClassName());
			if (pCtrl)
			{
				g_Player.SeekTo(pCtrl->GetScrollPos());
			}
		}
		break;
	case CMD_BACKWARD:
		{
			int			dwCurPos;

			dwCurPos = g_Player.GetPlayPos();
			dwCurPos -= 2000;
			if (dwCurPos < 0)
				dwCurPos = 0;
			dwCurPos -= dwCurPos % 1000;
			g_Player.SeekTo(dwCurPos);
		}
		break;
	case CMD_FORWARD:
		{
			int			dwCurPos;

			dwCurPos = g_Player.GetPlayPos();
			dwCurPos += 2000;
			if (dwCurPos < 0)
				dwCurPos = 0;
			dwCurPos -= dwCurPos % 1000;
			g_Player.SeekTo(dwCurPos);
		}
		break;
#ifdef _MPLAYER
	case CMD_VOL_INC:
		{
			long		vol;
			if (g_Player.GetVolume(vol))
			{
				if (vol < MP_VOLUME_MAX)
				{
					vol += 5;
					vol -= vol % 5;
					if (vol > MP_VOLUME_MAX)
						vol = MP_VOLUME_MAX;
					g_Player.SetVolume(vol);
					g_eventDispather.DispatchInfoText(CStrPrintf(_T("%s %d%%"), TOLOCAL(_T("Set Volume")), vol).c_str());
				}
			}
		}
		break;
	case CMD_VOL_DEC:
		{
			long		vol;
			if (g_Player.GetVolume(vol))
			{
				if (vol > 0)
				{
					vol -= 5;
					vol -= vol % 5;
					if (vol < 0)
						vol = 0;
					g_Player.SetVolume(vol);
					g_eventDispather.DispatchInfoText(CStrPrintf(_T("%s %d%%"), TOLOCAL(_T("Set Volume")), vol).c_str());
				}
			}
		}
		break;
	case CMD_MUTE:
		{
			bool		bMute = !g_Player.IsMute();
			if (g_Player.SetMute(bMute))
			{
				g_eventDispather.DispatchInfoText(TOLOCAL(bMute ? _T("Mute On") : _T("Mute Off")));
			}
		}
		break;
	case CMD_SHUFFLE:
		{
			bool		bShuffle = !g_Player.IsShuffle();
			if (g_Player.SetShuffle(bShuffle))
			{
				g_eventDispather.DispatchInfoText(TOLOCAL(bShuffle ? _T("Shuffle On") : _T("Shuffle Off")));
			}
		}
		break;
	case CMD_LOOP:
		{
			if (g_Player.SetToNextLoopMode())
			{
				tstring		str;
				MP_LOOP_MODE loopMode = g_Player.GetLoop();
				if (loopMode == MP_LOOP_OFF)
					str = _T("Repeat Off");
				else if (loopMode == MP_LOOP_TRACK)
					str =  _T("Repeat Track");
				else
					str =  _T("Repeat All");

				g_eventDispather.DispatchInfoText(str.c_str());
			}
		}
		break;
	case CMD_RATE:
		{
			CSkinRateCtrl	*pRateCtrl;
			pRateCtrl = (CSkinRateCtrl*)m_pSkinWnd->GetUIObjectById(CMD_RATE, CSkinRateCtrl::ClassName());
			if (pRateCtrl)
			{
				CMPAutoPtr<IMediaLibrary>	pMediaLib;
				CMPAutoPtr<IMedia>			pMedia;

				if (g_Player.m_spPlayer->GetMediaLibrary(&pMediaLib) != ERR_OK)
					break;

				if (g_Player.m_spPlayer->GetCurrentMedia(&pMedia) == ERR_OK)
				{
					pMediaLib->Rate(pMedia, pRateCtrl->GetRating());
				}
			}
		}
		break;
	case CMD_LP_SWITCH:
		{
			tstring		strWorkArea;
			CMPlayerSettings::GetPlayerUISettings(_T("WorkArea"), strWorkArea);

			if (_tcsicmp(strWorkArea.c_str(), _T("lyr")) == 0)
				strWorkArea = _T("pl");
			else if (_tcsicmp(strWorkArea.c_str(), _T("pl")) == 0)
				strWorkArea = _T("lyr-pl");
			else
				strWorkArea = _T("lyr");

			CMPlayerSettings::SetPlayerUISettings(_T("WorkArea"), strWorkArea.c_str());
		}
		break;
	case CMD_PL_ADD_FILE:
		OnCommand(IDC_ADDFILE);
		break;
	case CMD_PL_ADD_DIR:
		OnCommand(IDC_ADDDIR);
		break;
#endif

		//
		// Lyrics ....
		//
	case CMD_NEW_LRC:
		{
			g_mpApp.NewLyrics();
		}
		break;
	case CMD_OPEN_LRC:
		OnCommand(IDC_DOWNLOAD_CURRENT_LYRICS);
		break;
/*		{
			// open lyrics
			CDlgLyricsAssociate		LyricsAssociateDlg;
			int						n;

			LyricsAssociateDlg.SetInfo(g_Player.GetSrcMedia(), g_Player.GetFullTitle());

			n = LyricsAssociateDlg.DoModal(g_mpApp.GetMainWnd());
			if (n == IDOK)
			{
				g_mpApp.ResearchLyrics();
			}
		}
		break;*/
	case CMD_SAVE_LRC:
		{
			int			nRet;

			// notify to save...
			IEvent		*pEvent = new IEvent;
			pEvent->eventType = ET_LYRICS_ON_SAVE_EDIT;
			g_eventDispather.DispatchSyncEvent(pEvent);

			if (g_LyricData.DoesChooseNewFileName())
			{
				SaveAsLyricsFile(m_pSkinWnd, 
					LyricsConentTypeToFileType(g_LyricData.GetLyrContentType()));
			}
			else
			{
				nRet = g_LyricData.Save();
				if (nRet != ERR_OK)
				{
					if (g_LyricData.GetLyricsSourceType() != LST_FILE)
					{
						tstring		str;
						if (g_LyricData.ToString(str, FT_LYRICS_LRC, TRUE))
						{
							g_autoProcessEmbeddedLyrics.AddSaveEmbeddedJob(g_LyricData.GetSongFileName(), 
							_T(""), str.c_str(), g_LyricData.GetLyricsSourceType());
						}
					}
					m_pSkinWnd->MessageOut(CStrPrintf(_T("%s\n%s"), ERROR2STR_LOCAL(nRet), TOLOCAL(_T("Failed to Save embedded lyrics, MiniLyrics will auto try again later."))).c_str());
				}
			}
		}
		break;
	case CMD_SAVE_LRC_AS:
		{
			// notify to save...
			IEvent		*pEvent = new IEvent;
			pEvent->eventType = ET_LYRICS_ON_SAVE_EDIT;
			g_eventDispather.DispatchSyncEvent(pEvent);

			SaveAsLyricsFile(m_pSkinWnd,
				LyricsConentTypeToFileType(g_LyricData.GetLyrContentType()));
		}
		break;
	case CMD_FORWARD_LYRICS:
		{
			CUIObject	*pObj;
			pObj = m_pSkinWnd->GetUIObjectByClassName(CLyricShowTxtObj::ClassName());
			if (pObj)
			{
				if (pObj)
					pObj->OnKeyDown(VK_DOWN, 0);
			}
			else
			{
				g_LyricData.SetOffsetTime(g_LyricData.GetOffsetTime() + SET_SPEED_SPAN);
				g_eventDispather.DispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);

				g_eventDispather.DispatchSyncEvent(ET_LYRICS_EDITOR_RELOAD_TAG);

				g_eventDispather.DispatchInfoText(TOLOCAL(_T("Lyrics Forward 0.5 Sec")));
			}
		}
		break;
	case CMD_BACKWARD_LYRICS:
		{
			CUIObject	*pObj;
			pObj = m_pSkinWnd->GetUIObjectByClassName(CLyricShowTxtObj::ClassName());
			if (pObj)
			{
				// move down one line.
				if (pObj)
					pObj->OnKeyDown(VK_UP, 0);
			}
			else
			{
				g_LyricData.SetOffsetTime(g_LyricData.GetOffsetTime() - SET_SPEED_SPAN);
				g_eventDispather.DispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);

				g_eventDispather.DispatchSyncEvent(ET_LYRICS_EDITOR_RELOAD_TAG);

				g_eventDispather.DispatchInfoText(TOLOCAL(_T("Lyrics Backward 0.5 Sec")));
			}
		}
		break;
	case CMD_SEARCH_LYRICS:
		OnCommand(IDC_DOWNLOAD_CURRENT_LYRICS);
		break;

	case CMD_EDIT_TAG:
		{
			CTagInfoDlg		dlg;

			dlg.m_lyrProperties = g_LyricData.Properties();
			if (dlg.DoModal(m_pSkinWnd) != IDOK)
				break;

			if (g_LyricData.Properties().m_strTitle != dlg.m_lyrProperties.m_strTitle)
			{
				g_LyricData.Properties().m_strTitle = dlg.m_lyrProperties.m_strTitle;
			}
			if (g_LyricData.Properties().m_strArtist != dlg.m_lyrProperties.m_strArtist)
			{
				g_LyricData.Properties().m_strArtist = dlg.m_lyrProperties.m_strArtist;
			}
			if (g_LyricData.Properties().m_strAlbum != dlg.m_lyrProperties.m_strAlbum)
			{
				g_LyricData.Properties().m_strAlbum = dlg.m_lyrProperties.m_strAlbum;
			}
			if (g_LyricData.Properties().m_strBy != dlg.m_lyrProperties.m_strBy)
			{
				g_LyricData.Properties().m_strBy = dlg.m_lyrProperties.m_strBy;
			}
			if (g_LyricData.Properties().m_nTimeOffset != dlg.m_lyrProperties.m_nTimeOffset)
			{
				g_LyricData.Properties().m_nTimeOffset = dlg.m_lyrProperties.m_nTimeOffset;
			}
			g_eventDispather.DispatchSyncEvent(ET_LYRICS_EDITOR_RELOAD_TAG);
		}
		break;
	//
	// Lyrics editor command
	//
	case CMD_INSERTTAG_DOWN:
	case CMD_INSERTTAG:
	case CMD_FORWARD_CUR_LINE:
	case CMD_BACKWARD_CUR_LINE:
	case CMD_FORWARD_REMAIN_LINES:
	case CMD_BACKWARD_REMAIN_LINES:
	case CMD_JUMP:
	case CMD_DEL_TAG:
	case CMD_EDIT_UNDO:
	case CMD_EDIT_REDO:
	case CMD_EDIT_CUT:
	case CMD_EDIT_COPY:
	case CMD_EDIT_PASTE:
	case CMD_EDIT_DELETE:
	case CMD_EDIT_FIND:
	case CMD_EDIT_FINDNEXT:
	case CMD_EDIT_REPLACE:
	case CMD_AUTO_FILL_LYR_INFO:
	case CMD_REMOVE_ALL_TAG:
	case CMD_REMOVE_BLANK_LINE:
	case CMD_TRIM_WHITESPACE:
	case CMD_REMOVE_UNSYNC_LINES:
	case CMD_CAPITALIZE_LEADING_LETTER:
	case CMD_LYRICS_TO_LOWERCASE:
		{
			CEventLyricsEditorCmd		*pEvent = new CEventLyricsEditorCmd;
			pEvent->eventType = ET_LYRICS_EDITOR_CMD;
			pEvent->cmd = nID;
			g_eventDispather.DispatchSyncEvent(pEvent);
		}
		break;
	case CMD_MENU_LYR_EDIT:
		{
			POINT	pt;
			CMLMenu	menu;
			MLWidget::GetCursorPos(&pt);
			menu.LoadPopupMenu(IDM_MENU, 2);
			menu.TrackPopupMenu(pt.x, pt.y, m_pSkinWnd);
		}
		break;

	case CMD_MENU:
		{
			CPoint	pt;

			MLWidget::GetCursorPos(&pt);
			m_pSkinWnd->OnContexMenu(pt.x, pt.y);
		}
		break;
	case CMD_QUIT:
		g_mpMsgWnd.PostQuitMPMessage();
		break;
	case CMD_CLOSE:
#ifdef _WIN32
		::PostMessage(m_pSkinWnd->GetHandle(), WM_CLOSE, 0, 0);
#else
		m_pSkinWnd->Destroy();
#endif
		// m_pSkinWnd->Destroy();
		break;
	case ID_ALBUMART:
		{
			// album art
			CMLMenu		menu;
			CPoint		pt;

			MLWidget::GetCursorPos(&pt);
			menu.LoadPopupMenu(IDR_MENU_ALBUMART, 0);
			menu.TrackPopupMenu(pt.x, pt.y, m_pSkinWnd);
		}
		break;
	case CMD_RATE_LYR:
		{
			if (!g_LyricData.IsOK())
			{
				m_pSkinWnd->MessageOut(TOLOCAL(_T("No Lyrics file was opened.")));
				break;
			}
			TCHAR		szMD5[64];
			tstring		strUrl;

			EmptyStr(szMD5);
			if (g_mpApp.IsCachedMd5Lyrics())
				_tcscpy(szMD5, g_mpApp.m_strMd5Lyrics.c_str());
			else
				g_LyricData.GetLyricsFileServerMark(szMD5);
			strUrl = GetStrName(SN_HTTP_RATE_LRC);
			strUrl += szMD5;

			// show rate dialog
			CDlgRateLyrics	dlg;
			dlg.m_strLink = strUrl;
			dlg.DoModal(m_pSkinWnd);
		}
		break;
	default:
		return false;
	}

	return true;
}

bool CMPCommonCmdHandler::OnUIObjNotify(IUIObjNotify *pNotify)
{
	if (pNotify->nID == CMD_SEEK)
	{
		if (pNotify->pUIObject->IsKindOf(CSkinSeekCtrl::ClassName()))
		{
			CSkinSeekCtrlEventNotify	*pListCtrlNotify = (CSkinSeekCtrlEventNotify *)pNotify;
			if (pListCtrlNotify->cmd == CSkinSeekCtrlEventNotify::C_BEG_DRAG)
			{
				// Use seek time instead of playing time.
				g_Player.SetUseSeekTimeAsPlayingTime(true);
			}
			else if (pListCtrlNotify->cmd == CSkinSeekCtrlEventNotify::C_END_DRAG)
			{
				// Use seek time instead of playing time.
				g_Player.SetUseSeekTimeAsPlayingTime(false);
			}
		}

		return true;
	}

	return false;
}

bool CMPCommonCmdHandler::GetChecked(UINT nID, bool &bChecked)
{
	switch (nID)
	{
	case IDC_LDO_KARAOKE:
		bChecked = g_Profile.GetProfileBool(m_strSectName.c_str(), _T("Karaoke"), false);
		break;
	case IDC_FLOATING_LYRICS:
		bChecked = g_wndFloatingLyr.IsValid();
		break;
	default:
		return false;
	}

	return true;
}

bool CMPCommonCmdHandler::GetRadioChecked(vector<UINT> &vIDs, UINT &nIDChecked)
{
	switch (vIDs[0])
	{
	case IDC_LDO_NORMAL:
	case IDC_LDO_FADE_IN:
	case IDC_LDO_FADEOUT_BG:
		{
			tstring		strDrawOptions;

			strDrawOptions = g_Profile.GetProfileString(m_strSectName.c_str(), _T("LyrDrawOpt"), _T("fadein"));
			if (_tcsicmp(strDrawOptions.c_str(), _T("normal")) == 0)
				nIDChecked = IDC_LDO_NORMAL;
			else if (_tcsicmp(strDrawOptions.c_str(), _T("fadeout")) == 0)
				nIDChecked = IDC_LDO_FADEOUT_BG;
			else
				nIDChecked = IDC_LDO_FADE_IN;
		}
		break;
	case IDC_ALIGN_LYRICS_LEFT:
	case IDC_ALIGN_LYRICS_CENTER:
	case IDC_ALIGN_LYRICS_RIGHT:
		{
			tstring		strAlign;
			strAlign = g_Profile.GetProfileString(m_strSectName.c_str(), _T("LyrAlign"), _T("center"));
			if (_tcsicmp(strAlign.c_str(), _T("left")) == 0)
				nIDChecked = IDC_ALIGN_LYRICS_LEFT;
			else if (_tcsicmp(strAlign.c_str(), _T("right")) == 0)
				nIDChecked = IDC_ALIGN_LYRICS_RIGHT;
			else
				nIDChecked = IDC_ALIGN_LYRICS_CENTER;
		}
		break;
	default:
		return false;
	}

	return true;
}

bool CMPCommonCmdHandler::OnCommandCharEncoding(int nCmdId)
{
	// #ifdef _UNICODE
	//	将ANSI的歌词设置编码，并重新打开
	// #else _ANSI
	//  设置UNICODE 歌词的编码，并重新打开，同时设置字体的编码

	_CMDID_ENCODING		&cmdIdCharset = CmdIdToEncoding(nCmdId);
	if (cmdIdCharset.nCmdId != -1)
	{
		if (!g_LyricData.IsOK())
			return true;

		//
		// if (修改了歌词)
		//		提示需要重新加载歌词，提示用户是否继续
		if (g_LyricData.IsContentModified())
		{
			tstringex		str;

			str += TOLOCAL(_T("Change character encoding must reload lyrics, and your modification will be lost."));
			str += _T("\r\n");
			str += TOLOCAL(_T("Do you want to continue?"));
			if (m_pSkinWnd->MessageOut(str.c_str(), MB_ICONINFORMATION | MB_YESNO) != IDYES)
				return true;
		}

		// 以用户指定的编码重新打开歌词
		ENCODING_CODEPAGE &encodingNew = GetCharEncodingByID(cmdIdCharset.nEncodingId);
		ENCODING_CODEPAGE &encodingOld = GetCharEncodingByName(g_LyricData.GetEncoding());
		if (encodingNew.nEncodingId != encodingOld.nEncodingId)
		{
			TCHAR		szEncoding[256];
			_xcscpy(szEncoding, encodingNew.szEncoding);

			g_LyricData.ReopenLyrics(TRUE, 
				GetCharEncodingID(encodingNew.szEncoding));

			g_mpApp.DispatchLyricsChangedSyncEvent();
		}
		return true;
	}
	return false;
}

//
// 设置歌词显示背景图片
//
void CMPCommonCmdHandler::OnCommandSetBkImg()
{
	CFolderDialog	dlg;

	dlg.SetInitFolder(CMLProfile::GetDir(m_strSectName.c_str(), _T("BgPicFolder"), _T("")).c_str());
	if (dlg.DoBrowse(m_pSkinWnd))
	{
		CMPlayerSettings::SetSettings(m_etDispSettings, m_strSectName.c_str(), _T("UseBgImage"), SZ_TRUE);
		CMPlayerSettings::SetSettings(m_etDispSettings, m_strSectName.c_str(), _T("BgPicFolder"), dlg.GetFolder());
		CMLProfile::WriteDir(m_strSectName.c_str(), _T("BgPicFolder"), dlg.GetFolder());
	}
}

#ifndef _WIN32_WCE

void CMPCommonCmdHandler::OnCommandOpenLyricsInMp3()
{
	TCHAR		szSongFile[MAX_PATH];
	CDlgEmbeddedLyricsLoad		dlg;
	int			nRet;

	g_Player.GetSourceMedia(szSongFile, MAX_PATH);

	if (dlg.DoModal(m_pSkinWnd, szSongFile) != IDOK)
		return;

	nRet = g_mpApp.OpenLyrics(szSongFile, dlg.GetSelLyrics());
	if (nRet != ERR_OK)
		m_pSkinWnd->MessageOut((LPCTSTR)ERROR2STR_LOCAL(nRet));
	else
	{
		// associate the embedded lyrics.
		g_LyricSearch.AssociateLyrics(szSongFile, dlg.GetSelLyrics());
	}
}

void CMPCommonCmdHandler::OnCommandSaveLyricsToMp3()
{
	CDlgEmbeddedLyricsSave		dlg;
	TCHAR			szSongFile[MAX_PATH];
	CDlgSelFileEncoding	dlgFileEncoding;

	g_Player.GetSourceMedia(szSongFile, MAX_PATH);

	if (!FileIsExtSame(szSongFile, _T(".mp3")))
	{
		m_pSkinWnd->MessageOut(TOLOCAL(_T("This file doesn't support embedded lyrics.")));
		return;
	}

	if (!g_LyricData.IsOK())
	{
		m_pSkinWnd->MessageOut(TOLOCAL(_T("No Lyrics file was opened.")));
		return;
	}


	dlgFileEncoding.m_fileEncoding = g_LyricData.GetFileEncoding();
	dlgFileEncoding.m_bHideUtf8 = true;

	//
	// 需要让用户选择保存的字符串的编码类型
	if (dlgFileEncoding.DoModal(m_pSkinWnd) != IDOK)
		return;

#ifdef _UNICODE
	// 只有在UNICODE模式下，选择保存为 ANSI 文件时，才有必要选择编码
	if (dlgFileEncoding.m_fileEncoding == FE_ANSI)
	{
		CDlgSelMbcsEncoding	dlgMbcsEncoding;

		dlgMbcsEncoding.m_strEncoding = g_LyricData.GetEncoding();
		g_LyricData.ToString(dlgMbcsEncoding.m_strLyrics, FT_LYRICS_TXT, TRUE);

		if (!IsAnsiStr(dlgMbcsEncoding.m_strLyrics.c_str()))
		{
			if (dlgMbcsEncoding.DoModal(m_pSkinWnd) != IDOK)
				return;

			g_LyricData.SetEncoding(dlgMbcsEncoding.m_strEncoding.c_str());
		}
	}
#endif

	dlg.DoModal(m_pSkinWnd, szSongFile, dlgFileEncoding.m_fileEncoding, g_LyricData.GetEncodingIndex());
}

void CMPCommonCmdHandler::OnCommandRemoveLyricsInMp3()
{
	TCHAR		szSongFile[MAX_PATH];
	CDlgEmbeddedLyricsRemove		dlg;

	g_Player.GetSourceMedia(szSongFile, MAX_PATH);

	if (!FileIsExtSame(szSongFile, _T(".mp3")))
	{
		m_pSkinWnd->MessageOut(TOLOCAL(_T("This file doesn't support embedded lyrics.")));
		return;
	}

	dlg.DoModal(m_pSkinWnd, szSongFile);
}

#endif // _WIN32_WCE

// COMMENT:
//		保存歌词文件为：
//		FT_UNKNOWN			= 1,
//		FT_LYRICS_LRC		= 2,
//		FT_LYRICS_TXT		= 3,
// RETURN:
//		TRUE	-	保存成功，注意：如果没有歌词数据也返回TRUE；
//		FALSE	-	用户选择取消，或者保存失败;
BOOL CMPCommonCmdHandler::SaveAsLyricsFile(CBaseWnd *pWndParent, MLFileType DefFileType)
{
	TCHAR			szFile[MAX_PATH] = _T("");
	TCHAR			szTitle[MAX_PATH] = _T("");
	tstring			strLyrics;
	CDlgSelFileEncoding	dlgFileEncoding;

	if (!g_LyricData.IsOK())
	{
		pWndParent->MessageOut(TOLOCAL(_T("No Lyrics file was opened.")));
		return TRUE;
	}

	dlgFileEncoding.m_fileEncoding = g_LyricData.GetFileEncoding();

	g_LyricData.ToString(strLyrics, FT_LYRICS_TXT, TRUE);

	//
	// 当字符不是 ANSI 字符时，需要让用户选择保存的文件编码类型
	if (!IsAnsiStr(strLyrics.c_str()))
	{
		if (dlgFileEncoding.DoModal(pWndParent) != IDOK)
			return FALSE;

#ifdef _UNICODE
		// 只有在UNICODE模式下，选择保存为 ANSI 文件时，才有必要选择编码
		if (dlgFileEncoding.m_fileEncoding == FE_ANSI)
		{
			CDlgSelMbcsEncoding	dlgMbcsEncoding;

			dlgMbcsEncoding.m_strEncoding = g_LyricData.GetEncoding();
			g_LyricData.ToString(dlgMbcsEncoding.m_strLyrics, FT_LYRICS_TXT, TRUE);

			if (dlgMbcsEncoding.DoModal(pWndParent) != IDOK)
				return FALSE;

			g_LyricData.SetEncoding(dlgMbcsEncoding.m_strEncoding.c_str());
		}
#endif
	}

	// 设置新的文件名
	_tcscpy_safe(szFile, CountOf(szFile), g_LyricData.GetLyricsFileName());
	// make sure the dialog will be displayed.
	if (!IsFileExist(szFile))
	{
		EmptyStr(szFile);
		if (!g_LyricData.Properties().m_strArtist.empty())
		{
			_tcscpy_safe(szFile, CountOf(szFile), g_LyricData.Properties().m_strArtist.c_str());
			_tcscat_safe(szFile, CountOf(szFile), _T(" - "));
		}

		_tcscat_safe(szFile, CountOf(szFile), g_LyricData.Properties().m_strTitle.c_str());

		FileNameFilterInvalidChars(szFile);
	}

	// set saved file extention
	if (DefFileType == FT_LYRICS_LRC)
		FileSetExt(szFile, CountOf(szFile), _T(".lrc"));
	else if (DefFileType == FT_LYRICS_TXT)
		FileSetExt(szFile, CountOf(szFile), _T(".txt"));
	else if (DefFileType == FT_LYRICS_SNC)
		FileSetExt(szFile, CountOf(szFile), _T(".snc"));

	if (!IsFileExist(szFile))
	{
		// save as new file...
		// Is save in song file dir with song file name?
		DOWN_SAVE_DIR		DownSaveDir;
		DOWN_SAVE_NAME		DownSaveName;
		TCHAR				szDir[MAX_PATH] = _T("");
		TCHAR				szName[MAX_PATH] = _T("");
		TCHAR				szExt[MAX_PATH];

		FileGetPath(szDir, CountOf(szDir), szFile);
		FileGetTitle(szName, CountOf(szName), szFile);
		_tcscpy_safe(szExt, CountOf(szExt), FileGetExt(szFile));

		DownSaveDir = (DOWN_SAVE_DIR)g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("DownSaveInSongDir"), DOWN_SAVE_IN_CUSTOM_DIR);
		if (DownSaveDir == DOWN_SAVE_IN_SONG_DIR)
		{
			FileGetPath(szDir, CountOf(szDir), g_Player.GetSrcMedia());
		}

		DownSaveName = (DOWN_SAVE_NAME)g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("DownSaveName"), DOWN_SAVE_NAME_KEEP);
		if (DownSaveName == DOWN_SAVE_NAME_AS_SONG_NAME)
		{
			FileGetTitle(szName, CountOf(szName), g_Player.GetSrcMedia());
		}
		_tcscpy_safe(szFile, CountOf(szFile), szDir);
		_tcscat_safe(szFile, CountOf(szFile), szName);
		_tcscat_safe(szFile, CountOf(szFile), szExt);
	}

	TCHAR		szWndTitle[256];
	int			nFileExtIndex;
	if (DefFileType == FT_LYRICS_SNC)
		nFileExtIndex = 4;
	else if (DefFileType == FT_LYRICS_TXT)
		nFileExtIndex = 3;
	else if (DefFileType == FT_LYRICS_LRC)
		nFileExtIndex = 2;
	else
		nFileExtIndex = 1;

	_tcscpy(szWndTitle, TOLOCAL(_T("Save As")));
	CFileSaveDlg		dlg(szWndTitle, szFile,
		_T("All supported files (*.lrc; *.txt; *.snc)\0*.lrc;*.txt;*.snc\0LRC Lyrics File (*.lrc)\0*.lrc\0Text File (*.txt)\0*.txt\0Snc File (*.snc)\0*.snc\0\0"),
		nFileExtIndex);

	if (dlg.DoModal(pWndParent) == IDOK)
	{
		_tcscpy_safe(szFile, CountOf(szFile), dlg.GetSaveFile());
		if (IsEmptyString(FileGetExt(szFile)))
		{
			// Set file extend name as the user selected.
			FileSetExt(szFile, CountOf(szFile), dlg.GetSelectedExt());
		}
		MLFileType	fileType = MLGetFileType(szFile);
		if (fileType != FT_LYRICS_LRC && fileType != FT_LYRICS_TXT
			&& fileType != FT_LYRICS_SNC)
		{
			return FALSE;
		}
		bool	bUseNewFileName;
		int		nRet = g_LyricData.SaveAsFile(szFile, dlgFileEncoding.m_fileEncoding, bUseNewFileName);
		if (nRet != ERR_OK)
		{
			pWndParent->MessageOut(ERROR2STR_LOCAL(nRet));
			return FALSE;
		}
		if (bUseNewFileName)
			g_LyricSearch.AssociateLyrics(g_LyricData.GetSongFileName(), g_LyricData.GetLyricsFileName());
	}
	else
		return FALSE;

	return TRUE;
}
