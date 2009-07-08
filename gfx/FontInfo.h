#ifndef _FONT_INFO_INC_
#define _FONT_INFO_INC_

#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET		1
#endif

// Font slant
enum
{
	FS_NORMAL		= 0,
	FS_OBLIQUE,
	FS_ITALIC,
};

class CFontInfo
{
public:
	CFontInfo()
	{
		m_nHeight = 0;
		m_nWeight = FW_NORMAL;
		m_nItalic = 0;
		m_bUnderline = false;
	}
	virtual ~CFontInfo()
	{
	}

public:
	virtual void Set(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline = false)
	{
		m_strNameLatin9 = szFaceNameLatin9;
		if (!szFaceNameOthers || IsEmptyString(szFaceNameOthers))
			m_strNameOthers = szFaceNameLatin9;
		else
			m_strNameOthers = szFaceNameOthers;

		m_nHeight = nHeight;
		m_nWeight = nWeight;
		m_nItalic = nItalic;
		m_bUnderline = bUnderline;
	}

	virtual void Set(const CFontInfo &font)
	{
		m_strNameLatin9 = font.m_strNameLatin9;
		m_strNameOthers = font.m_strNameOthers;
		m_nHeight = font.m_nHeight;
		m_nWeight = font.m_nWeight;
		m_nItalic = font.m_nItalic;
		m_bUnderline = font.m_bUnderline;
	}

	bool IsSame(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline)
	{
		if (_tcscmp(szFaceNameLatin9, m_strNameLatin9.c_str()) != 0)
			return false;

		if (!szFaceNameOthers)
			szFaceNameOthers = szFaceNameLatin9;
		if (_tcscmp(szFaceNameOthers, m_strNameOthers.c_str()) != 0)
			return false;

		return nHeight == m_nHeight && nWeight == m_nWeight && m_nItalic == nItalic && bUnderline == m_bUnderline;
	}

	bool IsSame(const CFontInfo &font)
	{
		return font.m_strNameLatin9 == m_strNameLatin9
			&& font.m_strNameOthers == m_strNameOthers
			&& font.m_nHeight == m_nHeight
			&& font.m_nWeight == m_nWeight
			&& font.m_nItalic == m_nItalic
			&& font.m_bUnderline == m_bUnderline;
	}

	bool IsValid() { return !m_strNameLatin9.empty(); }

	LPCTSTR GetName() const { return m_strNameLatin9.c_str(); }
	LPCTSTR GetNameOthers() const { return m_strNameOthers.c_str(); }
	int GetHeight() const { return m_nHeight; }
	int GetWeight() const { return m_nWeight; }
	int GetItalic() const { return m_nItalic; }
	bool IsUnderline() const { return m_bUnderline; }

public:
	tstring		m_strNameLatin9, m_strNameOthers;
	int			m_nHeight;
	int			m_nWeight;
	byte		m_nItalic;
	bool		m_bUnderline;

};

#endif // _FONT_INFO_INC_
