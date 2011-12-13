
#pragma once

class CDlgSaveEmbeddedLyrics
{
public:
	CDlgSaveEmbeddedLyrics() { }
	virtual ~CDlgSaveEmbeddedLyrics() { }

	int DoModal(CBaseWnd *pWnd) { return IDOK; }

	
protected:
	tstring			m_strMediaUrl;
	bool			m_bId3v2Sync, m_bId3v2Unsyc, m_bLyrics3v2;
	CCharBuffer		m_bufLyrics;

};
