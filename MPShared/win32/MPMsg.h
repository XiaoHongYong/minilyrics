#pragma once

//
// Command line handler.
//

#define ML_SEND_CMD_LINE		0xF30B4730

void CmdLineAnalysis(LPCTSTR szCmdLine, vector<tstring> &vCmdLine);

LPCTSTR CmdLineNext(LPCTSTR szCmdLine);

void SendCommandLine(HWND hWnd, LPCTSTR szCmdLine);

