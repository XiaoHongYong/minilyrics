// LocalizeTool.cpp: implementation of the CLocalizeTool class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utils.h"
#include "LocalizeTool.h"

CLanguageTool			g_LangTool;

#define SZ_LANG_DIR		_T("lang")

#ifdef _WINCE_

#ifndef DEFAULT_GUI_FONT
#define DEFAULT_GUI_FONT  SYSTEM_FONT
#endif

#endif

tstring GetLangPackDir()
{
	tstring strLangDir = GetWorkingFolder();
	strLangDir += SZ_LANG_DIR _T(SZ_DIR_SLASH);

#if defined (_DEBUG) && defined (_WIN32)
	if (!IsDirExist(strLangDir.c_str()))
	{
		strLangDir = GetInstallShareDir();
		strLangDir += _T("lang\\");
	}
#endif
	return strLangDir;
}


// Remove '&'
tstring RemovePrefixOfAcckey(LPCTSTR szStr)
{
	LPCTSTR		p;

	p = _tcschr(szStr, '&');
	if (!p)
		return szStr;

	tstring		str;

	// Remove "(&A)"
	if (p > szStr && *(p - 1) == '(' && p[1] != '\0' && p[2] == ')' && p[3] == '\0')
	{
		str.append(szStr, (int)(p - szStr) - 1);
	}
	else
	{
		str.append(szStr, (int)(p - szStr));
		str.append(p + 1);
	}

	return str;
}

static bool FindStringValue(LPCTSTR szBuff, LPCTSTR szName, tstring &strValue)
{
	LPCTSTR		szBeg, szEnd;

	strValue.resize(0);

	szBeg = _tcsstr(szBuff, szName);
	if (!szBeg)
		return false;

	szBeg += _tcslen(szName);
	szEnd = szBeg;
	while (*szEnd && *szEnd != '\r' && *szEnd != '\n')
		szEnd++;

	strValue.append(szBeg, szEnd);

	return true;
}

//
// GetPrivateProfileString don't support UTF-8 encoding file.
// Use this function instead.
//
static bool GetLanguagePackInfo(LPCTSTR szFile, tstring &strLanguage, tstring &strLangCode, tstring &strLangCodeFull)
{
	char		buff[1024];
	FILE		*fp;
	FILE_ENCODING	fe;
	tstringex	strBuff;
	LPCTSTR		SZ_INFO = _T("[Info]");

	fp = _tfopen(szFile, _T("rb"));
	if (!fp)
		return false;

	if (fread(buff, 1, sizeof(buff), fp) != sizeof(buff))
		goto R_FAILED;

	fe = GetFileEncodingByBom(buff);
	if (fe == FE_ANSI)
		ConvertStr(buff, CountOf(buff), strBuff);
	else if (fe == FE_UTF8)
		ConvertStr(buff + 3, CountOf(buff) - 3, strBuff, ED_UTF8);
	else
		ConvertStr((WCHAR*)buff + 1, sizeof(buff) / sizeof(WCHAR) - 1, strBuff, ED_UTF8);

	// [Info]
	if (_tcsnicmp(strBuff.c_str(), SZ_INFO, _tcslen(SZ_INFO)) != 0)
		goto R_FAILED;

	if (!FindStringValue(strBuff.c_str(), _T("Language="), strLanguage))
		goto R_FAILED;

	if (!FindStringValue(strBuff.c_str(), _T("LanguageCode="), strLangCode))
		goto R_FAILED;

	if (!FindStringValue(strBuff.c_str(), _T("LanguageCodeFull="), strLangCodeFull))
		goto R_FAILED;

	fclose(fp);
	return true;

R_FAILED:
	fclose(fp);
	return false;
}

