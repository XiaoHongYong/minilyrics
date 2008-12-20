// MemFont.h: interface for the CMemFont class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMFONT_H__BBCA349E_7823_427D_902F_6A440C12FFAD__INCLUDED_)
#define AFX_MEMFONT_H__BBCA349E_7823_427D_902F_6A440C12FFAD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define		LYRICS_ROW_MAX			2


//
// 定义消息取得 WVS 显示歌词需要的冬冬
//
#define MYWM_WVSLYRICS		(WM_USER + 11)

#define WVSLYRICS_GETDATA_W	2

#define LSA_SHOW_DEFAULT	0			// 缺省的显示风格，为：LSA_SHOW_SIMPLE
#define LSA_SHOW_SIMPLE		1			// 简单的显示风格
#define LSA_SHOW_KARAOKE	2			// 逐字显示风格

#define LYRICS_LEN_MAX			255
typedef struct WVS_LYRICS_DATA_W
{
	typedef struct LYRICS_ROW_W
	{
		int			nRow;					// 行数
		DWORD		dwLyricsShowAttrb;		// 该行歌词的显示属性
		float		fKaraokePos;			// 在卡拉Ok显示方式时有效，表示显示位置比例
		WCHAR		szLyrics[LYRICS_LEN_MAX];			// 歌词
	}LYRICS_ROW_W;
	DWORD		dwLen;			// 该结构的长度
	DWORD		dwVersion;		// 该结构的版本
	int			nCurrentRow;	// 当前歌词行
	LYRICS_ROW_W	LyricsRow[2];	// 歌词行s
}WVS_LYRICS_DATA_W;


class CMemFont  
{
public:
	typedef struct LYRICS_ROW
	{
		WCHAR	szLyrics[LYRICS_LEN_MAX + 1];		// 字符串缓冲区
		int		nWidth;								// 显示的宽度，单位:象素
	}LYRICS_ROW;

public:
	void render(int *framebuffer, int w, int h);
	HWND FindMiniLyricsWnd();
	BOOL DCToMem();
	BOOL ReallocBuffers();
	void RenderLyrics(WCHAR *szText, int nDestX, int nDestY, DWORD dwShowAttrib, float fKaraokePos, BOOL bShowOutGrow, int *framebuffer, int w, int h);
	void SetFont(const LOGFONT &LogFont);

	CMemFont();
	virtual ~CMemFont();

public:
	int		m_nColorInner;
	int		m_nColorOuter;
	LOGFONT	m_LogFont;

protected:
	//
	// 以下几个成员描述了字符串缓冲区显示用到的缓冲区。
	//
	HDC		m_hdcBk;
	HBITMAP	m_hBmpBk;
	HBITMAP	m_hBmpOld;
	int		m_nWidth;				// 缓冲区的宽度
	int		m_nHeight;				// 高度
	HFONT	m_hFontOld;
	HFONT	m_hFont;


	//
	// 将字符串转换到内存中……
	LPBYTE			m_lpbBmpData;				//include BITMAPDATA  /////BITMAPINFO + 
	LPBITMAPINFO	m_lpbi;				//BITMAPINFO (include infoheader and RGBQUAD)
	DWORD			m_dwbiSize;			//the size of lpbif-->>
	DWORD			m_dwDataSize;			//The size of bmp data
	int				m_nBitCount;			// the bit count of bitmap

	//
	// 显示到Avs中……
	WCHAR			m_pszLyrics[2][LYRICS_LEN_MAX + 1];	// 在Avs中歌词分两行显示
	// int				m_nX;

	//
	// 字符串
	LYRICS_ROW	m_LyricsRow[LYRICS_ROW_MAX];		// 字符串缓冲区
	int		m_nCurObsLyricsRow;				// 当前可废弃的缓冲区，采用循环队列的形式，来释放

	//
	// MiniLyrics
	HWND			m_hWndMiniLyrics;
	WVS_LYRICS_DATA_W	m_LyricsData;
	int				m_nRowPitch;		// 歌词行间距

};

#endif // !defined(AFX_MEMFONT_H__BBCA349E_7823_427D_902F_6A440C12FFAD__INCLUDED_)
