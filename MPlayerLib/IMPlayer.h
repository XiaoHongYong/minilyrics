#ifndef _IMPLAYER_H_
#define _IMPLAYER_H_

#ifndef _WIN32
#define interface struct
#endif

interface IMediaDecode;
interface IMediaOutput;
interface IMediaInput;
interface IMediaLibrary;
interface IVis;
interface IDSP;

typedef int MLRESULT;

enum MPLAYER_ERROR_CODE
{
	ERR_AUDIO_DECODE_INITFAILED = ERR_PLAYER_ERROR_BASE + 1,
	ERR_END_OF_STREAM,
	ERR_DECODE_FAILED,
	ERR_BUFFER_FULL,		// 缓冲区已经写满了，需要等待
	ERR_NOT_INITED,			// 未初始化
	ERR_NOT_SUPPORT,
	ERR_READ_ONLY,

	ERR_MI_OPEN_SRC,
	ERR_MI_NOT_FOUND,		// the media source doesn't exist.
	ERR_MI_SEEK,
	ERR_MI_READ,
	ERR_MI_WRITE,

	ERR_PLAYER_INVALID_STATE,		// Invalid state of current object.
	ERR_PLAYER_INVALID_FILE,		// 
	ERR_SOUND_DEVICE_OPEN,
	ERR_SOUND_DEVICE_WRITE,
	ERR_DECODER_INNER_ERROR,
	ERR_DECODER_UNSUPPORTED_FEATURE,
	ERR_DECODER_INIT_FAILED,
	ERR_DISK_FULL,
	ERR_CREATE_THREAD,
	ERR_INVALID_HANDLE,
	ERR_END_OF_PLAYLIST,			// end of playlist, prev, next will return this value
	ERR_EMPTY_PLAYLIST,
	ERR_NO_DEVICE,

	ERR_ML_NOT_FOUND,				// Not found in media library.
};

//
// IMPlayer constant
//
enum
{
	MP_VOLUME_MAX			= 100,
	MP_VOL_BALANCE_MIN		= -100,
	MP_VOL_BALANCE_MAX		= 100,

};

enum MP_LOOP_MODE
{
	MP_LOOP_OFF,
	MP_LOOP_ALL,
	MP_LOOP_TRACK,
};

#ifndef _PLAYER_STATE_DEFINED
#define _PLAYER_STATE_DEFINED
enum PLAYER_STATE
{
	PS_STOPED				= 1,
	PS_PLAYING				= 2,
	PS_PAUSED				= 3
};
#endif // _PLAYER_STATE_DEFINED

enum MPInterfaceType
{
	MPIT_INVALID			= 0,
	MPIT_INPUT				= 1,
	MPIT_OUTPUT				= 2,
	MPIT_DECODE				= 3,
	MPIT_VIS				= 4,
	MPIT_DSP				= 5,
	MPIT_MEMALLOCATOR		= 6,
	MPIT_MEDIA_LIB			= 7,
	MPIT_GENERAL_PLUGIN		= 8,
	MPIT_INPUT_DECTOR		= 9,

};

enum
{
	EQ_BANDS_COUNT			= 18,
	EQ_dB_MIN				= -20,
	EQ_dB_DEF				= 0,
	EQ_dB_MAX				= 20,
};

struct EQualizer
{
	BOOL	bEnable;
	int		nPreamp;				//set value EQ_dB_MAX to EQ_dB_MIN (default EQ_dB_DEF)
	int		vData[EQ_BANDS_COUNT];	//set value EQ_dB_MAX to EQ_dB_MIN (default EQ_dB_DEF)
};

// XStr maybe UNICODE, ANSI or Utf8 str, for different platform.
// Windows	- UNICODE
// linux	- utf8
typedef LPCTSTR LPCXSTR;
typedef LPTSTR  LPXSTR;

interface IXStr
{
	virtual ~IXStr() { }

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual LPXSTR data() = 0;
	virtual LPCXSTR c_str() = 0;
	virtual size_t size() = 0;
	virtual uint32 capacity() = 0;
	virtual void resize(uint32 nSize) = 0;
	virtual MLRESULT reserve(uint32 nCapacity) = 0;
	virtual void copy(IXStr *pSrcStr) = 0;
	virtual void copy(LPCXSTR str) = 0;
	virtual void erase(int nOffset, int n) = 0;
	virtual void clear() = 0;
	virtual void insert(int nOffset, LPCXSTR str, int n) = 0;
	virtual void append(LPCXSTR str, int n) = 0;

};

