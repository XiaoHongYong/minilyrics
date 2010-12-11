#pragma once

#include <mmsystem.h>

class CVolumeOutMaster
{
public:
	CVolumeOutMaster();
	virtual ~CVolumeOutMaster();

	int GetVolume();
	void SetVolume(int nValue);

	bool Init(HWND hWndVolumeNotify);
	void Quit();

protected:
	bool		m_bEnabled;
	HMIXER		m_hMixer;
	UINT		m_uMixerID;

	DWORD	m_dwVolumeControlID;
	int		m_nChannelCount;

	DWORD	m_dwMinimalVolume;
	DWORD	m_dwMaximalVolume;

};
