#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgUpload.h"
#include "WaitingDlg.h"

#define SZ_EX_POOL_WORK_END		_T("WorkEnd")
#define SZ_EX_POOL_WORK_RESULT	_T("WorkResult")
#define SZ_EX_POOL_WORK_MSG		_T("WorkMsg")

// Session of lyrics upload and login
CMLClientSession			g_sessionUpload;

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

		m_nResult = g_sessionUpload.Connect();
		if (m_nResult == ERR_OK)
		{
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
				m_nResult = g_sessionUpload.Login(m_strMsg);
			}

			// Upload
			if (m_nResult == ERR_OK)
			{
				g_sessionUpload.Close();
				g_sessionUpload.Connect();
				m_nResult = g_sessionUpload.Upload(m_strLyricsContent, m_strLyrId, m_strMsg);
			}
			g_sessionUpload.Close();

			// Save Lyrics Id to lyrics content
			if (m_nResult == ERR_OK)
			{
				ASSERT(m_strLyrId.size());
				ASSERT(m_strMediaSource.size());
				ASSERT(m_strLyrSource.size());
				if (m_strLyrId.size())
				{
					// Save lyrics id
					if (_tcscmp(g_LyricData.GetSongFileName(), m_strMediaSource.c_str()) == 0
						&& _tcscmp(g_LyricData.GetLyricsFileName(), m_strLyrSource.c_str()) == 0)
					{
						g_LyricData.Properties().m_strId = m_strLyrId;
						g_LyricData.Save();
					}
					else
					{
						CMLData		lyrData;
						lyrData.OpenLyrics(m_strMediaSource.c_str(), 0,
							m_strLyrId.c_str());
						lyrData.Properties().m_strId = m_strLyrId;
						lyrData.Save();
					}
				}
			}
		}

		return CWorkObjBase::DoTheJob();
	}

public:
	tstring					m_strLyricsContent;
	tstring					m_strLyrSource;
	tstring					m_strMediaSource, m_strLyrId;

};

//////////////////////////////////////////////////////////////////////////

class CSkinWndUploadLyr : public CMPSkinWnd
{
public:
	CSkinWndUploadLyr()
	{
		m_pUploadWorkObj = new CUploadWorkObj();
	}

	~CSkinWndUploadLyr()
	{
		if (m_pUploadWorkObj)
			delete m_pUploadWorkObj;
	}

	void OnSkinLoaded()
	{
		CMPSkinWnd::OnSkinLoaded();

		LPCTSTR szSwitchToPage = g_sessionUpload.IsLogined() ? _T("Container.UploadLyrNotice") : _T("Container.Login");
		CUIObject *pToPage = m_rootConainter.GetUIObjectByClassName(szSwitchToPage);
		if (pToPage)
			pToPage->GetParent()->SwitchToPage(szSwitchToPage, false, 0, false);
	}

	CUploadWorkObj *GetUploadWorkObj() const { return m_pUploadWorkObj; }

protected:
	CUploadWorkObj		*m_pUploadWorkObj;

};

//////////////////////////////////////////////////////////////////////////

class CPageUploadWaitMessage : public CPageWaitMessage
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	CPageUploadWaitMessage()
	{
		m_bLoginWork = false;
	}

	void SetLoginWork(bool bLoginWork) { m_bLoginWork = bLoginWork; }

	virtual void OnWorkEnd();

protected:
	bool				m_bLoginWork;

};

UIOBJECT_CLASS_NAME_IMP(CPageUploadWaitMessage, _T("Container.UploadLyrWaitMsg"))

//////////////////////////////////////////////////////////////////////////

