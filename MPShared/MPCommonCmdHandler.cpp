// MPCommonCmdHandler.cpp: implementation of the CMPCommonCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MLCmd.h"
#include "MPCommonCmdHandler.h"
#include "LyricShowAgentObj.h"
#include "LyricShowTextEditObj.h"
#include "SkinRateCtrl.h"
#include "DlgSearchLyrics.h"
#include "../WidgetLite/win32/FileOpenDlg.h"
#include "DlgUpload.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "DlgSaveEmbeddedLyrics.h"
#include "PreferenceDlg.h"
#include "DlgAbout.h"
#include "MPSkinMenu.h"
#include "LyricsTool.h"
#ifdef _WIN32
#include "DlgAdjustHue.h"
#endif

#include "MPHelper.h"

#ifdef _WIN32_DESKTOP
#include "win32/MPFloatingLyrWnd.h"
#else
#include "MPFloatingLyrWnd.h"
#endif

bool		g_bInModalDlLrcSelDlg;

const int SET_SPEED_SPAN = 500;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPCommonCmdHandler::CMPCommonCmdHandler(bool bFloatingLyr)
{
	CMPlayerApp::GetInstance()->GetCurLyrDisplaySettingName(bFloatingLyr, m_strSectName, m_etDispSettings);
}

CMPCommonCmdHandler::~CMPCommonCmdHandler()
{

}

// if the command id is processed, return true.
bool CMPCommonCmdHandler::OnCommand(int nId)
{
	switch (nId)
	{
	case IDC_REPEAT_OFF:
		g_Player.SetLoop(MP_LOOP_OFF);
		break;
	case IDC_REPEAT_ALL:
		g_Player.SetLoop(MP_LOOP_ALL);
		break;
	case IDC_REPEAT_TRACK:
		g_Player.SetLoop(MP_LOOP_TRACK);
		break;
	case IDC_RATE_LYR_1:
	case IDC_RATE_LYR_2:
	case IDC_RATE_LYR_3:
	case IDC_RATE_LYR_4:
	case IDC_RATE_LYR_5:
		{
			int nRate;
			if (nId == IDC_RATE_LYR_2) nRate = 2;
			else if (nId == IDC_RATE_LYR_3) nRate = 3;
			else if (nId == IDC_RATE_LYR_4) nRate = 4;
			else if (nId == IDC_RATE_LYR_5) nRate = 5;
			else nRate = 1;

			tstring		strUrl;

			strUrl = GetStrName(SN_HTTP_RATE_LRC);
			strUrl += g_LyricData.Properties().m_strId;

			strUrl += CStrPrintf(_T("&rating=%d"), nRate).c_str();
			OpenUrl(m_pSkinWnd, strUrl.c_str());
		}
		break;

	case IDC_SL_CLEAR_TIME_STAMP:
		{
			if (g_LyricData.GetLyrContentType() == LCT_TXT)
			{
				CLyricShowTxtObj *pObj = (CLyricShowTxtObj*)m_pSkinWnd->GetUIObjectByClassName(CLyricShowTxtObj::ClassName());
				if (pObj)
					g_LyricData.ClearRecordedLyrScrollActions(pObj->GetLyrics());
			}
		}
		break;
	case IDC_EDITOR_LYR_COLOR:
	case IDC_EDITOR_HIGH_COLOR:
	case IDC_TAG_COLOR:
	case IDC_EDIT_LINE_COLOR:
	case IDC_EDITOR_BG_COLOR:
		{
			CDlgChooseColor		dlg;
			MLColor				clr(RGB(0, 0, 0));
			LPCTSTR				szColorValueName = NULL;

			if (nId == IDC_EDITOR_LYR_COLOR)
				szColorValueName = _T("Ed_LowColor");
			else if (nId == IDC_EDITOR_HIGH_COLOR)
				szColorValueName = _T("Ed_HighColor");
			else if (nId == IDC_TAG_COLOR)
				szColorValueName = _T("Ed_TagColor");
			else if (nId == IDC_EDITOR_BG_COLOR)
				szColorValueName = _T("Ed_BgColor");
			else
				szColorValueName = _T("Ed_FocusLineBgColor");

			ProfileGetColorValue(clr, SZ_SECT_LYR_DISPLAY, szColorValueName);

			if (dlg.DoModal(m_pSkinWnd, clr) == IDOK)
			{
				clr = dlg.GetColor();
				tstring		strColor;
				ColorToStr(clr, strColor);
				CMPlayerSettings::SetSettings(ET_LYRICS_DISPLAY_SETTINGS, 
					SZ_SECT_LYR_DISPLAY, szColorValueName, strColor.c_str());
			}
		}
		break;

	case IDC_IPOD_LYRICS_DOWNLOADER:
		CMPlayerApp::GetInstance()->RestartToAppMode(SA_IPOD_LYRICS_DOWNLOADER);
		break;

	case IDC_LDO_KARAOKE:
		CMPlayerSettings::SetSettings(m_etDispSettings, m_strSectName.c_str(), _T("Karaoke"), 
			!g_Profile.GetProfileBool(m_strSectName.c_str(), _T("Karaoke"), false));
		break;
	case IDC_LDO_NORMAL:
	case IDC_LDO_FADE_IN:
	case IDC_LDO_FADEOUT_BG:
		{
			DISPLAY_OPTIONS	displayOpt;

			if (nId == IDC_LDO_NORMAL)
				displayOpt = DO_NORMAL;
			else if (nId == IDC_LDO_FADE_IN)
				displayOpt = DO_FADEOUT_LOWCOLOR;
			else if (nId == IDC_LDO_FADEOUT_BG)
				displayOpt = DO_FADEOUT_BG;
			else
				return false;

			CMPlayerSettings::SetSettings(m_etDispSettings, m_strSectName.c_str(), _T("LyrDrawOpt"), 
				DisplayOptToStr(displayOpt));
		}
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
			}
			GfxRaw::EnableAntialiasFont(bAntialias);

			CMPlayerSettings::SetPlayerUISettings(_T("Antialias"), bAntialias);
		}
		break;
