#include "StdAfx.h"
#include "Skin.h"
#include "SkinApp.h"


//////////////////////////////////////////////////////////////////////////

CSkinApp	* CSkinApp::m_pInstance = NULL;
Mutex		CSkinApp::m_mutex;

CSkinApp::CSkinApp(void)
{
}

CSkinApp::~CSkinApp(void)
{
}

CSkinApp *CSkinApp::GetInstance()
{
	ASSERT(m_pInstance);
	return m_pInstance;
}

bool CSkinApp::Init()
{
	m_pEventDispatcher = NewEventPatcher();
	m_pSkinFactory = NewSkinFactory();

	if (!IsDirExist(m_pSkinFactory->GetSkinRootDir()))
	{
		// Sometimes, uninstallation can't be remove MiniLyrics.dll till next restart,
		// So, don't start MiniLyrics, after uninstallation.
		ERRLOG1(_T("Skin root folder: %s doesn't exist."), m_pSkinFactory->GetSkinRootDir());
		return false;
	}

	// Init base service
	m_pEventDispatcher->Init();

	m_pSkinFactory->Init();

	return true;
}


int CSkinApp::LoadDefaultSkin(LPCTSTR szDefaultSkin)
{
	tstring		strSkin;
	bool		bLoadSkinTheme = false;

	strSkin = g_Profile.GetProfileString(_T("DefaultSkin"), _T(""));
	if (strSkin.empty())
	{
		strSkin = szDefaultSkin;
		bLoadSkinTheme = true;
		g_Profile.WriteProfileString(_T("DefaultSkin"), strSkin.c_str());
	}

	return m_pSkinFactory->ChangeSkin(strSkin.c_str(), _T(""), _T(""), bLoadSkinTheme);
}

template <class _T>
void SafeDelete(_T &p)
{
	if (p)
	{
		delete p;
		p = NULL;
	}
}

void CSkinApp::Quit()
{
	m_pEventDispatcher->Quit();
	m_pSkinFactory->Quit();

	SafeDelete(m_pEventDispatcher);
	SafeDelete(m_pSkinFactory);
}

void CSkinApp::PostQuitMessage()
{
	::PostQuitMessage(0);
}

CEventsDispatcher *CSkinApp::NewEventPatcher()
{
	return new CEventsDispatcher();
}

CSkinFactory *CSkinApp::NewSkinFactory()
{
	return new CSkinFactory(this, NULL);
}
