// ID3v2Frame.cpp: implementation of the CID3v2Frame class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ID3v2Frame.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CID3v2Frame::CID3v2Frame(UINT frameID)
{
	m_framehdr.nFrameID = frameID;
}

CID3v2Frame::CID3v2Frame(const ID3v2FrameHdr &frameHdr, const char *data, int len)
{
	m_framehdr = frameHdr;
	m_frameData.append(data, len);
}

CID3v2Frame::~CID3v2Frame()
{

}

int CID3v2Frame::RenderFrame(const ID3v2Header *pHeader, CCharBuffer &buff)
{
	int		nRet;

	nRet = RenderFrameHdr(pHeader, buff);
	if (nRet != ERR_OK)
		return nRet;

	buff.append(m_frameData.c_str(), m_frameData.size());

	return ERR_OK;
}


int CID3v2Frame::RenderFrameHdr(const ID3v2Header *pHeader, CCharBuffer &buff)
{
	if (pHeader->byMajorVer == ID3v2Header::ID3V2_V2)
	{
		ID3v2FrameHeaderV2		fh2;
		ByteDataFromUInt(m_frameData.size(), fh2.bySize, CountOf(fh2.bySize));
		fh2.FromFrameUintID(m_framehdr.nFrameID);

		buff.append((const char *)&fh2, sizeof(fh2));
	}
	else if (pHeader->byMajorVer == ID3v2Header::ID3V2_V3)
	{
		ID3v2FrameHeaderV3		fh3;
		ByteDataFromUInt(m_frameData.size(), fh3.bySize, CountOf(fh3.bySize));
		fh3.FromFrameUintID(m_framehdr.nFrameID);
		memcpy(fh3.byFlags, m_framehdr.byFlags, CountOf(fh3.byFlags));

		{
			// first byte of flags
			std::bitset<8>	flags(0);
			flags[7] = m_framehdr.bTagAlterPreservation;
			flags[6] = m_framehdr.bFileAlterPreservation;
			flags[5] = m_framehdr.bReadOnly;
			fh3.byFlags[0] = (byte)flags.to_ulong();
		}
		{
			// second byte of flags
			std::bitset<8>	flags(0);
			flags[7] = m_framehdr.bCompression;
			flags[6] = m_framehdr.bEncryption;
			flags[5] = m_framehdr.bGroupingIdentity;
			fh3.byFlags[1] = (byte)flags.to_ulong();
		}

		buff.append((const char *)&fh3, sizeof(fh3));
	}
	else if (pHeader->byMajorVer == ID3v2Header::ID3V2_V4)
	{
		ID3v2FrameHeaderV3		fh3;
		SynchDataFromUInt(m_frameData.size(), fh3.bySize, CountOf(fh3.bySize));
		fh3.FromFrameUintID(m_framehdr.nFrameID);
		memcpy(fh3.byFlags, m_framehdr.byFlags, CountOf(fh3.byFlags));

		{
			// first byte of flags
			std::bitset<8>	flags(0);
			flags[6] = m_framehdr.bTagAlterPreservation;
			flags[5] = m_framehdr.bFileAlterPreservation;
			flags[4] = m_framehdr.bReadOnly;
			fh3.byFlags[0] = (byte)flags.to_ulong();
		}
		{
			// second byte of flags
			std::bitset<8>	flags(0);
			flags[6] = m_framehdr.bGroupingIdentity;
			flags[3] = m_framehdr.bCompression;
			flags[2] = m_framehdr.bEncryption;
			flags[1] = m_framehdr.bUnsyncronisation;
			flags[0] = m_framehdr.bDataLengthIndicator;
			fh3.byFlags[1] = (byte)flags.to_ulong();
		}

		buff.append((const char *)&fh3, sizeof(fh3));
	}
	else
		return ERR_NOT_SUPPORT_ID3V2_VER;

	return ERR_OK;
}

#ifdef DEBUG
void CID3v2Frame::DebugOutput()
{
	DBGLOG3(_T("FrameID: %S, length: %d, flag: %X"), m_framehdr.szFrameID, m_frameData.size(), m_framehdr.byFlags[0] << 8 | m_framehdr.byFlags[1]);
}
#endif
