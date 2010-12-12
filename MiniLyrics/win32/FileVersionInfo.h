
#pragma once

class CFileVersionInfo
{
public:
	CFileVersionInfo();
	~CFileVersionInfo();

	struct LANGANDCODEPAGE
	{
		WORD wLanguage;
		WORD wCodePage;
	};

public:
	bool Open(LPCTSTR lpszFileName);

	void GetFileVersion(int &nMajor, int &nMinor, int &nBuild, int &nRevision);
	void GetProductVersion(int &nMajor, int &nMinor, int &nBuild, int &nRevision);

	VS_FIXEDFILEINFO *GetFixedFileInfo() { return m_pFileInfo; }

	//
	// The method below is related to current language Id.
	//
	int GetLanguageCount() const { return m_nLanguageCount; }
	LANGANDCODEPAGE *GetAllLanguages() { return m_pLanguages; }
	void SetCurrentLanguage(int nIndex);
	tstring GetLanguageName(int nLanguageId);

	void AutoSetLanguage();

	bool GetValue(LPCTSTR szValueName, tstring &strValue);

	bool GetCompanyName(tstring &strValue) { return GetValue(_T("CompanyName"), strValue); }
	bool GetFileDescription(tstring &strValue) { return GetValue(_T("FileDescription"), strValue); }
	bool GetFileVersion(tstring &strValue) { return GetValue(_T("FileVersion"), strValue); }
	bool GetInternalName(tstring &strValue) { return GetValue(_T("InternalName"), strValue); }
	bool GetLegalCopyright(tstring &strValue) { return GetValue(_T("LegalCopyright"), strValue); }
	bool GetOriginalFileName(tstring &strValue) { return GetValue(_T("OriginalFileName"), strValue); }
	bool GetProductName(tstring &strValue) { return GetValue(_T("ProductName"), strValue); }
	bool GetProductVersion(tstring &strValue) { return GetValue(_T("ProductVersion"), strValue); }
	bool GetComments(tstring &strValue) { return GetValue(_T("Comments"), strValue); }
	bool GetLegalTrademarks(tstring &strValue) { return GetValue(_T("LegalTrademarks"), strValue); }
	bool GetPrivateBuild(tstring &strValue) { return GetValue(_T("PrivateBuild"), strValue); }
	bool GetSpecialBuild(tstring &strValue) { return GetValue(_T("SpecialBuild"), strValue); }

protected:
	CCharBuffer			m_buf;

	VS_FIXEDFILEINFO	*m_pFileInfo;

	LANGANDCODEPAGE		*m_pLanguages;
	int					m_nLanguageCount;

	tstring				m_strCurLangBlockName;

};
