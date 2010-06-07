#ifndef _MLFONT_INC_
#define _MLFONT_INC_

#pragma once

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CGFont.h>

#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET		1
#endif

class CMLFont : public CFontInfo
{
public:
	CMLFont(void);
	virtual ~CMLFont(void);

public:
	virtual bool Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline = false);
	virtual bool Create(const CFontInfo &font);

	virtual bool IsValid() { return true; }

	virtual void Destroy();

	// For MAC only.
	CGFontRef GetHandle() const { return m_font; }

protected:
	CGFontRef		m_font;

};

#endif // _MLFONT_INC_
