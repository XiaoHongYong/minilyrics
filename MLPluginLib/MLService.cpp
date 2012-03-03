#include "StdAfx.h"
#include "MLPlugin.h"
#include "MLService.h"


inline void appendMediaInfoStr(LPWSTR szInfo, size_t &offset, WORD &offsetTag, LPCWSTR tag, size_t lenTag)
{
	offsetTag = offset;
	memcpy(szInfo + offset, tag, (lenTag + 1) * sizeof(WCHAR));
	offset += lenTag + 1;
}

inline MLMediaInfo *NewMLMediaInfo(LPCWSTR artist, LPCWSTR album, LPCWSTR title, LPCWSTR mediaUrl, unsigned long mediaLength, WORD hasVideo = R_UNDETERMINED)
{
	size_t	lenArtist = wcslen(artist);
	size_t	lenAlbum = wcslen(album);
	size_t	lenTitle = wcslen(title);
	size_t	lenMediaUrl = wcslen(mediaUrl);

	// 8 is the extra characters '\0' of artist, album, title, mediaUrl.
	DWORD	sizeInfo = sizeof(MLMediaInfo) + (8 + lenArtist + lenAlbum + lenTitle + lenMediaUrl) * sizeof(WCHAR);
	MLMediaInfo *info = (MLMediaInfo *)new char[sizeInfo];

	info->size = sizeInfo;
	info->mediaLength = mediaLength;
	info->hasVideo = hasVideo;

	size_t offset = 0;
	appendMediaInfoStr(info->mediaInfo, offset, info->offsetArtist, artist, lenArtist);
	appendMediaInfoStr(info->mediaInfo, offset, info->offsetAlbum, album, lenAlbum);
	appendMediaInfoStr(info->mediaInfo, offset, info->offsetTitle, title, lenTitle);
	appendMediaInfoStr(info->mediaInfo, offset, info->offsetMediaUrl, mediaUrl, lenMediaUrl);
	ASSERT(offset * sizeof(WCHAR) + sizeof(MLMediaInfo) < sizeInfo);

	return info;
}

void CMLService::AddWndCloseto(HWND hWnd, LPCWSTR wndClass, LPCWSTR wndName)
{
	WndCloseToML	closeTo;
	closeTo.hWndCloseTo = hWnd;

	if (wndClass)
		wcscpy_safe(closeTo.className, CountOf(closeTo.className), wndClass);
	else
		EmptyStr(closeTo.className);

	if (wndName)
		wcscpy_safe(closeTo.wndName, CountOf(closeTo.wndName), wndName);
	else
		EmptyStr(closeTo.wndName);

	SendMessage(M2M_ADD_WND_CLOSE_TO, &closeTo, sizeof(closeTo));
}

void CMLService::OnMediaInfoChanged(LPCSTR szMediaUrl, LPCSTR szArtist, LPCSTR szAlbum, LPCSTR szTitle, unsigned long nMediaLength, int hasVideo)
{
	WCHAR		wszMediaUrl[512], wszArtist[512], wszAlbum[512], wszTitle[512];

	EmptyStr(wszMediaUrl);
	EmptyStr(wszArtist);
	EmptyStr(wszAlbum);
	EmptyStr(wszTitle);

	if (szMediaUrl)
		MbcsToWideChar(szMediaUrl, -1, wszMediaUrl, CountOf(wszMediaUrl), CP_ACP);
	if (szArtist)
		MbcsToWideChar(szArtist, -1, wszArtist, CountOf(wszArtist), CP_ACP);
	if (szAlbum)
		MbcsToWideChar(szAlbum, -1, wszAlbum, CountOf(wszAlbum), CP_ACP);
	if (szTitle)
		MbcsToWideChar(szTitle, -1, wszTitle, CountOf(wszTitle), CP_ACP);

	OnMediaInfoChanged(wszMediaUrl, wszArtist, wszAlbum, wszTitle, nMediaLength, hasVideo);
}

void CMLService::OnMediaInfoChanged(LPCWSTR szMediaUrl, LPCWSTR szArtist, LPCWSTR szAlbum, LPCWSTR szTitle, unsigned long nMediaLength, int hasVideo)
{
	if (szMediaUrl == NULL)
		szMediaUrl = L"";
	if (szArtist == NULL)
		szArtist = L"";
	if (szAlbum == NULL)
		szAlbum = L"";
	if (szTitle == NULL)
		szTitle = L"";

	MLMediaInfo *info = NewMLMediaInfo(szArtist, szAlbum, szTitle, szMediaUrl, nMediaLength, hasVideo);

	SendMessage(M2M_MEDIA_CHANGED, info, info->size);

	delete info;
}

void CMLService::SendMessage(MessageToML message, LPVOID param, size_t sizeParam)
{
	// ASSERT(m_mlInfo.hWndMLService);
	if (!m_mlInfo.hWndMLService)
		return;

	COPYDATASTRUCT	cds;
	cds.cbData = sizeParam;
	cds.dwData = message;
	cds.lpData = param;
	if (message == M2M_PLAYER_INFO)
		::SendMessage(m_mlInfo.hWndMLService, WM_COPYDATA, ML_MSG_VERSION, (LPARAM)&cds);
	else
	{
		ASSERT(m_hWndPlayerService != NULL);
		::SendMessage(m_mlInfo.hWndMLService, WM_COPYDATA, (WPARAM)m_hWndPlayerService, (LPARAM)&cds);
	}
}
