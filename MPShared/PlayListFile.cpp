// PlayList.cpp: implementation of the CPlayList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Player.h"
#include "PlayListFile.h"

void RemoveReturn(LPSTR str)
{
	LPSTR p = str + strlen(str) - 1;

	while (p >= str && (*p == ' ' || *p == '\r' || *p == '\n'))
	{
		*p = '\0';
		p--;
	}
}

bool LoadPlaylist(IMPlayer *player, IPlaylist *playList, LPCTSTR szFile)
{
	ASSERT(player);
	ASSERT(playList);

	if (FileIsExtSame(szFile, _T(".m3u")))
		return LoadM3uPlaylist(player, playList, szFile);
	else if (FileIsExtSame(szFile, _T(".pls")))
		return LoadPlsPlaylist(player, playList, szFile);
	else if (FileIsExtSame(szFile, _T(".wpl")))
		return LoadWplPlaylist(player, playList, szFile);
	else
		return false;
}

#define EXTINF	"#EXTINF"
#define EXTM3U	"#EXTM3U"
#define WPL_HEADER	"<?wpl version=\"1.0\"?>"
#define XML_HEADER	"<?xml version=\"1.0\" encoding='UTF-8'?>"

// three format of szFileName:
// G:\mp3\Britney Spears04\02.(You Drive Me ) Crazy.MP3
// \mp3\Britney Spears04\02.(You Drive Me ) Crazy.MP3
// 02.(You Drive Me ) Crazy.MP3
bool AppendMediaToList(IMPlayer *player, IPlaylist *playList, LPCTSTR szFileName, LPCTSTR szBasePath)
{
	MLRESULT	nRet;
	CMPAutoPtr<IMedia>	media;
	tstringex	strFile;

	if (szBasePath[1] == ':')
	{
		if (szFileName[0] == '\\')
		{
			strFile.append(szBasePath, 2);
			strFile += szFileName;
		}
		else if (szFileName[1] == ':')
			strFile = szFileName;
		else
		{
			strFile = szBasePath;
			strFile += szFileName;
		}
	}
	else if (szBasePath[0] == '\\')
	{
		if (szFileName[0] == '\\')
			strFile = szFileName;
		else
		{
			strFile = szBasePath;
			strFile += szFileName;
		}
	}
	else
		strFile = szFileName;

	if (player->NewMedia(&media, strFile.c_str()) == ERR_OK)
	{
		nRet = playList->InsertItem(-1, media);
		if (nRet != ERR_OK)
		{
			ERRLOG0(_T("playList->InsertItem FAILED!"));
			return false;
		}
	}
	else
	{
		ERRLOG0(_T("playList->NewMedia FAILED!"));
		return false;
	}

	return true;
};

bool SavePlaylistAsM3u(IPlaylist	*playList, LPCTSTR szFile)
{
	// M3U 
	// #EXTM3U
	// #EXTINF:211,Britney Spears - ...Baby One More Time
	// G:\mp3\Britney Spears04\01.Baby One More Time.MP3
	// ...

	FILE*		fp;
	long		nCount;
	MLRESULT	nRet;

	nCount = playList->GetCount();

	fp = _tfopen(szFile, _T("w"));
	if (!fp)
		return false;

	// header tag
	fprintf(fp, "#EXTM3U\n");

	// 
	for (long i = 0; i < nCount; i++)
	{
		CMPAutoPtr<IMedia>	media;
		nRet = playList->GetItem(i, &media);
		if (nRet == ERR_OK)
		{
			CXStr	strUrl;
			tstring	strFullTitle;
			int		nDuration = -1;

			nDuration = media->GetDuration();

			g_Player.FormatMediaTitle(media, strFullTitle);

			nRet = media->GetSourceUrl(&strUrl);
			if (nRet == ERR_OK)
			{
				stringex		str;

				// #EXTINF:211,Britney Spears - ...Baby One More Time
				// G:\mp3\Britney Spears04\01.Baby One More Time.MP3
				ConvertStr(strFullTitle.c_str(), strFullTitle.size(), str);
				if (nDuration == MEDIA_LENGTH_INVALID)
					fprintf(fp, "#EXTINF:%s\n", str.c_str());
				else
					fprintf(fp, "#EXTINF:%d,%s\n", nDuration, str.c_str());

				ConvertStr(strUrl.c_str(), strUrl.size(), str);
				fprintf(fp, "%s", str.c_str());
				fprintf(fp, "\n");
			}
		}
	}

	fclose(fp);

	return true;
}

