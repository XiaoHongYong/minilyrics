#import "../../base/base.h"
#import "../../Utils/Utils.h"
#import "../../MLLib/MLLib.h"
#import "CMLApp.h"
#import "helper.h"

CMLApp			*CMLApp::m_pAppInstance = NULL;

CMLApp::CMLApp(void)
{
}

CMLApp::~CMLApp(void)
{
}

CMLApp *CMLApp::GetInstance()
{
	if (!m_pAppInstance)
		m_pAppInstance = new CMLApp();

	return m_pAppInstance;
}

int CMLApp::Init()
{
	LPTSTR  vArgv[2] = { 0 };
	tstring strAppDirDir = tstring(GetAppDataDir()) + _T("MiniLyrics.app");
	vArgv[0] = (LPTSTR)strAppDirDir.c_str();

	InitBaseFrameWork(1, vArgv, NULL, _T("MiniLyrics.ini"), _T("MiniLyrics"));

	return ERR_OK;
}

void CMLApp::Quit()
{
}

#define MAKE_SURE_STR_NOT_NULL(str)		if (str == NULL) str = _T("")

void CMLApp::OnMediaChanged(LPCTSTR szMediaKey, LPCTSTR szArtist, LPCTSTR szAlbum, LPCTSTR szTitle, LPCTSTR szEmbeddedLyr, int nMediaLength)
{
	MAKE_SURE_STR_NOT_NULL(szMediaKey);
	MAKE_SURE_STR_NOT_NULL(szArtist);
	MAKE_SURE_STR_NOT_NULL(szAlbum);
	MAKE_SURE_STR_NOT_NULL(szTitle);
	MAKE_SURE_STR_NOT_NULL(szEmbeddedLyr);

	m_strCurMediaKey = szMediaKey;
	m_strArtist = szArtist;
	m_strAlbum = szAlbum;
	m_strTitle = szTitle;
	m_nMediaLength = nMediaLength;
	
	// Save lyrics changes
	if (m_lyrData.HasLyricsOpened() && m_lyrData.IsModified())
		m_lyrData.Save();

	m_lyrData.Close();

/*	tstring strTestLyr = GetResourceFile(_T("lyrics.lrc"));
	int nRet = m_lyrData.OpenLyrics(szMediaKey, nMediaLength, strTestLyr.c_str());

	PostNotifyLyricsChanged();
	
	return;*/

	if (m_strCurMediaKey.empty() && m_strArtist.empty() && m_strTitle.empty())
	{
		PostNotifyLyricsChanged();
		return;
	}

//	tstring strLyrics;
//	if (m_localLyrSearch.GetBestMatchLyrics(szMediaKey, szArtist, szTitle, strLyrics))
//	{
//		// Open lyrics
//		if (_tcsicmp(strLyrics.c_str(), NONE_LYRICS) == 0)
//		{
//			AddMediaInfo();
//			PostNotifyLyricsChanged();
//			return;
//		}
//		int nRet = m_lyrData.OpenLyrics(szMediaKey, m_nMediaLength, strLyrics.c_str(), false, ED_SYSDEF);
//		if (nRet == ERR_OK)
//		{
//			PostNotifyLyricsChanged();
//			return;
//		}
//	}

	AddMediaInfo();

	PostNotifyLyricsChanged();
}

void CMLApp::AddMediaInfo()
{
	V_Str		vMediaInfo;
	
	if (m_strArtist.size() > 0)
		vMediaInfo.push_back(tstring(_TLT("Artist:")) + _T(" ") + m_strArtist);
	if (m_strTitle.size() > 0)
		vMediaInfo.push_back(tstring(_TLT("Title:")) + _T(" ") + m_strTitle);
	if (m_strAlbum.size() > 0)
		vMediaInfo.push_back(tstring(_TLT("Album:")) + _T(" ") + m_strAlbum);
	
	m_lyrData.AddTextInLyrics(vMediaInfo);
}
