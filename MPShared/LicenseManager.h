#pragma once

class CLicenseManager
{
public:
	struct LicenseInfo
	{
		tstring			strName, strCode, strDeviceID, strStartDate, strExpireDate;
		int				nDurationDays, nLicensedUser;
		LicenseStatus	licenseStatus;
		bool			bOnlineVerified;		// Has License been verified online?
	};

public:
	CLicenseManager(void);
	~CLicenseManager(void);

	void Init();

	void GetLicenseInfo(LicenseInfo &info);
	tstring GetLicenseCode();

	bool VerifyCode(LPCTSTR szName, LPCTSTR szCode, bool bSave);
	int SaveActivateResult(const MLMsgRetActivateLicense &retActivation);

	inline bool IsLicenseValid() const { return m_licenseStatus == LS_ACTIVE || m_bChsEdition; }
	LicenseStatus GetLicenseStatus() const { return m_licenseStatus; }

protected:
	void CheckChsEdition();

protected:
	LicenseStatus			m_licenseStatus;
	bool					m_bOnlineVerified;
	bool					m_bChsEdition;

};

extern CLicenseManager		g_licenseManager;
