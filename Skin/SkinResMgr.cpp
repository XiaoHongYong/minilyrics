/********************************************************************
	Created  :	2002/04/03	19:33
	FileName :	ResManager.cpp
	Author   :	xhy
	
	Purpose  :	管理资源文件，如位图等的打开和关闭等
*********************************************************************/

#include "StdAfx.h"
#include "Skin.h"
#include "SkinResMgr.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinResMgr::CSkinResMgr()
{
	m_hue = 0.0;
	m_saturation = 0.0;
	m_luminance = 0.0;
	m_bAdjustHue = false;
}

CSkinResMgr::~CSkinResMgr()
{
	map<tstring, RES_COUNTER>::iterator	it;

	// 释放所有未释放的资源
	for (it = m_mapBitmap.begin(); it != m_mapBitmap.end(); it++)
	{
		if ((*it).second.nCount > 0)
		{
			// ERRLOG1(_T("Bitmap resource :%d is NOT freed before exit."), (int)(*it).second.hHandle);
			FreeRawImage((*it).second.hHandle);
		}
	}

	m_mapBitmap.clear();
}

void CSkinResMgr::OnClose()
{
	m_bAdjustHue = false;
	m_hue = 0;
	m_saturation = 0;
	m_luminance = 0;
}


void CSkinResMgr::DelFastBmp(LPTSTR szBmp)
{
	map<tstring, PRAW_IMAGE_DATA>::iterator	it;

	//
	// 先从m_mapFastBmp中查找……
	it = m_mapFastBmp.find(szBmp);
	if (it != m_mapFastBmp.end())
	{
		FreeRawImage((*it).second);
		m_mapFastBmp.erase(it);
	}
}

void CSkinResMgr::SetFastBmp(LPCTSTR szBmp, PRAW_IMAGE_DATA hBmp)
{
	map<tstring, PRAW_IMAGE_DATA>::iterator	it;

	//
	// 先从m_mapFastBmp中查找……
	it = m_mapFastBmp.find(szBmp);
	if (it != m_mapFastBmp.end())
	{
		FreeRawImage((*it).second);
		(*it).second = hBmp;
		if (hBmp == NULL)
			m_mapFastBmp.erase(it);
	}
	else if (hBmp)
		m_mapFastBmp[szBmp] = hBmp;
}

PRAW_IMAGE_DATA CSkinResMgr::LoadBitmap(LPCTSTR szBmp)
{
	map<tstring, RES_COUNTER>::iterator	it;
	RES_COUNTER		ResBmp;
	TCHAR			szFile[MAX_PATH];

	if (IsEmptyString(szBmp))
		return NULL;

	//
	// 先从m_mapFastBmp中查找……
	map<tstring, PRAW_IMAGE_DATA>::iterator	itFastBmp;
	itFastBmp = m_mapFastBmp.find(szBmp);
	if (itFastBmp != m_mapFastBmp.end())
	{
		return (*itFastBmp).second;
	}


	//
	// 资源的搜索顺序：szDir1, szDir2, 最后为 szBmp
	if (!GetResourcePathName(szFile, MAX_PATH, szBmp))
	{
		ERRLOG1(_T("Bitmap File: %s does NOT exist.\nPlease check the skin xml file."), szBmp);
		return NULL;
	}

	it = m_mapBitmap.find(szFile);

	if (it == m_mapBitmap.end())
	{
		// 不存在，则重新加载新的位图
		ResBmp.hHandle = LoadRawImageDataFromFile(szFile); // LoadBitmapFromFile(szFile);

		if (ResBmp.hHandle == NULL)
		{
			ERRLOG1(_T("Failed to Load Image: %s, Please check the skin xml file."), szFile);
			return NULL;
		}

		ResBmp.nCount = 1;
		m_mapBitmap[szFile] = ResBmp;

		if (m_hue != 0.0)
			AdjustImageHue(ResBmp.hHandle, m_hue);

		return (PRAW_IMAGE_DATA)ResBmp.hHandle;
	}
	else
	{
		// 存在，则使用前面的位图句柄
		(*it).second.nCount++;
		return (PRAW_IMAGE_DATA)((*it).second.hHandle);
	}
}

void CSkinResMgr::FreeBitmap(PRAW_IMAGE_DATA hBmp)
{
	assert(hBmp);

	{
		//
		map<tstring, PRAW_IMAGE_DATA>::iterator	itFastBmp;
		for (itFastBmp = m_mapFastBmp.begin(); itFastBmp != m_mapFastBmp.end(); itFastBmp++)
		{
			if ((*itFastBmp).second == (PRAW_IMAGE_DATA)hBmp)
				return;
		}
	}

	map<tstring, RES_COUNTER>::iterator	it;

	// 释放所有未释放的资源
	for (it = m_mapBitmap.begin(); it != m_mapBitmap.end(); it++)
	{
		RES_COUNTER		&res = (*it).second;
		if (res.hHandle == (PRAW_IMAGE_DATA)hBmp)
		{
			if (res.nCount <= 1)
			{
				FreeRawImage(res.hHandle);
				m_mapBitmap.erase(it);
			}
			else
				res.nCount--;
			return;
		}
	}

	ERRLOG1(_T("Delete, But Not Found Bitmap Object:%d"), (int)(long)hBmp);
	// FreeRawImage(hBmp);
}

