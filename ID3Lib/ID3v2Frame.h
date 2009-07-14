// ID3v2Frame.h: interface for the CID3v2Frame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ID3V2FRAME_H__438228A7_2CFF_4266_83AE_CC93D36452EE__INCLUDED_)
#define AFX_ID3V2FRAME_H__438228A7_2CFF_4266_83AE_CC93D36452EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ID3v2.h"

class CID3v2Frame  
{
public:
	CID3v2Frame();
	CID3v2Frame(const ID3v2FrameHdr &frameHdr, const char *data, int len);
	virtual ~CID3v2Frame();

	virtual int RenderFrame(const ID3v2Header *pHeader, CCharBuffer &buff);

	// CCharBuffer *GetData() { return &m_frameData; }
//	char *data() { return m_frameData.data(); }
//	size_t size() { return m_frameData.size(); }

#ifdef DEBUG
	virtual void DebugOutput();
#endif

protected:
	virtual int RenderFrameHdr(const ID3v2Header *pHeader, CCharBuffer &buff);

public:
	ID3v2FrameHdr		m_framehdr;
	CCharBuffer			m_frameData;

};

#endif // !defined(AFX_ID3V2FRAME_H__438228A7_2CFF_4266_83AE_CC93D36452EE__INCLUDED_)
