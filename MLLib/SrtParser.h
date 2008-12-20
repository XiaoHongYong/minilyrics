// SrtParser.h: interface for the CSrtParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SRTPARSER_H__C4C41834_C58B_4D99_BD54_B42E3F58DB90__INCLUDED_)
#define AFX_SRTPARSER_H__C4C41834_C58B_4D99_BD54_B42E3F58DB90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LyricsParser.h"

class CSrtParser : public CLyricsParser  
{
public:
	CSrtParser(CMLData *pMLData);
	virtual ~CSrtParser();

public:
	// 保存为文件
	virtual int SaveAsFile(LPCTSTR szFile, FILE_ENCODING fileEncoding);

	// 分析歌词文件，并且存储到 pMLData 中
	virtual int Parse(bool bForceUseEncoding);

	virtual LYRICS_CONTENT_TYPE GetLyrContentType();

protected:
	BOOL LyricsRowToText(PML_ROW pRow, tstringex &strBuff);
	
};

#endif // !defined(AFX_SRTPARSER_H__C4C41834_C58B_4D99_BD54_B42E3F58DB90__INCLUDED_)
