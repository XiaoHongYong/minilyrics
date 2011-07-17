#pragma once

class CMediaMonkey
{
public:
	CMediaMonkey(void);
	~CMediaMonkey(void);

	static bool GetCurrentMedia(LPWSTR szFile);
	static bool GetCurrentMediaInfo(wchar_t szArtist[], wchar_t szAlbum[], wchar_t szTitle[]);

	// static HWND CreateEmbeddedWnd();

	static void Close();

protected:
	static bool MakeSureInterfaceOK();

};
