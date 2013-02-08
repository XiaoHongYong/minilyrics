#include "stdafx.h"
#include "MPlayerApp.h"
#include "LicenseManager.h"

CLicenseManager		g_licenseManager;

#define SZ_REGCODE			_T("C_ode_Rge")
#define SZ_REGNAME			_T("N-ame_rge")
#define SZ_INVALID_RC		_T("Inva_d_Cod-e")
#define SZ_LICENSE_STATUS	_T("L4D9E7AE1S")
#define SZ_START_DATE		_T("S4F124300D")
#define SZ_DEVICE_ID		_T("D147D6E511I")
#define SZ_LICENSED_USER	_T("L8DCE90CDU")
#define SZ_DURATION_DAYS	_T("LA75981F99D")
#define SZ_ONLINE_VERIFYIED	_T("O7D43B825V")

#ifdef _WIN32

typedef RPC_STATUS (__stdcall *FuncUuidCreate)(UUID * Uuid );

RPC_STATUS MyUuidCreate(UUID * Uuid)
{
	HMODULE hModule = LoadLibrary(_T("rpcrt4.dll"));
	if (!hModule)
	{
		ERRLOG0(_T("Failed to load library rpcrt4.dll"));
		return RPC_E_NO_CONTEXT;
	}

	FuncUuidCreate funcUuidCreate = (FuncUuidCreate)GetProcAddress(hModule, "UuidCreate");
	RPC_STATUS status = funcUuidCreate(Uuid);

	FreeLibrary(hModule);
	return status;
}

tstring CreateDeviceID()
{
	// Generate and save the device ID, if does NOT exist.
	GUID guid;
	RPC_STATUS status = MyUuidCreate(&guid);
	if (status == RPC_S_OK)
	{
		TCHAR szDeviceId[64];
		MD5ToString((LPCSTR)&guid, sizeof(guid), szDeviceId);
		if (!g_Profile.EncryptWriteString(SZ_DEVICE_ID, szDeviceId))
		{
			ERRLOG1(_T("Failed to write ID: %s"), szDeviceId);
			MessageBox(NULL, _T("Failed to save MiniLyrics settings, please contact us for help."), _T("MiniLyrics"), MB_OK);
		}
		return szDeviceId;
	}
	else
	{
		ERRLOG1(_T("Failed to create ID with error: %x"), status);
		MessageBox(NULL, _T("Failed to generate ID, please contact us for help."), _T("MiniLyrics"), MB_OK);
	}

	return _T("");
}
#endif
#ifdef _MAC_OSX
#include <CoreFoundation/CoreFoundation.h>
tstring CreateDeviceID()
{
	CFUUIDRef theUUID = CFUUIDCreate(NULL);
    CFStringRef string = CFUUIDCreateString(NULL, theUUID);
    CFRelease(theUUID);
	char buf[128];
	CFStringGetCString(string, buf, CountOf(buf), kCFStringEncodingUTF8);
	CFRelease(string);

	TCHAR szDeviceId[64];
	MD5ToString((LPCSTR)&buf, strlen(buf), szDeviceId);
	if (!g_Profile.EncryptWriteString(SZ_DEVICE_ID, szDeviceId))
		ERRLOG1(_T("Failed to write ID: %s"), szDeviceId);
	return szDeviceId;
}
#endif

CLicenseManager::CLicenseManager(void)
{
	m_bOnlineVerified = false;
	m_licenseStatus = LS_TRIAL;
}

CLicenseManager::~CLicenseManager(void)
{
}

void CLicenseManager::Init()
{
	tstring strName = g_Profile.EncryptGetString(SZ_REGNAME, _T(""));
	tstring strCode = g_Profile.EncryptGetString(SZ_REGCODE, _T(""));
	tstring strCodeInvalid = g_Profile.EncryptGetString(SZ_INVALID_RC, _T(""));

	if (strCode.size() > 0 && _tcscmp(strCodeInvalid.c_str(), strCode.c_str()) == 0)
	{
		m_licenseStatus = LS_REVOKED;
		return;
	}

	if (!VerifyCode(strName.c_str(), strCode.c_str(), false))
		return;

	m_bOnlineVerified = Tobool(g_Profile.EncryptGetInt(SZ_ONLINE_VERIFYIED, false));
	m_licenseStatus = (LicenseStatus)g_Profile.EncryptGetInt(SZ_LICENSE_STATUS, LS_ACTIVE);
}

