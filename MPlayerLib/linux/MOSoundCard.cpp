// MOSoundCard.cpp: implementation of the CMOSoundCard class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IMPlayer.h"
#include "MOSoundCard.h"

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>
#include <alsa/pcm_plugin.h>

#include <dlfcn.h>
#include <ctype.h>

struct alsa_config
{
	const char *pcm_device;
	int mixer_card;
	const char *mixer_device;
	int buffer_time;
	int period_time;
	gboolean debug;
	gboolean mmap;
	struct
	{
		int left, right;
	} vol;
	gboolean soft_volume;
};

alsa_config g_alsa_cfg;

static snd_pcm_t *sound_handle = NULL;
static snd_output_t *errlog;
static snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
static int frag_size = 2048;
static int frag_count = 16;
static int nr_channels = 2;
static unsigned int output_rate = 44100;

CMOSoundCard::CMOSoundCard() : m_eventCanWrite(false, true)
{
	OBJ_REFERENCE_INIT;

	m_dwTotolBytesOffset = 0;

	m_fadeMode = FADE_NONE;
	m_bWrittingPausedData = false;

	memset(&g_alsa_cfg, 0, sizeof (g_alsa_cfg));
	g_alsa_cfg.buffer_time = 500;
	g_alsa_cfg.period_time = 50;
	g_alsa_cfg.debug = 0;
	g_alsa_cfg.mmap = 1;
	g_alsa_cfg.vol.left = 100;
	g_alsa_cfg.vol.right = 100;
	g_alsa_cfg.pcm_device = "default";
	g_alsa_cfg.mixer_device = "PCM";

	if (dlopen("libasound.so.2", RTLD_NOW | RTLD_GLOBAL) == NULL)
	{
		g_message("Cannot load alsa library: %s", dlerror());
		return;
	}

	//
	// Open sound device
	//
	int		err;
	err = snd_pcm_open(&sound_handle, g_alsa_cfg.pcm_device, stream, 0);
	if (err < 0)
	{
		ERRLOG2("snd_pcm_open: %s (%s)", snd_strerror(err), g_alsa_cfg.pcm_device);
		return;
	}
	err = snd_output_stdio_attach(&errlog, stderr, 0);
	if (err < 0) {
		ERRLOG1("snd_output_stdio_attach: %s", snd_strerror(err));
		return;
	}
}

CMOSoundCard::~CMOSoundCard()
{
	if (sound_handle) {
		snd_pcm_drain(sound_handle);
		snd_pcm_close(sound_handle);
		sound_handle = NULL;
	}	
	if (errlog)
		snd_output_close(errlog);
}

OBJ_REFERENCE_IMP(CMOSoundCard)

LPCXSTR CMOSoundCard::GetDescription()
{
	return _T("MPlayer soundcard output 1.0");
}

