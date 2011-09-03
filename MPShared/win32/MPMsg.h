#pragma once

//
// Command line handler.
//

#define ML_SEND_CMD_LINE		0xF30B4730
#define ML_ACTIVATE				0x803C27CA

#ifdef _MPLAYER
#define MSG_WND_CLASS_NAME		_T("ZikiPlayerMsgWnd")
#else
#define MSG_WND_CLASS_NAME		_T("MiniLyricsMsgWnd")
#endif

void CmdLineAnalysis(LPCTSTR szCmdLine, vector<tstring> &vCmdLine);

LPCTSTR CmdLineNext(LPCTSTR szCmdLine);

void SendCommandLine(HWND hWnd, LPCTSTR szCmdLine);

void SendActivateMainWnd(HWND hWnd);

