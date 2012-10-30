// ID3LibTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ID3LibTest.h"
#include "ID3LibTestDlg.h"
#include "../ID3Lib/ID3v2.h"
#include "../ID3Lib/ID3v2Frame.h"
#include "../ID3Lib/ID3v2FrameParser.h"
#include "../ID3Lib/ID3v1.h"
/*
void TestID3v2Lyrics(LPCTSTR szMp3File)
{
	CID3v2IF		id3v2;

	nRet = id3v2.Open(szMp3File, false, false);
	if (nRet == ERR_OK)
	{
		ID3v2LyricsInfo		lyricsInfo;
		tstringex			str;
		nRet = id3v2.ReadLyrics(lyricsInfo);
		if (nRet == ERR_OK)
		{
			{
				// sync lyrics
				ID3v2SynchLyrics	*plyrics;

				for (ID3v2LyricsInfo::LIST_SYNCHLYRICS::iterator it = lyricsInfo.m_synchLyrics.begin();
					it != lyricsInfo.m_synchLyrics.end(); it++)
				{
					plyrics = *it;
				}
			}

			{
				// unsync lyrics
				ID3v2UnsynchLyrics	*plyrics;
				for (ID3v2LyricsInfo::LIST_UNSYNLYRICS::iterator it = lyricsInfo.m_synchLyrics.begin();
					it != lyricsInfo.m_synchLyrics.end(); it++)
				{
					plyrics = *it;
				}
			}
		}
		id3v2.Close();
	}
}*/

void UintToByteStr(UINT n, char str[])
{
	LPCSTR p = (LPCSTR)&n;
	memcpy(str, p, 4);
	for (int i = 0; i < 4; i++)
		str[i] = p[3 - i];
	str[4] = 0;
}

