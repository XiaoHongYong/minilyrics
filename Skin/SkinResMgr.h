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

	void EnumFiles(LPCTSTR szFilter, vector<tstring> &vFiles, bool bEnumFullPath);

	BOOL GetResourcePathName(LPTSTR szPathName, int nSizePathName, LPCTSTR szResName) const;
	tstring GetResourcePathName(LPCTSTR szResName) const;

	void ClearRessourceDir() { m_vResSearchDirs.clear(); }

	// nPos = -1, append at tail
	void AddRessourceDir(LPCTSTR szResDir, int nPos = -1);

	void FreeBitmap(PRAW_IMAGE_DATA hBmp);
	void AddBitmapReference(PRAW_IMAGE_DATA hBmp);

	void SetFastBmp(LPCTSTR szBmp, PRAW_IMAGE_DATA hBmp);
	void DelFastBmp(LPTSTR szBmp);

	PRAW_IMAGE_DATA LoadBitmap(LPCTSTR szBmp);

	void AdjustHue(float hue, float saturation, float luminance);

	void GetAdjustHueParam(float &hue, float &saturation, float &luminance);

#ifdef _DEBUG
	void DbgOutLoadedImages();
#endif
protected:

protected:
	map<tstring, RES_COUNTER>		m_mapBitmap;
	map<tstring, PRAW_IMAGE_DATA>	m_mapFastBmp;

	V_Str							m_vResSearchDirs;

	bool							m_bAdjustHue;

	float							m_hue;
	float							m_saturation;
	float							m_luminance;

};

#endif // !defined(AFX_RESMANAGER_H__BA9335C0_1A94_11D6_B478_00E04C008BA3__INCLUDED_)
