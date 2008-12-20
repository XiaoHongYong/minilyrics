// MDLibmad.cpp: implementation of the CMDLibmad class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MDLibmad.h"
#include "audio.h"
#include "../ID3Lib/ID3v1.h"
#include "../ID3Lib/ID3v2.h"

#define		BPS		16
#define DECODE_BUFF_SIZE		(1024 * 32)
#define INPUT_BUFF_SIZE			(1024 * 16)

#define	MAD_FLAG_MPEG_V2		MAD_FLAG_LSF_EXT

struct GetHeadInfoParam
{
	IMediaInput		*m_pInput;
	MPEG_AUDIO_INFO	m_audioInfo;
	CCharBuffer		m_bufInput;
};


/*
 * NAME:	decode->input_read()
 * DESCRIPTION:	(re)fill decoder input buffer by reading a file descriptor
*/
static enum mad_flow GetHeadInfo_decode_input_read(void *data, struct mad_stream *stream)
{
	GetHeadInfoParam	*pmdMad = (GetHeadInfoParam *)data;
	CCharBuffer			&m_bufInput = pmdMad->m_bufInput;
	IMediaInput			*m_pInput = pmdMad->m_pInput;
	int					len;

	if (pmdMad->m_pInput->IsEOF())
		return MAD_FLOW_STOP;
	
	if (stream->next_frame) {
		ASSERT((int)((LPCSTR)stream->next_frame - m_bufInput.data()) > 0);
		m_bufInput.erase(0, int((LPCSTR)stream->next_frame - m_bufInput.data()));
	}

	len = m_pInput->Read(m_bufInput.data() + m_bufInput.size(), m_bufInput.capacity() - m_bufInput.size());
	if (len == 0) {
		if (m_pInput->IsError())
		{
			ERRLOG0(_T("m_pInput Read Error!"));
			return MAD_FLOW_BREAK;
		}

		assert(m_bufInput.size() >= MAD_BUFFER_GUARD);

		while (len < MAD_BUFFER_GUARD)
		{
			m_bufInput.append("\0", 1);
			len++;
		}
	}
	else
		m_bufInput.resize(m_bufInput.size() + len);

	mad_stream_buffer(stream, (const unsigned char *)m_bufInput.data(), m_bufInput.size());

	return MAD_FLOW_CONTINUE;
}


/*
 * NAME:	decode->input_read()
 * DESCRIPTION:	(re)fill decoder input buffer by reading a file descriptor
 */
static enum mad_flow decode_input_read(void *data, struct mad_stream *stream)
{
	CMDLibmad		*pmdMad = (CMDLibmad *)data;
	CCharBuffer		&m_bufInput = pmdMad->m_bufInput;
	IMediaInput		*m_pInput = pmdMad->m_pInput;
	int				len;

	if (pmdMad->m_bSeekFlag)
	{
		// do seek.
		uint32 lSeekTo;
		double	msPerFrame = (double)pmdMad->m_audioInfo.nSamplesPerFrame / pmdMad->m_audioInfo.header.samplerate * 1000;
		lSeekTo = (int)((double)pmdMad->m_nSeekPos / msPerFrame * pmdMad->m_audioInfo.nFrameSize) + pmdMad->m_audioInfo.nFirstFramePos;
		ASSERT(lSeekTo <= pmdMad->m_audioInfo.nMediaFileSize);
		m_pInput->Seek(lSeekTo, SEEK_SET);
		m_bufInput.clear();
		pmdMad->m_pOutput->Flush();
		pmdMad->m_bSeekFlag = false;
		stream->next_frame = NULL;
	}
	
	if (m_pInput->IsEOF())
		return MAD_FLOW_STOP;
	
	if (stream->next_frame) {
		ASSERT((int)((LPCSTR)stream->next_frame - m_bufInput.data()) > 0);
		m_bufInput.erase(0, int((LPCSTR)stream->next_frame - m_bufInput.data()));
	}
	
