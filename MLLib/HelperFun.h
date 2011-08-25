
#pragma once

void FilterSongFileExt(LPTSTR szFile);

BOOL AnalysisLyricsFileNameEx(tstring &strArtist, tstring &strTitle, LPCTSTR szFileName);

#ifdef WIN32
bool GetMLEncriptyData(stringex &strData);
#endif
