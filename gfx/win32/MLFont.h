#ifndef _MLFONT_INC_
#define _MLFONT_INC_

#pragma once

#include "UnicodeRange.h"
#include "FontInfo.h"

class CMLFont : public CFontInfo
{
public:
	CMLFont(void);
	virtual ~CMLFont(void);

public:
	virtual bool Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline = false);
	virtual bool Create(const CFontInfo &font);

	virtual bool IsValid();

	virtual void Destroy() { }
public:

};

extern BYTE		g_byFontQuality;

template<class _Font>
class CFontCollection
{
public:
	typedef	map<byte, _Font*>	MAP_FONT;

	CFontCollection()
		//: m_logFont(25, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, g_byFontQuality, DEFAULT_PITCH | FF_ROMAN, "Tahoma")
	{
	#ifdef _WINCE_
		LOGFONT lgfont = {25, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, g_byFontQuality, DEFAULT_PITCH | FF_ROMAN, _T("Tahoma")};
	#else
		LOGFONTA lgfont = {25, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, g_byFontQuality, DEFAULT_PITCH | FF_ROMAN, "Tahoma"};
	#endif

		m_logFontLatin9 = lgfont;
		m_logFontOthers = lgfont;
	}

	~CFontCollection()
	{
		Destroy();
	}

	void SetFont(CMLFont &font)
	{
		if (m_logFontLatin9.lfHeight == font.GetHeight() &&
			m_logFontLatin9.lfWeight == font.GetWeight() &&
			m_logFontLatin9.lfItalic == (BYTE)font.GetItalic() &&
			m_logFontLatin9.lfUnderline == (BYTE)font.IsUnderline())
		{
#ifdef _WINCE_
			if (_tcscmp(font.GetName(), m_logFontLatin9.lfFaceName) == 0
				&& _tcscmp(font.GetNameOthers(), m_logFontOthers.lfFaceName) == 0)
				return;
#else
			CHAR szLatin[LF_FACESIZE], szOthers[LF_FACESIZE];
			ConvertStr2(font.GetName(), -1, szLatin, CountOf(szLatin));
			ConvertStr2(font.GetNameOthers(), -1, szOthers, CountOf(szOthers));
			if (strcmp(szLatin, m_logFontLatin9.lfFaceName) == 0
				&& strcmp(szOthers, m_logFontOthers.lfFaceName) == 0)
				return;
#endif
		}

		Destroy();

		m_logFontOthers.lfHeight = m_logFontLatin9.lfHeight = font.GetHeight();
		m_logFontOthers.lfWeight = m_logFontLatin9.lfWeight = font.GetWeight();
		m_logFontOthers.lfItalic = m_logFontLatin9.lfItalic = font.GetItalic();
		m_logFontOthers.lfUnderline = m_logFontLatin9.lfUnderline = font.IsUnderline();
#ifdef _WINCE_
		if (IsEmptyString(font.GetName()))
			_tcscpy(m_logFontLatin9.lfFaceName, _T("Tahoma"));
		else
			_tcscpy_safe(m_logFontLatin9.lfFaceName, CountOf(m_logFontLatin9.lfFaceName), font.GetName());

		if (IsEmptyString(font.GetNameOthers()))
			_tcscpy(m_logFontOthers.lfFaceName, _T("Tahoma"));
		else
			_tcscpy_safe(m_logFontOthers.lfFaceName, CountOf(m_logFontOthers.lfFaceName), font.GetNameOthers());
#else
		if (IsEmptyString(font.GetName()))
			strcpy_safe(m_logFontLatin9.lfFaceName, CountOf(m_logFontLatin9.lfFaceName), "Tahoma");
		else
			ConvertStr2(font.GetName(), -1, m_logFontLatin9.lfFaceName, CountOf(m_logFontLatin9.lfFaceName));

		if (IsEmptyString(font.GetNameOthers()))
			strcpy_safe(m_logFontOthers.lfFaceName, CountOf(m_logFontOthers.lfFaceName), "Tahoma");
		else
			ConvertStr2(font.GetNameOthers(), -1, m_logFontOthers.lfFaceName, CountOf(m_logFontOthers.lfFaceName));
#endif
	}

	void Destroy()
	{
		if (m_mapFonts.empty())
			return;

		for (MAP_FONT::iterator it = m_mapFonts.begin(); it != m_mapFonts.end(); it++)
		{
			delete ((*it).second);
		}
		m_mapFonts.clear();
	}

	int GetFontHeight() const { return m_logFontLatin9.lfHeight; }

	template<class _Graphics, class _Worker>
	void ResovleText(_Graphics *pGraph, LPCTSTR szText, int nLen, _Worker &fun)
	{
		LPCTSTR		szBeg, szEnd;
		int			nClip;
		BYTE		lfCharSetAbove = DEFAULT_CHARSET, lfCharSet = DEFAULT_CHARSET;
		CHAR_ENCODING	encoding;

		if (nLen == -1)
			nLen = _tcslen(szText);

		szEnd = szText + nLen;

		while (szText < szEnd)
		{
			szBeg = szText;
			nClip = 0;
			while (szText < szEnd)
			{
				if (*szText != ' ')
				{
					encoding = FindCharEncUnicodeRange(*szText);

					lfCharSet = GetCharEncodingByID(encoding).nFontCharset;
					if (lfCharSet != lfCharSetAbove && nClip > 0)
						break;
					lfCharSetAbove = lfCharSet;
				}

				nClip++;
				szText++;
			}
			if (nClip)
			{
				fun(GetFont(pGraph, lfCharSetAbove), szBeg, nClip);
			}
		}
	}

	template<class _Graphics>
	_Font *GetFont(_Graphics *pGraph, BYTE byCharset)
	{
		MAP_FONT::iterator	it;
		_Font		*pFont;
		it = m_mapFonts.find(byCharset);
		if (it == m_mapFonts.end())
		{
			pFont = new _Font;
			if (byCharset == DEFAULT_CHARSET)
				pFont->Create(pGraph, m_logFontLatin9);
			else
				pFont->Create(pGraph, m_logFontOthers);
			m_mapFonts[byCharset] = pFont;
		}
		else
			pFont = (*it).second;

		return pFont;
	}

protected:
#ifdef _WINCE_
	LOGFONT			m_logFontLatin9;
	LOGFONT			m_logFontOthers;
#else
	LOGFONTA		m_logFontLatin9;
	LOGFONTA		m_logFontOthers;
#endif
	MAP_FONT		m_mapFonts;

};


#endif // _MLFONT_INC_
