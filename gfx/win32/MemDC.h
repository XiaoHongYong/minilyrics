// MemDC.h: interface for the CMemDC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMDC_H__339F6837_7191_4486_992D_B56FE3778304__INCLUDED_)
#define AFX_MEMDC_H__339F6837_7191_4486_992D_B56FE3778304__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMemDC  
{
public:
	SIZE GetSize() const;
	HDC GetDC();
	void DeleteDC();
//	void Init(int cx, int cy, HDC hdc = NULL);
//	void Init(int cx, int cy, HWND hWnd);

	int Width() const { return m_sizeDC.cx; }
	int Height() const { return m_sizeDC.cy; }

	bool Create(int cx, int cy, HDC hdc);

	CMemDC();
	virtual ~CMemDC();

protected:
	HDC			m_hdcMem;
	HBITMAP		m_bmpMem;
	HBITMAP		m_bmpOld;

	SIZE		m_sizeDC;
};

#endif // !defined(AFX_MEMDC_H__339F6837_7191_4486_992D_B56FE3778304__INCLUDED_)
