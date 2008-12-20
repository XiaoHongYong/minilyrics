/********************************************************************
	Created  :	2002/04/03	19:33
	FileName :	ResManager.cpp
	Author   :	xhy
	
	Purpose  :	管理资源文件，如位图等的打开和关闭等
*********************************************************************/

#include "stdafx.h"
#include "MLSkin.h"
#include "SkinResMgr.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinResMgr::CSkinResMgr()
{
	EmptyStr(m_szResDir1);
	EmptyStr(m_szResDir2);
	EmptyStr(m_szResDir3);
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
	ASSERT(m_mapBitmap.size() == 0);

	m_vAdjustHueOfImages.clear();
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

		if (ShouldHueOfImageBeAdjusted(szFile) && m_hue != 0.0)
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

	ERRLOG1(_T("Delete, But Not Found Bitmap Object:%d"), (int)hBmp);
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

	ERRLOG1(_T("Add reference, but Not Found Bitmap Object:%d"), (int)hBmp);
}

void CSkinResMgr::SetResourceDir1(LPCTSTR szResDir1)
{
	_tcscpy_safe(m_szResDir1, CountOf(m_szResDir1), szResDir1);
	if (!IsEmptyString(m_szResDir1))
		DirStringFormat(m_szResDir1);
}

void CSkinResMgr::SetResourceDir2(LPCTSTR szResDir2)
{
	_tcscpy_safe(m_szResDir2, CountOf(m_szResDir2), szResDir2);
	if (!IsEmptyString(m_szResDir2))
		DirStringFormat(m_szResDir2);
}

void CSkinResMgr::SetResourceDir3(LPCTSTR szResDir3)
{
	_tcscpy_safe(m_szResDir3, CountOf(m_szResDir3), szResDir3);
	if (!IsEmptyString(m_szResDir3))
		DirStringFormat(m_szResDir3);
}

void CSkinResMgr::EnumFiles(LPCTSTR szFilter, vector<tstring> &vFiles, BOOL bEnumFullPath)
{
	EnumFilesInDir(m_szResDir1, szFilter, vFiles, bEnumFullPath);
	EnumFilesInDir(m_szResDir2, szFilter, vFiles, bEnumFullPath);
	EnumFilesInDir(m_szResDir3, szFilter, vFiles, bEnumFullPath);
}

BOOL CSkinResMgr::GetResourcePathName(LPTSTR szPathName, int nSizePathName, LPCTSTR szResName) const
{
	EmptyStr(szPathName);

	//
	// 资源的搜索顺序：szDir1, szDir2, szDir3, 最后为 szBmp
	_tcscpy_safe(szPathName, nSizePathName, m_szResDir1);
	_tcscat(szPathName, szResName);
	if (!IsFileExist(szPathName))
	{
		_tcscpy_safe(szPathName, nSizePathName, m_szResDir2);
		_tcscat(szPathName, szResName);
		if (!IsFileExist(szPathName))
		{
			_tcscpy_safe(szPathName, nSizePathName, szResName);
			if (!IsFileExist(szPathName))
			{
				_tcscpy_safe(szPathName, nSizePathName, m_szResDir3);
				_tcscat(szPathName, szResName);
				if (!IsFileExist(szPathName))
				{
					ERRLOG1(_T("Resource File: %s does NOT exist!"), szResName);
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}

void CSkinResMgr::AdjustHueInfoFromXML(SXNode *pNodeAdjustHue)
{
	ASSERT(pNodeAdjustHue);

	SXNode::iterator	it, itEnd;

	itEnd = pNodeAdjustHue->listChildren.end();
	for (it = pNodeAdjustHue->listChildren.begin(); it != itEnd; ++it)
	{
		SXNode		*pNode = *it;

		if (_tcsicmp(pNode->strName.c_str(), _T("ImageFile")) == 0)
		{
			m_vAdjustHueOfImages.push_back(pNode->GetPropertySafe(_T("file")));
		}
	}
}

void CSkinResMgr::AdjustHue(float hue, float saturation, float luminance)
{
	if (!CanAdjustHue())
		return;

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

		if (ShouldHueOfImageBeAdjusted(szFile))
		{
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

/*	if (bReloadFile)
	{
		
	}
	else
	{
		// adjust the hue of opened images
		map<tstring, RES_COUNTER>::iterator	it;

		if (hue == 0)
			return;

		for (it = m_mapBitmap.begin(); it != m_mapBitmap.end(); it++)
		{
			RES_COUNTER		&item = (*it).second;

			if (ShouldHueOfImageBeAdjusted((*it).first.c_str()))
				AdjustImageHue(item.hHandle, hueOffset);
		}
	}*/
}

void CSkinResMgr::GetAdjustHueParam(float &hue, float &saturation, float &luminance)
{
	hue = m_hue;
	saturation = m_saturation;
	luminance = m_luminance;
}

bool CSkinResMgr::CanAdjustHue()
{
	return m_vAdjustHueOfImages.size() > 0;
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

bool CSkinResMgr::ShouldHueOfImageBeAdjusted(LPCTSTR szFile)
{
	int		nLen;

	nLen = _tcslen(szFile);

	for (int i = 0; i < (int)m_vAdjustHueOfImages.size(); i++)
	{
		if ((int)m_vAdjustHueOfImages[i].size() > nLen)
			continue;

		LPCTSTR		str = szFile + nLen - m_vAdjustHueOfImages[i].size();
		if (_tcsicmp(m_vAdjustHueOfImages[i].c_str(), str) == 0)
			return true;
	}

	return false;
}
