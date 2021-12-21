// SkinRgn.h: interface for the CSkinRgn class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINRGN_H__72A1EE62_3D19_11D6_B47A_00E04C008BA3__INCLUDED_)
#define AFX_SKINRGN_H__72A1EE62_3D19_11D6_B47A_00E04C008BA3__INCLUDED_


class CSkinRgn : public CRegion  
{
public:
	bool LoadTxtRgn2File(LPCTSTR szFile);
	bool LoadRgn(LPCTSTR szFile);

	void SetScaleY1(int nScaleY1);
	void SetScaleY2(int nScaleY2);

	void SetScaleX1(int nScaleX1);
	void SetScaleX2(int nScaleX2);

	int GetScaleY1(){ return m_nStretchStartY; }
	int GetScaleY2(){ return m_nStretchEndY; }

	int GetScaleX1(){ return m_nStretchStartX; }
	int GetScaleX2(){ return m_nStretchEndX; }

	int GetScaleHeightOrg(){ return m_nHeightOrg; }
	int GetScaleWidthOrg(){ return m_nWidthOrg; }

	void SetScaleHeightOrg(int nHeightOrg);
	void SetScaleWidthOrg(int nWidthOrg);

	BOOL StretchRgn(double dbScaleRadio, int nWidth, int nHeight);

	CSkinRgn();
	virtual ~CSkinRgn();

protected:
#ifdef _WIN32
	LPRGNDATA	m_lpRgnDataOrg;		// 初始的区域的数据
	int			m_nRgnDataSize;		// 他的大小
#endif

	int			m_nStretchStartX;
	int			m_nStretchEndX;
	int			m_nStretchStartY;
	int			m_nStretchEndY;
	int			m_nWidthOrg;
	int			m_nHeightOrg;


};

#endif // !defined(AFX_SKINRGN_H__72A1EE62_3D19_11D6_B47A_00E04C008BA3__INCLUDED_)
