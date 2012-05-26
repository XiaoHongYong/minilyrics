#include <sys/time.h>
#include "../base/base.h"
#include "MLService.h"

CMLService	g_MLService;


void Log(LPCTSTR szFormat, ...)
{
	static int _fd = -1;
	if (_fd == -1)
	{
		LPCTSTR SZ_PIPE_NAME = "/tmp/debug.pipe";
		int nRet = mkfifo(SZ_PIPE_NAME, S_IRWXO | S_IRWXG | S_IRWXU);
		if (nRet != 0 && errno != EEXIST)
			return;
		
		_fd = ::open(SZ_PIPE_NAME, O_NONBLOCK | O_WRONLY);
		if (_fd == -1)
			return;
	}
	
	va_list		args;
	va_start(args, szFormat);
	TCHAR buf[1024];
	int nBuf = _vstprintf_s(buf, CountOf(buf), szFormat, args);
	va_end(args);
	
	if (nBuf > 0) {
		::write(_fd, buf, nBuf);
		::write(_fd, "\n", 1);
	}
}


// Launch "MiniLyrics.exe /Pipe pipe_name /Player "iTunes"" to start MiniLyrics
BOOL RunMiniLyrics(LPCSTR pipeName, CPlayerServiceBase *pPlayerService, bool bManual)
{
	g_MLService.Init();

	char	szMiniLyricsExe[MAX_PATH];
	strcpy(szMiniLyricsExe, "/Users/showbrave/Library/Developer/Xcode/DerivedData/MiniLyricsMac-agclrmbubvezruavpgqfpggqmabs/Build/Products/Debug/MiniLyricsMac.app");

	pPlayerService->Create(pipeName);
	pPlayerService->OnConnected();

	// char	szCmdLine[MAX_PATH];
	// sprintf_s(szCmdLine, CountOf(szCmdLine), "%s /Pipe %s", szMiniLyricsExe, pipeName);
	
	pid_t child = fork();
	if (child == 0)
	{
		int ret = execl("/usr/bin/open", "-a", szMiniLyricsExe, "--args", "/Pipe", pipeName, NULL);
		if (ret == 0)
			Log("Succeeded to start MiniLyrics.");
		else
			Log("Failed to start MiniLyrics: %s", strerror(errno));
	}
	else if (child == -1)
		return false;
	else {
		int status = 0;
		waitpid(child, &status, WNOHANG);
	}

	return true;
}

void DisconnectMiniLyrics(CPlayerServiceBase *pPlayerService)
{
	pPlayerService->Destroy();
	
	if (g_MLService.IsValid())
		g_MLService.OnDisconnected();
}

inline void appendMediaInfoStr(LPSTR szInfo, size_t &offset, WORD &offsetTag, LPCSTR tag, size_t lenTag)
{
	offsetTag = offset;
	memcpy(szInfo + offset, tag, (lenTag + 1) * sizeof(char));
	offset += lenTag + 1;
}

inline MLMediaInfo *NewMLMediaInfo(LPCSTR artist, LPCSTR album, LPCSTR title, LPCSTR mediaUrl, unsigned long mediaLength, WORD hasVideo = R_UNDETERMINED)
{
	size_t	lenArtist = strlen(artist);
	size_t	lenAlbum = strlen(album);
	size_t	lenTitle = strlen(title);
	size_t	lenMediaUrl = strlen(mediaUrl);

	// 8 is the extra characters '\0' of artist, album, title, mediaUrl.
	DWORD	sizeInfo = sizeof(MLMediaInfo) + (8 + lenArtist + lenAlbum + lenTitle + lenMediaUrl) * sizeof(char);
	MLMediaInfo *info = (MLMediaInfo *)new char[sizeInfo];

	info->size = sizeInfo;
	info->mediaLength = mediaLength;
	info->hasVideo = hasVideo;

	size_t offset = 0;
	appendMediaInfoStr(info->mediaInfo, offset, info->offsetArtist, artist, lenArtist);
	appendMediaInfoStr(info->mediaInfo, offset, info->offsetAlbum, album, lenAlbum);
	appendMediaInfoStr(info->mediaInfo, offset, info->offsetTitle, title, lenTitle);
	appendMediaInfoStr(info->mediaInfo, offset, info->offsetMediaUrl, mediaUrl, lenMediaUrl);
	ASSERT(offset * sizeof(char) + sizeof(MLMediaInfo) < sizeInfo);

	return info;
}

bool CMLService::Init()
{
	if (m_msgPipeMiniLyrics.Open(SZ_PIPE_MINILYRICS_NAME, false))
		return true;

	::Log("Failed to open MiniLyrics pipe: %s", SZ_PIPE_MINILYRICS_NAME);
	return false;
}

void CMLService::OnMediaInfoChanged(LPCSTR szMediaUrl, LPCSTR szArtist, LPCSTR szAlbum, LPCSTR szTitle, unsigned long nMediaLength, int hasVideo)
{
	if (szMediaUrl == NULL)
		szMediaUrl = "";
	if (szArtist == NULL)
		szArtist = "";
	if (szAlbum == NULL)
		szAlbum = "";
	if (szTitle == NULL)
		szTitle = "";

	::Log("OnMediaInfoChanged, url: %s, %s, %s, %s", szMediaUrl, szArtist, szAlbum, szTitle);

	MLMediaInfo *info = NewMLMediaInfo(szArtist, szAlbum, szTitle, szMediaUrl, nMediaLength, hasVideo);

	SendMessage(M2M_MEDIA_CHANGED, info, info->size);

	delete info;
}

void CMLService::SendMessage(MessageToML message, LPVOID param, size_t sizeParam)
{
//	if (!m_msgPipeMiniLyrics.IsOK())
//		return;

	bool ok = m_msgPipeMiniLyrics.Write(message, param, sizeParam);
	
	::Log("Message was sent to ML: %d, len: %d, OK: %d", message, sizeParam, ok);
}
