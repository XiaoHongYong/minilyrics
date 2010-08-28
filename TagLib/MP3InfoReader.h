#pragma once

class CMP3InfoReader
{
public:
	enum MPAVersion
	{
		MPEG25 = 0,
		MPEGReserved,
		MPEG2,
		MPEG1		
	}m_Version;

	enum MPALayer
	{
		Layer1,
		Layer2,
		Layer3,
		LayerReserved
	}m_Layer;

	enum Emphasis
	{
		EmphNone = 0,
		Emph5015,
		EmphReserved,
		EmphCCITJ17
	}m_Emphasis;

	enum ChannelMode
	{
		Stereo,
		JointStereo,
		DualChannel,
		SingleChannel
	}m_ChannelMode;

	enum { PRE_READ_BUFF_SIZE	= 256 };

public:
	CMP3InfoReader(void);
	~CMP3InfoReader(void);

	void Attach(FILE *fp) { m_fp = fp; }

	// in ms
	int GetMediaLength();

protected:
	int FindFirstFrame(byte byHeader[PRE_READ_BUFF_SIZE]);

	int GetVbrInfoFrameCount(unsigned char const *frameBuff, int nBuffLen);

protected:
	FILE		*m_fp;

};
