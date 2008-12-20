/********************************************************************
	Created  :	2002/04/03	19:34
	FileName :	ResManager.h
	Author   :	xhy
	
	Purpose  :	管理资源文件，如位图等的打开和关闭等
*********************************************************************/

#if !defined(AFX_RESMANAGER_H__BA9335C0_1A94_11D6_B478_00E04C008BA3__INCLUDED_)
#define AFX_RESMANAGER_H__BA9335C0_1A94_11D6_B478_00E04C008BA3__INCLUDED_

class CSkinResMgr  
{
public:
	typedef struct RES_COUNTER
	{
		PRAW_IMAGE_DATA	hHandle;
		int				nCount;
	}RES_COUNTER;

	CSkinResMgr();
	virtual ~CSkinResMgr();

public:
	void OnClose();

	void EnumFiles(LPCTSTR szFilter, vector<tstring> &vFiles, BOOL bEnumFullPath);

	BOOL GetResourcePathName(LPTSTR szPathName, int nSizePathName, LPCTSTR szResName) const;

	void SetResourceDir3(LPCTSTR szResDir3);
	void SetResourceDir2(LPCTSTR szResDir2);
	void SetResourceDir1(LPCTSTR szResDir1);

	void FreeBitmap(PRAW_IMAGE_DATA hBmp);
	void AddBitmapReference(PRAW_IMAGE_DATA hBmp);

	void SetFastBmp(LPCTSTR szBmp, PRAW_IMAGE_DATA hBmp);
	void DelFastBmp(LPTSTR szBmp);

	PRAW_IMAGE_DATA LoadBitmap(LPCTSTR szBmp);

	void AdjustHueInfoFromXML(SXNode *pNodeAdjustHue);

	void AdjustHue(float hue, float saturation, float luminance);

	void GetAdjustHueParam(float &hue, float &saturation, float &luminance);

	bool CanAdjustHue();

#ifdef _DEBUG
	void DbgOutLoadedImages();
#endif
protected:
	bool ShouldHueOfImageBeAdjusted(LPCTSTR szFile);

protected:
	map<tstring, RES_COUNTER>		m_mapBitmap;
	map<tstring, PRAW_IMAGE_DATA>	m_mapFastBmp;

	// files whose hue and saturation can adjusted.
	V_Str							m_vAdjustHueOfImages;

	TCHAR							m_szResDir1[MAX_PATH];	// 资源的第一个搜索目录
	TCHAR							m_szResDir2[MAX_PATH];	// 资源的第二个搜索目录
	TCHAR							m_szResDir3[MAX_PATH];	// 资源的第三个搜索目录

	bool							m_bAdjustHue;

	float							m_hue;
	float							m_saturation;
	float							m_luminance;

};

#endif // !defined(AFX_RESMANAGER_H__BA9335C0_1A94_11D6_B478_00E04C008BA3__INCLUDED_)
