// VersionUpdate.h: interface for the CVersionUpdate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VERSIONUPDATE_H__67809D19_D62A_4871_8AD8_FCD90FB82F41__INCLUDED_)
#define AFX_VERSIONUPDATE_H__67809D19_D62A_4871_8AD8_FCD90FB82F41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
	BOOL From(LPCTSTR szVersion);
	tstring To()
	{
		return CStrPrintf(_T("%d.%d.%d"), nVerMajor, nVerMinor, nVerBuild).c_str();
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
	CVersion		verNew;
	tstring			strNewVerDate;			// 新版发布日期
	tstring			strFeature;				// 新版特性

};

class CVersionUpdate  
{
public:
	CVersionUpdate();
	virtual ~CVersionUpdate();

	void CheckNewVersion(bool bAutoCheck = true);

	virtual void OnDownloadOK(CDownloadTask *pTask);

	int GetUpdateInfo(CDownloadTask *pTask, CVersionInfo &versionInfo);

};

#endif // !defined(AFX_VERSIONUPDATE_H__67809D19_D62A_4871_8AD8_FCD90FB82F41__INCLUDED_)
