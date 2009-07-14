// ID3v2FrameParser.h: interface for the CID3v2FrameParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ID3V2FRAMEPARSER_H__749002BF_ECF1_4CCA_8869_17AFACE76F16__INCLUDED_)
#define AFX_ID3V2FRAMEPARSER_H__749002BF_ECF1_4CCA_8869_17AFACE76F16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ID3v2.h"

class CID3v2FrameParserBase  
{
public:
	CID3v2FrameParserBase() { }
	virtual ~CID3v2FrameParserBase() { }

//	virtual int Parse(CID3v2Frame *pFrame) = 0;
//	virtual int ToFrameData(CID3v2Frame *pFrame) = 0;

};

class CID3v2FrameParserSynLyrics : public CID3v2FrameParserBase
{
public:
	enum CONTENT_TYPE
	{
		CT_OTHER		= 0,
		CT_LYRICS		= 1,
		CT_TEXT_TRANS	= 2,
		CT_MOVEMENT		= 3,
		CT_EVENT		= 4,
		CT_CHORD		= 5
	};

	CID3v2FrameParserSynLyrics();
	virtual ~CID3v2FrameParserSynLyrics();

	virtual int ParseInfoOnly(CID3v2Frame *pFrame, ID3v2SynchLyrics &synchLyrics);
	virtual int Parse(CID3v2Frame *pFrame, ID3v2SynchLyrics &synchLyrics);
	virtual int ToFrameData(ID3v2SynchLyrics &synchLyrics, CID3v2Frame *pFrame);

	static bool IsFrameLanguageSame(CID3v2Frame *pFrame, const char *szLanguage, CONTENT_TYPE contentType);

protected:

//	ID3v2SynchLyrics		m_Lyrics;

};

class CID3v2FrameParserUnsynLyrics : public CID3v2FrameParserBase
{
public:
	CID3v2FrameParserUnsynLyrics();
	virtual ~CID3v2FrameParserUnsynLyrics();

	virtual int ParseInfoOnly(CID3v2Frame *pFrame, ID3v2UnsynchLyrics &unsynchLyrics);
	virtual int Parse(CID3v2Frame *pFrame, ID3v2UnsynchLyrics &unsynchLyrics);
	virtual int ToFrameData(ID3v2UnsynchLyrics &unsynchLyrics, CID3v2Frame *pFrame);

	static bool IsFrameLanguageSame(CID3v2Frame *pFrame, const char *szLanguage);

protected:
	// ID3v2UnsynchLyrics		m_Lyrics;

};

class CID3v2FrameParserText : public CID3v2FrameParserBase
{
public:
	CID3v2FrameParserText();
	virtual ~CID3v2FrameParserText();

	virtual int Parse(const CID3v2Frame *pFrame, ID3v2Text &text);
	virtual int ToFrameData(ID3v2Text &text, CID3v2Frame *pFrame);

protected:

//	ID3v2SynchLyrics		m_Lyrics;

};

//  <Header for 'Comment', ID: "COMM">
//  Text encoding          $xx
//  Language               $xx xx xx
//  Short content descrip. <text string according to encoding> $00 (00)
//  The actual text        <full text string according to encoding>
class CID3v2FrameParserComment : public CID3v2FrameParserBase
{
public:
	CID3v2FrameParserComment()
	{
		memset(m_szLanguage, 0, sizeof(m_szLanguage));
		m_EncodingType = IET_ANSI;
	}
	virtual int Parse(const CID3v2Frame *pFrame);
	virtual int ToFrame(CID3v2Frame *pFrame);

	void SetText(LPCTSTR szShortDesc, LPCTSTR szText);

public:
	ID3v2EncType	m_EncodingType;
	char			m_szLanguage[4];
	string			m_strAnsiShortDesc, m_strAnsiText;
	wstringex		m_strUcs2ShortDesc, m_strUcs2Text;

};

//  <Header for 'Attached picture', ID: "APIC">

// V2.2
//  Text encoding      $xx
//  Image format       $xx xx xx
//  Picture type       $xx
//  Description        <textstring> $00 (00)
//  Picture data       <binary data>
//
// V2.3
//  Text encoding      $xx
//  MIME type          <text string> $00
//  Picture type       $xx
//  Description        <text string according to encoding> $00 (00)
//  Picture data       <binary data>
class CID3v2FrameParserPic : public CID3v2FrameParserBase
{
public:
	CID3v2FrameParserPic()
	{
	}
	virtual int Parse(CID3v2 *pid3v2, const CID3v2Frame *pFrame, ID3v2Pictures::ITEM *picture);
	virtual int ToFrame(CID3v2 *pid3v2, CID3v2Frame *pFrame, ID3v2Pictures::ITEM *picture);

};


//  <Header for 'User defined URL link frame', ID: "WXXX">
//  Text encoding     $xx
//  Description       <text string according to encoding> $00 (00)
//  URL               <text string>
class CID3v2FrameParserUserDefinedUrl : public CID3v2FrameParserBase
{
public:
	virtual int Parse(const CID3v2Frame *pFrame);
	virtual int ToFrame(CID3v2Frame *pFrame);

public:
	ID3v2Text		m_textDesc;
	string			m_strUrl;

};


#endif // !defined(AFX_ID3V2FRAMEPARSER_H__749002BF_ECF1_4CCA_8869_17AFACE76F16__INCLUDED_)
