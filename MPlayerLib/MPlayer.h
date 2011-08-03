#pragma once

#include "IMPlayer.h"
#include "Playlist.h"
#include "MPTools.h"
#include "MPTime.h"
#include "MediaLibrary.h"
#include "MDAgent.h"

interface IMediaInput;
interface IMediaDecode;
interface IMediaOutput;

class CMemAllocator;
class CMPlayer;

class CFBuffer : public IFBuffer
{
public:
	CFBuffer(CMemAllocator *pMemAllocator);
	virtual ~CFBuffer();

	virtual void AddRef();
	virtual void Release();

	virtual char *data();
	virtual uint32 size();
	virtual uint32 capacity();
	virtual void resize(uint32 nSize);
	virtual MLRESULT reserve(uint32 nCapacity);

	bool New(uint32 nCapacity);

protected:
	char			*m_buf;
	uint32			m_nSize, m_nCapacity;
	CMemAllocator	*m_pMemAllocator;

	long			m_nReference;

};

class CMemAllocator : public IMemAllocator
{
OBJ_REFERENCE_DECL
public:
	CMemAllocator();
	virtual ~CMemAllocator();

	void Quit();

	virtual IFBuffer *AllocFBuffer(uint32 nCapacity);
	virtual IXStr *AllocStr();

	virtual void OnRelease(IFBuffer *pBuf);

protected:
	typedef list<IFBuffer *>		LIST_BUF;
	LIST_BUF						m_listFree;
	Mutex							m_mutex;

};

class CMOAgent : public IMediaOutput
{
OBJ_REFERENCE_DECL

public:
	CMOAgent();
	~CMOAgent();

	virtual LPCXSTR GetDescription();

	virtual MLRESULT Init(IMPlayer *pPlayer);
	virtual MLRESULT Quit();

	virtual MLRESULT Open(int nSampleRate, int nNumChannels, int nBitsPerSamp);

	virtual MLRESULT WaitForWrite();

	virtual MLRESULT Write(IFBuffer *pBuf);
	virtual MLRESULT Flush();

	virtual MLRESULT Pause(bool bPause);
	virtual bool IsPlaying();
	virtual MLRESULT Stop();

	virtual bool IsOpened();

	virtual uint32 GetPos();

	// volume
	virtual MLRESULT SetVolume(int nVolume, int nBanlance);

public:
	CMPAutoPtr<IMediaOutput>		m_pMediaOutput;

};

class CMPluginManagerAgent : public IMPluginManager
{
public:
	CMPluginManagerAgent() { }
	virtual ~CMPluginManagerAgent() { }

	virtual void AddRef() { }
	virtual void Release() { }

	virtual MLRESULT DetectPlugins();

	virtual MLRESULT OnInternalDecoderRegister(IMediaDecode *pDecoder) { if (m_pluginMgr) return m_pluginMgr->OnInternalDecoderRegister(pDecoder); else return ERR_OK; }

	virtual MLRESULT NewInput(LPCXSTR szMediaUrl, IMediaInput **ppInput);
	virtual MLRESULT NewDecoder(IMediaInput *pInput, IMediaDecode **ppDecoder);
	virtual MLRESULT NewOutput(IMediaOutput **ppOutput);

	virtual MLRESULT GetActiveDSP(IDSP **ppDSP);
	virtual MLRESULT GetActiveVis(IVector *pvVis);

public:
	CMPAutoPtr<IMPluginManager>			m_pluginMgr;

};


class CMPlayer : public IMPlayer
{
	OBJ_REFERENCE_DECL
public:
	CMPlayer();
	virtual ~CMPlayer();
	// IMPlayer

	virtual MLRESULT SetPluginManager(IMPluginManager *pPluginMgr);

	virtual MLRESULT QueryInterface(MPInterfaceType interfaceType, LPVOID *lpInterface);

	//
	// Player control
	//
	virtual MLRESULT Play();
	virtual MLRESULT Pause();
	virtual MLRESULT Unpause();
	virtual MLRESULT Stop();
	virtual MLRESULT Prev();
	virtual MLRESULT Next();
	virtual MLRESULT Seek(uint32 dwPos);

	virtual MLRESULT NewMedia(IMedia **ppMedia, LPCXSTR szUrl);
	virtual MLRESULT NewPlaylist(IPlaylist **ppPlaylist);

	virtual MLRESULT GetMediaLibrary(IMediaLibrary **ppMediaLib);

	virtual MLRESULT GetCurrentPlaylist(IPlaylist **ppPlaylist);
	virtual MLRESULT GetCurrentMedia(IMedia **ppMedia);

	virtual long GetCurrentMediaInPlaylist();
	virtual MLRESULT SetCurrentMediaInPlaylist(long nIndex);

	virtual MLRESULT SetCurrentPlaylist(IPlaylist *pPlaylist);
	virtual MLRESULT SetCurrentMedia(IMedia *pMedia);

	virtual MLRESULT SetCurrentMedia(LPCXSTR szSourceMedia);

	//
	// Current playing Media state
	//
	virtual uint32 GetLength();
	virtual uint32 GetPos();

	virtual PLAYER_STATE GetState();

