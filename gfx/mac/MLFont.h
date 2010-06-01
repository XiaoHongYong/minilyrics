#ifndef _MLFONT_INC_
#define _MLFONT_INC_

#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET		1
#endif

#include "FontInfo.h"

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

protected:

};

#endif // _MLFONT_INC_