class CPageLogin : public CSkinContainer
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	CPageLogin()
		{ CID_E_PASSWORD = CID_REMEMBER_PWD = CID_E_NAME = 0; m_strText = TOLOCAL_MARK(_T("Log in")); }

	void OnInitialUpdate()
	{
		CSkinContainer::OnInitialUpdate();

		GET_ID_BY_NAME3(CID_E_NAME, CID_REMEMBER_PWD, CID_E_PASSWORD);

		SetUIObjectProperty(_T("CID_SIGNUP"), _T("Link"), GetStrName(SN_HTTP_SIGNUP));

		SetUIObjectText(CID_E_NAME, g_sessionUpload.GetLoginName());
		bool bRememberPwd = g_Profile.GetProfileBool(_T("rememberPwd"), FALSE);
		if (bRememberPwd)
			SetUIObjectText(CID_E_PASSWORD, g_sessionUpload.GetLoginPwd());
		CheckButton(CID_REMEMBER_PWD, bRememberPwd);
	}

	void OnSwitchTo()
	{
		CSkinContainer::OnSwitchTo();

		if (GetExPoolBool(SZ_EX_POOL_WORK_END, false))
		{
			SetExPool(SZ_EX_POOL_WORK_END, false);

			if (m_loginWorkObj.m_strMsg.size())
				m_pSkin->MessageOut(m_loginWorkObj.m_strMsg.c_str());

			if (m_loginWorkObj.m_nResult != ERR_OK && !m_loginWorkObj.IsJobCanceled())
			{
				m_pSkin->MessageOut(ERROR2STR_LOCAL(m_loginWorkObj.m_nResult));
				return;
			}
		}
	}

	bool OnOK()
	{
		tstring strName = GetUIObjectText(CID_E_NAME);
		tstring strPwd = GetUIObjectText(CID_E_PASSWORD);
		bool bRememberePwd = IsButtonChecked(CID_REMEMBER_PWD);

		if (strName.empty() || strPwd.empty())
		{
			m_pSkin->MessageOut(TOLOCAL(_T("Please input user name and password!")));
			return true;
		}

		g_Profile.WriteProfileInt(_T("rememberPwd"), bRememberePwd);
		g_sessionUpload.SetUploader(strName.c_str(), strPwd.c_str(), bRememberePwd);

		CPageUploadWaitMessage *pPageWait = (CPageUploadWaitMessage *)m_pContainer->GetChildByClass(CPageUploadWaitMessage::ClassName());
		ASSERT(pPageWait);
		if (pPageWait)
		{
			pPageWait->SetLoginWork(true);
			pPageWait->StartWork(TOLOCAL(_T("Login")), 
				TOLOCAL(_T("Processing login, It may take a few seconds.")),
				&m_loginWorkObj);
		}

		return false;
	}

	bool OnCancel()
	{
		m_pContainer->SwitchToLastPage(0, true);
		return true;
	}

protected:
	int					CID_E_PASSWORD, CID_REMEMBER_PWD, CID_E_NAME;
	CLoginWorkObj		m_loginWorkObj;

};

UIOBJECT_CLASS_NAME_IMP(CPageLogin, _T("Container.Login"))

//////////////////////////////////////////////////////////////////////////

class CPageUploadNotice : public CSkinContainer
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	CPageUploadNotice()
	{
		m_pUploadObj = NULL;
		m_strText = TOLOCAL_MARK(_T("Upload Lyrics"));
	}

	void OnCreate()
	{
		CSkinContainer::OnCreate();

		m_pUploadObj = ((CSkinWndUploadLyr *)m_pSkin)->GetUploadWorkObj();
		ASSERT(m_pUploadObj);

		GET_ID_BY_NAME(CID_CHANGE_ACCOUNT);
	}

	void OnSwitchTo()
	{
		CSkinContainer::OnSwitchTo();

		ASSERT(g_sessionUpload.IsLogined());
		SetUIObjectText(_T("CID_ACCOUNT_NOTICE"), 
			CStrPrintf(TOLOCAL_T("Your lyrics will be uploaded under account: %s"), g_sessionUpload.GetLoginName()).c_str(), false);
	}

	bool OnOK()
	{
		CPageUploadWaitMessage *pPageWait = (CPageUploadWaitMessage *)m_pContainer->GetChildByClass(CPageUploadWaitMessage::ClassName());
		ASSERT(pPageWait);
		if (pPageWait)
		{
			pPageWait->SetLoginWork(false);
			pPageWait->StartWork(TOLOCAL(_T("Uploading")), 
				TOLOCAL(_T("MiniLyrics is uploading lyrics, It may take a few seconds.")),
				m_pUploadObj);
		}

		return false;
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == CID_CHANGE_ACCOUNT)
			m_pContainer->SwitchToPage(CPageLogin::ClassName(), false, 0, true);
		else
			return CSkinContainer::OnCommand(nId);

		return true;
	}

protected:
	int					CID_CHANGE_ACCOUNT;
	CLoginWorkObj		*m_pUploadObj;

};

UIOBJECT_CLASS_NAME_IMP(CPageUploadNotice, _T("Container.UploadLyrNotice"))

//////////////////////////////////////////////////////////////////////////

