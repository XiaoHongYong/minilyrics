
// section name
#define SZ_SECT_LYR_DISPLAY			_T("LyrDispaly")
#define SZ_SECT_FLOATING_LYR		_T("FloatingLyr")
#define SZ_SECT_PLAYER				_T("Player")
#define SZ_SECT_LYR_DL				_T("Download")

#ifdef _MPLAYER
#define SZ_SECT_UI			_T("ZikiPlayer")
#else
#define SZ_SECT_UI			_T("MiniLyrics")
#endif

#define SZ_SECT_LYR_DL		_T("Download")

// szValue format: Verdana, 13, thin, 1, 0, Tahoma
// Latin font, height, weight(bold,normal,thin), italic, underline, Other font
bool FontPropertyValueFromStr(LPCTSTR szValue, int &nHeight, int &nWeight, BYTE &byItalic, tstring &strFaceNameLatin9, tstring &strFaceNameOthers);
void FontPropertyValueToStr(tstring &strValue, int nHeight, int nWeight, BYTE byItalic, LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers);

void GetColorValue(MLColor &clr, LPCTSTR szColor);

void ProfileGetColorValue(COLORREF &clr, LPCTSTR szSectName, LPCTSTR szKeyName);
void ProfileGetColorValue(MLColor &clr, LPCTSTR szSectName, LPCTSTR szKeyName);

bool inline IsPropertyName(LPCTSTR szName, LPCTSTR szStr) { return _tcsicmp(szName, szStr) == 0; }

#define IsTRUE(szValue)			(_tcsicmp(_T("TRUE"), szValue) == 0 || _tcsicmp(_T("1"), szValue) == 0)

tstring GetAppDataDir();

void GetAppDataDir(TCHAR szDir[]);

tstring GetLyricsFolder();

BOOL SetFileNoReadOnly(LPCTSTR szFile);
