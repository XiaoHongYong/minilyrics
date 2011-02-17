#import "../../base/base.h"
#import "../../Utils/Utils.h"
#import "../../gfx/gfx.h"
#import "../../GfxRaw/GfxRaw.h"
#import "../../Utils/Utils.h"
#import "helper.h"

// FgColor="#FFFFFF"
void GetColorValue(MLColor &clr, LPCTSTR szColor)
{
	UINT		uRet;
	LPCTSTR p = ReadColorValue_t(szColor, uRet);
	if (*p == '\0')
		clr.set(uRet);
}

void ProfileGetColorValue(COLORREF &clr, LPCTSTR szSectName, LPCTSTR szKeyName)
{
	MLColor		c;

	c.set(clr);
	GetColorValue(c, g_Profile.GetProfileString(szSectName, szKeyName, _T("")));

	clr = c.get();
}

void ProfileGetColorValue(MLColor &clr, LPCTSTR szSectName, LPCTSTR szKeyName)
{
	GetColorValue(clr, g_Profile.GetProfileString(szSectName, szKeyName, _T("")));
}

// szValue format: Verdana, 13, thin, 1, 0, Tahoma
// Latin font, height, weight(bold,normal,thin), italic, underline, Other font
bool FontPropertyValueFromStr(LPCTSTR szValue, int &nHeight, int &nWeight, BYTE &byItalic, tstring &strFaceNameLatin9, tstring &strFaceNameOthers)
{
	int				nUnderLine;
	tstring			strWeight;
	CCSVLineT		csvLine;

	csvLine.SetReadingLine(szValue);

	// latin font name
	csvLine.IgnoreChars(_T(""));
	csvLine.ReadFieldString(strFaceNameLatin9);
	TrimStr(strFaceNameLatin9);

	// height
	csvLine.IgnoreChars(_T(" "));
	if (!csvLine.ReadFieldInt(nHeight))
		nHeight = -1;

	// font weight?
	csvLine.IgnoreChars(_T(" "));
	csvLine.ReadFieldString(strWeight);
	if (strWeight.empty())
		nWeight = -1;
	else
	{
		if (_tcsicmp(strWeight.c_str(), _T("bold")) == 0)
			nWeight = FW_BOLD;
		else if (_tcsicmp(strWeight.c_str(), _T("thin")) == 0)
			nWeight = FW_THIN;
		else
			nWeight = FW_NORMAL;
	}

	// italic
	csvLine.IgnoreChars(_T(" "));
	if (!csvLine.ReadFieldInt(byItalic))
		byItalic = -1;

	// underline
	csvLine.IgnoreChars(_T(" "));
	if (!csvLine.ReadFieldInt(nUnderLine))
		nUnderLine = -1;

	// other character set font name.
	csvLine.IgnoreChars(_T(" "));
	csvLine.ReadFieldString(strFaceNameOthers);
	TrimStr(strFaceNameOthers);

	return true;
}

void FontPropertyValueToStr(tstring &strValue, int nHeight, int nWeight, BYTE byItalic, LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers)
{
#define			SZ_FEILD_SEP	_T(", ")
	TCHAR			szBuff[64];

	strValue.resize(0);
	strValue += szFaceNameLatin9;
	strValue += SZ_FEILD_SEP;

	_itot_s(nHeight, szBuff, CountOf(szBuff), 10);
	strValue += szBuff; strValue += SZ_FEILD_SEP;

	if (nWeight >= FW_BOLD)
		strValue += _T("bold, ");
	else if (nWeight <= FW_THIN)
		strValue += _T("thin, ");
	else
		strValue += _T("normal, ");

	_itot_s(byItalic, szBuff, CountOf(szBuff), 10);
	strValue += szBuff; strValue += SZ_FEILD_SEP;

	strValue += _T("0, ");

	strValue += szFaceNameOthers;
}

tstring GetLyricsFolder()
{
	tstring strLyrDlFolder = GetAppDataDir();
	strLyrDlFolder += _T("lyrics") _T(SZ_DIR_SLASH);

	return strLyrDlFolder;
}

BOOL SetFileNoReadOnly(LPCTSTR szFile)
{
	return TRUE;
}
