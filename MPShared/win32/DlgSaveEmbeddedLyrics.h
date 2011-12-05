
#pragma once

class CDlgSaveEmbeddedLyrics : public CBaseDialog  
{
public:
	CDlgSaveEmbeddedLyrics();
	virtual ~CDlgSaveEmbeddedLyrics();

	virtual BOOL OnInitDialog();

	virtual void OnOK();

protected:
	tstring			m_strMediaUrl;
	bool			m_bId3v2Sync, m_bId3v2Unsyc, m_bLyrics3v2;
	CCharBuffer		m_bufLyrics;

};
