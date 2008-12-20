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

	// 保存为文件
	int SaveAsFile(LPCTSTR szFile, FILE_ENCODING fileEncoding);

	// 分析歌词文件，并且存储到 pMLData 中
	int Parse(bool bForceUseEncoding);

	virtual LYRICS_CONTENT_TYPE GetLyrContentType() { return LCT_UNKNOWN; }

protected:
	BOOL LyricsRowToText(PML_ROW pRow, tstringex &strBuff);

};

#endif // !defined(AFX_SNCPARSER_H__181F4326_B130_48BC_8EC8_510BD0BBD5F7__INCLUDED_)