bool GetUserDefaultLang(tstring &strLang, tstring &strLangFull)
{
#ifdef _WIN32
	struct LangCode
	{
		int			nLangId;
		const char	*szLangShort;
		const char	*szLangFull;
	};

	static LangCode	gLangCode[] = 
	{
		//	{ 0x0000, _T(""), _T("Language Neutral") },
		//	{ 0x007f, _T(""), _T("The language for the invariant locale (LOCALE_INVARIANT). See MAKELCID.") },
		//	{ 0x0400, _T(""), _T("Process or User Default Language") },
		//	{ 0x0800, _T(""), _T("System Default Language") },
		{ 0x0436, "Afrikaans", "Afrikaans" },
		{ 0x041c, "Albanian", "Albanian" },
		{ 0x0401, "Arabic", "Arabic (Saudi Arabia)" },
		{ 0x0801, "Arabic", "Arabic (Iraq)" },
		{ 0x0c01, "Arabic", "Arabic (Egypt)" },
		{ 0x1001, "Arabic", "Arabic (Libya)" },
		{ 0x1401, "Arabic", "Arabic (Algeria)" },
		{ 0x1801, "Arabic", "Arabic (Morocco)" },
		{ 0x1c01, "Arabic", "Arabic (Tunisia)" },
		{ 0x2001, "Arabic", "Arabic (Oman)" },
		{ 0x2401, "Arabic", "Arabic (Yemen)" },
		{ 0x2801, "Arabic", "Arabic (Syria)" },
		{ 0x2c01, "Arabic", "Arabic (Jordan)" },
		{ 0x3001, "Arabic", "Arabic (Lebanon)" },
		{ 0x3401, "Arabic", "Arabic (Kuwait)" },
		{ 0x3801, "Arabic", "Arabic (U.A.E.)" },
		{ 0x3c01, "Arabic", "Arabic (Bahrain)" },
		{ 0x4001, "Arabic", "Arabic (Qatar)" },
		{ 0x042b, "Armenian", "Armenian" },
		{ 0x042c, "Azeri", "Azeri (Latin)" },
		{ 0x082c, "Azeri", "Azeri (Cyrillic)" },
		{ 0x042d, "Basque", "Basque" },
		{ 0x0423, "Belarusian", "Belarusian" },
		{ 0x0402, "Bulgarian", "Bulgarian" },
		{ 0x0455, "Burmese", "Burmese" },
		{ 0x0403, "Catalan", "Catalan" },
		{ 0x0404, "Traditional Chinese", "Chinese (Taiwan)" },
		{ 0x0804, "Simplified Chinese", "Chinese (PRC)" },
		{ 0x0c04, "Chinese", "Chinese (Hong Kong SAR, PRC)" },
		{ 0x1004, "Simplified Chinese", "Chinese (Singapore)" },
		{ 0x1404, "Traditional Chinese", "Chinese (Macau SAR)" },
		{ 0x041a, "Croatian", "Croatian" },
		{ 0x0405, "Czech", "Czech" },
		{ 0x0406, "Danish", "Danish" },
		{ 0x0465, "Divehi", "Divehi" },
		{ 0x0413, "Dutch", "Dutch (Netherlands)" },
		{ 0x0813, "Dutch", "Dutch (Belgium)" },
		{ 0x0409, "English", "English (United States)" },
		{ 0x0809, "English", "English (United Kingdom)" },
		{ 0x0c09, "English", "English (Australian)" },
		{ 0x1009, "English", "English (Canadian)" },
		{ 0x1409, "English", "English (New Zealand)" },
		{ 0x1809, "English", "English (Ireland)" },
		{ 0x1c09, "English", "English (South Africa)" },
		{ 0x2009, "English", "English (Jamaica)" },
		{ 0x2409, "English", "English (Caribbean)" },
		{ 0x2809, "English", "English (Belize)" },
		{ 0x2c09, "English", "English (Trinidad)" },
		{ 0x3009, "English", "English (Zimbabwe)" },
		{ 0x3409, "English", "English (Philippines)" },
		{ 0x0425, "Estonian", "Estonian" },
		{ 0x0438, "Faeroese", "Faeroese" },
		{ 0x0429, "Farsi", "Farsi" },
		{ 0x040b, "Finnish", "Finnish" },
		{ 0x040c, "French", "French (Standard)" },
		{ 0x080c, "French", "French (Belgian)" },
		{ 0x0c0c, "French", "French (Canadian)" },
		{ 0x100c, "French", "French (Switzerland)" },
		{ 0x140c, "French", "French (Luxembourg)" },
		{ 0x180c, "French", "French (Monaco)" },
		{ 0x0456, "Galician", "Galician" },
		{ 0x0437, "Georgian", "Georgian" },
		{ 0x0407, "German", "German (Standard)" },
		{ 0x0807, "German", "German (Switzerland)" },
		{ 0x0c07, "German", "German (Austria)" },
		{ 0x1007, "German", "German (Luxembourg)" },
		{ 0x1407, "German", "German (Liechtenstein)" },
		{ 0x0408, "Greek", "Greek" },
		{ 0x0447, "Gujarati", "Gujarati" },
		{ 0x040d, "Hebrew", "Hebrew" },
		{ 0x0439, "Hindi", "Hindi" },
		{ 0x040e, "Hungarian", "Hungarian" },
		{ 0x040f, "Icelandic", "Icelandic" },
		{ 0x0421, "Indonesian", "Indonesian" },
		{ 0x0410, "Italian", "Italian (Standard)" },
		{ 0x0810, "Italian", "Italian (Switzerland)" },
		{ 0x0411, "Japanese", "Japanese" },
		{ 0x044b, "Kannada", "Kannada" },
		{ 0x0457, "Konkani", "Konkani" },
		{ 0x0412, "Korean", "Korean" },
		{ 0x0812, "Korean", "Korean (Johab)" },
		{ 0x0440, "Kyrgyz", "Kyrgyz" },
		{ 0x0426, "Latvian", "Latvian" },
		{ 0x0427, "Lithuanian", "Lithuanian" },
		{ 0x0827, "Lithuanian", "Lithuanian (Classic)" },
		{ 0x042f, "FYRO Macedonian", "FYRO Macedonian" },
		{ 0x043e, "Malay", "Malay (Malaysian)" },
		{ 0x083e, "Malay", "Malay (Brunei Darussalam)" },
		{ 0x044e, "Marathi", "Marathi" },
		{ 0x0450, "Mongolian", "Mongolian" },
		{ 0x0414, "Norwegian", "Norwegian (Bokmal)" },
		{ 0x0814, "Norwegian", "Norwegian (Nynorsk)" },
		{ 0x0415, "Polish", "Polish" },
		{ 0x0416, "Portuguese", "Portuguese (Brazil)" },
		{ 0x0816, "Portuguese", "Portuguese (Portugal)" },
		{ 0x0446, "Punjabi", "Punjabi" },
		{ 0x0418, "Romanian", "Romanian" },
		{ 0x0419, "Russian", "Russian" },
		{ 0x044f, "Sanskrit", "Sanskrit" },
		{ 0x0c1a, "Serbian", "Serbian (Cyrillic)" },
		{ 0x081a, "Serbian", "Serbian (Latin)" },
		{ 0x041b, "Slovak", "Slovak" },
		{ 0x0424, "Slovenian", "Slovenian" },
		{ 0x040a, "Spanish", "Spanish (Spain, Traditional Sort)" },
		{ 0x080a, "Spanish", "Spanish (Mexican)" },
		{ 0x0c0a, "Spanish", "Spanish (Spain, International Sort)" },
		{ 0x100a, "Spanish", "Spanish (Guatemala)" },
		{ 0x140a, "Spanish", "Spanish (Costa Rica)" },
		{ 0x180a, "Spanish", "Spanish (Panama)" },
		{ 0x1c0a, "Spanish", "Spanish (Dominican Republic)" },
		{ 0x200a, "Spanish", "Spanish (Venezuela)" },
		{ 0x240a, "Spanish", "Spanish (Colombia)" },
		{ 0x280a, "Spanish", "Spanish (Peru)" },
		{ 0x2c0a, "Spanish", "Spanish (Argentina)" },
		{ 0x300a, "Spanish", "Spanish (Ecuador)" },
		{ 0x340a, "Spanish", "Spanish (Chile)" },
		{ 0x380a, "Spanish", "Spanish (Uruguay)" },
		{ 0x3c0a, "Spanish", "Spanish (Paraguay)" },
		{ 0x400a, "Spanish", "Spanish (Bolivia)" },
		{ 0x440a, "Spanish", "Spanish (El Salvador)" },
		{ 0x480a, "Spanish", "Spanish (Honduras)" },
		{ 0x4c0a, "Spanish", "Spanish (Nicaragua)" },
		{ 0x500a, "Spanish", "Spanish (Puerto Rico)" },
		{ 0x0430, "Sutu", "Sutu" },
		{ 0x0441, "Swahili", "Swahili (Kenya)" },
		{ 0x041d, "Swedish", "Swedish" },
		{ 0x081d, "Swedish", "Swedish (Finland)" },
		{ 0x045a, "Syriac", "Syriac" },
		{ 0x0449, "Tamil", "Tamil" },
		{ 0x0444, "Tatar", "Tatar (Tatarstan)" },
		{ 0x044a, "Telugu", "Telugu" },
		{ 0x041e, "Thai", "Thai" },
		{ 0x041f, "Turkish", "Turkish" },
		{ 0x0422, "Ukrainian", "Ukrainian" },
		{ 0x0420, "Urdu", "Urdu (Pakistan)" },
		{ 0x0820, "Urdu", "Urdu (India)" },
		{ 0x0443, "Uzbek", "Uzbek (Latin)" },
		{ 0x0843, "Uzbek", "Uzbek (Cyrillic)" },
		{ 0x042a, "Vietnamese", "Vietnamese" },
		{ 0x0, NULL, NULL }
	};

	LANGID	wLangId = GetUserDefaultLangID();

	for (LangCode *pItem = gLangCode; pItem->szLangFull != NULL; pItem++)
	{
		if (pItem->nLangId == wLangId)
		{
			tstringex	str;
			ConvertStr(pItem->szLangShort, -1, str);
			strLang = str.c_str();
			ConvertStr(pItem->szLangFull, -1, str);
			strLangFull = str.c_str();
			return true;
		}
	}
#endif

	return false;
}