	len = m_pInput->Read(m_bufInput.data() + m_bufInput.size(), m_bufInput.capacity() - m_bufInput.size());
	if (len == 0) {
		if (m_pInput->IsError())
		{
			ERRLOG0(_T("m_pInput Read Error!"));
			return MAD_FLOW_BREAK;
		}

		assert(m_bufInput.size() >= MAD_BUFFER_GUARD);
		
		while (len < MAD_BUFFER_GUARD)
		{
			m_bufInput.append("\0", 1);
			len++;
		}
	}
	else
		m_bufInput.resize(m_bufInput.size() + len);
	
	mad_stream_buffer(stream, (const unsigned char *)m_bufInput.data(), m_bufInput.size());
	
	return MAD_FLOW_CONTINUE;
}

/*
 * NAME:	decode->output()
 * DESCRIPTION: configure audio module and output decoded samples
 */
/*
static enum mad_flow decode_output(void *data, struct mad_header const *header,
			    struct mad_pcm *pcm)
{
	CMDLibmad		*pmdMad = (CMDLibmad *)data;
	IMediaOutput	*pOutput = pmdMad->m_pOutput;
	IMemAllocator	*pMemAllocator = pmdMad->m_pMemAllocator;
	MLRESULT		nRet;

	mad_fixed_t const *ch1, *ch2;

	ch1 = pcm->samples[0];
	ch2 = pcm->samples[1];
	
	if (pcm->channels == 1)
		ch2 = 0;

	if (pOutput->GetChannels() != pcm->channels || 
		pOutput->GetSamplerate() != pcm->samplerate)
	{
		DBGLOG1(_T("output: decoded sample frequency %u Hz"),pcm->samplerate);

		pOutput->IsOpened();
		{
			pmdMad->m_nSeekPos = pmdMad->GetPos();
			pOutput->Stop();
		}
		pOutput->Open(pcm->samplerate, pcm->channels, BPS);
	}

	nRet = pOutput->WaitForWrite();
	if (nRet != ERR_OK)
		return MAD_FLOW_CONTINUE;

	if (pmdMad->m_bKillThread)
		return MAD_FLOW_STOP;

	IFBuffer	*pBuf = NULL;
	pBuf = pMemAllocator->AllocFBuffer(INPUT_BUFF_SIZE);
	ASSERT(pBuf);
	uint32 len = audio_pcm_s16le((unsigned char *)pBuf->data(), pcm->length,
		ch1, ch2, AUDIO_MODE_DITHER, &(pmdMad->m_audioStats));
	pBuf->resize(len);

	if (pmdMad->m_pPlayer->IsDspActive())
		pmdMad->m_pPlayer->DspProcess(pBuf, BPS, pcm->channels, pcm->samplerate);

	pmdMad->m_pPlayer->AddVisData(pBuf, BPS, pcm->channels, pcm->samplerate);

	pOutput->Write(pBuf);

	return MAD_FLOW_CONTINUE;
}
*/

static enum mad_flow error_default(void *data, struct mad_stream *stream,
			    struct mad_frame *frame)
{
	int *bad_last_frame = (int*)data;

