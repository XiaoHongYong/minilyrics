// Playlist.cpp: implementation of the CPlaylist class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayer.h"
#include "Playlist.h"


CPlaylist::CPlaylist(CMPlayer *pPlayer) : m_pPlayer(pPlayer)
{
	OBJ_REFERENCE_INIT
	if (m_pPlayer)
		m_pPlayer->AddRef();
}

CPlaylist::~CPlaylist()
{
	V_MEDIA::iterator	it, itEnd;

	itEnd = m_vMedia.end();
	for (it = m_vMedia.begin(); it != itEnd; ++it)
	{
		(*it)->Release();
	}
	m_vMedia.clear();

	if (m_pPlayer)
		m_pPlayer->Release();
}

uint32 CPlaylist::GetCount()
{
	CMutexAutolock	autolock(m_mutexDataAccess);

	return m_vMedia.size();
}

MLRESULT CPlaylist::GetItem(long nIndex, IMedia **ppMedia)
{
	CMutexAutolock	autolock(m_mutexDataAccess);

//	ASSERT(nIndex >= 0 && nIndex < (int)m_vMedia.size());

	if (nIndex >= 0 && nIndex < (int)m_vMedia.size())
	{
		*ppMedia = m_vMedia[nIndex];
		(*ppMedia)->AddRef();
		return ERR_OK;
	}

	return ERR_NOT_FOUND;
}

MLRESULT CPlaylist::GetName(IXStr *str)
{
	return ERR_OK;
}

MLRESULT CPlaylist::InsertItem(long nIndex, IMedia *pMedia)
{
	m_mutexDataAccess.Acquire();

	pMedia->AddRef();

	if (nIndex >= 0 && nIndex < (int)m_vMedia.size())
	{
		m_vMedia.insert(m_vMedia.begin() + nIndex, (CMedia*)pMedia);
	}
	else
	{
		m_vMedia.push_back((CMedia*)pMedia);
		nIndex = m_vMedia.size() - 1;
	}

	m_mutexDataAccess.Release();

	m_pPlayer->NotifyPlaylistChanged(this, IMPEvent::PCA_INSERT, nIndex, 0);

	return ERR_OK;
}

MLRESULT CPlaylist::MoveItem(long nIndexOld, long nIndexNew)
{
	CMutexAutolock	autolock(m_mutexDataAccess);
	CMedia		*pTemp;

	ASSERT(nIndexOld >= 0 && nIndexOld < (int)m_vMedia.size());
	ASSERT(nIndexNew >= 0 && nIndexNew < (int)m_vMedia.size());

	if (nIndexOld >= 0 && nIndexOld < (int)m_vMedia.size() &&
		nIndexNew >= 0 && nIndexNew < (int)m_vMedia.size())
	{
		if (nIndexOld < nIndexNew)
		{
			pTemp = m_vMedia[nIndexOld];
			for (int i = nIndexOld; i < nIndexNew; i++)
			{
				m_vMedia[i] = m_vMedia[i + 1];
			}
			m_vMedia[nIndexNew] = pTemp;
		}
		else if (nIndexOld > nIndexNew)
		{
			pTemp = m_vMedia[nIndexOld];
			for (int i = nIndexOld; i > nIndexNew; i--)
			{
				m_vMedia[i] = m_vMedia[i - 1];
			}
			m_vMedia[nIndexNew] = pTemp;
		}

		autolock.Unlock();
		m_pPlayer->NotifyPlaylistChanged(this, IMPEvent::PCA_MOVE, nIndexNew, nIndexOld);

		return ERR_OK;
	}
	else
		return ERR_NOT_FOUND;
}

MLRESULT CPlaylist::RemoveItem(long nIndex)
{
	CMutexAutolock	autolock(m_mutexDataAccess);

	ASSERT(nIndex >= 0 && nIndex < (int)m_vMedia.size());

	if (nIndex >= 0 && nIndex < (int)m_vMedia.size())
	{
		CMedia		*pTemp;
		pTemp = m_vMedia[nIndex];
		m_vMedia.erase(m_vMedia.begin() + nIndex);
		pTemp->Release();

		autolock.Unlock();
		m_pPlayer->NotifyPlaylistChanged(this, IMPEvent::PCA_REMOVE, nIndex, 0);
		return ERR_OK;
	}
	else
		return ERR_NOT_FOUND;
}

MLRESULT CPlaylist::Clear()
{
	CMutexAutolock	autolock(m_mutexDataAccess);

	if (m_vMedia.size() == 0)
		return ERR_OK;

	for (uint32 i = 0; i < m_vMedia.size(); i++)
	{
		CMedia		*pTemp;
		pTemp = m_vMedia[i];
		pTemp->Release();
	}

	m_vMedia.clear();

	autolock.Unlock();
	m_pPlayer->NotifyPlaylistChanged(this, IMPEvent::PCA_CLEAR, 0, 0);

	return ERR_OK;
}

MLRESULT CPlaylist::GetItemIndex(IMedia *pMedia, long &nIndex)
{
	CMutexAutolock	autolock(m_mutexDataAccess);

	nIndex = -1;

	if (m_vMedia.size() == 0)
		return ERR_NOT_FOUND;

	for (uint32 i = 0; i < m_vMedia.size(); i++)
	{
		CMedia		*pTemp;
		pTemp = m_vMedia[i];
		if (pTemp == pMedia)
		{
			nIndex = i;
			return ERR_OK;
		}
	}

	return ERR_NOT_FOUND;
}
