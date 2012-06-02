#include "StdAfx.h"
#include "MP3InfoReader.h"
#include "../ID3Lib/ID3v2.h"

class CFileStream
{
public:
	CFileStream(FILE *fp);

	inline byte ReadChar() {
		ASSERT(m_nBufPos < m_buf.size());
		return (byte)(m_buf.data()[m_nBufPos++]);
	}

	inline char *ReadBuff(int nSize) {
		ASSERT(m_nBufPos + nSize <= m_buf.size());
		char *p = m_buf.data() + m_nBufPos;
		m_nBufPos += nSize;
		return p;
	}

	bool Seek(long offset, int origin) { m_nBufPos = 0; m_buf.resize(0); return fseek(m_fp, offset, origin) == 0; }
	void BufferSeek(long offset);

	size_t GetBufferedSize() { return m_buf.size() - m_nBufPos; }

	int GetFilePos() { return ftell(m_fp) + m_nBufPos - m_buf.size(); }

	bool FeedBuffer();

protected:
	FILE			*m_fp;
	CCharBuffer		m_buf;
	size_t			m_nBufPos;

};

CFileStream::CFileStream(FILE *fp)
{
	m_fp = fp;
	fseek(fp, 0, SEEK_SET);
	m_buf.reserve(1024 * 4);
	m_nBufPos = 0;
}

void CFileStream::BufferSeek(long offset)
{
	// int pos = (int)m_nBufPos;
	m_nBufPos += offset;
	ASSERT(m_nBufPos >= 0 && m_nBufPos < (int)m_buf.size());
}

bool CFileStream::FeedBuffer()
{
	int nRemainSize = m_buf.size() - m_nBufPos;
	if (nRemainSize > 0)
		memmove(m_buf.data(), m_buf.data() + m_nBufPos, nRemainSize);

	int nRead = fread(m_buf.data() + nRemainSize, 1, m_buf.capacity() - nRemainSize, m_fp);
	if (nRead <= 0)
		return false;
	m_buf.resize(nRemainSize + nRead);
	m_nBufPos = 0;

	return true;
}


// sampling rates in hertz: 1. index = MPEG Version ID, 2. index = sampling rate index
const DWORD m_dwSamplingRates[4][3] = 
{ 
	{11025, 12000, 8000,  },	// MPEG 2.5
	{0,     0,     0,     },	// reserved
	{22050, 24000, 16000, },	// MPEG 2
	{44100, 48000, 32000  }		// MPEG 1
};

// bitrates: 1. index = LSF, 2. index = Layer, 3. index = bitrate index
const DWORD m_dwBitrates[2][3][15] =
{
	{	// MPEG 1
		{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},	// Layer1
		{0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},	// Layer2
		{0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,}	// Layer3
	},
	{	// MPEG 2, 2.5		
		{0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},		// Layer1
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},			// Layer2
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,}			// Layer3
	}
};

// Samples per Frame: 1. index = LSF, 2. index = Layer
const DWORD m_dwSamplesPerFrames[2][3] =
{
	{	// MPEG 1
		384,	// Layer1
			1152,	// Layer2	
			1152	// Layer3
	},
	{	// MPEG 2, 2.5
		384,	// Layer1
			1152,	// Layer2
			576		// Layer3
		}	
};

// Samples per Frame / 8
const DWORD m_dwCoefficients[2][3] =
{
	{	// MPEG 1
		12,		// Layer1	(must be multiplied with 4, because of slot size)
			144,	// Layer2
			144		// Layer3
	},
	{	// MPEG 2, 2.5
		12,		// Layer1	(must be multiplied with 4, because of slot size)
			144,	// Layer2
			72		// Layer3
		}	
};

// slot size per layer
const DWORD m_dwSlotSizes[3] =
{
	4,			// Layer1
	1,			// Layer2
	1			// Layer3
};