	switch (stream->error) {
	case MAD_ERROR_BADCRC:
		if (*bad_last_frame)
			mad_frame_mute(frame);
		else
			*bad_last_frame = 1;

		return MAD_FLOW_IGNORE;
	default:
		return MAD_FLOW_CONTINUE;
	}
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMDLibmad::CMDLibmad()
{
	OBJ_REFERENCE_INIT;

	m_pInput = NULL;
	m_pPlayer = NULL;
	m_pOutput = NULL;
	m_bPaused = false;
	m_bKillThread = false;
	m_nSeekPos = 0;
	m_bSeekFlag = false;
	m_state = PS_STOPED;
	m_bufInput.reserve(INPUT_BUFF_SIZE);
}

CMDLibmad::~CMDLibmad()
{
	if (m_pPlayer)
	{
		m_pPlayer->Release();
		m_pPlayer = NULL;
	}
}

OBJ_REFERENCE_IMP(CMDLibmad)

LPCXSTR CMDLibmad::GetDescription()
{
	return _T("Mp3 file decoder");
}

// get supported file's extentions.
LPCXSTR CMDLibmad::GetFileExtentions()
{
	return _T(".mp3\0mp3 files\0");
}

MLRESULT CMDLibmad::GetMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia)
{
	MLRESULT		nRet;

	nRet = GetHeadInfo(pInput);
	if (nRet != ERR_OK)
		return nRet;

	LPCXSTR szSource = pInput->GetSource();
	FILE	*fp = _tfopen(szSource, _T("rb"));
	if (fp)
	{
		bool	bUpdate = false;
		tstring  artist,  title,  album,  comment;
		tstring  track,  year,  genre,  composer;
		tstring  encoded,  url,  copyRight,  origArtist,  publisher,  bpm;

		CID3v1	id3v1;
		ID3V1	tag;
		if (id3v1.GetTag(fp, &tag) == ERR_OK)
		{
			tstringex	str;
			ConvertStr(tag.szTitle, -1, str);
			title = str.c_str();

			ConvertStr(tag.szArtist, -1, str);
			artist = str.c_str();

			ConvertStr(tag.szAlbum, -1, str);
			album = str.c_str();

			ConvertStr(tag.szYear, -1, str);
			year = str.c_str();

			ConvertStr(tag.szComment, -1, str);
			comment = str.c_str();

			if (tag.byTrack != 255)
				_itostr(tag.byTrack, track, 10);

			LPCSTR	szGenre = CID3v1::GetGenreDescription(tag.byGenre);
			if (szGenre)
			{
				ConvertStr(szGenre, -1, str);
				genre = str.c_str();
			}

			bUpdate = true;
		}
		fclose(fp);

		CID3v2IF	id3v2;

		nRet = id3v2.Open(szSource, false, false);
		if (nRet == ERR_OK)
		{
			nRet = id3v2.GetTags(artist,  title,  album,  comment,
			track,  year,  genre,  composer,
			encoded,  url,  copyRight,  origArtist,  publisher,  bpm);
			if (nRet == ERR_OK)
				bUpdate = true;
		}

		if (bUpdate)
		{
			pMedia->SetAttribute(MA_TITLE, title.c_str());
			pMedia->SetAttribute(MA_ARTIST, artist.c_str());
			pMedia->SetAttribute(MA_ALBUM, album.c_str());
			pMedia->SetAttribute(MA_YEAR, year.c_str());
			pMedia->SetAttribute(MA_COMMENT, comment.c_str());
			pMedia->SetAttribute(MA_TRACK_NUMB, track.c_str());
			pMedia->SetAttribute(MA_GENRE, genre.c_str());
		}
	}

    pMedia->SetAttribute(MA_FORMAT, _T("MP3"));
	pMedia->SetAttribute(MA_FILESIZE, m_audioInfo.nMediaFileSize);
	pMedia->SetAttribute(MA_DURATION, m_audioInfo.nMediaLength);
	pMedia->SetAttribute(MA_SAMPLE_RATE, m_audioInfo.header.samplerate);
	pMedia->SetAttribute(MA_CHANNELS, m_audioInfo.header.mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2);
	pMedia->SetAttribute(MA_BITRATE, m_audioInfo.nBitRate);
	pMedia->SetAttribute(MA_ISVBR, m_audioInfo.bVbr);

	pMedia->SetAttribute(MA_EXTRA_INFO,
		CStrPrintf(_T("Header offset: %d\n")
			_T("MPEG Version: %d\n")
			_T("MPEG Layer: %d\n")
			_T("Frames: %d\n")
			_T("CRC: %s\n")
			_T("Copyrighted: %s\n"),
			m_audioInfo.nFirstFramePos,
			(m_audioInfo.header.flags & MAD_FLAG_MPEG_V2) ? 2 : 1,
			m_audioInfo.header.layer,
			m_audioInfo.nFrameCount,
			m_audioInfo.header.crc_check ? _T("YES") : _T("NO"),
			m_audioInfo.header.private_bits ? _T("YES") : _T("NO")).c_str());

	return nRet;
}

bool CMDLibmad::IsSeekable()
{
	return true;
}

bool CMDLibmad::IsUseOutputPlug()
{
	return true;
}