void CLicenseManager::GetLicenseInfo(LicenseInfo &info)
{
	info.bOnlineVerified = m_bOnlineVerified;
	info.licenseStatus = m_licenseStatus;
	info.strName = g_Profile.EncryptGetString(SZ_REGNAME, _T(""));
	info.strCode = g_Profile.EncryptGetString(SZ_REGCODE, _T(""));
	info.strStartDate = g_Profile.EncryptGetString(SZ_START_DATE, _T(""));
	info.nDurationDays = g_Profile.EncryptGetInt(SZ_DURATION_DAYS, LICENSE_DURATION_LIFETIME);
	info.nLicensedUser = g_Profile.EncryptGetInt(SZ_LICENSED_USER, 1);
	info.strDeviceID = g_Profile.EncryptGetString(SZ_DEVICE_ID, _T(""));
	if (info.strDeviceID.empty())
		info.strDeviceID = CreateDeviceID();

	// Verify license start and end date.
	CBaseDate dateStart;
	if (dateStart.FromString(info.strStartDate.c_str()) && info.nDurationDays > 0)
	{
		uint32 timeExpire = dateStart.GetTime() + info.nDurationDays * CBaseDate::SECOND_IN_ONE_DAY;

		CBaseDate dateNow = CBaseDate::GetCurrentDate();
		if (timeExpire < dateNow.GetTime())
		{
			// Expired
			info.licenseStatus = m_licenseStatus = LS_EXPIRED;
		}

		CBaseDate dateExpire;
		dateExpire.FromTime(timeExpire);
		info.strExpireDate = dateExpire.ToUtcDateString();
	}
}

tstring CLicenseManager::GetLicenseCode()
{
	return g_Profile.EncryptGetString(SZ_REGCODE, _T(""));
}

int CLicenseManager::SaveActivateResult(const MLMsgRetActivateLicense &retActivation)
{
	if (retActivation.licenseStatus == LS_UNKNOWN)
		return ERR_FALSE;

	m_licenseStatus = retActivation.licenseStatus;
	m_bOnlineVerified = true;

	g_Profile.EncryptWriteString(SZ_START_DATE, retActivation.strCodeStartDate.c_str());
	g_Profile.EncryptWriteInt(SZ_DURATION_DAYS, retActivation.nCodeDurationDays);
	g_Profile.EncryptWriteInt(SZ_LICENSED_USER, retActivation.nLicensedUser);
	g_Profile.EncryptWriteInt(SZ_LICENSE_STATUS, m_licenseStatus);
	g_Profile.EncryptWriteInt(SZ_ONLINE_VERIFYIED, true);

	CMPlayerApp::GetEventsDispatcher()->DispatchUnsyncEvent(ET_LICENSE_CHANGED);

	return ERR_OK;
}

bool CLicenseManager::VerifyCode(LPCTSTR szName, LPCTSTR szCode, bool bSave)
{
	if (MLVerifyRegCodeV7Before(szName, szCode))
	{
		if (bSave)
		{
			m_licenseStatus = LS_ACTIVE;
			m_bOnlineVerified = false;

			g_Profile.EncryptWriteInt(SZ_LICENSE_STATUS, m_licenseStatus);
			g_Profile.EncryptWriteInt(SZ_ONLINE_VERIFYIED, false);
			g_Profile.EncryptWriteString(SZ_REGNAME, szName);
			g_Profile.EncryptWriteString(SZ_REGCODE, szCode);
		}
		return true;
	}
	return false;
}

void CLicenseManager::CheckChsEdition()
{
#if defined(_WIN32)
	// 根据 location 来设置中文版本差别
	LCID	lcid;
	TCHAR	szBuff[32];

	m_bChsEdition = false;
	lcid = GetUserDefaultLCID();
	if (GetLocaleInfo(lcid, LOCALE_ICOUNTRY, szBuff, CountOf(szBuff)) > 0)
	{
		if (_tcscmp(szBuff, _T("86")) == 0)
			m_bChsEdition = true;
	}
#endif

#ifdef _DEBUG
	// Read settings from Install.ini
	CProfile	file;
	tstring		strEng;
	file.Init(_T("install.ini"), _T("INSTALL"));
	strEng = file.GetString(_T("Edition"), _T(""));
	if (_tcscmp(strEng.c_str(), _T("english")) == 0)
		m_bChsEdition = false;
	else if (_tcscmp(strEng.c_str(), _T("chinese")) == 0)
		m_bChsEdition = true;
#endif

}