bool LoadM3uPlaylist(IMPlayer *player, IPlaylist *playList, LPCTSTR szFile)
{
	char		szBuff[MAX_PATH * 2];
	FILE		*fp;
	TCHAR		szPath[MAX_PATH];
	tstringex	strFile;
	tstringex	str;

	fp = _tfopen(szFile, _T("r"));
	if (!fp)
		return false;

	FileGetPath(szPath, CountOf(szPath), szFile);

	// #EXTM3U
	if (!fgets(szBuff, CountOf(szBuff), fp))
		return false;
	if (_strnicmp(szBuff, EXTM3U, strlen(EXTM3U)) != 0)
		return false;

	while (fgets(szBuff, CountOf(szBuff), fp))
	{
		RemoveReturn(szBuff);
		if (szBuff[0] == '#')
		{
// 			if (strncmp(szBuff, EXTINF, strlen(EXTINF)) == 0)
// 			{
// 				LPSTR		p;
// 				p = strchr(szBuff, ',');
// 				if (p)
// 				{
// 					p++;
// 					while (*p == ' ')
// 						p++;
// 					if (!IsEmptyString(p))
// 					{
// 						// p is title now
// 					}
// 				}
// 			}
			continue;
		}

		if (IsEmptyString(szBuff))
			continue;

		ConvertStr(szBuff, -1, str);

		AppendMediaToList(player, playList, str.c_str(), szPath);
	}
	fclose(fp);

	return true;
}

bool LoadPlsPlaylist(IMPlayer *player, IPlaylist *playList, LPCTSTR szFile)
{
	CProfile		file;
	int				n;
	TCHAR			szkey[256];
	TCHAR			szPath[MAX_PATH];

	FileGetPath(szPath, CountOf(szPath), szFile);

	file.Init(szFile, _T("playlist"));
	n = file.GetProfileInt(_T("NumberOfEntries"), 0);

	for (int i = 1; i < n; i++)
	{
		_stprintf(szkey, _T("File%d"), i);

		tstring		strFile;
		strFile = file.GetProfileString(szkey, _T(""));
		if (strFile.empty())
			continue;

		AppendMediaToList(player, playList, strFile.c_str(), szPath);
	}

	return true;
}

bool LoadWplPlaylist(IMPlayer *player, IPlaylist	*playList, LPCTSTR szFile)
{
	CCharBuffer		buffer;
	TCHAR			szPath[MAX_PATH];

	FileGetPath(szPath, CountOf(szPath), szFile);

	if (!OpenFileToBuffer(szFile, buffer))
		return false;

	if (strncmp(buffer.c_str(), WPL_HEADER, strlen(WPL_HEADER)) != 0)
	{
		ERRLOG1(_T("Unknown wmp file format(header): %s"), szFile);
		return false;
	}

	buffer.replace(0, strlen(WPL_HEADER), XML_HEADER, strlen(XML_HEADER));

	CSimpleXML		xml;

	if (!xml.ParseData(buffer.data(), buffer.size()))
	{
		ERRLOG1(_T("Unknown wmp file format(parse): %s"), szFile);
		return false;
	}
	// 	<smil>
	// 		<body>
	// 			<seq>
	// 				<media src="G:\mp3\Britney Spears04\02.(You Drive Me ) Crazy.MP3"/>
	// 			</seq>
	// 		</body>
	// 	</smil>

	SXNode		*pNode;
	pNode = xml.m_pRoot;
	if (_tcscmp(pNode->strName.c_str(), _T("smil")) != 0)
	{
		ERRLOG1(_T("Unknown wmp file format(field: smil): %s"), szFile);
		return false;
	}

	pNode = pNode->GetChild(_T("body"));
	if (!pNode)
	{
		ERRLOG1(_T("Unknown wmp file format(field: body): %s"), szFile);
		return false;
	}

	pNode = pNode->GetChild(_T("seq"));
	if (!pNode)
	{
		ERRLOG1(_T("Unknown wmp file format(field: seq): %s"), szFile);
		return false;
	}

	SXNode::iterator it = pNode->listChildren.begin();
	for (; it != pNode->listChildren.end(); ++it)
	{
		SXNode		*pNodeMedia = *it;
		if (_tcscmp(pNodeMedia->strName.c_str(), _T("media")) == 0)
		{
			LPCTSTR	szFile = pNodeMedia->GetPropertySafe(_T("src"));
			if (szFile && !IsEmptyString(szFile))
				AppendMediaToList(player, playList, szFile, szPath);
		}
	}

	return true;
}

