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

		g_sessionUpload.Init(GetAppNameLong().c_str());

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

		g_sessionUpload.Init(GetAppNameLong().c_str());

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
				m_nResult = g_sessionUpload.Upload(m_strLyricsContent, m_strLyrId, m_strMsg);
			}
		}
		g_sessionUpload.Close();

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

class CPageLogin : public CSkinContainer
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	CPageLogin()
	{
		m_pLoginObj = NULL;
	}

	void OnCreate()
	{
		CSkinContainer::OnCreate();

		m_pLoginObj = ((CSkinWndUploadLyr *)m_pSkin)->GetUploadWorkObj();
		ASSERT(m_pLoginObj);

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

	void OnSwitchTo()
	{
		CSkinContainer::OnSwitchTo();
		m_pSkin->SetCaptionText(TOLOCAL(_T("Log in")));

		if (m_pLoginObj)
		{
			if (m_pLoginObj->m_strMsg.size())
				m_pSkin->MessageOut(m_pLoginObj->m_strMsg.c_str());

			if (m_pLoginObj->m_nResult != ERR_OK && !m_pLoginObj->IsJobCanceled())
			{
				m_pSkin->MessageOut(ERROR2STR_LOCAL(m_pLoginObj->m_nResult));
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

		CPageWaitMessage *pPageWait = (CPageWaitMessage *)m_pContainer->GetChildByClass(CPageWaitMessage::ClassName());
		ASSERT(pPageWait);
		if (pPageWait)
		{
			pPageWait->StartWork(TOLOCAL(_T("Login")), 
				TOLOCAL(_T("Processing login, It may take a few seconds.")),
				m_pLoginObj);
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
	CLoginWorkObj		*m_pLoginObj;

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

		m_pSkin->SetCaptionText(TOLOCAL(_T("Upload Lyrics")));

		ASSERT(g_sessionUpload.IsLogined());
		SetUIObjectText(_T("CID_ACCOUNT_NOTICE"), CStrPrintf(_T("Your lyrics will be uploaded under account: %s"), g_sessionUpload.GetUser()).c_str(), false);
	}

	bool OnOK()
	{
		CPageWaitMessage *pPageWait = (CPageWaitMessage *)m_pContainer->GetChildByClass(CPageWaitMessage::ClassName());
		ASSERT(pPageWait);
		if (pPageWait)
		{
			pPageWait->StartWork(TOLOCAL(_T("Uploading")), 
				TOLOCAL(_T("MiniLyrics is uploading lyrics, It may take a few seconds.")),
				m_pUploadObj);
		}
		return true;
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
	CPageUploadResult()
	{
		m_pUploadWorkObj = NULL;
	}

	void OnCreate()
	{
		CSkinContainer::OnCreate();

		m_pUploadWorkObj = ((CSkinWndUploadLyr *)m_pSkin)->GetUploadWorkObj();
		ASSERT(m_pUploadWorkObj);

		GET_ID_BY_NAME(CID_TRY_AGAIN);
	}

	void OnSwitchTo()
	{
		CSkinContainer::OnSwitchTo();

		m_pSkin->SetCaptionText(TOLOCAL(_T("Uploading")));
		if (m_pUploadWorkObj)
		{
			bool	bUploadOK = (m_pUploadWorkObj->m_nResult == ERR_OK);
			SetUIObjectVisible(_T("CID_C_UPLOAD_OK"), bUploadOK, false);
			SetUIObjectVisible(_T("CID_C_UPLOAD_FAILED"), !bUploadOK, false);
			if (!bUploadOK)
				SetUIObjectText(_T("CID_RESULT"), CStrPrintf(_T("%s"), ERROR2STR_LOCAL(m_pUploadWorkObj->m_nResult)).c_str(), false);

			if (m_pUploadWorkObj->m_strMsg.size())
				m_pSkin->PostCustomCommandMsg(CMD_SHOW_ERR_RESULT);

			// Save lyrics id.
		}
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == CMD_SHOW_ERR_RESULT)
		{
			if (m_pUploadWorkObj && m_pUploadWorkObj->m_strMsg.size())
				m_pSkin->MessageOut(m_pUploadWorkObj->m_strMsg.c_str());
		}
		else if (nId == CID_TRY_AGAIN)
		{
			CPageWaitMessage *pPageWait = (CPageWaitMessage *)m_pContainer->GetChildByClass(CPageWaitMessage::ClassName());
			ASSERT(pPageWait);
			if (pPageWait)
			{
				pPageWait->StartWork(TOLOCAL(_T("Uploading")), 
					TOLOCAL(_T("MiniLyrics is uploading lyrics, It may take a few seconds.")),
					m_pUploadWorkObj);
			}
		}
		else
			return CSkinContainer::OnCustomCommand(nId);

		return true;
	}

protected:
	int					CID_TRY_AGAIN;
	CUploadWorkObj		*m_pUploadWorkObj;

};

UIOBJECT_CLASS_NAME_IMP(CPageUploadResult, _T("Container.UploadLyrResult"))

//////////////////////////////////////////////////////////////////////////

class CPageUploadWaitMessage : public CPageWaitMessage
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	virtual void OnWorkEnd()
	{
		if (m_pWorkObj->IsJobCanceled())
		{
			if (g_sessionUpload.IsLogined())
				m_pContainer->SwitchToPage(CPageUploadNotice::ClassName(), false, 0, true);
			else
				m_pContainer->SwitchToPage(CPageLogin::ClassName(), false, 0, true);
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
			m_pContainer->SwitchToPage(CPageUploadResult::ClassName(), false, 0, true);
		}
	}

};

UIOBJECT_CLASS_NAME_IMP(CPageUploadWaitMessage, _T("Container.UploadLyrWaitMsg"))

//////////////////////////////////////////////////////////////////////////

void ShowUploadLyrDialog(CSkinWnd *pParent, tstring &strLyricsContent, LPCTSTR szMediaSource, LPCTSTR szLyrSource)
{
	SkinWndStartupInfo skinWndStartupInfo(_SZ_SKINWND_CLASS_NAME, _SZ_SKINWND_CLASS_NAME, 
		_T("UploadLyrics.xml"), pParent);
	skinWndStartupInfo.pSkinWnd = new CSkinWndUploadLyr();
	CSkinApp::GetInstance()->GetSkinFactory()->ActiveOrCreateSkinWnd(skinWndStartupInfo);
}

void RegisterUploadLyrPage(CSkinFactory *pSkinFactory)
{
	AddUIObjNewer2(pSkinFactory, CPageLogin);
	AddUIObjNewer2(pSkinFactory, CPageUploadNotice);
	AddUIObjNewer2(pSkinFactory, CPageUploadWaitMessage);
	AddUIObjNewer2(pSkinFactory, CPageUploadResult);
}