#endif // _WIN32_DESKTOP
	case IDC_WEBHOME:
		OpenUrl(m_pSkinWnd, GetStrName(SN_HTTP_DOMAIN));
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
	case IDC_REGISTER:
		ShowActivateDialog(m_pSkinWnd);
		break;
// 	case IDC_MINIMIZE:
// #if !defined(_MPLAYER) && defined(_WIN32_DESKTOP)
// 		if (::GetParent(CMPlayerAppBase::GetMainWnd()->GetHandle()))
// 			break;
// #endif
// 		CMPlayerAppBase::GetMPSkinFactory()->MinizeAll();
// 		break;
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
// 	case IDC_VIEW_AS_TEXT:
// 		{
// 			// 转换尾纯文本，并使用用记事本打开
// 			TCHAR		szTempTxtFile[MAX_PATH];
// 
// 			GetAppDataDir(szTempTxtFile);
// 			_tcscat_safe(szTempTxtFile, CountOf(szTempTxtFile), _T("TempTxtLyrics.txt"));
// 			g_LyricData.SaveToTextFile(szTempTxtFile, true);
// 
// 			// 用记事本打开
// 			Execute(m_pSkinWnd, szTempTxtFile, NULL);
// 		}
// 		break;
#ifdef _WIN32_WCE
	case IDC_MEDIALIB:
		CMPlayerAppBase::GetMPSkinFactory()->OpenOrCloseSkinWnd(false, _SZ_SKINWND_CLASS_NAME, _T("Media Library"), _T("medialib"));
		break;
	case IDC_PLAYLIST:
		CMPlayerAppBase::GetMPSkinFactory()->OpenOrCloseSkinWnd(false, _SZ_SKINWND_CLASS_NAME, _T("Playlist"), _T("Playlist"));
		break;
	case IDC_LYRICS_VIEWER:
		CMPlayerAppBase::GetMPSkinFactory()->OpenOrCloseSkinWnd(false, _SZ_SKINWND_CLASS_NAME, _T("Lyrics"), _T("lyrics"));
		break;
#endif
#ifdef _WIN32
	case IDC_ADJUST_HUE:
		{
			ShowAdjustHueDialog(m_pSkinWnd);
		}
		break;
