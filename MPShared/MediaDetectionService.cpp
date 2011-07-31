#include "StdAfx.h"
#include "MPlayerAppBase.h"
#include "MediaDetectionService.h"

CMediaDetectionService		g_mediaDetectionService;

CMediaDetectionService::CMediaDetectionService(void)
{
}

CMediaDetectionService::~CMediaDetectionService(void)
{
}

void ListMedia(LPCTSTR szDir, vector<tstring> &vFiles)
{
	CMLFileFind		find;
	tstring			strDir;
	tstring			strFile;
	// TCHAR			szMsg[256];

	if (!find.OpenDir(szDir))
		return;

	strDir = szDir;
	DirStringFormat(strDir);

	while (find.FindNext())
	{
		if (find.IsCurDir())
		{
			if (_tcscmp(_T("."), find.GetCurName()) != 0 &&
				_tcscmp(_T(".."), find.GetCurName()) != 0)
			{
				strFile = strDir + find.GetCurName();
				ListMedia(strFile.c_str(), vFiles);
			}
		}
		else
		{
			if (g_Player.IsExtAudioFile(FileGetExt(find.GetCurName())))
			{
				strFile = strDir + find.GetCurName();
				vFiles.push_back(strFile);
				// _stprintf(szMsg, TOLOCAL(_T("%d files found")), m_vFiles.size());
				// SetMsg1(szMsg);
			}
		}
	}
}

void CMediaDetectionService::AddMediaInDir(LPCTSTR szDir)
{
	vector<tstring>				vFiles;

	ListMedia(szDir, vFiles);

	AddMedia(vFiles);
}

void CMediaDetectionService::AddMedia(vector<tstring> &vFiles)
{
	CMPAutoPtr<IMediaLibrary>	pMediaLib;
	int							nRet;
	//TCHAR						szMsg[256];
	int							n = 0;


// 	_stprintf(szMsg, TOLOCAL(_T("%d files added")), 0);
// 	SetMsg1(szMsg);
// 	SetMsg2(TOLOCAL(_T("Adding files...")));

	if (g_Player.GetMediaLibrary(&pMediaLib) == ERR_OK)
	{
		for (int i = 0; i < (int)vFiles.size(); i++)
		{
			CMPAutoPtr<IMedia>			pMedia;

			if (pMediaLib->GetMediaByUrl(vFiles[i].c_str(), &pMedia) == ERR_OK)
				continue;

			nRet = pMediaLib->Add(vFiles[i].c_str(), &pMedia);
			if (nRet == ERR_OK)
			{
				n++;
// 				_stprintf(szMsg, TOLOCAL(_T("%d files added")), n);
// 				SetMsg1(szMsg);
			}
		}
	}

//	SetMsg2(TOLOCAL(_T("The library has been updated. To continue, click Done.")));
}
