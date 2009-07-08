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

class CMLFont : public CFontInfo
{
public:
	CMLFont(void);
	virtual ~CMLFont(void);

public:
	virtual bool Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline = false);
	virtual bool Create(const CFontInfo &font);

	virtual bool IsValid() { return m_pFont != NULL; }

	virtual void Destroy();

public:
	// for gtk2
	GdkFont *GetHandle() const { return m_pFont; }

protected:
	GdkFont		*m_pFont;

};

#endif // _MLFONT_INC_