#endif
	case IDC_ABOUT:
		{
/*			CSkinWnd	*pWnd = new CDlgAbout();
			int nRet = CMPlayerAppBase::GetMPSkinFactory()->OpenOrCloseSkinWnd(_T("about_box"),
				_T("AboutBox"),
				_T("AboutBox.xml"), CMPlayerAppBase::GetMainWnd(), &pWnd);*/
			ShowAboutDialog(m_pSkinWnd);
		}
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

			CFileDlgExtFilter		strFileExt;
			strFileExt.AddExtention(TOLOCAL_T("All Picture Files"), _T("*.jpg;*.gif;*.bmp;*.png"));
			strFileExt.AddExtention(TOLOCAL_T("JPEG (*.jpg)"), _T("*.jpg"));
			strFileExt.AddExtention(TOLOCAL_T("PNG (*.png)"), _T("*.png"));
			strFileExt.AddExtention(TOLOCAL_T("Bitmap files (*.bmp)"), _T("*.bmp"));
			CFileOpenDlg		dlg(TOLOCAL_T("Browse album art picture file"), _T(""), 
				strFileExt.c_str(), 0);

			if (dlg.DoModal(m_pSkinWnd) != IDOK)
				break;

			strAlbumArtFileName += FileGetExt(dlg.GetOpenFile());

			if (_tcsicmp(strAlbumArtFileName.c_str(), dlg.GetOpenFile()) == 0)
				break;

			CopyFile(dlg.GetOpenFile(), strAlbumArtFileName.c_str(), false);

			CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(ET_PLAYER_CUR_MEDIA_INFO_CHANGED);
		}
		break;
	case IDC_HELP_STATIC_LYR:
		OpenUrl(m_pSkinWnd, GetStrName(SN_HTTP_HELP_STATIC_LYR));
		break;
	default:
		{
			if (OnCommandCharEncoding(nId))
				break;
			if (OnCommandSkin(nId))
				break;
			return false;
		}
	}

	return true;
}

