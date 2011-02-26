#include "StdAfx.h"
#include "iPodLyricsDownloaderApp.h"
#include "LDSkinFactory.h"

#define SZ_MACRO_PRODUCT_NAME				_T("$Product$")
#define SZ_MACRO_COMPANY_NAME				_T("$Company$")

#define SZ_PROFILE_NAME			_T("MiniLyric.ini")
#define SZ_TRANS_PREFIX			_T("ml")
#define SZ_MUTEX_RUNNING		_T("MiniLyricsRunning")
#define SZ_MAINWND_CLASSNAME	_T("iPodLyricsDownloader")
#define SZ_APP_NAME				_T("iPod Lyrics Downloader")
#define SZ_COMPANY_NAME			_T("CrintSoft")
#define SZ_DEFSKIN				_T("Default")

STRING_ID_MAP g_StringIdMap[] = 
{
	{ _T("ID_MINIMIZE"), CMD_MINIMIZE, 0, _T("Minimize") },
	{ _T("ID_MAXIMIZE"), CMD_MAXIMIZE, 0, _T("Maximize") },
	{ _T("ID_CLOSE"), CMD_CLOSE, IDC_CLOSE, _T("Close") },
	{ _T("ID_QUIT"), CMD_QUIT, IDC_EXIT, _T("Quit") },
	{ _T("ID_MENU"), CMD_MENU, 0, _T("Menu") },
	{ _T("ID_FILE_LIST"), CMD_FILE_LIST, 0, NULL },
	{ NULL, 0, 0, NULL },
};

int WINAPI _tWinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	InitBaseFrameWork(hInstance, _T("iPodLyricsDownloaderLog.txt"), _T("iPodLyricsDownloader.ini"), _T("iPodLyricsDownloader"));

	if (!CiPodLyricsDownloaderApp::GetInstance()->Init())
		return 0;

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CiPodLyricsDownloaderApp::GetInstance()->Quit();

	return msg.wParam;
}

CiPodLyricsDownloaderApp::CiPodLyricsDownloaderApp(void)
{
}

CiPodLyricsDownloaderApp::~CiPodLyricsDownloaderApp(void)
{
}

CiPodLyricsDownloaderApp *CiPodLyricsDownloaderApp::GetInstance()
{
	if (m_pInstance)
		return (CiPodLyricsDownloaderApp *)m_pInstance;

	m_mutex.Acquire();
	if (!m_pInstance)
		m_pInstance = new CiPodLyricsDownloaderApp();
	m_mutex.Release();

	return (CiPodLyricsDownloaderApp*)m_pInstance;
}

bool CiPodLyricsDownloaderApp::Init()
{
	CLanguageTool::SetTransFilePrefix(SZ_TRANS_PREFIX);

	g_LangTool.SetMacro(SZ_MACRO_PRODUCT_NAME, SZ_APP_NAME);
	g_LangTool.SetMacro(SZ_MACRO_COMPANY_NAME, SZ_COMPANY_NAME);

	if (!CSkinApp::Init())
		return false;

	m_pSkinFactory->SetSkinFileName(_T("iPodLyricsDownloader.xml"));

#ifdef _CPPUNIT_TEST
	CreateUnitResultWnd(GetAppInstance());

	RunUnitTest();
#endif

	int nRet = LoadDefaultSkin(SZ_DEFSKIN);
	if (nRet != ERR_OK)
		return false;

	m_pSkinFactory->GetMainWnd()->ShowWindow(SW_SHOW);

	return true;
}

void CiPodLyricsDownloaderApp::Quit()
{
	CSkinApp::Quit();
}

CSkinFactory *CiPodLyricsDownloaderApp::NewSkinFactory()
{
	return new CLDSkinFactory(this, g_StringIdMap);
}