class CPageUploadResult : public CSkinContainer
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	void OnCreate()
	{
		CSkinContainer::OnCreate();

		GET_ID_BY_NAME(CID_TRY_AGAIN);
	}

	void OnSwitchTo()
	{
		CSkinContainer::OnSwitchTo();

		if (GetExPoolBool(SZ_EX_POOL_WORK_END, false))
		{
			SetExPool(SZ_EX_POOL_WORK_END, false);

			int		nResult = GetExPoolInt(SZ_EX_POOL_WORK_RESULT, ERR_FALSE);
			bool	bUploadOK = (nResult == ERR_OK);
			SetUIObjectVisible(_T("CID_C_UPLOAD_OK"), bUploadOK, false);
			SetUIObjectVisible(_T("CID_C_UPLOAD_FAILED"), !bUploadOK, false);
			if (!bUploadOK)
				SetUIObjectText(_T("CID_RESULT"), CStrPrintf(_T("%s"), ERROR2STR_LOCAL(nResult)).c_str(), false);

			if (GetExPoolStr(SZ_EX_POOL_WORK_MSG).size())
				m_pSkin->PostCustomCommandMsg(CMD_SHOW_ERR_RESULT);
		}
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == CMD_SHOW_ERR_RESULT)
		{
			ASSERT(GetExPoolStr(SZ_EX_POOL_WORK_MSG).size());
			m_pSkin->MessageOut(GetExPoolStr(SZ_EX_POOL_WORK_MSG).c_str());
		}
		else if (nId == CID_TRY_AGAIN)
		{
			CPageUploadWaitMessage *pPageWait = (CPageUploadWaitMessage *)m_pContainer->GetChildByClass(CPageUploadWaitMessage::ClassName());
			ASSERT(pPageWait);
			if (pPageWait)
			{
				pPageWait->SetLoginWork(false);
				pPageWait->StartWork(TOLOCAL(_T("Uploading")), 
					TOLOCAL(_T("MiniLyrics is uploading lyrics, It may take a few seconds.")),
					((CSkinWndUploadLyr *)m_pSkin)->GetUploadWorkObj());
			}
		}
		else
			return CSkinContainer::OnCustomCommand(nId);

		return true;
	}

protected:
	int					CID_TRY_AGAIN;

};

UIOBJECT_CLASS_NAME_IMP(CPageUploadResult, _T("Container.UploadLyrResult"))

//////////////////////////////////////////////////////////////////////////

void CPageUploadWaitMessage::OnWorkEnd()
{
	if (m_pWorkObj->IsJobCanceled())
	{
		if (m_bLoginWork)
			m_pContainer->SwitchToPage(CPageLogin::ClassName(), false, 0, true);
		else
			m_pContainer->SwitchToPage(CPageUploadNotice::ClassName(), false, 0, true);
	}
	else
	{
		SetExPool(SZ_EX_POOL_WORK_END, true);

		if (m_bLoginWork)
		{
			CLoginWorkObj	*pLoginWork = (CLoginWorkObj *)m_pWorkObj;
			if (pLoginWork->m_nResult == ERR_OK)
				m_pContainer->SwitchToPage(CPageUploadNotice::ClassName(), false, 0, true);
			else
				m_pContainer->SwitchToLastPage(0, true);
		}
		else
		{
			CUploadWorkObj	*pUploadWork = (CUploadWorkObj *)m_pWorkObj;
			SetExPool(SZ_EX_POOL_WORK_RESULT, pUploadWork->m_nResult);
			SetExPool(SZ_EX_POOL_WORK_MSG, pUploadWork->m_strMsg.c_str());
			m_pContainer->SwitchToPage(CPageUploadResult::ClassName(), false, 0, true);
		}
	}
}

void ShowUploadLyrDialog(CSkinWnd *pParent, tstring &strLyricsContent, LPCTSTR szMediaSource, LPCTSTR szLyrSource)
{
	ASSERT(strLyricsContent.size() > 0 || !IsEmptyString(szLyrSource));

	g_sessionUpload.Init(GetAppNameLong().c_str());

	SkinWndStartupInfo skinWndStartupInfo(_SZ_SKINWND_CLASS_NAME, _SZ_SKINWND_CLASS_NAME, 
		_T("UploadLyrics.xml"), pParent);
	CSkinWndUploadLyr *pWndUpload = new CSkinWndUploadLyr();
	skinWndStartupInfo.pSkinWnd = pWndUpload;
	CUploadWorkObj *pUploadWork = pWndUpload->GetUploadWorkObj();
	pUploadWork->m_strLyricsContent = strLyricsContent;
	pUploadWork->m_strMediaSource = szMediaSource;
	pUploadWork->m_strLyrSource = szLyrSource;

	CSkinApp::GetInstance()->GetSkinFactory()->ActiveOrCreateSkinWnd(skinWndStartupInfo);
}

void RegisterUploadLyrPage(CSkinFactory *pSkinFactory)
{
	AddUIObjNewer2(pSkinFactory, CPageLogin);
	AddUIObjNewer2(pSkinFactory, CPageUploadNotice);
	AddUIObjNewer2(pSkinFactory, CPageUploadWaitMessage);
	AddUIObjNewer2(pSkinFactory, CPageUploadResult);
}
