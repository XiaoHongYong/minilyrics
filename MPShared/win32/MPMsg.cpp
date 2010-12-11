// MPMsg.cpp: implementation of the MPMsg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPMsg.h"


void CmdLineAnalysis(LPCTSTR szCmdLine, vector<tstring> &vCmdLine)
{
	LPCTSTR		szBeg, szEnd;
	TCHAR		chSign;
	tstring		str;

	szBeg = szCmdLine;

	while (*szBeg)
	{
		if (*szBeg == '"')
		{
			chSign = '"';
			szBeg++;
		}
		else
			chSign = ' ';

		szEnd = szBeg;
		while (*szEnd != '\0' && *szEnd != chSign)
			szEnd++;

		str = _T("");
		str.append(szBeg, szEnd);
		if (!str.empty())
			vCmdLine.push_back(str);

		if (*szEnd == '"')
			szEnd++;
		while (*szEnd == ' ')
			szEnd++;

		szBeg = szEnd;
	}
}

LPCTSTR CmdLineNext(LPCTSTR szCmdLine)
{
	TCHAR			chSign;

	if (*szCmdLine == '"')
	{
		chSign = '"';
		szCmdLine++;
	}
	else
		chSign = ' ';

	while (*szCmdLine != '\0' && *szCmdLine != chSign)
		szCmdLine++;
	if (*szCmdLine == '"')
		szCmdLine++;
	while (*szCmdLine == ' ')
		szCmdLine++;

	return szCmdLine;
}

void SendCommandLine(HWND hWnd, LPCTSTR szCmdLine)
{
	COPYDATASTRUCT	copyData;

	copyData.dwData = ML_SEND_CMD_LINE;
	copyData.lpData = (LPVOID)szCmdLine;
	copyData.cbData = sizeof(TCHAR) * (_tcslen(szCmdLine) + 1);

	SendMessage(hWnd, WM_COPYDATA, ML_SEND_CMD_LINE, (LPARAM)&copyData);
}
