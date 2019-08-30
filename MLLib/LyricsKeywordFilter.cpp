// LyricsKeywordFilter.cpp: implementation of the CLyricsKeywordFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "LyricsKeywordFilter.h"

bool IsStrNumeric(LPCWSTR szString);

WORD		CLyricsKeywordFilter::m_wTableUcs2[WORD_MAX];

void EraseNumb(wstringex &str)
{
	WCHAR	*szPtr, *szNew;
	szNew = szPtr = str.data();
	while (*szPtr)
	{
		if (!IsDigit(*szPtr))
		{
			*szNew = *szPtr;
			szNew++;
		}
		szPtr++;
	}
	*szNew = '\0';
	str.resize(wcslen(str.c_str()));
}

void EraseSpace(wstringex &str)
{
	strdel(str.data(), ' ');
	str.resize(wcslen(str.c_str()));
}

void ToLower(stringex &str)
{
	char	*szPtr, *szNew;
	szNew = szPtr = str.data();
	while (*szPtr)
	{
        if ( (*szPtr >= 'A') && (*szPtr <= 'Z') )
            *szPtr = *szPtr + ('a' - 'A');
		szPtr++;
	}
}

//////////////////////////////////////////////////////////////////////

CLyricsKeywordFilter::CLyricsKeywordFilter()
{
}

CLyricsKeywordFilter::~CLyricsKeywordFilter()
{

}

void CLyricsKeywordFilter::Init(LPCTSTR dataDir)
{
	for (int i = 0; i < WORD_MAX; i++)
	{
		m_wTableUcs2[i] = (WORD)i;
	}

	CSimpleXML		xml;
	tstring			strKeywordFilter;

    if (dataDir) {
        strKeywordFilter = dataDir;
    } else {
        strKeywordFilter = GetAppResourceDir();
    }

	strKeywordFilter += _T("LyrKeywordFilter.xml");
	if (xml.ParseFile(strKeywordFilter.c_str()))
	{
		ASSERT(_tcscmp(xml.m_pRoot->strName.c_str(), _T("KeywordFilter")) == 0);
		for (SXNode::iterator it = xml.m_pRoot->listChildren.begin(); 
			it != xml.m_pRoot->listChildren.end(); ++it)
		{
			SXNode	*pNode = *it;

			if (_tcscmp(pNode->strName.c_str(), _T("Remove")) == 0)
			{
				// remove
#ifdef UNICODE
				Ucs2TableDelChars(pNode->strContent.c_str());
#else
				wstringex	strW;

				Utf8ToUCS2(pNode->strContent.c_str(), pNode->strContent.size(), strW);
				Ucs2TableDelChars(strW.c_str());
#endif
			}
			else if (_tcscmp(pNode->strName.c_str(), _T("Replace")) == 0)
			{
#ifdef UNICODE
				Ucs2TableReplaceChars(pNode->GetPropertySafe(_T("src")), pNode->GetPropertySafe(_T("dst")));
#else
				// replace
				wstringex	strWSrc, strWDst;

				Utf8ToUCS2(pNode->GetPropertySafe(_T("src")), -1, strWSrc);
				Utf8ToUCS2(pNode->GetPropertySafe(_T("dst")), -1, strWDst);
				Ucs2TableReplaceChars(strWSrc.c_str(), strWDst.c_str());
#endif
			}
		}
	}
	else
	{
		ERRLOG1(_T("Failed to parse lyrics keyword filter: %s"), strKeywordFilter.c_str());
	}

	xTableDelChars(" !\"#$%&'*+,-./:;<=>?@\\^_`|~");
	xTableReplaceChars("[]{}", "()()");
	xTableReplaceChars("ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		"abcdefghijklmnopqrstuvwxyz");
//
//	TestCLyricsKeywordFilter();
//	TestCLyricsKeywordFilterUtf8();
}

void CLyricsKeywordFilter::xTableDelChars(const char *szChars)
{
	wstringex	strOut;
	Utf8ToUCS2(szChars, -1, strOut);
	Ucs2TableDelChars(strOut.c_str());
}

void CLyricsKeywordFilter::xTableReplaceChars(const char *szReplace, const char *szTo)
{
	wstringex	strReplaceW, strToW;
	Utf8ToUCS2(szReplace, -1, strReplaceW);
	Utf8ToUCS2(szTo, -1, strToW);
	Ucs2TableReplaceChars(strReplaceW.c_str(), strToW.c_str());
}

void CLyricsKeywordFilter::Ucs2TableDelChars(const WCHAR *szChars)
{
	while (*szChars)
	{
		m_wTableUcs2[*szChars] = 0;
		szChars++;
	}
}

void CLyricsKeywordFilter::Ucs2TableReplaceChars(const WCHAR *szReplace, const WCHAR *szTo)
{
	if (wcslen(szTo) == 1)
	{
		WCHAR ch = szTo[0];
		while (*szReplace)
		{
			m_wTableUcs2[*szReplace] = ch;
			szReplace++;
		}
		return;
	}

	while (*szReplace && *szTo)
	{
		m_wTableUcs2[*szReplace] = *szTo;
		szReplace++;
		szTo++;
	}
}

void CLyricsKeywordFilter::Filter(const char *szTarg, stringex &strOut)
{
	wstringex		strTargUcs2, strOutUcs2;

	Utf8ToUCS2(szTarg, -1, strTargUcs2);

	Filter(strTargUcs2.c_str(), strOutUcs2);

	UCS2ToUtf8(strOutUcs2.c_str(), strOutUcs2.size(), strOut);
}

void CLyricsKeywordFilter::Filter(LPCWSTR szInput, wstringex &strOut)
{
	if (m_wTableUcs2['a'] == 0)
	{
		// Isn't initialized?
		ASSERT(m_wTableUcs2['a'] != 0);
	}

	strOut.clear();

	int nCommentDeep = 0;

	LPCWSTR p = szInput;
	while (*p)
	{
		WORD	wTo;

		wTo = m_wTableUcs2[*p];
		p++;

		// target?
		// comment: between ( and ) is comment. 
		if (wTo != 0)
		{
			if (nCommentDeep == 0)
			{
				if (wTo == '(')
				{
					nCommentDeep++;
				}
				else if (wTo != ')')
					strOut += wTo;
			}
			else
			{
				if (wTo == ')')
					nCommentDeep--;
				else if (wTo == '(')
					nCommentDeep++;
			}
		}
	}

	if (!IsStrNumeric(strOut.c_str()))
		EraseNumb(strOut);

	if (strOut.empty())
	{
		strOut = szInput;
		EraseSpace(strOut);
	}
}
