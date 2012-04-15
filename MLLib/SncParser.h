// SncParser.h: interface for the CSncParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SNCPARSER_H__181F4326_B130_48BC_8EC8_510BD0BBD5F7__INCLUDED_)
#define AFX_SNCPARSER_H__181F4326_B130_48BC_8EC8_510BD0BBD5F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LyricsParser.h"

class CSncParser : public CLyricsParser  
{
public:
	CSncParser(CMLData *pMLData);
	virtual ~CSncParser();

	virtual int ParseFile(bool bUseSpecifiedEncoding, CHAR_ENCODING encoding) { return ERR_FALSE; }
	virtual int SaveAsFile(LPCTSTR szFile, CHAR_ENCODING newEncoding);

	virtual LYRICS_CONTENT_TYPE GetLyrContentType() { return LCT_UNKNOWN; }

protected:
	BOOL LyricsRowToText(LyricsLine *pRow, tstringex &strBuff);

};

#endif // !defined(AFX_SNCPARSER_H__181F4326_B130_48BC_8EC8_510BD0BBD5F7__INCLUDED_)
