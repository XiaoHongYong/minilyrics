// LyricsKeywordFilter.h: interface for the CLyricsKeywordFilter class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

//
// CLyricsKeywordFilter is to filter the lyrics search keywords.
//
// Reference the "TestCLyricsKeywordFilter()" function for more.
//
class CLyricsKeywordFilter  
{
public:
	CLyricsKeywordFilter();
	virtual ~CLyricsKeywordFilter();

	static void Init(LPCTSTR dataDir = NULL);

	static void Filter(const char *szTarg, stringex &strOut);
	static void Filter(LPCWSTR szTarg, wstringex &strOut);

#define WORD_MAX		0xFFFF

protected:
	static void xTableDelChars(const char *szChars);
	static void xTableReplaceChars(const char *szReplace, const char *szTo);
	static void Ucs2TableDelChars(const WCHAR *szChars);
	static void Ucs2TableReplaceChars(const WCHAR *szReplace, const WCHAR *szTo);

protected:
	static WORD		m_wTableUcs2[WORD_MAX];

};
