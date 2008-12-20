// MPMsg.h: interface for the MPMsg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPMSG_H__73DE6A4D_72A3_4652_9FC8_8A2353C34B13__INCLUDED_)
#define AFX_MPMSG_H__73DE6A4D_72A3_4652_9FC8_8A2353C34B13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
// MiniLyrics command line handler.
//
// Copy data
enum MSG_WM_CPDATA
{
	MC_OPEN_FILES	= 1,
};

void CmdLineAnalysis(LPCTSTR szCmdLine, vector<tstring> &vCmdLine);

LPCTSTR CmdLineNext(LPCTSTR szCmdLine);

void SendCommandLine(HWND hWnd, LPCTSTR szCmdLine);

#endif // !defined(AFX_MPMSG_H__73DE6A4D_72A3_4652_9FC8_8A2353C34B13__INCLUDED_)