MLRESULT CMOSoundCard::Open(int nSampleRate, int nNumChannels, int nBitsPerSamp)
{
	DBGLOG3(_T("Open, SapmleRate: %d, Channels: %d, BitsPerSample: %d"), nSampleRate, nNumChannels, nBitsPerSamp);

	m_fadeMode = FADE_NONE;

	m_nChannels = nNumChannels;
	m_nSamplerate = nSampleRate;
	m_nBitsPerSamp = nBitsPerSamp;

	ASSERT(nBitsPerSamp == 16);

	int		err;
	snd_pcm_hw_params_t *hw_params;

	/* Allocate Hardware Parameters structures and fills it with config space for PCM */
	err = snd_pcm_hw_params_malloc (&hw_params);
	if (err < 0)
	{
		ERRLOG1("snd_pcm_hw_params_malloc failed (%s).", snd_strerror(err));
		return ERR_NO_MEM;
	}

	err = snd_pcm_hw_params_any (sound_handle, hw_params);
	{
		ERRLOG1("cannot initialize hardware parameter structure(%s).", snd_strerror(err));
		return ERR_SOUND_DEVICE_OPEN;
	}

	/* Set parameters : interleaved channels, 16 bits little endian, 44100Hz, 2 channels */
	err = snd_pcm_hw_params_set_access (sound_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	{
		ERRLOG1("cannot set access type (%s).", snd_strerror(err));
		return ERR_SOUND_DEVICE_OPEN;
	}
	err = snd_pcm_hw_params_set_format (sound_handle, hw_params, SND_PCM_FORMAT_S16_LE);
	{
		ERRLOG1("cannot set sample format (%s).", snd_strerror(err));
		return ERR_SOUND_DEVICE_OPEN;
	}
	err = snd_pcm_hw_params_set_rate_near (sound_handle, hw_params, (unsigned int *)&nSampleRate, 0);
	{
		ERRLOG1("cannot set sample rate (%s).", snd_strerror(err));
		return ERR_SOUND_DEVICE_OPEN;
	}
	err = snd_pcm_hw_params_set_channels (sound_handle, hw_params, nNumChannels);
	{
		ERRLOG1("cannot set channel count (%s).", snd_strerror(err));
		return ERR_SOUND_DEVICE_OPEN;
	}

	/* Assign them to the playback handle and free the parameters structure */
	err = snd_pcm_hw_params (sound_handle, hw_params);
	{
		ERRLOG1("cannot set parameters (%s).", snd_strerror(err));
		return ERR_SOUND_DEVICE_OPEN;
	}
	snd_pcm_hw_params_free (hw_params);

	/* Prepare & Play */
	err = snd_pcm_prepare (sound_handle);
	{
		ERRLOG1("cannot prepare audio interface for use (%s).", snd_strerror(err));
		return ERR_SOUND_DEVICE_OPEN;
	}

	return ERR_OK;
}

MLRESULT CMOSoundCard::WaitForWrite()
{
	/* wait till the interface is ready for data, or 1 second
	has elapsed.
	*/
	int		err;
	if ((err = snd_pcm_wait (sound_handle, 1000)) < 0) {
		fprintf (stderr, "poll failed (%s)\n", strerror (errno));
		return ERR_FALSE;
	}

	return ERR_OK;
}

MLRESULT CMOSoundCard::Write(IFBuffer *pBuf)
{
	snd_pcm_uframes_t fcount;
	int err;

	DBGLOG1("Write sound data: %d", pBuf->size());

	fcount = (snd_pcm_uframes_t) (pBuf->size() / 4);
	if (!sound_handle)
	{
		ERRLOG0("Sound handle is NULL.");
		return ERR_SOUND_DEVICE_WRITE;
	}
	err = snd_pcm_writei(sound_handle, pBuf->data(), fcount);
// 	if (err < 0)
// 	{
// 		if (xrun_recovery(sound_handle, err) < 0)
// 		{
// 			ERRLOG0("alsa: xrun_recovery failed.");
// 			return ERR_SOUND_DEVICE_WRITE;
// 		}
// 		err = snd_pcm_writei(sound_handle, pBuf->data(), fcount);
// 		if (err < 0)
// 		{
// 			if (xrun_recovery(sound_handle, err) < 0)
// 			{
// 				ERRLOG0("alsa: xrun_recovery failed. again");
// 				return ERR_SOUND_DEVICE_WRITE;
// 			}
// 		}
// 	}
	pBuf->Release();

	return ERR_OK;
}

MLRESULT CMOSoundCard::Flush()
{
	m_eventCanWrite.Set();

	return ERR_OK;
}

MLRESULT CMOSoundCard::Pause(bool bPause)
{
	return ERR_OK;
}

bool CMOSoundCard::IsPlaying()
{
	return false;
}

MLRESULT CMOSoundCard::Stop()
{	return ERR_OK;
}

bool CMOSoundCard::IsOpened()
{
	return false;
}

// volume
MLRESULT CMOSoundCard::SetVolume(int nVolume, int nBanlance)
{
	return ERR_FALSE;
}

#define MAXINT32 0x7FFFFFFF

uint32 CMOSoundCard::GetPos()
{
	return 0;
}

