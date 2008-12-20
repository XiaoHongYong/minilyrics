// MemFont.cpp: implementation of the CMemFont class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <string.h>
#include "MemFont.h"
#include "assert.h"
// #include "../BaseFrmWork/BaseFrmWork.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemFont::CMemFont()
{
	m_hBmpBk = NULL;
	m_hdcBk = NULL;
	m_hBmpOld = NULL;
	m_hFontOld = NULL;

	m_nHeight = 0;
	m_nWidth = 0;

	m_hFont = NULL;

	m_hWndMiniLyrics = FindMiniLyricsWnd();

	m_nRowPitch = 10;

	m_nColorInner = RGB(255, 255, 255);
	m_nColorOuter = RGB(128, 128, 255);

	// 清除文本
	for (int i = 0; i <LYRICS_ROW_MAX; i++)
	{
		m_LyricsRow[i].szLyrics[0] = '\0';
		m_LyricsRow[i].nWidth = 0;
	}

	m_LyricsData.dwVersion = 1;
	m_LyricsData.dwLen = sizeof(m_LyricsData);
	m_LyricsData.nCurrentRow = -1;
	m_LyricsData.LyricsRow[0].nRow = -1;
	m_LyricsData.LyricsRow[1].nRow = -1;

	m_nCurObsLyricsRow = 0;

	m_lpbi = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];//按照256色调色板（最大值）分配内存
	LPBITMAPINFOHEADER lpbih = &m_lpbi->bmiHeader;//(LPBITMAPINFOHEADER) m_lpbBmpData;
	lpbih->biBitCount = 8;
	lpbih->biClrImportant = 0;
	lpbih->biClrUsed = 0;
	lpbih->biCompression = BI_RGB;
	lpbih->biHeight = 0;
	lpbih->biPlanes = 1;
	lpbih->biSize = sizeof(BITMAPINFOHEADER);
	lpbih->biSizeImage = 0;
	lpbih->biWidth = 0;
	lpbih->biXPelsPerMeter = 0;
	lpbih->biYPelsPerMeter = 0;

	m_lpbBmpData = NULL;
	m_dwbiSize = 0;
	m_dwDataSize = 0;
	m_nBitCount = 24;

	LOGFONT lgfont = {28, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH | FF_ROMAN, "Verdana"};
	m_LogFont = lgfont;
	m_hFont = CreateFontIndirect(&lgfont);

	m_pszLyrics[0][0] = '\0';
	m_pszLyrics[1][0] = '\0';
}

CMemFont::~CMemFont()
{
	if (m_hdcBk)
	{
		SelectObject(m_hdcBk, m_hBmpOld);
		SelectObject(m_hdcBk, m_hFontOld);
		::ReleaseDC(NULL, m_hdcBk);
	}

	if (m_hBmpBk)
		DeleteObject(m_hBmpBk);

	if (m_hFont)
		DeleteObject(m_hFont);

	if (m_lpbBmpData)
		delete[] m_lpbBmpData;

	if (m_lpbi)
		delete[] m_lpbi;
}

void CMemFont::SetFont(const LOGFONT &LogFont)
{
//	if (memcmp(&LogFont, &m_LogFont, sizeof(m_LogFont)) != 0)
	{
		memcpy(&m_LogFont, &LogFont, sizeof(m_LogFont));

		// 通过设置 m_nWidth 为0，来在下一次绘制时清除缓冲区
		m_nWidth = 0;

		if (m_hFont)
			DeleteObject(m_hFont);

		m_hFont = CreateFontIndirect(&m_LogFont);

		m_hFontOld = (HFONT)SelectObject(m_hdcBk, m_hFont);
	}
}