void CSkinResMgr::AddBitmapReference(PRAW_IMAGE_DATA hBmp)
{
	assert(hBmp);

	{
		//
		map<tstring, PRAW_IMAGE_DATA>::iterator	itFastBmp;
		for (itFastBmp = m_mapFastBmp.begin(); itFastBmp != m_mapFastBmp.end(); itFastBmp++)
		{
			if ((*itFastBmp).second == (PRAW_IMAGE_DATA)hBmp)
				return;
		}
	}

	map<tstring, RES_COUNTER>::iterator	it;

	for (it = m_mapBitmap.begin(); it != m_mapBitmap.end(); it++)
	{
		if ((*it).second.hHandle == (PRAW_IMAGE_DATA)hBmp)
		{
			(*it).second.nCount++;
			return;
		}
	}

	ERRLOG1(_T("Add reference, but Not Found Bitmap Object:%d"), (int)(long)hBmp);
}

void CSkinResMgr::AddRessourceDir(LPCTSTR szResDir, int nPos)
{
	if (IsEmptyString(szResDir))
		return;

	tstring		strDir = szResDir;
	DirStringFormat(strDir);

	for (uint i = 0; i < m_vResSearchDirs.size(); i++)
	{
		if (_tcsicmp(strDir.c_str(), m_vResSearchDirs[i].c_str()) == 0)
			return;
	}

	if (nPos < 0 || nPos >= (int)m_vResSearchDirs.size())
		m_vResSearchDirs.push_back(strDir);
	else
		m_vResSearchDirs.insert(m_vResSearchDirs.begin() + nPos, strDir);
}

void CSkinResMgr::EnumFiles(LPCTSTR szFilter, vector<tstring> &vFiles, bool bEnumFullPath)
{
	for (uint i = 0; i < m_vResSearchDirs.size(); i++)
	{
		EnumFilesInDir(m_vResSearchDirs[i].c_str(), szFilter, vFiles, bEnumFullPath);
	}
}

BOOL CSkinResMgr::GetResourcePathName(LPTSTR szPathName, int nSizePathName, LPCTSTR szResName) const
{
	EmptyStr(szPathName);

	for (uint i = 0; i < m_vResSearchDirs.size(); i++)
	{
		_tcscpy_safe(szPathName, nSizePathName, m_vResSearchDirs[i].c_str());
		_tcscat_safe(szPathName, nSizePathName, szResName);
		if (IsFileExist(szPathName))
			return TRUE;
	}

	ERRLOG1(_T("Resource File: %s does NOT exist!"), szResName);
	return FALSE;
}

tstring CSkinResMgr::GetResourcePathName(LPCTSTR szResName) const
{
	TCHAR szPathName[MAX_PATH];
	EmptyStr(szPathName);
	
	GetResourcePathName(szPathName, CountOf(szPathName), szResName);

	return szPathName;
}


void CSkinResMgr::AdjustHue(float hue, float saturation, float luminance)
{
	if (hue == m_hue)
		return;

	m_hue = hue;
	m_luminance = luminance;
	m_saturation = saturation;

	// adjust the hue of opened images
	map<tstring, RES_COUNTER>::iterator	it;

	for (it = m_mapBitmap.begin(); it != m_mapBitmap.end(); it++)
	{
		RES_COUNTER		&item = (*it).second;
		LPCTSTR			szFile = (*it).first.c_str();

		PRAW_IMAGE_DATA	hImage;

		hImage = LoadRawImageDataFromFile(szFile); // LoadBitmapFromFile(szFile);
		if (hImage == NULL)
			continue;

		item.hHandle->Switch(hImage);
		FreeRawImage(hImage);

		if (hue != 0)
			AdjustImageHue(item.hHandle, hue);
	}
}

void CSkinResMgr::GetAdjustHueParam(float &hue, float &saturation, float &luminance)
{
	hue = m_hue;
	saturation = m_saturation;
	luminance = m_luminance;
}

#ifdef _DEBUG
void CSkinResMgr::DbgOutLoadedImages()
{
	if (m_mapFastBmp.size())
	{
		DBGLOG1(_T("Loaded %d FastBmp"), m_mapFastBmp.size());

		//
		map<tstring, PRAW_IMAGE_DATA>::iterator	itFastBmp;
		for (itFastBmp = m_mapFastBmp.begin(); itFastBmp != m_mapFastBmp.end(); itFastBmp++)
		{
			DBGLOG1(_T("    FastBmp: %s"), (*itFastBmp).first.c_str());
		}
	}

	map<tstring, RES_COUNTER>::iterator	it;

	if (m_mapBitmap.empty())
		return;

	DBGLOG1(_T("Loaded %d Bmp"), m_mapBitmap.size());

	// 释放所有未释放的资源
	for (it = m_mapBitmap.begin(); it != m_mapBitmap.end(); it++)
	{
		DBGLOG2(_T("    FastBmp: %d : %s"), (*it).second.nCount, (*it).first.c_str());
	}
}
#endif