interface IVXStr
{
	virtual ~IVXStr() { }

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual size_t size() = 0;
	virtual void push_back(LPCXSTR szStr) = 0;
	virtual void clear() = 0;
	virtual LPCXSTR at(int index) = 0;
	virtual void set(int index, LPCXSTR szStr) = 0;
	virtual void insert(int index, LPCTSTR szStr) = 0;

};

interface IVector
{
	virtual ~IVector() { }

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual size_t size() = 0;
	virtual void push_back(LPVOID p) = 0;
	virtual void clear() = 0;
	virtual LPVOID at(int index) = 0;
	virtual void set(int index, LPVOID p) = 0;
	virtual void insert(int index, LPVOID p) = 0;

};

interface IVInt
{
	virtual ~IVInt() { }

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual size_t size() = 0;
	virtual void push_back(int nData) = 0;
	virtual void clear() = 0;
	virtual int at(int index) = 0;
	virtual void set(int index, int nData) = 0;
	virtual void insert(int index, int nData) = 0;

};

interface IFBuffer
{
	virtual ~IFBuffer() { }

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual char *data() = 0;
	virtual uint32 size() = 0;
	virtual uint32 capacity() = 0;
	virtual void resize(uint32 nSize) = 0;
	virtual MLRESULT reserve(uint32 nCapacity) = 0;

};

interface IMemAllocator
{
	virtual ~IMemAllocator() { }
	
	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual IFBuffer *AllocFBuffer(uint32 nCapacity) = 0;
	virtual IXStr *AllocStr() = 0;

};
/*

interface IMediaTag
{
	virtual void AddRef() = 0;
	virtual void Release() = 0;

};
*/

#define MEDIA_LENGTH_INVALID	0
#define MEDIA_ID_INVALID		0

enum MediaAttribute
{
	MA_ARTIST		= 0,
	MA_ALBUM		= 1,
	MA_TITLE		= 2,
	MA_TRACK_NUMB	= 3,
	MA_YEAR			= 4,
	MA_GENRE		= 5,
	MA_COMMENT		= 6,
	MA_DURATION		= 7,
	MA_FILESIZE		= 8,
	MA_TIME_ADDED	= 9,
	MA_TIME_PLAYED	= 10,
	MA_IS_USER_RATING	= 12,
	MA_RATING		= 11,
	MA_TIMES_PLAYED	= 13,
	MA_TIMES_PLAY_SKIPPED	= 14,
	MA_LYRICS_FILE	= 15,

	// the following info will not be stored in media library
	MA_FORMAT		= 16,
	MA_ISVBR		= 17,
	MA_BITRATE		= 18,
	MA_BPS			= 19,
	MA_CHANNELS		= 20,
	MA_SAMPLE_RATE	= 21,
	MA_EXTRA_INFO	= 22,
};

interface IMedia
{
	virtual ~IMedia() { }

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	// the ID in media library, 0 for not existing in media library
	virtual long GetID() = 0;

	virtual MLRESULT GetSourceUrl(IXStr *strUrl) = 0;
	virtual MLRESULT GetArtist(IXStr *strArtist) = 0;
	virtual MLRESULT GetTitle(IXStr *strTitle) = 0;
	virtual MLRESULT GetAlbum(IXStr *strAlbum) = 0;

	virtual long GetDuration() = 0;

	virtual MLRESULT SetSourceUrl(LPCXSTR strUrl) = 0;

	virtual bool IsInfoUpdatedToMediaLib() = 0;
	virtual MLRESULT SetInfoUpdatedToMediaLib(bool bUpdated) = 0;

	//
	// attribute methods
	//
	virtual MLRESULT GetAttribute(MediaAttribute mediaAttr, IXStr *strValue) = 0;
	virtual MLRESULT SetAttribute(MediaAttribute mediaAttr, LPCXSTR szValue) = 0;

	virtual MLRESULT GetAttribute(MediaAttribute mediaAttr, int *pnValue) = 0;
	virtual MLRESULT SetAttribute(MediaAttribute mediaAttr, int nValue) = 0;

};

