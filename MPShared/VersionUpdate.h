
#pragma once

class CVersionInfo
{
public:
	int FromXML(SXNode *pNodeVer);

public:
	VersionNumber	verNew;
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