// size of side information (only for Layer III)
// 1. index = LSF, 2. index = mono
const DWORD m_dwSideInfoSizes[2][2] =
{
	// MPEG 1
	{32,17},
	// MPEG 2/2.5
	{17,9}
};

// allowed combination of bitrate (1.index) and mono (2.index)
const bool m_bAllowedModes[15][2] =
{
	// {stereo, intensity stereo, dual channel allowed,single channel allowed}
	{true,true},		// free mode
	{false,true},		// 32
	{false,true},		// 48
	{false,true},		// 56
	{true,true},		// 64
	{false,true},		// 80
	{true,true},		// 96
	{true,true},		// 112
	{true,true},		// 128
	{true,true},		// 160
	{true,true},		// 192
	{true,false},		// 224
	{true,false},		// 256
	{true,false},		// 320
	{true,false}		// 384
};

CMP3InfoReader::CMP3InfoReader(void)
{
}

CMP3InfoReader::~CMP3InfoReader(void)
{
}

int CMP3InfoReader::FindFirstFrame(byte byHeader[PRE_READ_BUFF_SIZE])
{
	const int		HEADER_TAG_SIZE = 3;
	CFileStream		stream(m_fp);

	if (!stream.FeedBuffer())
		return -1;

	// ID3tag, ignore it.
	if (stream.GetBufferedSize() > sizeof(ID3v2Header))
	{
		ID3v2Header *pHeader = (ID3v2Header*)stream.ReadBuff(sizeof(ID3v2Header));
		if (strncmp(pHeader->szID, ID3_TAGID, ID3_TAGIDSIZE) == 0)
		{
			int nHeaderLen = SynchDataToUInt(pHeader->bySize, CountOf(pHeader->bySize)) + ID3_TAGHEADERSIZE;

			stream.Seek(nHeaderLen, SEEK_SET);
		}
	}

	do
	{
		while (stream.GetBufferedSize() > HEADER_TAG_SIZE)
		{
			// Here we have to loop because there could be several of the first
			// (11111111) byte, and we want to check all such instances until we find
			// a full match (11111111 111) or hit the end of the buffer.
			if (stream.ReadChar() == 0xFF
				// && stream.ReadChar() == 0xFF
				&& (stream.ReadChar() & 0xE0) == 0xE0)
			{
				stream.BufferSeek(-2);
				if (stream.GetBufferedSize() < PRE_READ_BUFF_SIZE)
					stream.FeedBuffer();
				memcpy(byHeader, stream.ReadBuff(PRE_READ_BUFF_SIZE), PRE_READ_BUFF_SIZE);
				return stream.GetFilePos() - PRE_READ_BUFF_SIZE;
			}
		}
	}
	while (stream.FeedBuffer());

	return -1;
}