interface IPlaylist
{
	virtual ~IPlaylist() { }

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual uint32 GetCount() = 0;

	virtual MLRESULT GetItem(long nIndex, IMedia **ppMedia) = 0;

	virtual MLRESULT GetName(IXStr *str) = 0;

	virtual MLRESULT InsertItem(long nIndex, IMedia *pMedia) = 0;

	virtual MLRESULT MoveItem(long nIndexOld, long nIndexNew) = 0;

	virtual MLRESULT RemoveItem(long nIndex) = 0;

	virtual MLRESULT Clear() = 0;
};

enum MediaLibOrderBy
{
	MLOB_NONE,
	MLOB_ARTIST,
	MLOB_ALBUM,
	MLOB_ARTIST_ALBUM,
	MLOB_GENRE,
	MLOB_RATING,

};

interface IMediaLibrary
{
	virtual ~IMediaLibrary() { }

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual MLRESULT GetAllArtist(IVXStr **ppvArtist) = 0;

	virtual MLRESULT GetAllAlbum(IVXStr **ppvAlbum) = 0;

	virtual MLRESULT GetAllGenre(IVXStr **ppvAlbum) = 0;

	virtual MLRESULT GetAllYear(IVInt **ppvYear) = 0;

	virtual MLRESULT GetAlbumOfArtist(LPCXSTR szArtist, IVXStr **ppvAlbum) = 0;

	virtual uint32 GetMediaCount() = 0;

	virtual MLRESULT GetMediaByUrl(LPCXSTR szUrl, IMedia **pMedia) = 0;

	virtual MLRESULT Add(LPCXSTR szMediaUrl, IMedia **ppMedia) = 0;

	// Add media to media library fast, media info will not be reloaded from media file immediately.
	virtual MLRESULT AddFast(LPCXSTR szMediaUrl, LPCXSTR szArtist, LPCXSTR szTitle, IMedia **ppMedia) = 0;

	virtual MLRESULT UpdateMediaInfo(IMedia *pMedia) = 0;

	// removes the specified item from the media library
	virtual MLRESULT Remove(IMedia **pMedia, bool bDeleteFile) = 0;

	// If the media file was removed temporarily, set this flag on.
	virtual MLRESULT SetDeleted(IMedia **pMedia) = 0;

	// nTopN = -1 : query all
	virtual MLRESULT GetAll(IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) = 0;

	virtual MLRESULT GetByArtist(LPCXSTR szArtist, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) = 0;

	virtual MLRESULT GetByAlbum(LPCXSTR szAlbum, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) = 0;

	virtual MLRESULT GetByAlbum(LPCXSTR szArtist, LPCXSTR szAlbum, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) = 0;

	virtual MLRESULT GetByTitle(LPCXSTR szTitle, IPlaylist **ppPlaylist) = 0;

	virtual MLRESULT GetByGenre(LPCXSTR szGenre, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) = 0;

	virtual MLRESULT GetByYear(int nYear, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) = 0;

	virtual MLRESULT GetByRating(int nRating, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) = 0;

	virtual MLRESULT GetRecentPlayed(uint32 nCount, IPlaylist **ppPlaylist) = 0;

	virtual MLRESULT GetRecentPlayed(int nDayAgoBegin, int nDayAgoEnd, IPlaylist **ppPlaylist) = 0;

	virtual MLRESULT GetTopPlayed(uint32 nCount, IPlaylist **ppPlaylist) = 0;

	virtual MLRESULT GetTopRating(uint32 nCount, IPlaylist **ppPlaylist) = 0;

	virtual MLRESULT GetRecentAdded(uint32 nCount, IPlaylist **ppPlaylist) = 0;

	virtual MLRESULT GetRandom(uint32 nCount, IPlaylist **ppPlaylist) = 0;

	virtual MLRESULT GetRandomByTime(uint32 nDurationInMin, IPlaylist **ppPlaylist) = 0;

	// 0-5, 0 for unrate.
	virtual MLRESULT Rate(IMedia *pMedia, uint32 nRating) = 0;

