#ifndef _MLFONT_INC_
#define _MLFONT_INC_

#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET		1
#endif

void FormatFontName(tstring &strFontName, LPCTSTR szFaceName, int nHeight, int nWeight, int nItalic, bool bUnderline);

bool ScanFontName(LPCTSTR szFontName, tstring &strFaceName, int &nHeight, int &nWeight, int &nItalic, bool &bUnderline);

// Font slant
enum
{
	FS_NORMAL		= 0,
	FS_OBLIQUE,
	FS_ITALIC,
};

class CMLFont
{
public:
	CMLFont(void);
	virtual ~CMLFont(void);

public:
	virtual bool Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline = false);
	virtual bool CreateFrom(const CMLFont &font);

	virtual bool IsValid() { return m_pFont != NULL; }

	virtual void Destroy();

	LPCTSTR GetName() const { return m_strNameLatin9.c_str(); }
	LPCTSTR GetNameOthers() const { return m_strNameOthers.c_str(); }
	int GetHeight() const { return m_nHeight; }
	int GetWeight() const { return m_nWeight; }
	int GetItalic() const { return m_nItalic; }
	bool IsUnderline() const { return m_bUnderline; }


public:
	// for gtk2
	GdkFont *GetHandle() const { return m_pFont; }

protected:
	tstring		m_strNameLatin9, m_strNameOthers;
	int			m_nHeight;
	int			m_nWeight;
	byte		m_nItalic;
	bool		m_bUnderline;

	GdkFont		*m_pFont;

};

#endif // _MLFONT_INC_