void EnumID3v3Tag(LPCTSTR szMp3File)
{
	CID3v2		id3v2(ED_SYSDEF);
	int			nRet;

	nRet = id3v2.Open(szMp3File, false, false);
	if (nRet == ERR_OK)
	{
		CID3v2::FrameIterator		it, itEnd;

		DBGLOG1(_T("ID3v2:%d"), id3v2.m_id3v2Header.byMajorVer);

		itEnd = id3v2.FrameEnd();
		for (it = id3v2.FrameBegin(); it != itEnd; ++it)
		{
			CID3v2Frame		*fr = *it;
			char			szFrameID[5];
			UintToByteStr(fr->m_framehdr.nFrameID, szFrameID);
			DBGLOG2(_T("%S, %d"), szFrameID, fr->m_framehdr.nLen);

			if (fr->m_framehdr.nFrameID == ID3V2_2_USER_TEXT || fr->m_framehdr.nFrameID == ID3V2_3_USER_TEXT)
			{
				CID3v2FrameParserTextUserDefined	parser(ED_SYSDEF);
				ID3v2TextUserDefined	text;
				nRet = parser.Parse(fr, text);
				if (nRet != ERR_OK)
					ERRLOG1(_T("Parse User defined Text frame Failed: %s"), (LPCTSTR)Error2Str(nRet));
				else
				{
					DBGLOG1(_T("User defined Text: %s"), text.GetDescription());
					SaveStringAsFile(CStrPrintf(_T("c:\\TXXX%s.txt"), text.GetDescription()).c_str(), text.GetValue(), _tcslen(text.GetValue()));
				}
			}
			else if (szFrameID[0] == 'T')
			{
				CID3v2FrameParserText	txtParser(ED_SYSDEF);
				ID3v2Text		txt;
				nRet = txtParser.Parse(fr, txt);
				if (nRet != ERR_OK)
					ERRLOG1(_T("Parse Txt frame Failed: %s"), (LPCTSTR)Error2Str(nRet));
				else
				{
					DBGLOG2(_T("Encoding: %d, Value: %s"), txt.m_EncodingType, txt.GetValue());
				}
			}
			else if (fr->m_framehdr.nFrameID == ID3V2_3_COMMENT || fr->m_framehdr.nFrameID == ID3V2_2_COMMENT)
			{
				CID3v2FrameParserComment	parser(ED_SYSDEF);
				nRet = parser.Parse(fr);
				if (nRet != ERR_OK)
					ERRLOG1(_T("Parse Txt frame Failed: %s"), (LPCTSTR)Error2Str(nRet));
				else
				{
					DBGLOG3(_T("Comment: %s, %s, %s"), parser.m_szLanguage, parser.m_strShortDesc.c_str(), parser.m_strText.c_str());
				}
			}
			else if (fr->m_framehdr.nFrameID == ID3V2_3_PICTURE || fr->m_framehdr.nFrameID == ID3V2_2_PICTURE)
			{
				CID3v2FrameParserPic	parser(ED_SYSDEF);
				ID3v2Pictures::ITEM		picture;
				nRet = parser.Parse(&id3v2, fr, &picture);
				if (nRet != ERR_OK)
					ERRLOG1(_T("Parse APIC frame Failed: %s"), (LPCTSTR)Error2Str(nRet));
				else
				{
					DBGLOG3(_T("Pic: %S, %s, %d"), picture.m_strMimeType.c_str(), picture.m_text.GetValue(), picture.m_picType);
					SaveDataAsFile(_T("c:\\1.jpg"), picture.m_buffPic.c_str(), picture.m_buffPic.size());
				}
			}
			else if (fr->m_framehdr.nFrameID == ID3V2_3_SYLT || fr->m_framehdr.nFrameID == ID3V2_2_SYLT)
			{
				CID3v2FrameParserSynLyrics	parser(ED_SYSDEF);
				ID3v2SynchLyrics		lyrics;

				nRet = parser.Parse(fr, lyrics);
				if (nRet != ERR_OK)
					ERRLOG1(_T("Parse SYLT frame Failed: %s"), (LPCTSTR)Error2Str(nRet));
				else
				{
					ERRLOG1(_T("Lyrics: %s"), lyrics.m_strContentDesc.c_str());
				}
			}
			else if (fr->m_framehdr.nFrameID == ID3V2_3_USLT || fr->m_framehdr.nFrameID == ID3V2_2_USLT)
			{
				CID3v2FrameParserUnsynLyrics	parser(ED_SYSDEF);
				ID3v2UnsynchLyrics				lyrics;

				nRet = parser.Parse(fr, lyrics);
				if (nRet != ERR_OK)
					ERRLOG1(_T("Parse SYLT frame Failed: %s"), (LPCTSTR)Error2Str(nRet));
				else
				{
					ERRLOG1(_T("Lyrics: %s"), lyrics.m_strContentDesc.c_str());
				}
			}
		}
	}
	else
		ERRLOG1(_T("Open Failed: %s"), (LPCTSTR)Error2Str(nRet));
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CID3LibTestDlg dialog

CID3LibTestDlg::CID3LibTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CID3LibTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CID3LibTestDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CID3LibTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CID3LibTestDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CID3LibTestDlg, CDialog)
	//{{AFX_MSG_MAP(CID3LibTestDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPEN_MP3, OnOpenMp3)
	ON_BN_CLICKED(IDC_MODIFY, OnModify)
	ON_BN_CLICKED(IDC_ADD_IMG, OnAddImg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CID3LibTestDlg message handlers

BOOL CID3LibTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	LPCSTR *szGenre = CID3v1::GetAllGenreDescription();
	int		n = CID3v1::GetGenreCount();
	for (int i = 0; i < n; i++)
	{
		wstringex	str;
		ConvertStr(szGenre[i], -1, str);
		SendDlgItemMessage(IDC_CB_GENRE, CB_ADDSTRING, 0, (LPARAM)str.c_str());
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CID3LibTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CID3LibTestDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CID3LibTestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CID3LibTestDlg::OnOpenMp3() 
{
	CFileDialog		dlg(true, NULL, g_Profile.GetString(_T("LastOpenedMp3File"), _T("")), 0, _T("*.mp3|*.mp3||"));

	if (dlg.DoModal() != IDOK)
		return;

	g_Profile.WriteString(_T("LastOpenedMp3File"), dlg.GetPathName());

	EnumID3v3Tag(dlg.GetPathName());

	m_strMp3File = dlg.GetPathName();

	CID3v2IF	id3v2(ED_SYSDEF);
	int			nRet;

	nRet = id3v2.Open(m_strMp3File.c_str(), false, false);
	if (nRet == ERR_OK)
	{
		nRet = id3v2.GetTags(artist,  title,  album,  comment,
		track,  year,  genre);
		if (nRet == ERR_OK)
		{
			SetDlgItemText(IDC_E_ARTIST, artist.c_str());
			SetDlgItemText(IDC_E_ALBUM, album.c_str());
			SetDlgItemText(IDC_E_TITLE, title.c_str());
			SetDlgItemText(IDC_E_TRACK, track.c_str());
			SetDlgItemText(IDC_E_YEAR, year.c_str());
			SetDlgItemText(IDC_E_COMPOSER, composer.c_str());
			SetDlgItemText(IDC_E_COPYRIGHT, copyRight.c_str());
			SetDlgItemText(IDC_E_ENCODED, encoded.c_str());
			SetDlgItemText(IDC_E_URL, url.c_str());
			SetDlgItemText(IDC_E_ORIG_ARTIST, origArtist.c_str());
			SetDlgItemText(IDC_E_PUBLISHER, publisher.c_str());
			SetDlgItemText(IDC_E_BPM, bpm.c_str());
			SetDlgItemText(IDC_E_COMMENT, comment.c_str());
			SetDlgItemText(IDC_CB_GENRE, genre.c_str());
		}
	}
}

#define GetNewValue(strOld, strNew)		_tcscmp(strOld.c_str(), strNew.c_str()) == 0 ? NULL : strNew.c_str()

void CID3LibTestDlg::OnModify() 
{
	tstring		strArtist, strTitle, strAlbum, strComment;
	tstring		strTrack, strYear, strGenre, strComposer;
	tstring		strEncoded, strUrl, strCopyright, strOrigArtist, strPublisher, strBpm;

	GetDlgItemText(IDC_E_ARTIST, strArtist);
	GetDlgItemText(IDC_E_ALBUM, strAlbum);
	GetDlgItemText(IDC_E_TITLE, strTitle);
	GetDlgItemText(IDC_E_TRACK, strTrack);
	GetDlgItemText(IDC_E_YEAR, strYear);
	GetDlgItemText(IDC_E_COMPOSER, strComposer);
	GetDlgItemText(IDC_E_COPYRIGHT, strCopyright);
	GetDlgItemText(IDC_E_ENCODED, strEncoded);
	GetDlgItemText(IDC_E_URL, strUrl);
	GetDlgItemText(IDC_E_ORIG_ARTIST, strOrigArtist);
	GetDlgItemText(IDC_E_PUBLISHER, strPublisher);
	GetDlgItemText(IDC_E_BPM, strBpm);
	GetDlgItemText(IDC_CB_GENRE, strGenre);
	GetDlgItemText(IDC_E_COMMENT, strComment);

	CID3v2IF	id3v2(ED_SYSDEF);
	int			nRet;

	nRet = id3v2.Open(m_strMp3File.c_str(), true, true);
	if (nRet == ERR_OK)
	{
		nRet = id3v2.SetTags(GetNewValue(artist, strArtist), GetNewValue(title, strTitle), GetNewValue(album, strAlbum), GetNewValue(comment, strComment),
			GetNewValue(track, strTrack), GetNewValue(year, strYear), GetNewValue(genre, strGenre)
			);
		if (nRet != ERR_OK)
		{
			AfxMessageBox(LPCTSTR(Error2Str(nRet)));
			return;
		}
		nRet = id3v2.Save();
		if (nRet != ERR_OK)
		{
			AfxMessageBox(LPCTSTR(Error2Str(nRet)));
		}
	}
}

void CID3LibTestDlg::GetDlgItemText(int nID, tstring &str)
{
	CString	s;
	CWnd::GetDlgItemText(nID, s);
	str = s;
}

void CID3LibTestDlg::OnAddImg() 
{
	tstring			strImage;
	CFileDialog		dlg(true, NULL, g_Profile.GetString(_T("LastOpenedPicFile"), _T("")), 0, _T("*.jpg|*.jpg||"));

	if (dlg.DoModal() != IDOK)
		return;

	g_Profile.WriteString(_T("LastOpenedPicFile"), dlg.GetPathName());

	strImage = dlg.GetPathName();


	CID3v2IF	id3v2(ED_SYSDEF);
	int			nRet;

	nRet = id3v2.Open(m_strMp3File.c_str(), true, true);
	if (nRet == ERR_OK)
	{
		ID3v2Pictures::ITEM		picture;

		picture.SetImageFile(strImage.c_str());
		picture.m_picType = ID3v2Pictures::PT_COVER_FRONT;
		picture.m_text.SetValue(_T("cover xxx"));
		nRet = id3v2.UpdatePicFrame(&picture);
		if (nRet != ERR_OK)
		{
			AfxMessageBox(LPCTSTR(Error2Str(nRet)));
			return;
		}
		nRet = id3v2.Save();
		if (nRet != ERR_OK)
		{
			AfxMessageBox(LPCTSTR(Error2Str(nRet)));
		}
	}
}