	virtual MLRESULT UpdatePlayedTime(IMedia *pMedia) = 0;
	virtual MLRESULT MarkPlayFinished(IMedia *pMedia) = 0;
	virtual MLRESULT MarkPlaySkipped(IMedia *pMedia) = 0;

};

interface IMPEvent
{
	virtual ~IMPEvent() { }

	enum MP_SETTING_TYPE
	{
		MPS_SHUFFLE,	// bool
		MPS_LOOP,		// bool
		MPS_MUTE,		// bool
		MPS_VOLUME,		// long
		MPS_BALANCE,	// long
	};

	enum MP_PLAYLIST_CHANGE_ACTION
	{
		PCA_FULL_UPDATE,
		PCA_INSERT,
		PCA_MOVE,
		PCA_REMOVE,
		PCA_CLEAR,
	};

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual void OnStateChanged(PLAYER_STATE state) = 0;
	virtual void OnSeek() = 0;
	virtual void OnCurrentMediaChanged() = 0;
	// nIndex the new/cur item which was modified. nIndexOld, is used for move.
	virtual void OnCurrentPlaylistChanged(MP_PLAYLIST_CHANGE_ACTION action, long nIndex, long nIndexOld) = 0;

	virtual void OnSettingChanged(MP_SETTING_TYPE settingType, long nValue) = 0;
	virtual void OnEQSettingChanged(const EQualizer *eq) = 0;

	virtual void OnPlayHaltError(MLRESULT nError) = 0;

};

interface IMPluginManager
{
	virtual ~IMPluginManager() { }

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual MLRESULT DetectPlugins() = 0;

	virtual MLRESULT OnInternalDecoderRegister(IMediaDecode *pDecoder) = 0;

	virtual MLRESULT NewInput(LPCXSTR szMediaUrl, IMediaInput **ppInput) = 0;
	virtual MLRESULT NewDecoder(IMediaInput *pInput, IMediaDecode **ppDecoder) = 0;
	virtual MLRESULT NewOutput(IMediaOutput **ppOutput) = 0;
	virtual MLRESULT GetActiveDSP(IDSP **ppDSP) = 0;
	virtual MLRESULT GetActiveVis(IVector *pvVis) = 0;

};

interface IMPlayer
{
	virtual ~IMPlayer() { }

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual MLRESULT SetPluginManager(IMPluginManager *pPluginMgr) = 0;

	virtual MLRESULT QueryInterface(MPInterfaceType interfaceType, LPVOID *lpInterface) = 0;

	//
	// Player control
	//
	virtual MLRESULT Play() = 0;
	virtual MLRESULT Pause() = 0;
	virtual MLRESULT Unpause() = 0;
	virtual MLRESULT Stop() = 0;
	virtual MLRESULT Prev() = 0;
	virtual MLRESULT Next() = 0;
	virtual MLRESULT Seek(uint32 dwPos) = 0;

	virtual MLRESULT NewMedia(IMedia **ppMedia, LPCXSTR szUrl) = 0;
	virtual MLRESULT NewPlaylist(IPlaylist **ppPlaylist) = 0;

	virtual MLRESULT GetMediaLibrary(IMediaLibrary **ppMediaLib) = 0;

	virtual MLRESULT GetCurrentPlaylist(IPlaylist **ppPlaylist) = 0;
	virtual MLRESULT GetCurrentMedia(IMedia **ppMedia) = 0;

	virtual long GetCurrentMediaInPlaylist() = 0;
	virtual MLRESULT SetCurrentMediaInPlaylist(long nIndex) = 0;

	virtual MLRESULT SetCurrentPlaylist(IPlaylist *pPlaylist) = 0;
	virtual MLRESULT SetCurrentMedia(IMedia *pMedia) = 0;

	virtual MLRESULT SetCurrentMedia(LPCXSTR szSourceMedia) = 0;

	//
	// Current playing Media state
	//
	virtual uint32 GetLength() = 0;
	virtual uint32 GetPos() = 0;

	virtual PLAYER_STATE GetState() = 0;

	//
	// plugin interface
	//
	// write sound data, and do dsp and vis processing.
	virtual void OutputWrite(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) = 0;

// 	virtual bool IsDspActive() = 0;
// 	virtual void DspProcess(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) = 0;
// 
// 	virtual void AddVisData(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) = 0;

