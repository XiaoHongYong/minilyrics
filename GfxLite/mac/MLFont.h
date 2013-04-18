#ifndef _MLFONT_INC_
#define _MLFONT_INC_

#pragma once

#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET		1
#endif

typedef const struct __CTFont * CTFontRef;

class CMLFont : public CFontInfo
{
public:
	CMLFont(void);
	virtual ~CMLFont(void);

public:
	virtual bool Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nSize, int nWeight, int nItalic, bool bUnderline = false);
	virtual bool Create(const CFontInfo &font);

	virtual bool IsValid() { return true; }

	virtual void Destroy();

	// For MAC only.
	CTFontRef GetHandle() const { return m_font; }

protected:
	CTFontRef		m_font;

};

#endif // _MLFONT_INC_