bool CLanguageFile::Init(LPCTSTR szProfile)
{
	//
	// Init and load all the strings section
	//
	tstringex	str;
	LPCTSTR		szBeg, szEnd;
	tstring		strAppName, strKey, strValue;

	if (!OpenFileToTString(szProfile, str))
		return false;

	//
	// 查找 AppName
	szBeg = str.c_str();
	while (*szBeg)
	{
		while (*szBeg == ' ' || *szBeg == '\t' || *szBeg == '\n')
			szBeg++;

		if (*szBeg != '[')
		{
			while (*szBeg && *szBeg != '\n')
				szBeg++;
			continue;
		}
		szBeg++;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		szEnd = szBeg;
		while (*szEnd && *szEnd != ']' && *szEnd != '\n')
			szEnd++;

		if (*szEnd != ']')
		{
			while (*szBeg && *szBeg != '\n')
				szBeg++;
			continue;
		}

		// found appname now
		strAppName.resize(0);
		strAppName.append(szBeg, szEnd);
		TrimStr(strAppName);

		szBeg = szEnd + 1;

		// only process "string" section
		if (_tcsicmp(strAppName.c_str(), _T("string")) != 0)
			continue;

		while (*szBeg)
		{
			// get key and values
			while (*szBeg == ' ' || *szBeg == '\t')
				szBeg++;

			while (*szBeg == '\r' || *szBeg == '\n')
				szBeg++;

			if (*szBeg == '[')
				break;

			szEnd = szBeg;
			while (*szEnd && *szEnd != '=' && *szEnd != '\n')
				szEnd++;

			if (*szEnd != '=')
			{
				szBeg = szEnd;
				while (*szBeg && *szBeg != '\n')
					szBeg++;
				continue;
			}

			// found key
			strKey.resize(0);
			strKey.append(szBeg, szEnd);

			szBeg = szEnd + 1;
			while (*szEnd && *szEnd != '\n')
				szEnd++;

			while (*szEnd == '\r' || *szEnd == '\n')
				szEnd--;
			if (*szEnd)
				szEnd++;
			strValue.resize(0);
			strValue.append(szBeg, szEnd);

			m_mapString[strKey] = strValue;

			szBeg = szEnd;
		}
	}

	return true;
}

