
#pragma once

DECLAR_CPPUNIT_TEST_REG(HelperFun)

void AnalysisLyricsFileNameEx(tstring &strArtist, tstring &strTitle, LPCTSTR szFile);

/**
* Format media title in format of "artist - title"
**/
tstring FormatMediaTitle(LPCTSTR szArtist, LPCTSTR szTitle);

#ifdef WIN32
bool GetMLEncriptyData(stringex &strData);
#endif