	//
	// Player settings
	//
	virtual void SetShuffle(bool bShuffle) = 0;
	virtual void SetLoop(MP_LOOP_MODE loopMode) = 0;
	virtual bool GetShuffle() = 0;
	virtual MP_LOOP_MODE GetLoop() = 0;

	virtual void SetMute(bool bMute) = 0;
	virtual bool GetMute() = 0;

	// 0 ~ 100
	virtual MLRESULT SetVolume(long nVolume) = 0;
	virtual long GetVolume() = 0;

	// -100 ~ 100
	virtual MLRESULT SetBalance(long nBalance) = 0;
	virtual long GetBalance() = 0;

	virtual MLRESULT SetEQ(const EQualizer *eq) = 0;
	virtual MLRESULT GetEQ(EQualizer *eq) = 0;

	//
	// Event tracer
	//
	virtual void RegisterEvent(IMPEvent *pEventHandler) = 0;
	virtual void UnregisterEvent(IMPEvent *pEventHandler) = 0;

	// decoder notify to player that the decoding is ended.
	// nError:	ERR_OK, the process ended ok
	virtual void NotifyEod(IMediaDecode *pDecoder, MLRESULT nError) = 0;

	// Reload Media tag info: artist, title, bitarte, bps, etc.
	virtual MLRESULT LoadMediaTagInfo(IMedia *pMedia, bool bForceReload) = 0;

	// Vis:
	virtual int RegisterVis(IVis *pVis) = 0;
	virtual int UnregisterVis(IVis *pVis) = 0;

	// Dsp:
	virtual int RegisterDsp(IDSP *pVis) = 0;
	virtual int UnregisterDsp(IDSP *pVis) = 0;

};

#define MAXCHANNELS		2

#define VIS_N_WAVE_SAMPLE		512
#define VIS_N_SPTR_SAMPLE		512

struct VisParam
{
	int				nChannels;				// number of channels
	unsigned char	spectrumData[2][VIS_N_WAVE_SAMPLE];	// 
	unsigned char	waveformData[2][VIS_N_SPTR_SAMPLE];
};

interface IVis
{
	virtual ~IVis() { }
	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual MLRESULT Init(IMPlayer *pPlayer) = 0;
	virtual MLRESULT Quit() = 0;

	virtual int Render(VisParam *visParam) = 0;
};

interface IDSP
{
	virtual ~IDSP() { }

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual MLRESULT Init(IMPlayer *pPlayer) = 0;
	virtual MLRESULT Quit() = 0;

	virtual void Process(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) = 0;
};

interface IMediaOutput// : IUnknown
{
	IMediaOutput() : m_nChannels(0), m_nSamplerate(0), m_nBitsPerSamp(0) { }
	virtual ~IMediaOutput() { }

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual LPCXSTR GetDescription() = 0;

	virtual MLRESULT Init(IMPlayer *pPlayer) = 0;
	virtual MLRESULT Quit() = 0;

	virtual MLRESULT Open(int nSampleRate, int nNumChannels, int nBitsPerSamp) = 0;

	virtual MLRESULT WaitForWrite() = 0;

	virtual MLRESULT Write(IFBuffer *pBuf) = 0;
	virtual MLRESULT Flush() = 0;

	virtual MLRESULT Pause(bool bPause) = 0;
	virtual bool IsPlaying() = 0;
	virtual MLRESULT Stop() = 0;

	virtual bool IsOpened() = 0;

	virtual uint32 GetPos() = 0;

	// volume
	virtual MLRESULT SetVolume(int nVolume, int nBanlance) = 0;

	int GetChannels() { return m_nChannels; }
	int GetSamplerate() { return m_nSamplerate; }
	int GetBPS() { return m_nBitsPerSamp; }
protected:
	int			m_nChannels;
	int			m_nSamplerate;
	int			m_nBitsPerSamp;

};

interface IMediaInputDetector
{
	virtual ~IMediaInputDetector() { }
	
	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual MLRESULT NewInput(LPCXSTR szSourceMedia, IMediaInput **ppInput) = 0;
	virtual MLRESULT GetDescription(IXStr *strDesc);

};

interface IMediaInput
{
	virtual ~IMediaInput() { }
	virtual void AddRef() = 0;
	virtual void Release() = 0;

