#include "StdAfx.h"
#include "../MPlayerLib/IMPlayer.h"
#include "VolumeOutMaster.h"

////////////////////////////////////////////////////////////

//{{{ Audio specific functions
#define AUDFREQ			22050	// Frequency
#define AUDCHANNELS		1		// Number of channels
#define AUDBITSMPL		16		// Number of bits per sample
inline
void SetDeviceType(WAVEFORMATEX* pwfe)
{
	memset(pwfe, 0, sizeof(WAVEFORMATEX));
	WORD  nBlockAlign = (AUDCHANNELS*AUDBITSMPL)/8;
	DWORD nSamplesPerSec = AUDFREQ;
	pwfe->wFormatTag = WAVE_FORMAT_PCM;
	pwfe->nChannels = AUDCHANNELS;
	pwfe->nBlockAlign = nBlockAlign;
	pwfe->nSamplesPerSec = nSamplesPerSec;
	pwfe->wBitsPerSample = AUDBITSMPL;
	pwfe->nAvgBytesPerSec = nSamplesPerSec*nBlockAlign;
}
//}}} Audio specific functions

/////////////////////////////////////////////////////////////////////////////
// 		Implementation
//////////////
CVolumeOutMaster::CVolumeOutMaster()
{
	m_bEnabled = false;
	m_hMixer = NULL;
	m_uMixerID = 0;

}
//////////////
CVolumeOutMaster::~CVolumeOutMaster()
{
	Quit();
}

int CVolumeOutMaster::GetVolume()
{
	if (!m_bEnabled)
		return 0;

	MIXERCONTROLDETAILS_UNSIGNED* aDetails = (MIXERCONTROLDETAILS_UNSIGNED*)malloc(m_nChannelCount*sizeof(MIXERCONTROLDETAILS_UNSIGNED));
	if (!aDetails)
		return 0;
	MIXERCONTROLDETAILS ControlDetails;
	memset(&ControlDetails, 0, sizeof(MIXERCONTROLDETAILS));
	ControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	ControlDetails.dwControlID = m_dwVolumeControlID;
	ControlDetails.cChannels = m_nChannelCount;
	ControlDetails.cMultipleItems = 0;
	ControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	ControlDetails.paDetails = &aDetails[0];
	MMRESULT mmResult = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &ControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	DWORD dw = aDetails[0].dwValue;
	free(aDetails);
	if (mmResult != MMSYSERR_NOERROR)
		return 0;

	if (m_dwMaximalVolume - m_dwMinimalVolume > 0)
		return dw * MP_VOLUME_MAX / (m_dwMaximalVolume - m_dwMinimalVolume);
	else
		return 0;
}

void CVolumeOutMaster::SetVolume(int nValue)
{
	if (!m_bEnabled)
		return;

	nValue = nValue * (m_dwMaximalVolume - m_dwMinimalVolume) / MP_VOLUME_MAX;

	MIXERCONTROLDETAILS_UNSIGNED* aDetails = (MIXERCONTROLDETAILS_UNSIGNED*)malloc(m_nChannelCount*sizeof(MIXERCONTROLDETAILS_UNSIGNED));
	if (!aDetails)
		return;
	for (int i = 0; i < m_nChannelCount; i++)
	{
		aDetails[i].dwValue = nValue;
	}
	MIXERCONTROLDETAILS ControlDetails;
	memset(&ControlDetails, 0, sizeof(MIXERCONTROLDETAILS));
	ControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	ControlDetails.dwControlID = m_dwVolumeControlID;
	ControlDetails.cChannels = m_nChannelCount;
	ControlDetails.cMultipleItems = 0;
	ControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	ControlDetails.paDetails = &aDetails[0];
	MMRESULT mmResult = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &ControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	free(aDetails);
}

bool CVolumeOutMaster::Init(HWND hWndVolumeNotify)
{
	if (!mixerGetNumDevs())
		return false;

	HWAVEOUT hwaveOut;
	MMRESULT mmResult;
	WAVEFORMATEX WaveFmt;
	SetDeviceType(&WaveFmt);
	mmResult = waveOutOpen(&hwaveOut, WAVE_MAPPER, &WaveFmt, 0L, 0L, CALLBACK_NULL);
	if (mmResult != MMSYSERR_NOERROR)
		return false;
	else
	{
		mmResult = mixerGetID((HMIXEROBJ)hwaveOut, &m_uMixerID, MIXER_OBJECTF_HWAVEOUT);
		waveOutClose(hwaveOut);
		if (mmResult != MMSYSERR_NOERROR)
			return false;
	}

	// Exposing Window to Mixer
	mmResult = mixerOpen((LPHMIXER)&m_hMixer, m_uMixerID, (DWORD)hWndVolumeNotify, 0L, CALLBACK_WINDOW);
	if (mmResult != MMSYSERR_NOERROR)
		return false;

	MIXERLINE MixerLine = { 0 };
	MixerLine.cbStruct = sizeof(MIXERLINE);
	MixerLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
	mmResult = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &MixerLine, MIXER_GETLINEINFOF_COMPONENTTYPE);
	if (mmResult != MMSYSERR_NOERROR)
		return false;

	MIXERCONTROL Control;
	memset(&Control, 0, sizeof(MIXERCONTROL));
	Control.cbStruct = sizeof(MIXERCONTROL);

	MIXERLINECONTROLS LineControls;
	memset(&LineControls, 0, sizeof(MIXERLINECONTROLS));
	LineControls.cbStruct = sizeof(MIXERLINECONTROLS);

	LineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	LineControls.dwLineID = MixerLine.dwLineID;
	LineControls.cControls = 1;
	LineControls.cbmxctrl = sizeof(MIXERCONTROL);
	LineControls.pamxctrl = &Control;
	mmResult = mixerGetLineControls((HMIXEROBJ)m_hMixer, &LineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE);
	if (mmResult == MMSYSERR_NOERROR)
	{
		if (!(Control.fdwControl & MIXERCONTROL_CONTROLF_DISABLED))
		{
			m_bEnabled = true;
		}
	}

	m_nChannelCount = MixerLine.cChannels;
	m_dwVolumeControlID = Control.dwControlID;
	m_dwMinimalVolume = Control.Bounds.dwMinimum;
	m_dwMaximalVolume = Control.Bounds.dwMaximum;

	return true;
}

void CVolumeOutMaster::Quit()
{
	if (m_hMixer)
	{
		mixerClose((HMIXER)m_hMixer);
		m_hMixer = NULL;
	}
}
