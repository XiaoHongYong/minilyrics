#pragma once

class CLyrTimeStamp
{
public:
	static bool IsTimeStamps(LPCTSTR szTimeStamps);

	static bool Parse(LPCTSTR szTimeStamps, CMLData *pMLData);
	static void ToString(tstring &strTimeStamps, int nOffsetTime, VLyrLines &lyrLines);

};
