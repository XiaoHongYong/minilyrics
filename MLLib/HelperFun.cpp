#include "StdAfx.h"
#include "HelperFun.h"
#ifdef WIN32
#ifdef _WINCE_
#define _mbschr		strchr
#else
#include "mbstring.h"
#endif
#endif
#include "LrcTag.h"

//////////////////////////////////////////////////////////////////////////
//

const TCHAR *_SZ_SONG_EXT[] = {
	_T("mp3"), _T("669"), _T("aac"), _T("amf"), _T("aif"), _T("aiff"), _T("ape"), _T("avi"), _T("asf"), _T("au"), _T("cda"), 
	_T("far"), _T("it"), _T("kar"), _T("m2v"), _T("m3u"), _T("m4a"), _T("mac"), _T("mdz"), _T("mid"), _T("midi"), 
	_T("miz"), _T("mod"), _T("mp2"), _T("mp1"), _T("mpg"), _T("mp4"), _T("mpeg"), _T("mtm"), 
	_T("nst"), _T("nsa"), _T("nsv"), _T("ogg"), _T("okt"), _T("pls"), _T("ptm"), _T("rmi"), _T("s3m"), 
	_T("s3z"), _T("snd"), _T("stm"), _T("stz"), _T("voc"), _T("wav"), _T("wma"), _T("wmv"), _T("xm"), 
	_T("xmz"), _T("mkv"), _T("mka"), _T("ult"), _T("lrc"), _T("txt"), NULL
};

void FilterSongFileExt(LPTSTR szFile)
{
	LPTSTR	szDot;
	LPCTSTR	*szPos;

	szDot = _tcsrchr(szFile, '.');
	if (szDot)
	{
		szDot++;
		for (szPos = _SZ_SONG_EXT; *szPos; szPos++)
		{
			if (_tcsicmp(szDot, *szPos) == 0)
			{
				szDot--;
				szDot[0] = '\0';
				return;
			}
		}
	}
}

BOOL AnalysisLyricsFileNameEx(tstring &strArtist, tstring &strTitle, LPCTSTR szFileName)
{
	LPCTSTR		szEndArtist;
	LPCTSTR		szBegTitle;
	TCHAR		szFullTitleNew[MAX_PATH];

	strArtist.resize(0);
	strTitle.resize(0);

	//
	// 将FullTitle中的.mp3, .ra, .rm ....等替换掉
	_tcscpy_safe(szFullTitleNew, MAX_PATH, szFileName);
	FilterSongFileExt(szFullTitleNew);

	// 作者和标题之间是以：' - ' or ' － ' or '-'为分隔
	szEndArtist = szBegTitle = _tcsstr(szFullTitleNew, _T(" - "));
	if (szBegTitle)
		szBegTitle += 3;	// strlen(_T(" - "));
	else
	{
		szEndArtist = szBegTitle = _tcschr(szFullTitleNew, '-');
		if (szBegTitle)
			szBegTitle++;	// -
	}


	//
	// 处理作者……
	if (szEndArtist == NULL)
	{
		//
		// 认为 szFullTitle 都是 Title, Artist 为空
		szBegTitle = szFullTitleNew;
	}
	else
	{
		strArtist.append(szFullTitleNew, int(szEndArtist - szFullTitleNew));
		TrimStr(strArtist);
	}

	strTitle = szBegTitle;
	TrimStr(strTitle);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//

void AnalysisProxySetting(LPCTSTR szProxySetting, TCHAR szServer[], int nMaxSize, int &nPort)
{
	LPCTSTR		szColon;

	szColon = _tcschr(szProxySetting, ':');
	if (szColon == NULL)
	{
		_tcscpy_safe(szServer, nMaxSize, szProxySetting);
		nPort = 80;
	}
	else
	{
		_tcsncpy_safe(szServer, nMaxSize, szProxySetting,
			int(szColon - szProxySetting));
		nPort = _ttoi(szColon + 1);
	}
}

#ifdef WIN32
bool GetMLEncriptyData(stringex &strData)
{
	TCHAR			szFile[MAX_PATH];
	stringex		str;
	TCHAR			szMD5[64];

	GetModuleFileName(GetAppInstance(), szFile, CountOf(szFile));

	if (!OpenFileToBuffer(szFile, str))
		return false;

	MD5Str(str.c_str() + 0x100, str.size() - 0x100, szMD5);

	// 0x51 ~ 0x78
	// decodebase64(str.c_str() + 0x51, 0x78 - 0x51, strData);
	strData.append(str.c_str() + 0x51, 0x78 - 0x51);

	for (uint i = 0; i < 32; i++)
		strData[i] = strData[i] ^ szMD5[i];

	// 解密之后的数据了，这里存储的是核心数据

	return true;
}
#endif
