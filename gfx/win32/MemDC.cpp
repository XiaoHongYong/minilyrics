// MemDC.cpp: implementation of the CMemDC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MemDC.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemDC::CMemDC()
{
	m_hdcMem = NULL;
	m_bmpOld = NULL;
	m_bmpMem = NULL;
	m_sizeDC.cx = 0;
	m_sizeDC.cy = 0;
}

CMemDC::~CMemDC()
{
	DeleteDC();
}
//
//void CMemDC::Init(int cx, int cy, HWND hWnd)
//{
//	HDC		hdc = ::GetDC(hWnd);
//
//	Init(cx, cy, hdc);
//
//	::ReleaseDC(hWnd, hdc);
//}

bool CMemDC::Create(int cx, int cy, HDC hdc)
{
	BOOL	bFree = FALSE;

	DeleteDC();

	if (hdc == NULL)
	{
		hdc = ::GetDC(NULL);
		bFree = TRUE;
	}

	m_sizeDC.cx = cx;
	m_sizeDC.cy = cy;
	
	// 根据新的窗口 m_rcBoundBox 的大小, 重新创建绘图缓冲区
	if (m_hdcMem)
	{
		SelectObject(m_hdcMem, m_bmpOld);
		::DeleteDC(m_hdcMem);
	}
	m_hdcMem = CreateCompatibleDC(hdc);
	if (m_hdcMem == NULL)
		ERRLOG1(_T("CreateCompatibleDC Failed! GetLastError: %d"), GetLastError());

	if (m_bmpMem)
		DeleteObject(m_bmpMem);
	m_bmpMem = CreateCompatibleBitmap(hdc, cx, cy);
	if (m_bmpMem == NULL)
		ERRLOG1(_T("CreateCompatibleBitmap Failed! GetLastError: %d"), GetLastError());

	m_bmpOld = (HBITMAP)SelectObject(m_hdcMem, m_bmpMem);

	BitBlt(m_hdcMem, 0, 0, cx, cy, NULL, 0, 0, WHITENESS);

	// 设置缺省的剪裁区域为该Skin的大小
	HRGN hRgn = CreateRectRgn(0, 0, cx, cy);
	SelectClipRgn(m_hdcMem, hRgn);
	DeleteObject(hRgn);

	if (bFree)
		ReleaseDC(NULL, hdc);

	return true;
}

void CMemDC::DeleteDC()
{
	if (m_hdcMem)
	{
		SelectObject(m_hdcMem, m_bmpOld);
		::DeleteDC(m_hdcMem);
		m_hdcMem = NULL;
	}
	
	if (m_bmpMem)
	{
		DeleteObject(m_bmpMem);
		m_bmpMem = NULL;
	}
}

HDC CMemDC::GetDC()
{
	return m_hdcMem;
}

SIZE CMemDC::GetSize() const
{
	return m_sizeDC;
}
