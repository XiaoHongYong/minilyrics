// MPPlaylistCmdHandler.h: interface for the CMPPlaylistCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPPLAYLISTCMDHANDLER_H__1F469055_53D8_450C_A633_89109621D419__INCLUDED_)
#define AFX_MPPLAYLISTCMDHANDLER_H__1F469055_53D8_450C_A633_89109621D419__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMPPlaylistCmdHandler : public ISkinCmdHandler  
{
public:
	CMPPlaylistCmdHandler();
	virtual ~CMPPlaylistCmdHandler();

	// if the command id is processed, return true.
	virtual bool OnCommand(int nId);
	virtual bool OnCustomCommand(int nID);

};

#endif // !defined(AFX_MPPLAYLISTCMDHANDLER_H__1F469055_53D8_450C_A633_89109621D419__INCLUDED_)
