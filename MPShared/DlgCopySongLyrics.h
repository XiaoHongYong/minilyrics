// DlgCopySongLyrics.h: interface for the CDlgCopySongLyrics class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLGCOPYSONGLYRICS_H__300222EA_C73B_4D27_9D55_BFB24F23B20F__INCLUDED_)
#define AFX_DLGCOPYSONGLYRICS_H__300222EA_C73B_4D27_9D55_BFB24F23B20F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDlgCopySongLyrics : public CBaseDialog  
{
public:
	CDlgCopySongLyrics();
	virtual ~CDlgCopySongLyrics();

	virtual BOOL OnInitDialog();

	virtual void OnOK();
	virtual void OnCommand(UINT uID, UINT nNotifyCode);

public:
	tstring		m_strSong;
	tstring		m_strLyrics;

};

#endif // !defined(AFX_DLGCOPYSONGLYRICS_H__300222EA_C73B_4D27_9D55_BFB24F23B20F__INCLUDED_)
