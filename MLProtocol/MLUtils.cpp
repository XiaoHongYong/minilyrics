#include "stdafx.h"
#include "MLUtils.h"


MLFileType MLGetFileType(LPCTSTR szFile)
{
	LPCTSTR		szExt;

	szExt = _tcsrchr(szFile, '.');
	if (szExt == NULL)
		return FT_UNKNOWN;

	if (_tcsicmp(szExt, _T(".lrc")) == 0)
		return FT_LYRICS_LRC;
	else if (_tcsicmp(szExt, _T(".txt")) == 0)
		return FT_LYRICS_TXT;
	else if (_tcsicmp(szExt, _T(".snc")) == 0)
		return FT_LYRICS_SNC;
	else if (_tcsicmp(szExt, _T(".srt")) == 0)
		return FT_SUBTITLE_SRT;

	return FT_UNKNOWN;
}
