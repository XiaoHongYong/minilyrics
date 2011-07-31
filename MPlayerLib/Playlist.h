#pragma once

#include "IMPlayer.h"
#include "MPTools.h"
#include "MPTime.h"
#include "Media.h"

class CMPlayer;

class CPlaylist : public IPlaylist  
{
	OBJ_REFERENCE_DECL
public:
	CPlaylist(CMPlayer *pPlayer);
	virtual ~CPlaylist();

	virtual uint32 GetCount();

	virtual MLRESULT GetItem(long nIndex, IMedia **ppMedia);

	virtual MLRESULT GetName(IXStr *str);

	virtual MLRESULT InsertItem(long nIndex, IMedia *pMedia);

	virtual MLRESULT MoveItem(long nIndexOld, long nIndexNew);

	virtual MLRESULT RemoveItem(long nIndex);

	virtual MLRESULT Clear();

public:
	MLRESULT GetItemIndex(IMedia *pMedia, long &nIndex);

protected:
	typedef vector<CMedia *>		V_MEDIA;

	Mutex				m_mutexDataAccess;
	V_MEDIA				m_vMedia;

	CMPlayer			*m_pPlayer;

};