bool CMPCommonCmdHandler::OnCustomCommand(int nID)
{
	switch (nID)
	{
#ifdef _WIN32_DESKTOP
	case CMD_TOGGLE_MP:
		{
			if (m_pSkinWnd->IsIconic() || ::GetForegroundWindow() != m_pSkinWnd->GetHandle())
				m_pSkinWnd->ActivateWindow();
			else
			{
				m_pSkinWnd->ShowWindow(SW_MINIMIZE);
				if (m_pSkinWnd->IsToolWindow())
					m_pSkinWnd->ShowWindow(SW_HIDE);
			}
		}
		break;
	case CMD_PREFERENCES:
		ShowPreferenceDialog(m_pSkinWnd, m_etDispSettings != ET_LYRICS_FLOATING_SETTINGS);
		break;
	case CMD_DISPLAY_OPT:
		ShowPreferenceDialog(m_pSkinWnd, m_etDispSettings != ET_LYRICS_FLOATING_SETTINGS, PAGE_LYR_DISPLAY);
		break;
	case CMD_FLOATING_LYRICS:
		{
			g_Profile.WriteProfileInt(_T("FloatingLyr"), !g_wndFloatingLyr.IsValid());
			if (!g_wndFloatingLyr.IsValid())
				g_wndFloatingLyr.Create();
			else
				g_wndFloatingLyr.Destroy();
		}
		break;
#endif // _WIN32_DESKTOP

	case CMD_LYR_SCROLL_ENABLE_RECORD:
	case CMD_LYR_SCROLL_ENABLE_REPLAY:
		{
			CLyricShowTxtObj *pObj = (CLyricShowTxtObj*)m_pSkinWnd->GetUIObjectByClassName(CLyricShowTxtObj::ClassName());
			if (pObj)
			{
				if (nID == CMD_LYR_SCROLL_ENABLE_RECORD)
					pObj->EnableRecordScrollingActions(!pObj->IsRecordScrollingActionsEnabled());
				else
					pObj->EnableReplayScrollingActions(!pObj->IsReplayScrollingActionsEnabled());
			}
		}
		break;

	case CMD_LYR_SCROLL_MENU:
		{
			CMLMenu menu;
			POINT pt;
			MLWidget::GetCursorPos(&pt);
			menu.LoadPopupMenu(IDR_MENU_STATIC_LYR, 0);
			menu.EnableItem(IDC_SL_CLEAR_TIME_STAMP, g_LyricData.GetLyrContentType() == LCT_TXT);
			menu.TrackPopupMenu(pt.x, pt.y, m_pSkinWnd, NULL);
		}
		break;
	case CMD_LYR_EDITOR:
		{
			SkinWndStartupInfo skinWndStartupInfo(_T("LyrEditor"), TOLOCAL(_T("Lyrics Editor")), _T("LyricsEditor.xml"), NULL);
			CMPlayerAppBase::GetMPSkinFactory()->ActiveOrCreateSkinWnd(skinWndStartupInfo);
		}
		break;
	case CMD_EDIT_HELP:
		OpenUrl(m_pSkinWnd, GetStrName(SN_HTTP_HELP_EDIT_LYR));
		break;
	case CMD_HELP:
		{
			CUIObject	*pObjLyrEdit = m_pSkinWnd->GetUIObjectByClassName(CLyricShowTextEditObj::ClassName());
			if (pObjLyrEdit && pObjLyrEdit->IsVisible())
				OpenUrl(m_pSkinWnd, GetStrName(SN_HTTP_HELP_EDIT_LYR));
			else
				OpenUrl(m_pSkinWnd, GetStrName(SN_HTTP_HELP));
		}
		break;
	case CMD_PAUSE:
		g_Player.Pause();
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
	case CMD_VOL_INC:
		{
			long		vol;
			vol = g_Player.GetVolume();
			if (vol < MP_VOLUME_MAX)
			{
				vol += 5;
				vol -= vol % 5;
				if (vol > MP_VOLUME_MAX)
					vol = MP_VOLUME_MAX;
				g_Player.SetVolume(vol);
				CMPlayerAppBase::GetInstance()->DispatchInfoText(CStrPrintf(_T("%s %d%%"), TOLOCAL(_T("Set Volume")), vol).c_str());
			}
		}
		break;
	case CMD_VOL_DEC:
		{
			long		vol;
			vol = g_Player.GetVolume();
			if (vol > 0)
			{
				vol -= 5;
				vol -= vol % 5;
				if (vol < 0)
					vol = 0;
				g_Player.SetVolume(vol);
				CMPlayerAppBase::GetInstance()->DispatchInfoText(CStrPrintf(_T("%s %d%%"), TOLOCAL(_T("Set Volume")), vol).c_str());
			}
		}
		break;
	case CMD_MUTE:
		{
			bool		bMute = !g_Player.IsMute();
			g_Player.SetMute(bMute);
			CMPlayerAppBase::GetInstance()->DispatchInfoText(TOLOCAL(bMute ? _T("Mute On") : _T("Mute Off")));
		}
		break;
	case CMD_SHUFFLE:
		{
			bool		bShuffle = !g_Player.IsShuffle();
			g_Player.SetShuffle(bShuffle);
			CMPlayerAppBase::GetInstance()->DispatchInfoText(TOLOCAL(bShuffle ? _T("Shuffle On") : _T("Shuffle Off")));
		}
		break;
	case CMD_LOOP:
		{
			g_Player.SetToNextLoopMode();

			tstring		str;
			MP_LOOP_MODE loopMode = g_Player.GetLoop();
			if (loopMode == MP_LOOP_OFF)
				str = _T("Repeat Off");
			else if (loopMode == MP_LOOP_TRACK)
				str =  _T("Repeat Track");
			else
				str =  _T("Repeat All");

			CMPlayerAppBase::GetInstance()->DispatchInfoText(str.c_str());
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

				if (g_Player.GetMediaLibrary(&pMediaLib) != ERR_OK)
					break;

				if (g_Player.GetCurrentMedia(&pMedia) == ERR_OK)
				{
					pMediaLib->Rate(pMedia, pRateCtrl->GetRating());
				}
			}
		}
		break;
	case CMD_PL_OPEN_FILE:
		OnSongOpenFileCmd(m_pSkinWnd, true);
		break;
	case CMD_PL_ADD_FILE:
		OnSongOpenFileCmd(m_pSkinWnd, false);
		break;
	case CMD_PL_OPEN_DIR:
		OnSongOpenDirCmd(m_pSkinWnd, true);
		break;
	case CMD_PL_ADD_DIR:
		OnSongOpenDirCmd(m_pSkinWnd, false);
		break;

		//
		// Lyrics ....
		//
	case CMD_NEW_LRC:
		{
			CMPlayerAppBase::GetInstance()->GetEventsDispatcher()->DispatchSyncEvent(ET_LYRICS_ON_SAVE_EDIT);

			if (g_LyricData.IsContentModified())
			{
				tstring		strMessage = CStrPrintf(TOLOCAL(_T("The lyrics of the %s file have changed.")), 
					FileGetName(g_LyricData.GetSongFileName())).c_str();
				strMessage += _T("\r\n\r\n");
				strMessage += TOLOCAL(_T("Do you want to save the changes?"));
				int nRet = m_pSkinWnd->MessageOut(strMessage.c_str(), MB_ICONQUESTION | MB_YESNOCANCEL);
				if (nRet == IDCANCEL)
					break;
				else if (nRet == IDYES)
				{
					if (!CMPCommonCmdHandler::SaveCurrentLyrics(m_pSkinWnd, false))
						break;
				}
			}
			CMPlayerAppBase::GetInstance()->NewLyrics();
		}
		break;
	case CMD_OPEN_LRC:
		//
		// 歌词下载
		//
		{
			if (!g_bInModalDlLrcSelDlg)
			{
				g_bInModalDlLrcSelDlg = TRUE;

				ShowSearchLyricsDialog(m_pSkinWnd);
				g_bInModalDlLrcSelDlg = FALSE;
			}
		}
		break;
	case CMD_SAVE_LRC:
		SaveCurrentLyrics(m_pSkinWnd, true);
		break;
	case CMD_SAVE_LRC_AS:
		{
			// notify to save...
			IEvent		*pEvent = new IEvent;
			pEvent->eventType = ET_LYRICS_ON_SAVE_EDIT;
			CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(pEvent);

			SaveAsLyricsFile(m_pSkinWnd,
				LyricsConentTypeToFileType(g_LyricData.GetLyrContentType()), false);
		}
		break;
	case CMD_SAVE_LYR_IN_SONG_FILE:
		{
			if (!g_LyricData.HasLyricsOpened())
			{
				m_pSkinWnd->MessageOut(TOLOCAL(_T("No Lyrics file was opened.")));
				break;
			}

			CDlgSaveEmbeddedLyrics dlg;
			dlg.DoModal(m_pSkinWnd);
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
				CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);

				CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(ET_LYRICS_EDITOR_RELOAD_TAG);

				// CMPlayerAppBase::GetInstance()->DispatchInfoText(TOLOCAL(_T("Lyrics Forward 0.5 Sec")));
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
				CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);

				CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(ET_LYRICS_EDITOR_RELOAD_TAG);

				// CMPlayerAppBase::GetInstance()->DispatchInfoText(TOLOCAL(_T("Lyrics Backward 0.5 Sec")));
			}
		}
		break;

	case CMD_FONT_SIZE_INC:
	case CMD_FONT_SIZE_DEC:
		{
			// Increase the font size.
			int		nHeight, nWeight;
			BYTE	byItalic;
			tstring	strFaceNameLatin9, strFaceNameOthers;

			// get default settings in profile
			ProfileGetLyricsFont(m_strSectName.c_str(), nHeight, nWeight, 
				byItalic, strFaceNameLatin9, strFaceNameOthers);

			if (nID == CMD_FONT_SIZE_INC)
			{
				nHeight++;
				if (nHeight >= 96 * 2)	// 96 is the max font size in font choosing dialog.
					break;
			}
			else
			{
				nHeight--;
				if (nHeight < 8)		// 8 is a reasonable smallest font size.
					break;
			}

			// now save to profile
			ProfileWriteLyricsFont(m_etDispSettings, m_strSectName.c_str(), nHeight, nWeight,
				byItalic, NULL, NULL);
		}
		break;
	case CMD_CLR_PREV_HUE:
	case CMD_CLR_NEXT_HUE:
		{
			float hue = (float)(g_Profile.GetProfileInt(CMPlayerAppBase::GetMPSkinFactory()->GetSkinName(), _T("Hue"), 0));

			if (nID == CMD_CLR_NEXT_HUE)
				hue += 30;
			else
				hue -= 30;
			if (hue >= 360)   hue -= 360;
			else if (hue < 0) hue += 360;

			g_Profile.WriteProfileInt(CMPlayerAppBase::GetMPSkinFactory()->GetSkinName(), _T("Hue"), (int)hue);
			CMPlayerAppBase::GetMPSkinFactory()->AdjustHue(hue);
		}
		break;
	case CMD_LYR_HIGH_CLR_LIST:
		{
			CRect	rc;
			CUIObject *pObj = m_pSkinWnd->GetUIObjectById(CMD_LYR_HIGH_CLR_LIST, NULL);
			if (pObj)
			{
				rc = pObj->m_rcObj;
				m_pSkinWnd->ScreenToClient(rc);
			}
			else
			{
				CPoint	pt;
				MLWidget::GetCursorPos(&pt);
				rc.SetRect(pt.x, pt.y, pt.x + 100, pt.y + 25);
			}
			m_popupHighClrListWnd.Create(m_pSkinWnd, rc, true,
				m_etDispSettings == ET_LYRICS_FLOATING_SETTINGS, NULL);
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

	case CMD_OK:
	case CMD_CANCEL:
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
			if (!g_LyricData.HasLyricsOpened())
			{
				m_pSkinWnd->MessageOut(TOLOCAL(_T("No Lyrics file was opened.")));
				break;
			}

			CMLMenu	menu;
			menu.LoadPopupMenu(IDR_MENU_RATE, 0);

			CPoint	pt;
			MLWidget::GetCursorPos(&pt);
			menu.TrackPopupMenu(pt.x, pt.y, m_pSkinWnd);
		}
		break;
	case CMD_RELOAD_LYR:
		CMPlayerAppBase::GetInstance()->DispatchResearchLyrics();
		break;
	case CMD_J_PREV_LINE:
	case CMD_J_NEXT_LINE:
		{
			// Jump to next/previous line of lyrics.
			VLyrLines	vLyrics = g_LyricData.GetRawLyrics();

			int			nCurLine = g_LyricData.GetCurPlayLine(vLyrics);
			if (nCurLine >= 0 && nCurLine < (int)vLyrics.size())
			{
				if (nID == CMD_J_NEXT_LINE)
				{
					if (nCurLine == vLyrics.size() - 1)
						return true;
					nCurLine++;
				}
				else
				{
					if (nCurLine > 0)
						nCurLine--;
				}

				g_Player.SeekTo(vLyrics[nCurLine]->nBegTime);
			}
		}
		break;
	case CMD_ADVANCED_SAVE_LYR:
		SaveAsLyricsFile(m_pSkinWnd,
			LyricsConentTypeToFileType(g_LyricData.GetLyrContentType()), true);
		break;
	case CMD_UPLOAD_LYR:
		{
			if (!g_LyricData.HasLyricsOpened())
				m_pSkinWnd->MessageOut(TOLOCAL(_T("No Lyrics file was opened.")));

			// show upload lyrics dialog
			tstring	strLyrics;
			g_LyricData.ToString(strLyrics, FT_LYRICS_LRC, true);
			ShowUploadLyrDialog(m_pSkinWnd, 
					strLyrics, 
					g_LyricData.GetSongFileName(),
					g_LyricData.GetLyricsFileName());
		}
		break;
	case CMD_EXTERNAL_LYR_EDIT:
		//
		// Edit lyrics with external editor.
		if (g_LyricData.GetLyricsSourceType() != LST_FILE)
		{
			m_pSkinWnd->MessageOut(TOLOCAL(_T("Embedded lyrics can not be edited with external editors.")));
			break;
		}

		if (g_LyricData.HasLyricsOpened())
		{
			if (!CMPlayerAppBase::GetInstance()->OnLyricsChangingSavePrompt())
				break;

			tstring		strEditor;
			GetNotepadEditor(strEditor);
			Execute(m_pSkinWnd, 
				CMLProfile::GetDir(SZ_SECT_UI, _T("LyricsEditor"), strEditor.c_str()).c_str(),
				g_LyricData.GetLyricsFileName());
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
	}
	else
		return false;

	return true;
}