MLRESULT CMDLibmad::Play(IMPlayer *pPlayer, IMediaInput *pInput)
{
	MLRESULT		nRet;

	m_state = PS_STOPED;
	m_bPaused = false;
	m_nSeekPos = 0;
	m_bSeekFlag = false;

	m_pPlayer = pPlayer;
	m_pPlayer->AddRef();

	m_pInput = pInput;
	m_pInput->AddRef();

	nRet = GetHeadInfo(m_pInput);
	if (nRet != ERR_OK)
		goto R_FAILED;

	nRet = m_pPlayer->QueryInterface(MPIT_OUTPUT, (LPVOID *)&m_pOutput);
	if (nRet != ERR_OK)
		goto R_FAILED;

	nRet = m_pPlayer->QueryInterface(MPIT_MEMALLOCATOR, (LPVOID *)&m_pMemAllocator);
	if (nRet != ERR_OK)
		goto R_FAILED;

	nRet = m_pOutput->Open(m_audioInfo.header.samplerate, 
		m_audioInfo.header.mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2,
		BPS);
	if (nRet != ERR_OK)
		goto R_FAILED;

	m_state = PS_PLAYING;
	m_bKillThread = false;

	m_threadDecode.Create(DecodeThread, this);
	m_threadDecode.SetPriority(THREAD_PRIORITY_HIGHEST);
//#ifdef _WIN32_WCE
// 	HMODULE hModule = LoadLibrary(_T("Coredll.dll"));
// 	if (hModule) {
// 		BOOL (WINAPI *pCeSetThreadPriority)(HANDLE hThread, int nPriority); 
// 		(FARPROC&)pCeSetThreadPriority = GetProcAddress(hModule, _T("CeSetThreadPriority"));
// 		if (pCeSetThreadPriority) 
// 			pCeSetThreadPriority(GetCurrentThread(), 160);
// 		FreeLibrary(hModule);
// 	}
//#endif

	return ERR_OK;

R_FAILED:
	if (m_pPlayer)
	{
		m_pPlayer->Release();
		m_pPlayer = NULL;
	}
	if (m_pInput)
	{
		m_pInput->Release();
		m_pInput = NULL;
	}
	if (m_pOutput)
	{
		m_pOutput->Release();
		m_pOutput = NULL;
	}
	if (m_pMemAllocator)
	{
		m_pMemAllocator->Release();
		m_pMemAllocator = NULL;
	}
	return nRet;
}

MLRESULT CMDLibmad::Pause()
{
	if (m_state != PS_PLAYING)
		return ERR_PLAYER_INVALID_STATE;
	m_state = PS_PAUSED;
	ASSERT(m_pOutput);
	return m_pOutput->Pause(true);
}

MLRESULT CMDLibmad::Unpause()
{
	if (m_state != PS_PAUSED)
		return ERR_PLAYER_INVALID_STATE;
	m_state = PS_PLAYING;
	ASSERT(m_pOutput);
	return m_pOutput->Pause(false);
}

MLRESULT CMDLibmad::Stop()
{
	m_bKillThread = true;

	if (m_state == PS_PAUSED)
	{
		ASSERT(m_pOutput);
		m_pOutput->Flush();
	}

	m_threadDecode.Join();

	return ERR_OK;
}

uint32 CMDLibmad::GetLength()
{
	return m_audioInfo.nMediaLength;
}

MLRESULT CMDLibmad::Seek(uint32 dwPos)
{
	m_bSeekFlag = true;
	m_nSeekPos = dwPos;

	ASSERT(m_pOutput);
	if (m_pOutput && m_state == PS_PLAYING)
		m_pOutput->Flush();

	return ERR_OK;
}

uint32 CMDLibmad::GetPos()
{
	if (m_pOutput && !m_bSeekFlag)
		return m_nSeekPos + m_pOutput->GetPos();
	else
		return m_nSeekPos;
}

MLRESULT CMDLibmad::SetVolume(int nVolume, int nBanlance)
{
	if (!m_pOutput)
		return ERR_PLAYER_INVALID_STATE;

	return m_pOutput->SetVolume(nVolume, nBanlance);
}

