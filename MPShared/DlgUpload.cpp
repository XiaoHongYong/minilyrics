#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgUpload.h"
#include "WaitingDlg.h"

class CLoginWorkObj : public CWorkObjBase
{
public:
	CLoginWorkObj()
	{
		m_bEnableCancel = true;
		m_nResult = ERR_OK;
	}

	virtual DWORD DoTheJob()
	{
		m_strMsg.clear();

		g_sessionUpload.Init(SZ_ML_VERSION);

		m_nResult = g_sessionUpload.Connect();
		if (m_nResult == ERR_OK)
		{
			g_sessionUpload.SetLoginUserPwdMask(m_strName.c_str(), m_strPwdMask.c_str());
			m_nResult = g_sessionUpload.Login(m_strMsg);
		}
		g_sessionUpload.Close();

		return CWorkObjBase::DoTheJob();
	}

	virtual void CancelJob()
	{
		g_sessionUpload.Close();

		CWorkObjBase::CancelJob();
	}

public:
	tstring					m_strMsg;
	int						m_nResult;
	tstring					m_strName, m_strPwdMask;

};


//////////////////////////////////////////////////////////////////////////

class CUploadWorkObj : public CLoginWorkObj
{
public:
	CUploadWorkObj()
	{
		m_bEnableCancel = true;
	}

	virtual DWORD DoTheJob()
	{
		m_strMsg.clear();

		m_nResult = g_sessionUpload.Connect();
		if (m_nResult == ERR_OK)
		{
			if (!g_sessionUpload.IsLogined())
			{
				// Log in
				ASSERT(m_strName.size() && m_strPwdMask.size());
				g_sessionUpload.SetLoginUserPwdMask(m_strName.c_str(), m_strPwdMask.c_str());
				m_nResult = g_sessionUpload.Login(m_strMsg);
			}

			// Upload
			if (m_nResult == ERR_OK)
			{
				g_sessionUpload.Close();
				g_sessionUpload.Connect();
				m_nResult = g_sessionUpload.Upload(m_strLyricsContent, m_strLyrId, m_strMsg, m_listExistFiles);
			}
		}
		g_sessionUpload.Close();

		return CWorkObjBase::DoTheJob();
	}

public:
	list<MLFileInfo>		m_listExistFiles;
	tstring					m_strLyricsContent;
	tstring					m_strLyrSource;
	tstring					m_strMediaSource, m_strLyrId;

};

//////////////////////////////////////////////////////////////////////////

class CPageLogin : public CSkinPageHandler
{
	DECLARE_PAGE_HANDLER(_T("CID_C_LOGIN"))
public:
	CPageLogin(CLoginWorkObj *pLoginObj) : CSkinPageHandler(GetPageIdStr())
	{
		m_pLoginObj = pLoginObj;
	}

	void OnCreate()
	{
		CSkinPageHandler::OnCreate();

		GET_ID_BY_NAME(CID_E_NAME);
		GET_ID_BY_NAME(CID_REMEMBER_PWD);
		GET_ID_BY_NAME(CID_E_PASSWORD);

		SetUIObjectText(CID_E_NAME, g_Profile.GetProfileString(_T("LoginName"), _T("")));
		SetUIObjectProperty(_T("CID_SIGNUP"), _T("Link"), GetStrName(SN_HTTP_SIGNUP));

		bool bRememberPwd = g_Profile.GetProfileBool(_T("rememberPwd"), FALSE);
		if (bRememberPwd)
			SetUIObjectText(CID_E_PASSWORD, g_Profile.GetProfileString(_T("LoginPwdMask"), _T("")));
		CheckButton(CID_REMEMBER_PWD, bRememberPwd);
	}

	void OnShow()
	{
		CSkinPageHandler::OnShow();
		m_pSkinWnd->SetCaptionText(TOLOCAL(_T("Log in")));

		if (m_pLoginObj)
		{
			if (m_pLoginObj->m_strMsg.size())
				m_pSkinWnd->MessageOut(m_pLoginObj->m_strMsg.c_str());

			if (m_pLoginObj->m_nResult != ERR_OK && !m_pLoginObj->IsJobCanceled())
			{
				m_pSkinWnd->MessageOut(ERROR2STR_LOCAL(m_pLoginObj->m_nResult));
				return;
			}
		}
	}