int CMP3InfoReader::GetMediaLength()
{
	byte	byHeader[PRE_READ_BUFF_SIZE];

	int		nHeaderPos = FindFirstFrame(byHeader);
	if (nHeaderPos == -1)
		return 0;

	DWORD m_dwSamplesPerSec;
	DWORD m_dwSamplesPerFrame;
	DWORD m_dwBitrate;	// in bit per second (1 kb = 1000 bit, not 1024)
	DWORD m_dwPaddingSize;
	WORD m_wBound;		// only valid for intensity stereo
	WORD m_wAllocationTableIndex;	// only valid for MPEG 1 Layer II (0=table a, 1=table b,...)

	// flags
	bool m_bCopyright, m_bPrivate, m_bOriginal;
	bool m_bCRC; 
	BYTE m_ModeExt;
	bool m_bLSF;		// true means lower sampling frequencies (=MPEG2/MPEG2.5)


	// Get MPEG version [bit 11,12]
	m_Version = (MPAVersion)((byHeader[1] >> 3) & 0x03);	// mask only the rightmost 2 bits
	if (m_Version == MPEGReserved)
		return 0;

	if (m_Version == MPEG1)
		m_bLSF = false;
	else
		m_bLSF = true;

	// get layer (0 = layer1, 2 = layer2, ...)  [bit 13,14]
	m_Layer = (MPALayer)(3 - ((byHeader[1] >> 1) & 0x03));	
	if (m_Layer == LayerReserved)
		return 0;

	// protection bit (inverted) [bit 15]
	m_bCRC = !((byHeader[1]) & 0x01);

	// bitrate [bit 16..19]
	BYTE bBitrateIndex = (BYTE)((byHeader[2] >> 4) & 0x0F);
	if (bBitrateIndex == 0x0F)		// all bits set is reserved
		return 0;
	m_dwBitrate = m_dwBitrates[m_bLSF][m_Layer][bBitrateIndex] * 1000; // convert from kbit to bit

	if (m_dwBitrate == 0)	// means free bitrate (is unsupported yet)
		return 0;

	// sampling rate [bit 20,21]
	BYTE bIndex = (BYTE)((byHeader[2] >> 2) & 0x03);
	if (bIndex == 0x03)		// all bits set is reserved
		return 0;
	m_dwSamplesPerSec = m_dwSamplingRates[m_Version][bIndex];

	// padding bit [bit 22]
	m_dwPaddingSize = 1 * ((byHeader[2] >> 1) & 0x01);	// in Slots (always 1)

	m_dwSamplesPerFrame = m_dwSamplesPerFrames[m_bLSF][m_Layer];

	// private bit [bit 23]
	m_bPrivate = (byHeader[2]) & 0x01;

	// channel mode [bit 24,25]
	m_ChannelMode = (ChannelMode)((byHeader[3] >> 6) & 0x03);

	// mode extension [bit 26,27]
	m_ModeExt = (BYTE)((byHeader[3] >> 4) & 0x03);

	// determine the bound for intensity stereo
	if (m_ChannelMode == JointStereo)
		m_wBound = 4 + m_ModeExt * 4;

	// copyright bit [bit 28]
	m_bCopyright = (byHeader[3] >> 3) & 0x01;

	// original bit [bit 29]
	m_bOriginal = (byHeader[3] >> 2) & 0x01;

	// emphasis [bit 30,31]
	m_Emphasis = (Emphasis)((byHeader[3]) & 0x03);
	if (m_Emphasis == EmphReserved)
		return 0;

	// extended check for Layer II
	if (m_Layer == Layer2)
	{
		// MPEG 1
		if (m_Version == MPEG1)
		{
			if (!m_bAllowedModes[bBitrateIndex][m_ChannelMode == SingleChannel])
				return 0;	

			// which allocation table is used
			switch (m_dwBitrate/1000/(m_ChannelMode == SingleChannel?1:2))
			{
			case 32:
			case 48:
				if (m_dwSamplesPerSec == 32000)
					m_wAllocationTableIndex = 3;	// table d
				else
					m_wAllocationTableIndex = 2;	// table c
				break;
			case 56:
			case 64:
			case 80:
				if (m_dwSamplesPerSec != 48000)
				{
					m_wAllocationTableIndex = 0;	// table a
					break;
				}
			case 96:
			case 112:
			case 128:
			case 160:
			case 192:
				if (m_dwSamplesPerSec != 48000)
				{
					m_wAllocationTableIndex = 1;	// table b
					break;
				}
				else
					m_wAllocationTableIndex = 0;	// table a
				break;
			}
		}
		else	// MPEG 2/2.5
			m_wAllocationTableIndex = 4;
	}

	fseek(m_fp, 0, SEEK_END);
	int nFileSize = ftell(m_fp);

	int nSamplesPerFrame = m_dwSamplesPerFrames[m_bLSF][m_Layer];;
	int nFrameSize = (((m_dwCoefficients[m_bLSF][m_Layer] * m_dwBitrate / m_dwSamplesPerSec) + m_dwPaddingSize)) * m_dwSlotSizes[m_Layer];

	int nFrameCount = GetVbrInfoFrameCount(byHeader, PRE_READ_BUFF_SIZE);
	if (nFrameCount < 0)
		nFrameCount = (nFileSize - nHeaderPos) / nFrameSize;
	int nMediaLength = int((double)nSamplesPerFrame * nFrameCount * 1000 / m_dwSamplesPerSec);

	return nMediaLength;
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

class CBigEndianBuffReader
{
public:
	class COutOfRangeException {
	public:
		COutOfRangeException() {}
	};

	CBigEndianBuffReader(const byte *buf, int nLen) : m_buf(buf)
	{
		m_bufEndPos = m_buf + nLen;
	}

	int ExtractI1()
	{
		if (m_buf + 1 > m_bufEndPos)
			throw COutOfRangeException();

		m_buf++;

		return m_buf[0];
	}

	int ExtractIx(int nBytes)
	{
		if (m_buf + nBytes > m_bufEndPos)
			throw COutOfRangeException();

		int x = m_buf[0];
		// big endian extract

		for (int i = 1; i < nBytes; i++)
		{
			x <<= 8;
			x |= m_buf[i];
		}

		m_buf += nBytes;

		return x;
	}

	void Advance(int nBytes)
	{
		m_buf += nBytes;
		if (m_buf > m_bufEndPos)
			throw COutOfRangeException();
	}

protected:
	const byte		*m_buf, *m_bufEndPos;

};


int CMP3InfoReader::GetVbrInfoFrameCount(unsigned char const *frameBuff, int nBuffLen)
{
	int					nOffset;
	unsigned char const *buff;

	if (m_Version == MPEG2)
		nOffset = m_ChannelMode == SingleChannel ? 9 : 17;
	else
		nOffset = m_ChannelMode == SingleChannel ? 17 : 32;
	nOffset += 4;

	buff = frameBuff + nOffset;

	int		nFrameCount = -1;

	if ((buff[0] == 'X' && buff[1] == 'i' && buff[2] == 'n' && buff[3] == 'g')
		|| (buff[0] == 'I' && buff[1] == 'n' && buff[2] == 'f' && buff[3] == 'o'))
	{
		/* XING VBR-Header

		size	description
		4		'Xing' or 'Info'
		4		flags (indicates which fields are used)
		4		frames (optional)
		4		bytes (optional)
		100	toc (optional)
		4		a VBR quality indicator: 0=best 100=worst (optional)

		*/
		int		nHeadFlags;
		CBigEndianBuffReader	bufReader(buff + 4, nBuffLen - nOffset - 4);

		try
		{
			nHeadFlags = bufReader.ExtractIx(4);

			if (nHeadFlags & FRAMES_FLAG)
				nFrameCount = bufReader.ExtractIx(4);
		}
		catch(CBigEndianBuffReader::COutOfRangeException &)
		{
			return -1;
		}
	}
	else if (buff[0] == 'V' && buff[1] == 'B' && buff[2] == 'R' && buff[3] == 'I')
	{
		// VBRI header
		/* FhG VBRI Header

		size	description
		4		'VBRI' (ID)
		2		version
		2		delay
		2		quality
		4		# bytes
		4		# frames
		2		table size (for TOC)
		2		table scale (for TOC)
		2		size of table entry (max. size = 4 byte (must be stored in an integer))
		2		frames per table entry

		??		dynamic table consisting out of frames with size 1-4
		whole length in table size! (for TOC)

		*/

		CBigEndianBuffReader	bufReader(buff + 4, nBuffLen - nOffset - 4);

		// extract all fields from header (all mandatory)
		DWORD	dwVersion, dwQuality;
		float	fDelay;

		try
		{
			dwVersion = bufReader.ExtractIx(2);
			fDelay = (float)bufReader.ExtractIx(2);
			dwQuality = bufReader.ExtractIx(2);
			/* int nFileSizeOrg = */bufReader.ExtractIx(4);
			nFrameCount = bufReader.ExtractIx(4);
		}
		catch(CBigEndianBuffReader::COutOfRangeException &)
		{
			return -1;
		}
	}

	return nFrameCount;
}