	// if FAILED, return ERR_MI_OPEN_SRC, ERR_MI_NOT_FOUND
	virtual MLRESULT Open(LPCXSTR szSourceMedia) = 0;
	virtual uint32 Read(LPVOID lpBuffer, uint32 dwSize) = 0;
	// nOrigin: SEEK_SET, SEEK_CUR, SEEK_END
	virtual MLRESULT Seek(uint32 dwOffset, int nOrigin) = 0;
	virtual MLRESULT GetSize(uint32 &dwSize) = 0;
	virtual uint32 GetPos() = 0;

	virtual bool IsEOF() = 0;
	virtual bool IsError() = 0;

	virtual void Close() = 0;

	virtual LPCXSTR GetSource() = 0;

};
/*
interface IMediaDecodeSimple
{
	virtual void AddRef() = 0;
	virtual void Release() = 0;

	//
	// individual methods
	//

	virtual LPCXSTR GetDescription() = 0;
	virtual LPCXSTR GetFileExtentions() = 0;
	virtual MLRESULT GetMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia) = 0;

	//
	// decode media file related methods
	//

	virtual bool IsSeekable() = 0;

	//	ERR_PLAYER_INVALID_FILE:	decoder can't decode the file
	//  ERR_DECODER_INNER_ERROR:	inner error occurs at decoder
	//  ERR_DECODER_UNSUPPORTED_FEATURE:
	//  ERR_DECODER_INIT_FAILED:
	virtual MLRESULT Play(IMPlayer *pPlayer, IMediaInput *pInput) = 0;

	// media length, pos related functions, unit: ms
	virtual uint32 GetLength() = 0;
	virtual MLRESULT Seek(uint32 nPos) = 0;
	virtual uint32 GetPos() = 0;

};
*/

interface IMediaDecode// : IUnknown
{
	virtual ~IMediaDecode() { }

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	//
	// individual methods
	//

	virtual LPCXSTR GetDescription() = 0;
	// Get supported file extensions. Example: .mp3|MP3 files|.mp2|MP2 files
	virtual LPCXSTR GetFileExtentions() = 0;
	virtual MLRESULT GetMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia) = 0;

	//
	// decode media file related methods
	//

	virtual bool IsSeekable() = 0;
	virtual bool IsUseOutputPlug() = 0;

	//	ERR_PLAYER_INVALID_FILE:	decoder can't decode the file
	//  ERR_DECODER_INNER_ERROR:	inner error occurs at decoder
	//  ERR_DECODER_UNSUPPORTED_FEATURE:
	//  ERR_DECODER_INIT_FAILED:
	virtual MLRESULT Play(IMPlayer *pPlayer, IMediaInput *pInput) = 0;
	virtual MLRESULT Pause() = 0;
	virtual MLRESULT Unpause() = 0;
	virtual MLRESULT Stop() = 0;

	// media length, pos related functions, unit: ms
	virtual uint32 GetLength() = 0;
	virtual MLRESULT Seek(uint32 nPos) = 0;
	virtual uint32 GetPos() = 0;

	// volume
	virtual MLRESULT SetVolume(int nVolume, int nBanlance) = 0;

};

// MLRESULT GetPlayerInstance(IMPlayer **ppPlayer);

//
// A plugin DLL may contain serveral plugins, use ZikiPlayerQueryPluginIF to query all the plugin interface
// that it supported.
//
// Query will start with nIndex = 0, till !ERR_OK returned.
//
// nIndex = 0, return its interface type, and description, and lpInterface.
// strDescription and lpInterface can be NULL, and only its description or lpInterface is queried.
//
// extern "C" __declspec(dllexport) MLRESULT ZikiPlayerQueryPluginIF(
// 	int nIndex,
// 	MPInterfaceType *pInterfaceType,
// 	IXStr *strDescription,
// 	LPVOID *lpInterface
// );
typedef MLRESULT (*ZikiPlayerQueryPluginIF_t)(
	int nIndex,
	MPInterfaceType *pInterfaceType,
	IXStr *strDescription,
	LPVOID *lpInterface
);

#define SZ_FUNC_ZP_QUERY_PLUGIN_IF		"ZikiPlayerQueryPluginIF"

#endif // _IMPLAYER_H_
