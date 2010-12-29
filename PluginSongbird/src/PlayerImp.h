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

class CPlayerImp : public CPlayerServiceBase
{
public:
	CPlayerImp();
	virtual ~CPlayerImp();

public:
	virtual void OnGetInfo(PlayerInfo &playerInfo);

	virtual void OnConnected();
	virtual void OnDisconnect();

	virtual void Play();
	virtual void Pause();
	virtual void PlayPause();
	virtual void Stop();
	virtual void Prev();
	virtual void Next();
	virtual void SeekTo(int nMsPos);

public:
	BOOL DoCheckMediaChange();
	virtual void OnTimer(int nTimerID);

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

	DWORD					m_dwTimeBeg, m_nPosQuery;

	bool IsSBReady() { return mMM != NULL; }

};