	//
	// plugin interface
	//
	virtual void OutputWrite(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

	//
	// Player settings
	//
	virtual void SetShuffle(bool bShuffle);
	virtual void SetLoop(MP_LOOP_MODE loopMode);
	virtual bool GetShuffle();
	virtual MP_LOOP_MODE GetLoop();

	virtual void SetMute(bool bMute);
	virtual bool GetMute();

	// 0 ~ 100
	virtual MLRESULT SetVolume(long nVolume);
	virtual long GetVolume();

	// -100 ~ 100
	virtual MLRESULT SetBalance(long nBalance);
	virtual long GetBalance();

	virtual MLRESULT SetEQ(const EQualizer *eq);
	virtual MLRESULT GetEQ(EQualizer *eq);

	virtual bool IsAutoAddToMediaLib() { return m_bAutoAddToMediaLib; }

	//
	// Event tracer
	//
	virtual void RegisterEvent(IMPEvent *pEventHandler);
	virtual void UnregisterEvent(IMPEvent *pEventHandler);

	// decoder notify to player that the decoding is ended.
	// nError:	ERR_OK, the process ended ok
	virtual void NotifyEod(IMediaDecode *pDecoder, MLRESULT nError);

	// Reload Media tag info: artist, title, bitarte, bps, etc.
	virtual MLRESULT LoadMediaTagInfo(IMedia *pMedia, bool bForceReload);

	// Vis:
	virtual MLRESULT RegisterVis(IVis *pVis);
	virtual MLRESULT UnregisterVis(IVis *pVis);

	// Dsp:
	virtual MLRESULT RegisterDsp(IDSP *pVis);
	virtual MLRESULT UnregisterDsp(IDSP *pVis);

protected:
	virtual MLRESULT MakeOutputReadyForDecode();

	virtual MLRESULT DoStop();
	virtual MLRESULT DoNext(bool bLoop = false);

	virtual void CurrentMediaChanged();
	void NotifyPlayStateChanged();
	void NotifyCurrentPlaylistChanged(IMPEvent::MP_PLAYLIST_CHANGE_ACTION action, long nIndex, long nIndexOld);
	void NotifySettingsChanged(IMPEvent::MP_SETTING_TYPE settingType, long nValue);
	void NotifyEQSettingsChanged(const EQualizer *peq);
	void NotifySeek();

	void GenerateShuffleMediaQueue();

protected:
	// dsp plugin interface

public:
	static MLRESULT GetInstance(IMPlayer **ppPlayer);
	static MLRESULT QuitInstance(IMPlayer **ppPlayer);

	void NotifyPlaylistChanged(CPlaylist *playlist, IMPEvent::MP_PLAYLIST_CHANGE_ACTION action, long nIndex, long nIndexOld);

protected:
	friend class CMDAgent;
	friend class CMDWmpCore;

	bool				m_bAutoPlayNext;
	PLAYER_STATE		m_state;
	static CMPlayer		*m_spPlayer;

	CMPluginManagerAgent m_pluginMgrAgent;

	Mutex				m_mutexDSP;
	CMPAutoPtr<IDSP>	m_pDsp;
	CMPAutoPtr<CMDAgent>		m_pMDAgent;
	CMPAutoPtr<CMOAgent>		m_pMOAgent;

	CMPAutoPtr<CMemAllocator>	m_pMemAllocator;
	CMPAutoPtr<CMediaLibrary>	m_pMediaLib;

	vector<long>		m_vShuffleMedia;
	int					m_nCurrentShuffleMedia;
	bool				m_bShuffle;
	bool				m_bMute;
	MP_LOOP_MODE		m_loopMode;
	int					m_nVolume;
	int					m_nBalance;

	EQualizer			m_Equalizer;

	CPlaylist			*m_currentPlaylist;
	IMedia				*m_pCurrentMedia;
	bool				m_bCurMediaPlayed;
	long				m_nCurrentMedia;

	// 
	// CMDAgent			*m_pMDAgentPreload;
	// IMedia				*m_pPreloadMedia;
	// long				m_nPreloadMedia;

	list<IMPEvent *>	m_listEventHandler;
	Mutex				m_mutexEventHandler;
	Mutex				m_mutexDataAccess;

	bool				m_bAutoAddToMediaLib;

protected:
	//
	// Vis
	//
	struct VisDataPre
	{
		IFBuffer	*pBuf;
		int			nBps;
		int			nChannels;
		int			nSampleRate;
		int			nPlayingPos;
	};
	typedef list<VisDataPre>		QueueVisDataPre;
	QueueVisDataPre		m_queVisDataPre;
	int					m_nBufferedVisMs;
	int					m_nMODelay;
	Mutex				m_mutexVisdataAccess;
	CThread				m_threadVis;
	bool				m_bQuitVis;
	typedef vector<IVis*>			V_VIS;
	V_VIS				m_vVis;

	MLRESULT StartVis();
	MLRESULT StopVis();
	void CalcVisParam(VisParam &param);
	static void ThreadVisProc(LPVOID lpData);
	void ThreadVis();

	virtual bool IsVisActive();
	virtual void AddVisData(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

};
