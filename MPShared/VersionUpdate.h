// VersionUpdate.h: interface for the CVersionUpdate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VERSIONUPDATE_H__67809D19_D62A_4871_8AD8_FCD90FB82F41__INCLUDED_)
#define AFX_VERSIONUPDATE_H__67809D19_D62A_4871_8AD8_FCD90FB82F41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CInvalidRegCodeDlg : public CBaseDialog
{
public:
	CInvalidRegCodeDlg() : CBaseDialog(IDD_INVALID_REG_CODE)	{};

	~CInvalidRegCodeDlg()	{};

	void OnCommand(UINT uID, UINT nNotifyCode)
	{
		CBaseDialog::OnCommand(uID, nNotifyCode);

		if (uID == IDC_VISITWEB)
		{
			OpenUrl(this, GetStrName(SN_HTTP_DOMAIN));
			OnOK();
		}
	}
};

class CVersion
{
public:
	CVersion()
	{
		nVerMajor = 0;
		nVerMinor = 0;
		nVerBuild = 0;
	}
	CVersion(int _nVerMajor, int _nVerMinor, int _nVerBuild) 
		: nVerMajor(_nVerMajor), nVerMinor(_nVerMinor), nVerBuild(_nVerBuild)
	{
	}
	BOOL From(LPCTSTR szVersion)
	{
		int		nRet;
		nRet = _stscanf(szVersion, _T("%d.%d.%d"), &nVerMajor, &nVerMinor, &nVerBuild);
		if (nRet == 2)
			nVerBuild = 0;
		else if (nRet <= 1)
			return FALSE;

		return TRUE;
	}
	void To(TCHAR szVersion[])
	{
		_stprintf(szVersion, _T("%d.%d.%d"), nVerMajor, nVerMinor, nVerBuild);
	}

	bool operator==(CVersion &ver2)
	{
		return nVerMajor == ver2.nVerMajor && nVerMinor == ver2.nVerMinor && nVerBuild == ver2.nVerBuild;
	}

	bool operator>(CVersion &ver2)
	{
		if (nVerMajor > ver2.nVerMajor)
			return true;
		else if (nVerMajor == ver2.nVerMajor)
		{
			if (nVerMinor > ver2.nVerMinor)
				return true;
			else if (nVerMinor == ver2.nVerMinor)
			{
				return nVerBuild > ver2.nVerBuild;
			}
		}
		return false;
	}

	bool operator>=(CVersion &ver2)
	{
		if (nVerMajor > ver2.nVerMajor)
			return true;
		else if (nVerMajor == ver2.nVerMajor)
		{
			if (nVerMinor > ver2.nVerMinor)
				return true;
			else if (nVerMinor == ver2.nVerMinor)
			{
				return nVerBuild >= ver2.nVerBuild;
			}
		}
		return false;
	}

	bool operator<(CVersion &ver2)
	{
		return ver2 > *this;
	}

	bool operator<=(CVersion &ver2)
	{
		return ver2 >= *this;
	}

	int		nVerMajor, nVerMinor, nVerBuild;		// 最新版本

};

class CVersionInfo
{
public:
	CVersionInfo();
	int FromXML(SXNode *pNodeVer);

public:
	// int		nNewVerMajor, nNewVerMinor, nNewVerBuild;		// 最新版本
	// int		nObsVerMajor, nObsVerMinor, nObsVerBuild;		// 废弃版本
	CVersion		verNew;
	CVersion		verObs;
	tstring			strNewVerDate;			// 新版发布日期
	tstring			strFeature;				// 新版特性

	int				nTrialDays;				// 试用日期

	bool			bUseAffiliate;			// default is true

	//
	// New Version Download Web Page info
	tstring			strNewVerDlPage;		// New Version Download Web Page
	int				nPageWidth, nPageHeight;
};

class CVersionUpdate  
{
public:
	void CheckNewVersion(CLyricsDownloadMgr *pMgr);
	void CheckNewVersionManual(CLyricsDownloadMgr *pMgr);

	virtual void OnDownloadOK(CDownloadTask *pTask);

	CVersionUpdate();
	virtual ~CVersionUpdate();
	
protected:
	void OnVersionInfoDownOK(CDownloadTask *pTask, bool bManualCheck);

};

#endif // !defined(AFX_VERSIONUPDATE_H__67809D19_D62A_4871_8AD8_FCD90FB82F41__INCLUDED_)