void CMDLibmad::DecodeThread(LPVOID lpParam)
{
	CMDLibmad		*pMDRow;

	pMDRow = (CMDLibmad *)lpParam;

	pMDRow->AddRef();
	pMDRow->DecodeThreadProc();
	pMDRow->Release();
}

DWORD CMDLibmad::DecodeThreadProc()
{
	m_bufInput.clear();
	m_pInput->Seek(0, SEEK_SET);

	mad_decoder_init(&m_decoder, this,
		   decode_input_read,
		   NULL/*decode_header*/, /*decode_filter*/NULL,
		   NULL,
		   NULL, 0);

	int	result = mad_decoder_run(&m_decoder);

	mad_decoder_finish(&m_decoder);

	while (!m_bKillThread && m_pOutput->IsPlaying())
	{
		Sleep(10);
	}

	m_state = PS_STOPED;

	m_pOutput->Stop();
	m_pOutput->Release();
	m_pOutput = NULL;

	m_pInput->Release();
	m_pInput = NULL;

	m_pMemAllocator->Release();
	m_pMemAllocator = NULL;

	m_pPlayer->NotifyEod(this, ERR_OK);
	m_pPlayer->Release();
	m_pPlayer = NULL;

	ERRLOG0(_T("End of decode thread! Quit."));

	return 0;
}

template<class T>
T* TNew(T* pointer)
{
	T* p = new T;
	return p;
}

//
// for xing header
//
#define FRAMES_FLAG     0x0001
#define BYTES_FLAG      0x0002
#define TOC_FLAG        0x0004
#define VBR_SCALE_FLAG  0x0008
#define FRAMES_AND_BYTES (FRAMES_FLAG | BYTES_FLAG)
#define MAX_XING_HEADER_LEN			(32 + 4 * 6 + 100)

static int ExtractI4(unsigned char const*buf)
{
    int x;
    // big endian extract

    x = buf[0];
    x <<= 8;
    x |= buf[1];
    x <<= 8;
    x |= buf[2];
    x <<= 8;
    x |= buf[3];

    return x;
}

MLRESULT GetXingOrVbriHeaderInfo(unsigned char const *frameBuff, struct mad_header *header, int &nFrameCount)
{
	int					nOffset;
	unsigned char const *buff;

	if (header->flags & MAD_FLAG_MPEG_V2)
		nOffset = header->mode == MAD_MODE_SINGLE_CHANNEL ? 9 : 17;
	else
		nOffset = header->mode == MAD_MODE_SINGLE_CHANNEL ? 17 : 32;

	nFrameCount = -1;
	buff = frameBuff + nOffset + 4;
	if (buff[0] == 'X' && buff[1] == 'i' && buff[2] == 'n' && buff[3] == 'g')
	{
		// xing header
		int		nHeadFlags;

		buff += 4;

		nHeadFlags = ExtractI4(buff);
		buff += 4;

		if (nHeadFlags & FRAMES_FLAG)
		{
			nFrameCount = ExtractI4(buff);
			buff += 4;
		}
// 		if (nHeadFlags & BYTES_FLAG)
// 		{
// 			nBytes = ExtractI4(buff);
// 			buff += 4;
// 		}
	}
	else if (buff[0] == 'V' && buff[1] == 'B' && buff[2] == 'R' && buff[3] == 'I')
	{
		// VBRI header
		buff += 10;

		nFrameCount = ExtractI4(buff);
	}

	if (nFrameCount <= 0)
		return ERR_NOT_FOUND;

	return ERR_OK;
}

