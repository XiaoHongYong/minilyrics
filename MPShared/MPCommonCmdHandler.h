// MPCommonCmdHandler.h: interface for the CMPCommonCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPCOMMONCMDHANDLER_H__C770324D_2964_450B_AB1C_5DD1C118B0FD__INCLUDED_)
#define AFX_MPCOMMONCMDHANDLER_H__C770324D_2964_450B_AB1C_5DD1C118B0FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MPEventsDispatcher.h"
#include "LyrDisplayClrListWnd.h"

class CMPCommonCmdHandler : public ISkinCmdHandler  
{
public:
	CMPCommonCmdHandler(bool bFloatingLyr = false);
	virtual ~CMPCommonCmdHandler();

	// if the command id is processed, return true.
	virtual bool OnCommand(int nId);
	virtual bool OnCustomCommand(int nID);
	virtual bool OnUIObjNotify(IUIObjNotify *pNotify);

	virtual bool GetChecked(UINT nID, bool &bChecked);
	virtual bool GetRadioChecked(vector<UINT> &vIDs, UINT &nIDChecked);

	static BOOL SaveAsLyricsFile(CBaseWnd *pWndParent, MLFileType DefFileType, bool bAdvancedSaveOpt);
	static bool SaveCurrentLyrics(CSkinWnd *pSkinWnd, bool bDispatchOnSave);

protected:
	bool OnCommandCharEncoding(int nCmdId);

	VLyrLines &GetDisplayLyrics();

protected:
	tstring			m_strSectName;
	EventType		m_etDispSettings;

	CLyrDisplayClrListWnd	m_popupHighClrListWnd;

};

#endif // !defined(AFX_MPCOMMONCMDHANDLER_H__C770324D_2964_450B_AB1C_5DD1C118B0FD__INCLUDED_)
