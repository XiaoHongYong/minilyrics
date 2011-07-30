#pragma once

#include "../PluginWMP/wmp.h"

class CMDWmpCore: public IMediaDecode
{
	OBJ_REFERENCE_DECL
public:
	CMDWmpCore(void);
	~CMDWmpCore(void);

	//
	// individual methods
	//

	virtual LPCXSTR GetDescription();

	// get supported file's extentions. exm: mp3;mp3 files;mp2;mp2 files;
	virtual LPCXSTR GetFileExtentions();
	virtual MLRESULT GetMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia);

	//
	// decode media file related methods
	//

	virtual bool IsSeekable();
	virtual bool IsUseOutputPlug();

	// ERR_PLAYER_INVALID_FILE: decoder can't decode the file
	// ERR_DECODER_INNER_ERROR: inner error occurs at decoder
	// ERR_DECODER_UNSUPPORTED_FEATURE:
	// ERR_DECODER_INIT_FAILED:
	virtual MLRESULT Play(IMPlayer *pPlayer, IMediaInput *pInput);
	virtual MLRESULT Pause();
	virtual MLRESULT Unpause();
	virtual MLRESULT Stop();

	// media length, pos related functions, unit: ms
	virtual uint32 GetLength();
	virtual MLRESULT Seek(uint32 nPos);
	virtual uint32 GetPos();

	// volume
	virtual MLRESULT SetVolume(int nVolume, int nBanlance);

protected:
	bool Init();

protected:
	friend class CWmpEventsSink;

	CMPAutoPtr<IMPlayer> m_mplayer;

	CMPAutoPtr<IWMPPlayer> m_player;
	CMPAutoPtr<IWMPControls> m_controls;
	CMPAutoPtr<IWMPPlaylist> m_playlist;
	CMPAutoPtr<IWMPMediaCollection> m_collection;

	CMPAutoPtr<IConnectionPoint> m_pConnectionPoint;
	DWORD m_dwAdviseCookie;

};