void CMemFont::RenderLyrics(WCHAR *szText, int nDestX, int nDestY, DWORD dwShowAttrib, float fKaraokePos, BOOL bShowOutGrow, int *framebuffer, int w, int h)
{
	int		nCurLyricsPos = 0;

	if (m_nWidth != w + 2)
	{
		m_nWidth = w + 2;
		m_nHeight = (m_LogFont.lfHeight + 2) * LYRICS_ROW_MAX + 2;

		if (!ReallocBuffers())
			return;

		// 需要重新TextOut
		// 没有找到，则需要清除一个缓存的字符串，重新绘画字符串
		nCurLyricsPos = m_nCurObsLyricsRow;
		wcscpy(m_LyricsRow[nCurLyricsPos].szLyrics, szText);
		m_nCurObsLyricsRow++;
		m_nCurObsLyricsRow %= LYRICS_ROW_MAX;

		int		nLen;
		SIZE	Size;
		nLen = wcslen(m_LyricsRow[nCurLyricsPos].szLyrics);
		TextOutW(m_hdcBk, 1, (m_LogFont.lfHeight + 2) * nCurLyricsPos + 2, 
			m_LyricsRow[nCurLyricsPos].szLyrics, nLen);
		GetTextExtentPoint32W(m_hdcBk, m_LyricsRow[nCurLyricsPos].szLyrics, 
			nLen, &Size);
		m_LyricsRow[nCurLyricsPos].nWidth = Size.cx;

		if (!DCToMem())
			return;
	}
	else
	{
		for (nCurLyricsPos = 0; nCurLyricsPos <LYRICS_ROW_MAX; nCurLyricsPos++)
		{
			if (wcscmp(szText, m_LyricsRow[nCurLyricsPos].szLyrics) == 0)
			{
				// match..
			}
		}

		if (nCurLyricsPos == LYRICS_ROW_MAX)
		{
			// 没有找到，则需要清除一个缓存的字符串，重新绘画字符串
			nCurLyricsPos = m_nCurObsLyricsRow;
			wcscpy(m_LyricsRow[nCurLyricsPos].szLyrics, szText);
			m_nCurObsLyricsRow++;
			m_nCurObsLyricsRow %= LYRICS_ROW_MAX;

			int		nLen;
			SIZE	Size;
			nLen = wcslen(m_LyricsRow[nCurLyricsPos].szLyrics);
			TextOutW(m_hdcBk, 1, (m_LogFont.lfHeight + 2) * nCurLyricsPos + 2, 
				m_LyricsRow[nCurLyricsPos].szLyrics, nLen);
			GetTextExtentPoint32W(m_hdcBk, m_LyricsRow[nCurLyricsPos].szLyrics, 
				nLen, &Size);
			m_LyricsRow[nCurLyricsPos].nWidth = Size.cx;


			if (!DCToMem())
				return;
		}
	}

	// nCurLyricsPos 所在的行，为当前用来绘画的行
	// 绘制到framebuffer 中
	DWORD		dwWidthBytes;
	int			nSrcX, nSrcY, nSrcWidth, nSrcHeight;

	assert(m_lpbBmpData);

	dwWidthBytes = (m_nWidth + 3) & ~3;


	//
	// 计算待复制的目标和源位置
	//
	nSrcY = (m_LogFont.lfHeight + 2) * nCurLyricsPos + 2;
	nSrcHeight = m_LogFont.lfHeight;
	if (nDestY < 0)
	{
		nSrcY += nDestY;
		nSrcHeight += nDestY;
		nDestY = 0;
	}
	if (nDestY + nSrcHeight > h)
		nSrcHeight = h - nDestY;

	nSrcX = 1;
	if (nDestX + m_LyricsRow[nCurLyricsPos].nWidth > w)
	{
		if (m_LyricsRow[nCurLyricsPos].nWidth > w)
		{
			nSrcWidth = w;
			nDestX = 0;
		}
		else
		{
			nSrcWidth = m_LyricsRow[nCurLyricsPos].nWidth;
			nDestX = (w - nSrcWidth) / 2;
		}
		// nSrcWidth = w - nDestX;
	}
	else
		nSrcWidth = m_LyricsRow[nCurLyricsPos].nWidth;

	LPBYTE	lpSrcBuffer;
	int		*lpDestBuffer;

	lpSrcBuffer = m_lpbBmpData + 
		dwWidthBytes * (m_nHeight - (m_LogFont.lfHeight + 2) * nCurLyricsPos - 2);
	lpDestBuffer = framebuffer + nDestY * w + nDestX;

	if (dwShowAttrib == LSA_SHOW_SIMPLE)
	{
		//
		// 简单显示风格
		//
		if (bShowOutGrow)
		{
			//
			// 在显示的字体外增加边缘
			//
			for (int i = 0; i <= nSrcHeight; i++)
			{
				for (int k = 0; k < nSrcWidth; k++)
				{
					if (lpSrcBuffer[k ] != 0)
						lpDestBuffer[k] = m_nColorInner;
					else
					{
						if (lpSrcBuffer[(k - 1) ] != 0
							|| lpSrcBuffer[(k + 1) ] != 0
							|| lpSrcBuffer[k  - dwWidthBytes] != 0
							|| lpSrcBuffer[k  - dwWidthBytes - 1] != 0
							|| lpSrcBuffer[k  - dwWidthBytes + 1] != 0
							|| lpSrcBuffer[k  + dwWidthBytes] != 0
							|| lpSrcBuffer[k  + dwWidthBytes - 1] != 0
							|| lpSrcBuffer[k  + dwWidthBytes + 1] != 0)
						lpDestBuffer[k] = m_nColorOuter;
						//lpDestBuffer[k] = RGB( ((int)GetRValue(lpDestBuffer[k])+ GetRValue(m_nColorOuter) * 7) >> 3,
						//	((int)GetGValue(lpDestBuffer[k]) + GetGValue(m_nColorOuter) * 7) >> 3, 
						//	((int)GetBValue(lpDestBuffer[k]) + GetBValue(m_nColorOuter) * 7) >> 3);
					}
				}

				lpDestBuffer += w;
				lpSrcBuffer -= dwWidthBytes;
			}
		}
		else
		{
			for (int i = 0; i <= nSrcHeight; i++)
			{
				for (int k = 0; k < nSrcWidth; k++)
				{
					if (lpSrcBuffer[k ] != 0)
						lpDestBuffer[k] = m_nColorInner;
				}

				lpDestBuffer += w;
				lpSrcBuffer -= dwWidthBytes;
			}
		}
	}
	else
	{
		//
		// Karaoke
		//
		int		nKaraokePos;

		nKaraokePos = int(m_LyricsRow[nCurLyricsPos].nWidth * fKaraokePos);

		if (nKaraokePos > nSrcWidth)
			nKaraokePos = nSrcWidth;

		for (int i = 0; i <= nSrcHeight; i++)
		{
			for (int k = 0; k < nKaraokePos; k++)
			{
				if (lpSrcBuffer[k ] != 0)
					lpDestBuffer[k] = m_nColorInner;
				else
				{
					if (lpSrcBuffer[(k - 1) ] != 0
						|| lpSrcBuffer[(k + 1) ] != 0
						|| lpSrcBuffer[k  - dwWidthBytes] != 0
						|| lpSrcBuffer[k  - dwWidthBytes - 1] != 0
						|| lpSrcBuffer[k  - dwWidthBytes + 1] != 0
						|| lpSrcBuffer[k  + dwWidthBytes] != 0
						|| lpSrcBuffer[k  + dwWidthBytes - 1] != 0
						|| lpSrcBuffer[k  + dwWidthBytes + 1] != 0)
					lpDestBuffer[k] = m_nColorOuter;
				}
			}
			for (k = nKaraokePos; k < nSrcWidth; k++)
			{
				if (lpSrcBuffer[k ] != 0)
					lpDestBuffer[k] = m_nColorInner;
			}

			lpDestBuffer += w;
			lpSrcBuffer -= dwWidthBytes;
		}
	}
}

