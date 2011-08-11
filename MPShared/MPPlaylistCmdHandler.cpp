// MPPlaylistCmdHandler.cpp: implementation of the CMPPlaylistCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "MPPlaylistCmdHandler.h"
#include "MPHelper.h"
#include "DlgMediaInfo.h"
#include "MPlaylistCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPPlaylistCmdHandler::CMPPlaylistCmdHandler()
{

}

CMPPlaylistCmdHandler::~CMPPlaylistCmdHandler()
{

}

// if the command id is processed, return true.
bool CMPPlaylistCmdHandler::OnCommand(int nId)
{
	switch (nId)
	{
	case IDC_OPEN_PL:
		{
			CFileOpenDlg		dlg(_T("Open Playlist"), g_Player.GetCurrentPlaylistFile().c_str(), _T("M3u file\0*.m3u\0All Files\0*.*\0\0"), 0);
			if (dlg.DoModal(m_pSkinWnd) == IDOK)
			{
				g_Player.ClearPlaylist();
				g_Player.LoadPlaylist(dlg.GetOpenFile());
			}
		}
		break;
	case IDC_CLEAR:
		{
			g_Player.ClearPlaylist();
			g_Player.SetPlaylistModified(true);
		}
		break;
	default:
		return false;
	}

	return true;
}

bool CMPPlaylistCmdHandler::OnCustomCommand(int nID)
{
	switch (nID)
	{
	case CMD_PL_SAVE:
		{
			CFileSaveDlg		dlg(_T("Save Playlist"), g_Player.GetCurrentPlaylistFile().c_str(), _T("M3u file\0*.m3u\0All Files\0*.*\0\0"), 0);
			if (dlg.DoModal(m_pSkinWnd) == IDOK)
			{
				if (IsEmptyString(FileGetExt(dlg.GetSaveFile())))
				{
					TCHAR		szFile[MAX_PATH];
					_tcscpy_safe(szFile, CountOf(szFile), dlg.GetSaveFile());
					FileSetExt(szFile, CountOf(szFile), dlg.GetSelectedExt());
					g_Player.SaveCurrentPlaylistAs(szFile);
				}
				else
					g_Player.SaveCurrentPlaylistAs(dlg.GetSaveFile());
			}
		}
		break;
	case CMD_PL_DEL:
		{
			CMPlaylistCtrl	*plCtrl = (CMPlaylistCtrl*)m_pSkinWnd->GetUIObjectByClassName(CMPlaylistCtrl::ClassName());
			if (!plCtrl)
				break;
			plCtrl->DeleteSelectedItems();
		}
		break;
	case CMD_PL_UP:
	case CMD_PL_DOWN:
		{
			CMPlaylistCtrl	*plCtrl = (CMPlaylistCtrl*)m_pSkinWnd->GetUIObjectByClassName(CMPlaylistCtrl::ClassName());
			if (!plCtrl)
				break;

			plCtrl->OffsetAllSelectedItems(nID == CMD_PL_DOWN);
		}
		break;
	case CMD_PL_PROPERTY:
		{
			CMPAutoPtr<IPlaylist>	playlist;
			CMPAutoPtr<IMedia>		media;

			CMPlaylistCtrl	*plCtrl = (CMPlaylistCtrl*)m_pSkinWnd->GetUIObjectByClassName(CMPlaylistCtrl::ClassName());
			if (plCtrl && plCtrl->IsVisible() && plCtrl->IsParentVisible())
			{
				int				nSel;
				nSel = plCtrl->GetNextSelectedItem(-1);
				if (nSel == -1)
					break;

				if (g_Player.GetCurrentPlaylist(&playlist) != ERR_OK)
					break;

				if (playlist->GetItem(nSel, &media) != ERR_OK)
					break;
			}
			else
			{
				if (g_Player.GetCurrentMedia(&media) != ERR_OK)
					break;
			}

			ShowMediaInfoDialog(m_pSkinWnd, media);
			// dlg.DoModal(m_pSkinWnd, media);

/*			CXStr		str;
			media->GetSourceUrl(&str);

			CID3v2		id3v2;
			int			nRet;

			nRet = id3v2.Open(str.c_str(), false, false);
			if (nRet == ERR_OK)
			{
				CID3v2::FrameIterator		it, itEnd;

				itEnd = id3v2.FrameEnd();
				for (it = id3v2.FrameBegin(); it != itEnd; ++it)
				{
					CID3v2Frame		*fr = *it;
					DBGLOG2(_T("%S, %d"), fr->m_framehdr.szFrameID, fr->m_framehdr.nLen);
					// if (strcmp(fr->m_framehdr.szFrameID, "TIT2") == 0)
					if (fr->m_framehdr.szFrameID[0] == 'T')
					{
						CID3v2FrameParserText	txtParser;
						ID3v2Text		txt;
						nRet = txtParser.Parse(fr->m_framehdr, fr->m_frameData.c_str(), fr->m_frameData.size(), txt);
						if (nRet != ERR_OK)
							ERRLOG1(_T("Parse Txt frame Failed: %s"), (LPCTSTR)Error2Str(nRet));
						else
						{
							DBGLOG1(_T("Value: %s"), txt.GetValue());
						}
					}
					else if (strcmp(fr->m_framehdr.szFrameID, "COMM") == 0)
					{
						CID3v2FrameParserComment	parser;
						nRet = parser.Parse(*fr);
						if (nRet != ERR_OK)
							ERRLOG1(_T("Parse Txt frame Failed: %s"), (LPCTSTR)Error2Str(nRet));
						else
						{
							if (!parser.m_bUnicode)
							{
								ConvertStr(parser.m_strAnsiShortDesc.c_str(), -1, parser.m_strUcs2ShortDesc);
								ConvertStr(parser.m_strAnsiText.c_str(), -1, parser.m_strUcs2Text);
							}

							DBGLOG3(_T("Comment: %s, %s, %s"), parser.m_szLanguage, parser.m_strUcs2ShortDesc.c_str(), parser.m_strUcs2Text.c_str());
						}
					}
				}
			}
			else
				ERRLOG1(_T("Open Failed: %s"), (LPCTSTR)Error2Str(nRet));*/
		}
		break;
	default:
		return false;
	}

	return true;
}