bool CMPCommonCmdHandler::GetChecked(UINT nID, bool &bChecked)
{
	switch (nID)
	{
	case IDC_LDO_KARAOKE:
		bChecked = g_Profile.GetProfileBool(m_strSectName.c_str(), _T("Karaoke"), false);
		break;
#if defined(_WIN32_DESKTOP)
	case IDC_FLOATING_LYRICS:
		bChecked = g_wndFloatingLyr.IsValid();
		break;
#endif
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
			DISPLAY_OPTIONS		displayOpt = DisplayOptFromStr(
				g_Profile.GetProfileString(m_strSectName.c_str(), _T("LyrDrawOpt"), _T("")));

			if (displayOpt == DO_NORMAL)
				nIDChecked = IDC_LDO_NORMAL;
			else if (displayOpt == DO_FADEOUT_LOWCOLOR)
				nIDChecked = IDC_LDO_FADE_IN;
			else
				nIDChecked = IDC_LDO_FADEOUT_BG;
		}
		break;
// 	case IDC_ALIGN_LYRICS_LEFT:
// 	case IDC_ALIGN_LYRICS_CENTER:
// 	case IDC_ALIGN_LYRICS_RIGHT:
// 		{
// 			tstring		strAlign;
// 			strAlign = g_Profile.GetProfileString(m_strSectName.c_str(), _T("LyrAlign"), _T("center"));
// 			if (_tcsicmp(strAlign.c_str(), _T("left")) == 0)
// 				nIDChecked = IDC_ALIGN_LYRICS_LEFT;
// 			else if (_tcsicmp(strAlign.c_str(), _T("right")) == 0)
// 				nIDChecked = IDC_ALIGN_LYRICS_RIGHT;
// 			else
// 				nIDChecked = IDC_ALIGN_LYRICS_CENTER;
// 		}
// 		break;
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

	int nEncodingId = CmdIdToEncoding(nCmdId);
	if (nEncodingId == -1)
		return false;

	if (!g_LyricData.HasLyricsOpened())
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
	CHAR_ENCODING encodingOld = GetCharEncodingID(g_LyricData.Properties().GetEncoding());
	if (nEncodingId != encodingOld)
	{
		g_LyricData.ReopenLyrics(TRUE, (CHAR_ENCODING)nEncodingId);

		CMPlayerAppBase::GetInstance()->DispatchLyricsChangedSyncEvent();
	}

	return true;
}