BOOL CMemFont::ReallocBuffers()
{
	HDC		hdc;

	//
	// 清除文本
	for (int i = 0; i <LYRICS_ROW_MAX; i++)
	{
		m_LyricsRow[i].szLyrics[0] = '\0';
		m_LyricsRow[i].nWidth = 0;
	}

	m_nCurObsLyricsRow = 0;

	if (m_hdcBk)
	{
		SelectObject(m_hdcBk, m_hBmpOld);
		SelectObject(m_hdcBk, m_hFontOld);
		::ReleaseDC(NULL, m_hdcBk);
	}

	if (m_hBmpBk)
		DeleteObject(m_hBmpBk);

//	if (m_hFont)
//		DeleteObject(m_hFont);


	//
	// 重新分配
	//
	hdc = ::GetDC(NULL);
	assert(hdc);

	m_hdcBk = ::CreateCompatibleDC(hdc);
	assert(m_hdcBk);
	if (m_hdcBk == NULL)
	{
		// log.. MessageBox(NULL, "CreateCompatibleDC Failed!", "!", MB_OK);
		return FALSE;
	}

	m_hBmpBk = ::CreateCompatibleBitmap(hdc, m_nWidth, m_nHeight);
	if (m_hBmpBk == NULL)
	{
		// log...
		return FALSE;
	}

	m_hBmpOld = (HBITMAP)SelectObject(m_hdcBk, m_hBmpBk);
	if (m_hBmpOld == NULL)
	{
		// MessageBox(NULL,"SelectObject Failed!","SaveDC!",MB_OK);
		return FALSE;
	}

	if (!BitBlt(m_hdcBk, 0, 0, m_nWidth, m_nHeight, hdc, 0, 0, BLACKNESS))
	{
		// log...
		return FALSE;
	}

	assert(m_hFont);
	m_hFontOld = (HFONT)SelectObject(m_hdcBk, m_hFont);
//	if (m_hFontOld == NULL)
//	{
		// MessageBox(NULL,"SelectObject Failed!","SaveDC!",MB_OK);
//		return FALSE;
//	}

	SetTextColor(m_hdcBk, RGB(255, 255, 255));
	SetBkColor(m_hdcBk, RGB(0, 0, 0));

	ReleaseDC(NULL, hdc);

	return TRUE;
}