	void OnOK()
	{
		tstring strName = GetUIObjectText(CID_E_NAME);
		tstring strPwd = GetUIObjectText(CID_E_PASSWORD);
		bool bRememberePwd = IsButtonChecked(CID_REMEMBER_PWD);

		if (strName.empty() || strPwd.empty())
		{
			m_pSkinWnd->MessageOut(TOLOCAL(_T("Please input user name and password!")));
			return;
		}

		bool bPwdChanged = _tcscmp(strPwd.c_str(), g_Profile.GetProfileString(_T("LoginPwdMask"), _T(""))) != 0;
		if (bPwdChanged)
		{
			char		szPwdMask[50];
			stringex	strPwdAnsi;
			TStringToMBCS(strPwd.c_str(), strPwd.size(), strPwdAnsi);
			MD5Gen16(strPwdAnsi.c_str(), szPwdMask);
			MBCSToTString(szPwdMask, -1, strPwd);
		}

		g_Profile.WriteProfileString(_T("LoginName"), strName.c_str());
		if (bRememberePwd)
		{
			if (bPwdChanged)
				g_Profile.WriteProfileString(_T("LoginPwdMask"), strPwd.c_str());
		}
		g_Profile.WriteProfileInt(_T("rememberPwd"), bRememberePwd);

		m_pLoginObj->m_strName = strName;
		m_pLoginObj->m_strPwdMask = strPwd;

		CPageWaitMessage	*pPageWait = (CPageWaitMessage *)m_pParent->GetPage(CPageWaitMessage::GetPageIdStr());
		ASSERT(pPageWait);
		if (pPageWait)
		{
			pPageWait->StartWork(TOLOCAL(_T("Login")), 
				TOLOCAL(_T("Processing login, It may take a few seconds.")),
				m_pLoginObj);
		}
	}

	void OnCancel()
	{
		m_pParent->SwitchToLastPage();
	}

protected:
	int					CID_E_PASSWORD, CID_REMEMBER_PWD, CID_E_NAME;
	CLoginWorkObj		*m_pLoginObj;

};

//////////////////////////////////////////////////////////////////////////

class CPageUploadNotice : public CSkinPageHandler
{
	DECLARE_PAGE_HANDLER(_T("CID_C_UPLOAD_NOTICE"))
public:
	CPageUploadNotice(CUploadWorkObj *pUploadObj) : CSkinPageHandler(GetPageIdStr())
	{
		m_pUploadObj = pUploadObj;
	}

	void OnCreate()
	{
		CSkinPageHandler::OnCreate();

		GET_ID_BY_NAME(CID_CHANGE_ACCOUNT);
	}

	void OnShow()
	{
		CSkinPageHandler::OnShow();
		m_pSkinWnd->SetCaptionText(TOLOCAL(_T("Upload Lyrics")));

		ASSERT(g_sessionUpload.IsLogined());
		SetUIObjectText(_T("CID_ACCOUNT_NOTICE"), CStrPrintf(_T("Your lyrics will be uploaded under account: %s"), g_sessionUpload.GetUser()).c_str(), false);
	}

	void OnOK()
	{
		CPageWaitMessage	*pPageWait = (CPageWaitMessage *)m_pParent->GetPage(CPageWaitMessage::GetPageIdStr());
		ASSERT(pPageWait);
		if (pPageWait)
		{
			pPageWait->StartWork(TOLOCAL(_T("Uploading")), 
				TOLOCAL(_T("MiniLyrics is uploading lyrics, It may take a few seconds.")),
				m_pUploadObj);
		}
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == CID_CHANGE_ACCOUNT)
			m_pParent->SwitchToPage(_T("CID_C_LOGIN"), true);
		else
			return CSkinPageHandler::OnCommand(nId);

		return true;
	}

protected:
	int					CID_CHANGE_ACCOUNT;
	CLoginWorkObj		*m_pUploadObj;

};

//////////////////////////////////////////////////////////////////////////

class CPageUploadResult : public CSkinPageHandler
{
	DECLARE_PAGE_HANDLER(_T("CID_UPLOAD_RESULT"))
public:
	CPageUploadResult(CUploadWorkObj *pUploadWorkObj) : CSkinPageHandler(GetPageIdStr())
	{
		m_pUploadWorkObj = pUploadWorkObj;
	}

	~CPageUploadResult()
	{
	}

	void OnCreate()
	{
		CSkinPageHandler::OnCreate();

		GET_ID_BY_NAME(CID_TRY_AGAIN);
	}

