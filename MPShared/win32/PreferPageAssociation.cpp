// PreferPageAssociation.cpp: implementation of the CPreferPageAssociation class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "PreferPageAssociation.h"
#include "MLCmd.h"


#define MEDIA_FILE_ICON_ID		2

#define KEY_LOCATION			_T("Software\\Crintsoft\\ZikiPlayer\\MP3\\Location")
#define NAME_LOCATION			_T("Location")

#define KEY_AUDIOFILE			_T("ZikiPlayer AudioFile")
#define NAME_AUDIOFILE			_T("Audio File")
#define KEY_PLAYLIST			_T("ZikiPlayer Playlist")
#define NAME_PLAYLIST			_T("Play List File")
#define VALUE_BACKUP			_T("ZikiPlayer Backup")

void AssociateFile(LPCTSTR pszExt, LPCTSTR pszKey, LPCTSTR pszName, UINT uIconID)
{

}

void UnassociateFile(LPCTSTR pszExt, LPCTSTR pszKey)
{

}

BOOL IsAssociatedFile(LPCTSTR pszExt, LPCTSTR pszKey)
{

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPreferPageAssociation::CPreferPageAssociation() : CPreferencePageBase(IDD_PG_MEDIA_ASSOCIATE, _T("Associations"))
{
}

CPreferPageAssociation::~CPreferPageAssociation()
{

}

BOOL CPreferPageAssociation::OnInitDialog()
{
	CPreferencePageBase::OnInitDialog();

	m_wndHelper.AddAlignRightBottom(IDC_LIST_MEDIA_TYPE, FALSE, FALSE);
	m_wndHelper.AddAlignBottom(IDC_CHECK_ALL, TRUE);
	m_wndHelper.AddAlignBottom(IDC_CLEAR_ALL, TRUE);

	m_wndHelper.AddAlignCenter(IDC_CHECK_ALL, false);
	m_wndHelper.AddAlignCenter(IDC_CLEAR_ALL, false);

	m_listCtrl.Attach(this, IDC_LIST_MEDIA_TYPE);
	m_listCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	m_listCtrl.InsertColumn(0, _TLT("File Types"), LVCFMT_LEFT, 150);

	vector<tstring>		vExt;
	g_Player.GetSupportedExtentions(vExt);
	for (int i = 0; i < (int)vExt.size(); i++)
	{
		int		n = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), vExt[i].c_str());
		if (g_Player.IsExtAudioFile(vExt[i].c_str()))
			m_listCtrl.SetCheck(n, IsAssociatedFile(vExt[i].c_str(), KEY_AUDIOFILE));
		else
			m_listCtrl.SetCheck(n, IsAssociatedFile(vExt[i].c_str(), KEY_PLAYLIST));
	}

	return TRUE;
}

void CPreferPageAssociation::OnDestroy()
{
	TCHAR		szExt[256];

	for (int i = 0; i < m_listCtrl.GetItemCount(); i++)
	{
		EmptyStr(szExt);
		m_listCtrl.GetItemText(i, 0, szExt, CountOf(szExt));
		BOOL		bPlaylist;
		BOOL		bAssociated;

		bPlaylist = g_Player.IsExtPlaylistFile(szExt);
		if (bPlaylist)
			bAssociated = IsAssociatedFile(szExt, KEY_PLAYLIST);
		else
			bAssociated = IsAssociatedFile(szExt, KEY_AUDIOFILE);

		if (m_listCtrl.GetCheck(i))
		{
			if (!bAssociated)
			{
				if (bPlaylist)
					AssociateFile(szExt, KEY_PLAYLIST, NAME_PLAYLIST, IDI_MEDIATYPE2);
				else
					AssociateFile(szExt, KEY_AUDIOFILE, NAME_AUDIOFILE, IDI_MEDIATYPE2);
			}
		}
		else
		{
			if (bAssociated)
			{
				if (bPlaylist)
					UnassociateFile(szExt, KEY_PLAYLIST);
				else
					UnassociateFile(szExt, KEY_AUDIOFILE);
			}
		}
	}

	CPreferencePageBase::OnDestroy();
}

void CPreferPageAssociation::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);

	switch (uID)
	{
	case IDC_CHECK_ALL:
	case IDC_CLEAR_ALL:
		{
			bool	bCheck = uID == IDC_CHECK_ALL;
			for (int i = 0; i < m_listCtrl.GetItemCount(); i++)
			{
				m_listCtrl.SetCheck(i, bCheck);
			}
		}
		break;
	}
}