int run_get_header(struct mad_decoder *decoder)
{
	enum mad_flow (*error_func)(void *, struct mad_stream *, struct mad_frame *);
	void *error_data;
	int bad_last_frame = 0;
	struct mad_stream *stream;
	struct mad_frame *frame;
	struct mad_synth *synth;
	int		result = 0;
	int		nFrameCheced = 0;

	stringex		buffXingHeader;
	GetHeadInfoParam	*pmdMad = (GetHeadInfoParam *)decoder->cb_data;
	MPEG_AUDIO_INFO		&audioInfo = pmdMad->m_audioInfo;
	struct mad_header	&header_prev = audioInfo.header;
	memset(&header_prev, 0, sizeof(header_prev));

	if (decoder->input_func == 0)
		return 0;

	if (decoder->error_func) {
		error_func = decoder->error_func;
		error_data = decoder->cb_data;
	}
	else {
		error_func = error_default;
		error_data = &bad_last_frame;
	}
	
	stream = &decoder->sync->stream;
	frame  = &decoder->sync->frame;
	synth  = &decoder->sync->synth;
	
	mad_stream_init(stream);
	mad_frame_init(frame);
	mad_synth_init(synth);
	
	mad_stream_options(stream, decoder->options);
	
#define MAX_RETYIES		32
	stream->error = MAD_ERROR_BUFLEN;
	for (int i = 0; i < MAX_RETYIES && stream->error == MAD_ERROR_BUFLEN; i++)
	{
		switch (decoder->input_func(decoder->cb_data, stream)) {
		case MAD_FLOW_STOP:
			goto done;
		case MAD_FLOW_BREAK:
			goto fail;
		case MAD_FLOW_IGNORE:
			continue;
		case MAD_FLOW_CONTINUE:
			break;
		}
		
		while (1) {
			if (mad_header_decode(&frame->header, stream) == -1) {
				if (!MAD_RECOVERABLE(stream->error))
					break;
				
				switch (error_func(error_data, stream, frame)) {
				case MAD_FLOW_STOP:
					goto done;
				case MAD_FLOW_BREAK:
					goto fail;
				case MAD_FLOW_IGNORE:
				case MAD_FLOW_CONTINUE:
				default:
					continue;
				}
			}

			struct mad_header		*header = &frame->header;

			if (nFrameCheced == 0)
			{
				// copy xing and vbri header
				int		n = int(stream->bufend - stream->this_frame);
				if (n > MAX_XING_HEADER_LEN)
					n = MAX_XING_HEADER_LEN;
				buffXingHeader.append((const char *)stream->this_frame, n);
				ASSERT((LPCSTR)stream->this_frame - pmdMad->m_bufInput.c_str() >= 0);
				audioInfo.nFirstFramePos = pmdMad->m_pInput->GetPos() + (int)((LPCSTR)stream->this_frame - pmdMad->m_bufInput.c_str()) - pmdMad->m_bufInput.size();
			}

			nFrameCheced++;

			if (header_prev.layer != header->layer || 
				header_prev.mode != header->mode ||
				header_prev.samplerate != header->samplerate)
				header_prev = *header;
			else
			{
				int				nPadding;
				if (header->flags & MAD_FLAG_PADDING)
					nPadding = 1;
				else
					nPadding = 0;
				if (header->layer == MAD_LAYER_I)
				{
					audioInfo.nSamplesPerFrame = 384;
					audioInfo.nFrameSize = (12 * header->bitrate / header->samplerate + nPadding) * 4;
				}
				else
				{
					if (header->flags & MAD_FLAG_MPEG_V2)
					{
						// LSF
						audioInfo.nSamplesPerFrame = 576;
						audioInfo.nFrameSize = 72 * header->bitrate / header->samplerate + nPadding;
					}
					else
					{
						audioInfo.nSamplesPerFrame = 1152;
						audioInfo.nFrameSize = 144 * header->bitrate / header->samplerate + nPadding;
					}
				}

				pmdMad->m_pInput->GetSize(audioInfo.nMediaFileSize);
				if (GetXingOrVbriHeaderInfo((unsigned char const *)buffXingHeader.c_str(),
					header, audioInfo.nFrameCount) == ERR_OK)
				{
					if (audioInfo.nFrameCount > 0)
					{
						audioInfo.bVbr = true;
						audioInfo.nMediaLength = (int)((double)audioInfo.nSamplesPerFrame * audioInfo.nFrameCount * 1000 / header->samplerate);
						ASSERT(audioInfo.nMediaLength > 0);
						if (audioInfo.nMediaLength <= 0)
							audioInfo.nMediaLength = 1;
						audioInfo.nFrameSize = (audioInfo.nMediaFileSize - audioInfo.nFirstFramePos) / audioInfo.nFrameCount;
						audioInfo.nBitRate = (int)(((double)audioInfo.nMediaFileSize - audioInfo.nFirstFramePos) * 8 * 1000 / audioInfo.nMediaLength);
						goto done;
					}
				}
				else
				{
					audioInfo.bVbr = false;
					audioInfo.nFrameCount = (audioInfo.nMediaFileSize - audioInfo.nFirstFramePos) / audioInfo.nFrameSize;
					audioInfo.nMediaLength = int((double)audioInfo.nSamplesPerFrame * audioInfo.nFrameCount * 1000 / header->samplerate);
					audioInfo.nBitRate = audioInfo.header.bitrate;
					goto done;
				}
			}
		}
	}

fail:
	result = -1;
	
done:
	mad_synth_finish(synth);
	mad_frame_finish(frame);
	mad_stream_finish(stream);
	
	return result;
}

