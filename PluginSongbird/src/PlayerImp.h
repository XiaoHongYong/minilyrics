#pragma once

#include "IPlayer.h"
#include "MLPlugin.h"

#include <nsAutoPtr.h>
#include <nsDOMJSUtils.h>
#include <nsIArray.h>
#include <nsICategoryManager.h>
#include <nsIContentViewer.h>
#include <nsIInterfaceRequestorUtils.h>
#include <nsIJSContextStack.h>
#include <nsIPrefBranch.h>
#include <nsIPresShell.h>
#include <nsIPrivateDOMEvent.h>
#include <nsIPromptService.h>
#include <nsIScriptGlobalObject.h>
#include <nsIScriptNameSpaceManager.h>
#include <nsIStringBundle.h>
#include <nsITreeSelection.h>
#include <nsITreeView.h>
#include <nsIURI.h>
#include <nsIVariant.h>
#include <nsIWindowMediator.h>
#include <nsMemory.h>
#include <nsNetUtil.h>
#include <nsServiceManagerUtils.h>
#include <nsStringGlue.h>
#include <prlog.h>

#include <sbIDataRemote.h>

class CPlayerImp : public IPlayer
{
public:
	CPlayerImp();
	virtual ~CPlayerImp();

public:
	// Init and destroy.
	BOOL Init();
	void Quit();

	// Player Info
	int GetPlayerName(char * lpszBuffer, int nBufSize);

	HWND GetMainWnd();

	// get player state and song info
	DWORD GetPlayerState();

	DWORD GetPlayPos();

	// control player
	BOOL Play();
	BOOL Pause();
	BOOL PlayPause();
	BOOL Stop();
	BOOL Prev();
	BOOL Next();
	BOOL SeekTo(int nMsPos);

	void OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan);

	BOOL DoCheckMediaChange();

protected:
	nsCOMPtr<nsIWeakReference> mMM;

	// stash these for quick reference
	nsCOMPtr<sbIDataRemote> mdrCurrentArtist;
	nsCOMPtr<sbIDataRemote> mdrCurrentAlbum;
	nsCOMPtr<sbIDataRemote> mdrCurrentTrack;
	nsCOMPtr<sbIDataRemote> mdrCurrentUrl;
	nsCOMPtr<sbIDataRemote> mdrPlaying;
	nsCOMPtr<sbIDataRemote> mdrPaused;
	nsCOMPtr<sbIDataRemote> mdrPosition;
	nsCOMPtr<sbIDataRemote> mdrCurrentLength;

	bool IsSBReady() { return mMM != NULL; }

};

extern CPlayerImp		g_Player;