BOOL CMemFont::DCToMem()
{
	assert(m_hdcBk);
	if (m_hBmpBk == NULL)
		return FALSE;

	DWORD dwCapDataSize = ((m_nWidth + 3) & ~3) * m_nHeight;

	//biBitCount == 8
	//Allocate memory for the bitmap data
	if (m_lpbBmpData == NULL)
	{
		m_dwDataSize = dwCapDataSize;
		m_lpbBmpData = new BYTE[m_dwDataSize];
		assert(m_lpbBmpData);
	}
	else
	{
		if (dwCapDataSize != m_dwDataSize)
		{
			delete m_lpbBmpData;
			m_dwDataSize = dwCapDataSize;
			m_lpbBmpData = new BYTE[m_dwDataSize];
			assert(m_lpbBmpData);
		}
	}

	if (m_lpbBmpData == NULL || m_lpbi == NULL)
		return FALSE;

	//Initialize the bitmap file structure
	LPBITMAPINFOHEADER lpbih = &m_lpbi->bmiHeader;//(LPBITMAPINFOHEADER) m_lpbBmpData;

	lpbih->biBitCount = 8;
	lpbih->biHeight = m_nHeight;
	lpbih->biWidth = m_nWidth;
	m_dwbiSize = sizeof(BITMAPINFOHEADER);

	if (!GetDIBits(m_hdcBk, m_hBmpBk, 0, m_nHeight, m_lpbBmpData, m_lpbi, DIB_RGB_COLORS))
	{
		// MessageBox(NULL,"GetDIBits Failed!","SaveDC!",MB_OK);
		return FALSE;
	}

	return TRUE;
}

UINT String2HexUINT(char buff[])
{
	int len = strlen(buff);
	int i;
	UINT	re;
	re = 0;
	for (i = 0; i < len; i++)
	{
		if ('0' <= buff[i] && buff[i] <= '9')
			re = re * 16 + buff[i] - '0';
		else if ('a' <= buff[i] && buff[i] <= 'f')
			re = re * 16 + buff[i] - 'a' + 10;
		else if ('A' <= buff[i] && buff[i] <= 'F')
			re = re * 16 + buff[i] - 'A' + 10;
	}
	return re;
}