/*
 * NAME:	decoder->run()
 * DESCRIPTION:	run the decoder thread either synchronously or asynchronously
 */
int CMDLibmad::mad_decoder_run(struct mad_decoder *decoder)
{
	int result;

	// decoder->sync = ((sync*)malloc(sizeof(*decoder->sync));
	decoder->sync = TNew(decoder->sync);
	if (decoder->sync == 0)
		return -1;

	result = run_sync(decoder);

	delete decoder->sync;
	decoder->sync = 0;

	return result;
}

int CMDLibmad::run_sync(struct mad_decoder *decoder)
{
	enum mad_flow (*error_func)(void *, struct mad_stream *, struct mad_frame *);
	void *error_data;
	int bad_last_frame = 0;
	struct mad_stream *stream;
	struct mad_frame *frame;
	struct mad_synth *synth;

	IFBuffer		*pBuf = NULL;
	int				nChannels = 0, nSampleRate = 0;

	int result = 0;

	if (decoder->input_func == 0)
		return 0;

	if (decoder->error_func) {
		error_func = decoder->error_func;
		error_data = decoder->cb_data;
	}
	else {
		error_func = error_default;
		error_data = &bad_last_frame;
	}

	stream = &decoder->sync->stream;
	frame  = &decoder->sync->frame;
	synth  = &decoder->sync->synth;

	mad_stream_init(stream);
	mad_frame_init(frame);
	mad_synth_init(synth);

	mad_stream_options(stream, decoder->options);
	
	do {
		switch (decoder->input_func(decoder->cb_data, stream)) {
		case MAD_FLOW_STOP:
			goto done;
		case MAD_FLOW_BREAK:
			goto fail;
		case MAD_FLOW_IGNORE:
			continue;
		case MAD_FLOW_CONTINUE:
			break;
		}

    while (1) {
      if (decoder->header_func) {
	if (mad_header_decode(&frame->header, stream) == -1) {
	  if (!MAD_RECOVERABLE(stream->error))
	    break;

	  switch (error_func(error_data, stream, frame)) {
	  case MAD_FLOW_STOP:
	    goto done;
	  case MAD_FLOW_BREAK:
	    goto fail;
	  case MAD_FLOW_IGNORE:
	  case MAD_FLOW_CONTINUE:
	  default:
	    continue;
	  }
	}

	switch (decoder->header_func(decoder->cb_data, &frame->header)) {
	case MAD_FLOW_STOP:
	  goto done;
	case MAD_FLOW_BREAK:
	  goto fail;
	case MAD_FLOW_IGNORE:
	  continue;
	case MAD_FLOW_CONTINUE:
	  break;
	}
      }

      if (mad_frame_decode(frame, stream) == -1) {
	if (!MAD_RECOVERABLE(stream->error))
	  break;

	switch (error_func(error_data, stream, frame)) {
	case MAD_FLOW_STOP:
	  goto done;
	case MAD_FLOW_BREAK:
	  goto fail;
	case MAD_FLOW_IGNORE:
	  break;
	case MAD_FLOW_CONTINUE:
	default:
	  continue;
	}
      }
      else
	bad_last_frame = 0;

      if (decoder->filter_func) {
	switch (decoder->filter_func(decoder->cb_data, stream, frame)) {
	case MAD_FLOW_STOP:
	  goto done;
	case MAD_FLOW_BREAK:
	  goto fail;
	case MAD_FLOW_IGNORE:
	  continue;
	case MAD_FLOW_CONTINUE:
	  break;
	}
      }

      mad_synth_frame(synth, frame);

	  struct mad_header const *header = &frame->header;
	  struct mad_pcm *pcm = &synth->pcm;
	  mad_fixed_t const *ch1, *ch2;

	  ch1 = pcm->samples[0];
	  ch2 = pcm->samples[1];

	  if (pcm->channels == 1)
		  ch2 = 0;

	  if (nChannels != pcm->channels || 
		  nSampleRate != pcm->samplerate)
	  {
		  DBGLOG1(_T("output: decoded sample frequency %u Hz"),pcm->samplerate);

		  if (m_pOutput->IsOpened())
		  {
			  // if (pBuf && OutputWrite(pBuf, nChannels, nSampleRate))
			  //	  pBuf = NULL;

			  // while (m_pOutput->IsPlaying())
			  //	  Sleep(10);

			  // m_nSeekPos = GetPos();
			  m_pOutput->Stop();
		  }
		  m_pOutput->Open(pcm->samplerate, pcm->channels, BPS);
		  nChannels = pcm->channels;
		  nSampleRate = pcm->samplerate;
	  }

	  pBuf = m_pMemAllocator->AllocFBuffer(DECODE_BUFF_SIZE);
	  uint32 len = audio_pcm_s16le((unsigned char *)pBuf->data() + pBuf->size(), pcm->length,
		  ch1, ch2, AUDIO_MODE_DITHER, &m_audioStats);
	  pBuf->resize(len);
	  m_pOutput->WaitForWrite();

	  if (m_bKillThread)
	  {
		  pBuf->Release();
		  goto done;
	  }

	  if (m_bSeekFlag)
	  {
		  pBuf->Release();
		  break;
	  }

	  m_pPlayer->OutputWrite(pBuf, BPS, pcm->channels, pcm->samplerate);
    }
  }
  while (stream->error == MAD_ERROR_BUFLEN);

 fail:
  result = -1;

 done:
  if (!m_bKillThread)
  {
	  while (m_pOutput->IsPlaying())
		  Sleep(10);
  }

  mad_synth_finish(synth);
  mad_frame_finish(frame);
  mad_stream_finish(stream);

  return result;
}