	void OnShow()
	{
		CSkinPageHandler::OnShow();

		m_pSkinWnd->SetCaptionText(TOLOCAL(_T("Uploading")));
		if (m_pUploadWorkObj)
		{
			bool	bUploadOK = (m_pUploadWorkObj->m_nResult == ERR_OK);
			SetUIObjectVisible(_T("CID_C_UPLOAD_OK"), bUploadOK, false);
			SetUIObjectVisible(_T("CID_C_UPLOAD_FAILED"), !bUploadOK, false);
			if (!bUploadOK)
				SetUIObjectText(_T("CID_RESULT"), CStrPrintf(_T("%s"), ERROR2STR_LOCAL(m_pUploadWorkObj->m_nResult)).c_str(), false);

			if (m_pUploadWorkObj->m_strMsg.size())
				m_pSkinWnd->PostCustomCommandMsg(CMD_SHOW_ERR_RESULT);

			// Save lyrics id.
		}
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == CMD_SHOW_ERR_RESULT)
		{
			if (m_pUploadWorkObj && m_pUploadWorkObj->m_strMsg.size())
				m_pSkinWnd->MessageOut(m_pUploadWorkObj->m_strMsg.c_str());
		}
		else if (nId == CID_TRY_AGAIN)
		{
			CPageWaitMessage	*pPageWait = (CPageWaitMessage *)m_pParent->GetPage(CPageWaitMessage::GetPageIdStr());
			ASSERT(pPageWait);
			if (pPageWait)
			{
				pPageWait->StartWork(TOLOCAL(_T("Uploading")), 
					TOLOCAL(_T("MiniLyrics is uploading lyrics, It may take a few seconds.")),
					m_pUploadWorkObj);
			}
		}
		else
			return CSkinPageHandler::OnCustomCommand(nId);

		return true;
	}

protected:
	int					CID_TRY_AGAIN;
	CUploadWorkObj		*m_pUploadWorkObj;

};

//////////////////////////////////////////////////////////////////////////

class CPageUploadWaitMessage : public CPageWaitMessage
{
public:
	CPageUploadWaitMessage() : CPageWaitMessage(false)
	{
		m_nAnimationShowFRight = UID_INVALID;
		m_nAnimationShowFLeft = UID_INVALID;
		m_nAnimationHide2Left = UID_INVALID;
		m_nAnimationHide2Right = UID_INVALID;
	}

	virtual void OnWorkEnd()
	{
		if (m_pWorkObj->IsJobCanceled())
		{
			if (g_sessionUpload.IsLogined())
				m_pParent->SwitchToPage(CPageUploadNotice::GetPageIdStr(), true);
			else
				m_pParent->SwitchToPage(CPageLogin::GetPageIdStr(), true);
		}
		else
		{
			CUploadWorkObj	*pUploadWorkObj = (CUploadWorkObj *)m_pWorkObj;
			if (pUploadWorkObj->m_nResult == ERR_OK)
			{
				ASSERT(pUploadWorkObj->m_strLyrId.size());
				ASSERT(pUploadWorkObj->m_strMediaSource.size());
				ASSERT(pUploadWorkObj->m_strLyrSource.size());
				if (pUploadWorkObj->m_strLyrId.size())
				{
					// Save lyrics id
					if (_tcscmp(g_LyricData.GetSongFileName(), pUploadWorkObj->m_strMediaSource.c_str()) == 0
						&& _tcscmp(g_LyricData.GetLyricsFileName(), pUploadWorkObj->m_strLyrSource.c_str()) == 0)
					{
						g_LyricData.Properties().m_strId = pUploadWorkObj->m_strLyrId;
						g_LyricData.Save();
					}
					else
					{
						CMLData		lyrData;
						lyrData.OpenLyrics(pUploadWorkObj->m_strMediaSource.c_str(), 0,
							pUploadWorkObj->m_strLyrId.c_str());
						lyrData.Properties().m_strId = pUploadWorkObj->m_strLyrId;
						lyrData.Save();
					}
				}
			}
			m_pParent->SwitchToPage(CPageUploadResult::GetPageIdStr(), true);
		}
	}

protected:

};

//////////////////////////////////////////////////////////////////////////

CDlgUpload::CDlgUpload(tstring &strLyricsContent, LPCTSTR szMediaSource, LPCTSTR szLyrSource) : CMPSkinDialog(_T("UploadLyrics.xml"))
{
	m_pUploadWorkObj = new CUploadWorkObj();
	m_pUploadWorkObj->m_strLyricsContent = strLyricsContent;
	m_pUploadWorkObj->m_strMediaSource = szMediaSource;
	m_pUploadWorkObj->m_strLyrSource = szLyrSource;

	CSkinPageHandler	*pPageLogin = new CPageLogin(m_pUploadWorkObj);
	CSkinPageHandler	*pPageUploadNotice = new CPageUploadNotice(m_pUploadWorkObj);
	m_pageHandler.AddPage(pPageUploadNotice);
	m_pageHandler.AddPage(pPageLogin);
	m_pageHandler.AddPage(new CPageUploadWaitMessage());
	m_pageHandler.AddPage(new CPageUploadResult(m_pUploadWorkObj));

	if (g_sessionUpload.IsLogined())
		m_pageHandler.SetDefaultPage(pPageUploadNotice->GetPageId());
	else
		m_pageHandler.SetDefaultPage(pPageLogin->GetPageId());
}

CDlgUpload::~CDlgUpload()
{
	if (m_pUploadWorkObj)
		delete m_pUploadWorkObj;
}