HWND CMemFont::FindMiniLyricsWnd()
{
	HWND	hWndPrev = NULL;

	TCHAR		szVar[64] = "";
	if (GetEnvironmentVariable("MiniLyricsWnd", szVar, 64))
	{
		hWndPrev = (HWND)String2HexUINT(szVar);
		if (!IsWindow(hWndPrev))
			hWndPrev = NULL;
		else
			return hWndPrev;
	}

	do
	{
		hWndPrev = FindWindowEx(NULL, hWndPrev, "MiniLyrics", NULL);

		if (hWndPrev)
		{
			DWORD	dwProcessID;
			GetWindowThreadProcessId(hWndPrev, &dwProcessID);
			if (dwProcessID == GetCurrentProcessId())
			{
				// OK在同一进程
				return hWndPrev;
			}
		}
	}
	while (hWndPrev);

	// winamp3?
	hWndPrev = FindWindow("BaseWindow_RootWnd", "Minilyrics for winamp3");
	if (hWndPrev)
	{
		DWORD	dwProcessID;
		GetWindowThreadProcessId(hWndPrev, &dwProcessID);
		if (dwProcessID == GetCurrentProcessId())
		{
			// OK在同一进程
			return hWndPrev;
		}
	}

	return NULL;
}

void CMemFont::render(int *framebuffer, int w, int h)
{
	int		nDestX, nDestY;

	if (!IsWindow(m_hWndMiniLyrics))
		m_hWndMiniLyrics = FindMiniLyricsWnd();

	if (m_hWndMiniLyrics == NULL)
	{
		nDestY = h - h / 20 + 5 - m_LogFont.lfHeight * 2 - m_nRowPitch;
		nDestX = w / 20 + 10;
		RenderLyrics(L"MiniLyrics is not installed or running!", nDestX, nDestY, LSA_SHOW_SIMPLE, 0, TRUE, framebuffer, w, h);

		nDestY = h - h / 20 + 5 - m_LogFont.lfHeight * 1;
		nDestX = w / 20 + 10 + m_LogFont.lfHeight * 2;
		RenderLyrics(L"Please run MiniLyrics first!", nDestX, nDestY, LSA_SHOW_SIMPLE, 0, TRUE, framebuffer, w, h);
		return;
	}

	DWORD	dwResult;

	if (!SendMessageTimeout(m_hWndMiniLyrics, MYWM_WVSLYRICS, WVSLYRICS_GETDATA_W, (LPARAM)&m_LyricsData, SMTO_ABORTIFHUNG | SMTO_NORMAL, 500, &dwResult))
		return;
	if (!dwResult)
		return;

	// DBGLOG2("%d, %s", dwResult, m_LyricsData.LyricsRow[0].szLyrics);

	if (m_LyricsData.LyricsRow[0].nRow != -1)
	{
		nDestY = h - h / 20 + 5 - m_LogFont.lfHeight * 2 - m_nRowPitch;
		nDestX = w / 20 + 10;
		RenderLyrics(m_LyricsData.LyricsRow[0].szLyrics, nDestX, nDestY, 
			m_LyricsData.LyricsRow[0].dwLyricsShowAttrb,
			m_LyricsData.LyricsRow[0].fKaraokePos,
			m_LyricsData.LyricsRow[0].nRow <= m_LyricsData.nCurrentRow,
			framebuffer, w, h);
	}

	if (m_LyricsData.LyricsRow[1].nRow != -1)
	{
		nDestY = h - h / 20 + 5 - m_LogFont.lfHeight * 1;
		nDestX = w / 20 + 10 + m_LogFont.lfHeight * 2;
		RenderLyrics(m_LyricsData.LyricsRow[1].szLyrics, nDestX, nDestY,
			m_LyricsData.LyricsRow[1].dwLyricsShowAttrb,
			m_LyricsData.LyricsRow[1].fKaraokePos,
			m_LyricsData.LyricsRow[1].nRow <= m_LyricsData.nCurrentRow,
			framebuffer, w, h);
	}
}