void CLanguageFile::Close()
{
	m_mapString.clear();
}

LPCTSTR CLanguageFile::GetTranslation(LPCTSTR szString)
{
	MapString::iterator		it;
	it = m_mapString.find(szString);
	if (it == m_mapString.end())
		return szString;

	if ((*it).second.empty())
		return szString;

	return (*it).second.c_str();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLanguageTool::CLanguageTool()
{
	m_bEnglish = true;
	m_bInitialized = false;
	m_nCurStr = 0;
}

CLanguageTool::~CLanguageTool()
{
}

bool CLanguageTool::GetCurrentLanguageFile(tstring &strLanguageFile)
{
	TCHAR	szLanFileName[MAX_PATH] = _T("");

	strLanguageFile = GetLangPackDir();

	if (g_Profile.GetProfileString(_T("Language"), _T(""), szLanFileName, MAX_PATH) == 0)
	{
		tstring		strLang, strLangFull;

		if (!GetUserDefaultLang(strLang, strLangFull))
			return false;

		if (_tcsicmp(strLang.c_str(), _T("English")) == 0)
			return false;

		V_TRANSFILEINFO			vTransFiles;
		tstring					strFileName, strFileNameBest;

		ListTransFiles(GetLangPackDir().c_str(), vTransFiles);

		// search for full match
		for (int i = 0; i < (int)vTransFiles.size(); i++)
		{
			TransFileInfo	&item = vTransFiles[i];
			if (_tcscmp(item.strLanguageCodeFull.c_str(), strLangFull.c_str()) == 0)
			{
				strFileNameBest = item.strFileName;
				break;
			}
			if (strFileName.empty() && _tcscmp(item.strLanguageCode.c_str(), strLang.c_str()) == 0)
			{
				strFileName = item.strFileName;
			}
		}

		if (strFileNameBest.size())
			_tcscpy_safe(szLanFileName, CountOf(szLanFileName), strFileNameBest.c_str());
		else if (strFileName.size())
			_tcscpy_safe(szLanFileName, CountOf(szLanFileName), strFileName.c_str());

		// save auto detected translation file
		if (!IsEmptyString(szLanFileName))
			g_Profile.WriteProfileString(_T("Language"), szLanFileName);
	}

	strLanguageFile += szLanFileName;
	return (!IsEmptyString(szLanFileName) && IsFileExist(strLanguageFile.c_str()));
}

bool CLanguageTool::ListTransFiles(LPCTSTR szDir, V_TRANSFILEINFO &vTransFiles)
{
	CMLFileFind		find;
	tstring			str;
	TransFileInfo	item;
	tstring			strFilter, strEngTransFileName;
	tstring			strLangDir;

	if (szDir && !IsEmptyString(szDir))
		strLangDir = szDir;
	else
		strLangDir = GetLangPackDir();

	// Add english default translation
	item.strFileName = _T("English");
	item.strLanguage = _T("English");
	item.strLanguageCode = _T("English");
	item.strLanguageCodeFull = _T("English");
	vTransFiles.push_back(item);

	strFilter = _T("*.ini");
	strEngTransFileName = _T("English.ini");

	if (!find.OpenDir(strLangDir.c_str(), strFilter.c_str()))
		return false;

	while (find.FindNext())
	{
		if (!find.IsCurDir())
		{
			TransFileInfo	item;

			if (_tcsicmp(find.GetCurName(), strEngTransFileName.c_str()) == 0)
				continue;

			str = strLangDir;
			DirStringFormat(str);
			str += find.GetCurName();

			if (GetLanguagePackInfo(str.c_str(), item.strLanguage, 
				item.strLanguageCode, item.strLanguageCodeFull))
			{
				item.strFileName = find.GetCurName();
				vTransFiles.push_back(item);
			}
		}
	}

	return true;
}

// COMMENT:
//		将字符串转换为本地语言。
// RETURN:
//		return translated string
LPCTSTR CLanguageTool::ToLocalString(LPCTSTR szString)
{
	if (!m_bInitialized)
	{
		// Init select correct language...
		OnLanguageChanged();
	}

	szString = ReplaceMacroStr(szString);

	if (m_bEnglish)
		return szString;

	return m_LanguageFile.GetTranslation(szString);
}

void CLanguageTool::SetMacro(LPCTSTR szName, LPCTSTR szValue)
{
	m_mapMacros[szName] = szValue;
}

LPCTSTR CLanguageTool::ReplaceMacroStr(LPCTSTR szString)
{
	LPCTSTR		szPos;

	for (map_string::iterator it = m_mapMacros.begin(); it != m_mapMacros.end(); ++it)
	{
		szPos = _tcsstr(szString, (*it).first.c_str());
		while (szPos)
		{
			tstring		&str = m_vStrCached[m_nCurStr];
			str.resize(0);
			str.append(szString, (int)(szPos - szString));
			str += (*it).second;
			str += szPos + (*it).first.size();

			szString = str.c_str();
			m_nCurStr++;
			if (m_nCurStr >= MAX_CACHED_STR)
				m_nCurStr = 0;

			szPos = _tcsstr(szString, (*it).first.c_str());
		}
	}

	return szString;
}

void CLanguageTool::Close()
{
	m_LanguageFile.Close();
	m_bInitialized = false;
	m_bEnglish = true;
	m_mapMacros.clear();
}

bool CLanguageTool::IsEnglish()
{
	if (!m_bInitialized)
	{
		// Init select correct language...
		OnLanguageChanged();
	}

	return m_bEnglish;
}

void CLanguageTool::OnLanguageChanged()
{
	m_bInitialized = true;
	m_bEnglish = true;

	tstring		strLangFile;
	if (GetCurrentLanguageFile(strLangFile))
	{
		m_bEnglish = false;
		m_LanguageFile.Init(strLangFile.c_str());
	}
}
