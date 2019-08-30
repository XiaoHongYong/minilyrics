// SpamLyricsFilter.h: interface for the CSpamLyricsFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPAMLYRICSFILTER_H__25E609A1_E1CF_43FC_AAE0_0EDBC5048FAD__INCLUDED_)
#define AFX_SPAMLYRICSFILTER_H__25E609A1_E1CF_43FC_AAE0_0EDBC5048FAD__INCLUDED_

#include "../../MLLib/LrcTag.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

bool IsFakeLrc(stringex &str);

bool ConvertFakeLrcToTxt(wstringex &str);

bool ConvertFakeLrcToTxt(stringex &str);


class CSpamLyricsFilter  
{
public:
	enum
	{
		SF_ABS_SPAM			= 1,
		SF_HAS_SPAM_INFO	= 2,
		SF_FILE_SIZE		= 3,
		SF_FILE_LINES_TOO_LESS		= 4,
		SF_FILE_LINES_TOO_LONG		= 5,
		SF_FAKE_LRC			= 6,
		SF_NAME_ONE_CHAR	= 7,
		SF_TXT_TO_LRC		= 8,
	};

	CSpamLyricsFilter(LPCTSTR szCfgFilePrefix);
	virtual ~CSpamLyricsFilter();

	struct ITEMA
	{
		int		nLevel;
		string	str;
	};
	struct ITEMW
	{
		int		nLevel;
		wstring	str;
	};
	class CVFilterA : public vector<ITEMA>
	{
	public:
		void Add(LPCSTR szStr, int nLevel)
		{
			for (iterator it = begin(); it != end(); it++)
			{
				ITEMA	&item = *it;
				if (stristr(item.str.c_str(), szStr))
				{
					erase(it);
					ITEMA	x;
					x.nLevel = nLevel;
					x.str = szStr;
					push_back(x);
					return;
				}
			}
			ITEMA	x;
			x.nLevel = nLevel;
			x.str = szStr;
			push_back(x);
		}
	};

#ifdef _UNICODE
	class CVFilterW : public vector<ITEMW>
	{
	public:
		void Add(LPCWSTR szStr, int nLevel)
		{
			for (iterator it = begin(); it != end(); it++)
			{
				ITEMW	&item = *it;
				if (stristr(item.str.c_str(), szStr))
				{
					erase(it);
					ITEMW	x;
					x.nLevel = nLevel;
					x.str = szStr;
					push_back(x);
					return;
				}
			}
			ITEMW	x;
			x.nLevel = nLevel;
			x.str = szStr;
			push_back(x);
		}
	};
#endif

	bool Load(LPCTSTR dataDir);
	// bool Save();

	void SetFilterExcept(LPCTSTR szExcept);
	LPCWSTR GetFilterExcept() { return m_strFilterExceptW.c_str(); }

	bool SetNameFilter(LPCTSTR szFilter);
	LPCTSTR GetNameFilter();

	bool SetContentFilter(LPCTSTR szFilter);
	LPCTSTR GetContentFilter();

	int GetMinContentSize() { return m_nMinContentSize; }
	void SetMinContentSize(int nSize);

	bool IsNameFiltered(LPCTSTR szArtist, LPCTSTR szAlbum, LPCTSTR szTitle, LPCTSTR szFileName, int &nLevel, tstring *pstrReason);

	bool UseContentFilter() { return !m_vContentFilter.empty(); }

	bool IsContentFiltered(LYRICS_CONTENT_TYPE lyrContentType, FILE_ENCODING fileEncoding, LPCSTR szContent, int nLen, int &nLevel, tstring *pstrReason);

protected:
	tstring			strCfgFilePrefix;

	stringex		m_strFilterExcept;
	wstringex		m_strFilterExceptW;
	int				m_nMinContentLines;
	int				m_nMinContentSize;
	tstringex		m_strNameFilter, m_strContentFilter;
#ifdef _UNICODE
	CVFilterW		m_vNameFilter;
    CVFilterW        m_vContentFilterW;
#else
	CVFilterA		m_vNameFilter;
#endif
	CVFilterA		m_vContentFilter;

};

#endif // !defined(AFX_SPAMLYRICSFILTER_H__25E609A1_E1CF_43FC_AAE0_0EDBC5048FAD__INCLUDED_)