/*
bool CMDLibmad::OutputWrite(IFBuffer *pBuf, int nChannels, int nSampleRate)
{
	m_pOutput->WaitForWrite();

	if (m_bKillThread)
		return false;

	if (m_bSeekFlag)
	{
		pBuf->Release();
		return true;
	}

	if (m_pPlayer->IsDspActive())
		m_pPlayer->DspProcess(pBuf, BPS, nChannels, nSampleRate);

	m_pPlayer->AddVisData(pBuf, BPS, nChannels, nSampleRate);
	m_pOutput->Write(pBuf);

	return true;
}*/

MLRESULT CMDLibmad::GetHeadInfo(IMediaInput *pInput)
{
	m_bufInput.clear();

	int					result = -1;
	GetHeadInfoParam	param;
	param.m_pInput = pInput;
	param.m_bufInput.reserve(INPUT_BUFF_SIZE);
	memset(&param.m_audioInfo, 0, sizeof(param.m_audioInfo));

	mad_decoder_init(&m_decoder, &param,
		   GetHeadInfo_decode_input_read,
		   NULL, /*decode_filter*/NULL,
		   NULL,
		   NULL, 0);

	m_decoder.sync = TNew(m_decoder.sync);
	if (m_decoder.sync)
	{
		result = run_get_header(&m_decoder);

		delete m_decoder.sync;
		m_decoder.sync = 0;

		m_audioInfo = param.m_audioInfo;

	}

	mad_decoder_finish(&m_decoder);

	if (result == 0)
		return ERR_OK;
	else
		return ERR_PLAYER_INVALID_FILE;
}