// COMMENT:
//		保存歌词文件为：
//		FT_UNKNOWN			= 1,
//		FT_LYRICS_LRC		= 2,
//		FT_LYRICS_TXT		= 3,
// RETURN:
//		TRUE	-	保存成功，注意：如果没有歌词数据也返回TRUE；
//		FALSE	-	用户选择取消，或者保存失败;
BOOL CMPCommonCmdHandler::SaveAsLyricsFile(CBaseWnd *pWndParent, MLFileType DefFileType, bool bAdvancedSaveOpt)
{
	if (!g_LyricData.HasLyricsOpened())
	{
		pWndParent->MessageOut(TOLOCAL(_T("No Lyrics file was opened.")));
		return TRUE;
	}

	tstring strFile = g_LyricData.GetLyricsFileName();
	if (strFile.empty() || !IsFileExist(strFile.c_str()))
	{
		DOWN_SAVE_DIR downSaveDir = (DOWN_SAVE_DIR)g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("DownSaveInSongDir"), DOWN_SAVE_IN_CUSTOM_DIR);
		if (downSaveDir == DOWN_SAVE_IN_SONG_DIR)
			FileGetPath(g_LyricData.GetSongFileName(), strFile);

		DOWN_SAVE_NAME downSaveName = (DOWN_SAVE_NAME)g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("DownSaveName"), DOWN_SAVE_NAME_KEEP);
		if (downSaveName == DOWN_SAVE_NAME_AS_SONG_NAME)
		{
			// Uses media file title.
			tstring strTitle;
			FileGetTitle(g_LyricData.GetSongFileName(), strTitle);
			strFile += strTitle;
		}
		else
		{
			// Uses artist - title
			if (!g_LyricData.Properties().m_strArtist.empty())
			{
				strFile += g_LyricData.Properties().m_strArtist;
				strFile += _T(" - ");
			}

			strFile += g_LyricData.Properties().m_strTitle;
		}

		strFile += _T(".lrc");
		strFile = FileNameFilterInvalidChars(strFile.c_str());
	}

	// set saved file extension
	if (DefFileType == FT_LYRICS_LRC)
		FileSetExt(strFile, _T(".lrc"));
	else if (DefFileType == FT_LYRICS_TXT)
		FileSetExt(strFile, _T(".txt"));
	else if (DefFileType == FT_LYRICS_SNC)
		FileSetExt(strFile, _T(".snc"));

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

	_tcscpy_safe(szWndTitle, CountOf(szWndTitle), TOLOCAL(_T("Save As")));
	CFileSaveDlg		dlg(szWndTitle, strFile.c_str(),
		_T("All supported files (*.lrc; *.txt; *.snc)\0*.lrc;*.txt;*.snc\0LRC Lyrics File (*.lrc)\0*.lrc\0Text File (*.txt)\0*.txt\0Snc File (*.snc)\0*.snc\0\0"),
		nFileExtIndex);

	if (dlg.DoModal(pWndParent) == IDOK)
	{
		strFile = dlg.GetSaveFile();
		if (IsEmptyString(FileGetExt(strFile.c_str())))
		{
			// Set file extend name as the user selected.
			FileSetExt(strFile, dlg.GetSelectedExt());
		}
		MLFileType	fileType = MLGetFileType(strFile.c_str());
		if (fileType != FT_LYRICS_LRC && fileType != FT_LYRICS_TXT
			&& fileType != FT_LYRICS_SNC)
			return FALSE;

		tstring			strLyrics;
		g_LyricData.ToString(strLyrics, FT_LYRICS_TXT, TRUE);

		CHAR_ENCODING	fileEncoding = ED_UNICODE;
		if (IsAnsiStr(strLyrics.c_str()))
			fileEncoding = ED_SYSDEF;
		else
			fileEncoding = ED_UNICODE;

		bool	bUseNewFileName;
		int		nRet = g_LyricData.SaveAsFile(strFile.c_str(), fileEncoding, bUseNewFileName);
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

bool CMPCommonCmdHandler::SaveCurrentLyrics(CSkinWnd *pSkinWnd, bool bDispatchOnSave)
{
	int			nRet;

	if (bDispatchOnSave)
	{
		// Dispatch on save lyrics message.
		IEvent		*pEvent = new IEvent;
		pEvent->eventType = ET_LYRICS_ON_SAVE_EDIT;
		CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(pEvent);
	}

	if (g_LyricData.DoesChooseNewFileName())
	{
		if (!SaveAsLyricsFile(pSkinWnd, 
			LyricsConentTypeToFileType(g_LyricData.GetLyrContentType()), false))
			return false;
	}
	else
	{
		nRet = g_LyricData.Save();
		if (nRet != ERR_OK)
		{
			if (g_LyricData.GetLyricsSourceType() != LST_FILE)
			{
				tstring		str;
				g_LyricData.ToString(str, FT_LYRICS_LRC, TRUE);

				CCharBuffer bufLyrics;
				TStringToProperFileData(str.c_str(), bufLyrics);
				g_autoProcessEmbeddedLyrics.SaveEmbeddedLyrics(g_LyricData.GetSongFileName(), 
					NULL, &bufLyrics, g_LyricData.GetLyricsSourceType());
			}
			pSkinWnd->MessageOut(CStrPrintf(_T("%s\n%s"), ERROR2STR_LOCAL(nRet), TOLOCAL(_T("Failed to Save embedded lyrics, $Product$ will auto try again later."))).c_str());
		}
	}

	return true;
}

VLyrLines &CMPCommonCmdHandler::GetDisplayLyrics()
{
	CLyricShowObj *pObj = (CLyricShowObj*)m_pSkinWnd->GetUIObjectByClassName(CLyricShowMultiRowObj::ClassName());
	if (pObj)
		return pObj->GetLyrics();
	else
		return g_LyricData.GetRawLyrics();
}
