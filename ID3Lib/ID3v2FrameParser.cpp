// ID3v2FrameParser.cpp: implementation of the CID3v2FrameParser class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ID3v2FrameParser.h"
#include "ID3Helper.h"

// The ID3v2 tags needed to process
// 4.30.   Audio seek point index


//    Frames that allow different types of text encoding contains a text
//    encoding description byte. Possible encodings:
// 
//      $00   ISO-8859-1 [ISO-8859-1]. Terminated with $00.
//      $01   UTF-16 [UTF-16] encoded Unicode [UNICODE] with BOM. All
//            strings in the same frame SHALL have the same byteorder.
//            Terminated with $00 00.
//      $02   UTF-16BE [UTF-16] encoded Unicode [UNICODE] without BOM.
//            Terminated with $00 00.
//      $03   UTF-8 [UTF-8] encoded Unicode [UNICODE]. Terminated with $00.

//#define SizeOfUcs2StrWithBom(strUcs2)	((strUcs2.size() + 1 + 1) * sizeof(wchar_t))

//////////////////////////////////////////////////////////////////////////
// CID3v2FrameParserSynLyrics

CID3v2FrameParserSynLyrics::CID3v2FrameParserSynLyrics()
{
}

CID3v2FrameParserSynLyrics::~CID3v2FrameParserSynLyrics()
{
}

int CID3v2FrameParserSynLyrics::ParseInfoOnly(CID3v2Frame *pFrame, ID3v2SynchLyrics &synchLyrics)
{
//     Text encoding        $xx
//     Language             $xx xx xx
//     Time stamp format    $xx
//     Content type         $xx
//     Content descriptor   <textstring> $00 (00)

//   Content type:   $00 is other
//                   $01 is lyrics
//                   $02 is text transcription
//                   $03 is movement/part name (e.g. "Adagio")
//                   $04 is events (e.g. "Don Quijote enters the stage")
//                   $05 is chord (e.g. "Bb F Fsus")
//   Time stamp format is:
//     
//     $01  Absolute time, 32 bit sized, using MPEG [MPEG] frames as unit
//     $02  Absolute time, 32 bit sized, using milliseconds as unit

//   The text that follows the frame header differs from that of the
//   unsynchronised lyrics/text transcription in one major way. Each
//   syllable (or whatever size of text is considered to be convenient by
//   the encoder) is a null terminated string followed by a time stamp
//   denoting where in the sound file it belongs. Each sync thus has the
//   following structure:
//
//     Terminated text to be synced (typically a syllable)
//     Sync identifier (terminator to above string)   $00 (00)
//     Time stamp                                     $xx (xx ...)
	const ID3v2FrameHdr &frameHdr = pFrame->m_framehdr;
	const char *data = pFrame->m_frameData.c_str();
	int		len = pFrame->m_frameData.size();

	if (len <= 6)
		return ERR_INVALID_ID3V2_FRAME;

	synchLyrics.frameUID = frameHdr.frameUID;

	synchLyrics.m_encodingType = (ID3v2EncType)data[0];

	strncpysz_safe(synchLyrics.m_szLanguage, CountOf(synchLyrics.m_szLanguage), data + 1, 3);

	if (data[4] == 02)
		synchLyrics.m_bTimeStampMs = true;
	else
		synchLyrics.m_bTimeStampMs = false;

	synchLyrics.m_byContentType = data[5];

	data += 6;
	len -= 6;

	if (synchLyrics.m_encodingType == IET_ANSI)
	{
		// ISO-8859-1
		synchLyrics.m_strAnsiContentDesc.reserve(len);
		strncpysz_safe(synchLyrics.m_strAnsiContentDesc.data(), synchLyrics.m_strAnsiContentDesc.capacity(), data, len);
		synchLyrics.m_strAnsiContentDesc.resize(strlen(synchLyrics.m_strAnsiContentDesc.c_str()));
	}
	else
	{
		// Unicode
		CopyUcs2StrByBom(synchLyrics.m_strUcs2ContentDesc, synchLyrics.m_encodingType, (const byte *)data, len);
	}

	return ERR_OK;
}

int CID3v2FrameParserSynLyrics::Parse(CID3v2Frame *pFrame, ID3v2SynchLyrics &synchLyrics)
{
//     Text encoding        $xx
//     Language             $xx xx xx
//     Time stamp format    $xx
//     Content type         $xx
//     Content descriptor   <textstring> $00 (00)

//   Content type:   $00 is other
//                   $01 is lyrics
//                   $02 is text transcription
//                   $03 is movement/part name (e.g. "Adagio")
//                   $04 is events (e.g. "Don Quijote enters the stage")
//                   $05 is chord (e.g. "Bb F Fsus")
//   Time stamp format is:
//     
//     $01  Absolute time, 32 bit sized, using MPEG [MPEG] frames as unit
//     $02  Absolute time, 32 bit sized, using milliseconds as unit

//   The text that follows the frame header differs from that of the
//   unsynchronised lyrics/text transcription in one major way. Each
//   syllable (or whatever size of text is considered to be convenient by
//   the encoder) is a null terminated string followed by a time stamp
//   denoting where in the sound file it belongs. Each sync thus has the
//   following structure:
//
//     Terminated text to be synced (typically a syllable)
//     Sync identifier (terminator to above string)   $00 (00)
//     Time stamp                                     $xx (xx ...)
	const ID3v2FrameHdr &frameHdr = pFrame->m_framehdr;
	const char *data = pFrame->m_frameData.c_str();
	int		len = pFrame->m_frameData.size();

	if (len <= 6)
		return ERR_INVALID_ID3V2_FRAME;

	synchLyrics.frameUID = frameHdr.frameUID;

	synchLyrics.m_encodingType = (ID3v2EncType)data[0];

	strncpysz_safe(synchLyrics.m_szLanguage, CountOf(synchLyrics.m_szLanguage), data + 1, 3);

	if (data[4] == 02)
		synchLyrics.m_bTimeStampMs = true;
	else
		synchLyrics.m_bTimeStampMs = false;

	synchLyrics.m_byContentType = data[5];

	data += 6;
	len -= 6;

	if (synchLyrics.m_encodingType == IET_ANSI)
	{
		// ISO-8859-1
		synchLyrics.m_strAnsiContentDesc.reserve(len);
		strncpysz_safe(synchLyrics.m_strAnsiContentDesc.data(), synchLyrics.m_strAnsiContentDesc.capacity(), data, len);
		synchLyrics.m_strAnsiContentDesc.resize(strlen(synchLyrics.m_strAnsiContentDesc.c_str()));

		len -= synchLyrics.m_strAnsiContentDesc.size() + 1;
		data += synchLyrics.m_strAnsiContentDesc.size() + 1;

		while (len > 0)
		{
			LrcSyllable<char>		syllable;

			while (data[0] == 0xA)
			{
				syllable.bNewLine = true;
				data++;
				len--;
				synchLyrics.m_vAnsiSynLyrics.push_back(syllable);
				synchLyrics.m_bAllSyllableIsNewLine = false;
			}

			if (len < 5)
			{
				if (len != 0)
					ERRLOG1(_T("Can't recognize all lyrics data, len: %d"), len);
				return ERR_OK;
			}

			syllable.bNewLine = false;
			syllable.strText = data;
			data += syllable.strText.size() + 1;
			len -= syllable.strText.size() + 1;

			syllable.nTime = ByteDataToUInt((byte *)data, 4);
			data += 4;
			len -= 4;

			synchLyrics.m_vAnsiSynLyrics.push_back(syllable);
		}
	}
	else
	{
		// Unicode
		int		n;
		n = CopyUcs2StrByBom(synchLyrics.m_strUcs2ContentDesc, synchLyrics.m_encodingType, (const byte *)data, len);
		data += n;
		len -= n;

		const wchar_t		*wdata = (const wchar_t *)data;
		int			wlen = len / sizeof(wchar_t);
		while (wlen > 0)
		{
			LrcSyllable<wchar_t>		syllable;

			while (wdata[0] == 0xA)
			{
				syllable.bNewLine = true;
				wdata++;
				wlen--;
				synchLyrics.m_vUcs2SynLyrics.push_back(syllable);
				synchLyrics.m_bAllSyllableIsNewLine = false;
			}

			if (wlen < 3)
			{
				if (wlen != 0)
					ERRLOG1(_T("Can't recognize all lyrics data, len: %d"), wlen);
				return ERR_OK;
			}

			syllable.bNewLine = false;
			n = CopyUcs2StrByBom(syllable.strText, synchLyrics.m_encodingType, (const byte *&)wdata, wlen * sizeof(wchar_t));
			wdata += n / 2;
			wlen -= n / 2;

			syllable.nTime = ByteDataToUInt((byte *)wdata, 4);
			wdata += 2;
			wlen -= 2;

			synchLyrics.m_vUcs2SynLyrics.push_back(syllable);
		}
	}

	return ERR_OK;
}

int CID3v2FrameParserSynLyrics::ToFrameData(ID3v2SynchLyrics &synchLyrics, CID3v2Frame *pFrame)
{
	CCharBuffer &buff = pFrame->m_frameData;

	buff += (char)synchLyrics.m_encodingType;

	if (strlen(synchLyrics.m_szLanguage) != 3)
		strcpy_safe(synchLyrics.m_szLanguage, CountOf(synchLyrics.m_szLanguage), "XXX");
	buff += synchLyrics.m_szLanguage;

	buff += (char)synchLyrics.m_encodingType;

	ASSERT(synchLyrics.m_byContentType == CT_LYRICS);
	buff += synchLyrics.m_byContentType;

	if (synchLyrics.m_encodingType == IET_ANSI)
	{
		// ISO-8859-1
		buff.append(synchLyrics.m_strAnsiContentDesc.c_str(), synchLyrics.m_strAnsiContentDesc.size() + 1);

		byte		byTime[10];
		for (ID3v2SynchLyrics::LIST_SYLRC::iterator it = synchLyrics.m_vAnsiSynLyrics.begin();
			it != synchLyrics.m_vAnsiSynLyrics.end(); it++)
		{
			LrcSyllable<char>		&syllable = *it;
			if (syllable.bNewLine)
				buff += (char)0xA;
			else
			{
				buff.append(syllable.strText.c_str(), syllable.strText.size() + 1);

				ByteDataFromUInt(syllable.nTime, byTime, 4);
				buff.append((const char *)byTime, 4);
			}
		}
	}
	else
	{
		// Unicode
		AppendUcs2Str(buff, synchLyrics.m_encodingType, synchLyrics.m_strUcs2ContentDesc.c_str(),
			synchLyrics.m_strUcs2ContentDesc.size());

		byte		byTime[10];
		for (ID3v2SynchLyrics::LIST_SYLRC_W::iterator it = synchLyrics.m_vUcs2SynLyrics.begin();
			it != synchLyrics.m_vUcs2SynLyrics.end(); it++)
		{
			LrcSyllable<wchar_t>		&syllable = *it;
			if (syllable.bNewLine)
			{
				buff += (char)0xA;
				buff += (char)0x0;
			}
			else
			{
				AppendUcs2Str(buff, synchLyrics.m_encodingType, syllable.strText.c_str(),
					syllable.strText.size());

				ByteDataFromUInt(syllable.nTime, byTime, 4);
				buff.append((const char *)byTime, 4);
			}
		}
	}

	return ERR_OK;
}

bool CID3v2FrameParserSynLyrics::IsFrameLanguageSame(CID3v2Frame *pFrame, const char *szLanguage, CONTENT_TYPE contentType)
{
	const char *data = pFrame->m_frameData.c_str();
	int		len = pFrame->m_frameData.size();

	if (len <= 6)
		return false;

	if (strncmp(szLanguage, data + 1, 3) == 0
		&& contentType == data[5])
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////
// CID3v2FrameParserUnsynLyrics

CID3v2FrameParserUnsynLyrics::CID3v2FrameParserUnsynLyrics()
{
}

CID3v2FrameParserUnsynLyrics::~CID3v2FrameParserUnsynLyrics()
{
}

int CID3v2FrameParserUnsynLyrics::ParseInfoOnly(CID3v2Frame *pFrame, ID3v2UnsynchLyrics &unsynchLyrics)
{
/*****************************************************************
<Header for 'Unsynchronised lyrics/text transcription', ID: "USLT"> 
	Text encoding		$xx 
	Language			$xx xx xx 
	Content descriptor	<text string according to encoding> $00 (00) 
	Lyrics/text			<full text string according to encoding> 
*****************************************************************/
	const ID3v2FrameHdr &frameHdr = pFrame->m_framehdr;
	const char *data = pFrame->m_frameData.c_str();
	int		len = pFrame->m_frameData.size();

	if (len <= 4)
		return ERR_INVALID_ID3V2_FRAME;

	unsynchLyrics.frameUID = frameHdr.frameUID;

	unsynchLyrics.m_encodingType = (ID3v2EncType)data[0];

	strncpysz_safe(unsynchLyrics.m_szLanguage, CountOf(unsynchLyrics.m_szLanguage), data + 1, 3);

	data += 4;
	len -= 4;

	if (unsynchLyrics.m_encodingType == IET_ANSI)
	{
		// ISO-8859-1
		unsynchLyrics.m_strAnsiContentDesc.reserve(len);
		strncpysz_safe(unsynchLyrics.m_strAnsiContentDesc.data(), unsynchLyrics.m_strAnsiContentDesc.capacity(), data, len);
		unsynchLyrics.m_strAnsiContentDesc.resize(strlen(unsynchLyrics.m_strAnsiContentDesc.c_str()));
	}
	else
	{
		// Unicode
		CopyUcs2StrByBom(unsynchLyrics.m_strUcs2ContentDesc, unsynchLyrics.m_encodingType, (const byte *)data, len);
	}
	
	return ERR_OK;
}

int CID3v2FrameParserUnsynLyrics::Parse(CID3v2Frame *pFrame, ID3v2UnsynchLyrics &unsynchLyrics)
{
/*****************************************************************
<Header for 'Unsynchronised lyrics/text transcription', ID: "USLT"> 
	Text encoding		$xx 
	Language			$xx xx xx 
	Content descriptor	<text string according to encoding> $00 (00) 
	Lyrics/text			<full text string according to encoding> 
*****************************************************************/
	const ID3v2FrameHdr &frameHdr = pFrame->m_framehdr;
	const char *data = pFrame->m_frameData.c_str();
	int		len = pFrame->m_frameData.size();

	if (len <= 4)
		return ERR_INVALID_ID3V2_FRAME;

	unsynchLyrics.frameUID = frameHdr.frameUID;

	unsynchLyrics.m_encodingType = (ID3v2EncType)data[0];

	strncpysz_safe(unsynchLyrics.m_szLanguage, CountOf(unsynchLyrics.m_szLanguage), data + 1, 3);

	data += 4;
	len -= 4;

	if (unsynchLyrics.m_encodingType == IET_ANSI)
	{
		// ISO-8859-1

		unsynchLyrics.m_strAnsiContentDesc.reserve(len);
		strncpysz_safe(unsynchLyrics.m_strAnsiContentDesc.data(), unsynchLyrics.m_strAnsiContentDesc.capacity(), data, len);
		unsynchLyrics.m_strAnsiContentDesc.resize(strlen(unsynchLyrics.m_strAnsiContentDesc.c_str()));

		data += unsynchLyrics.m_strAnsiContentDesc.size() + 1;
		len -= unsynchLyrics.m_strAnsiContentDesc.size() + 1;

		if (len > 0)
		{
			unsynchLyrics.m_strAnsiLyrics.reserve(len);
			strncpysz_safe(unsynchLyrics.m_strAnsiLyrics.data(), unsynchLyrics.m_strAnsiLyrics.capacity(), data, len);
			unsynchLyrics.m_strAnsiLyrics.resize(strlen(unsynchLyrics.m_strAnsiLyrics.c_str()));
		}
	}
	else
	{
		// Unicode
		int		n;

		n = CopyUcs2StrByBom(unsynchLyrics.m_strUcs2ContentDesc, unsynchLyrics.m_encodingType, (const byte *)data, len);
		data += n;
		len -= n;
		if (len > 0)
			CopyUcs2StrByBom(unsynchLyrics.m_strUcs2Lyrics, unsynchLyrics.m_encodingType, (const byte *)data, len);
	}
	
	return ERR_OK;
}

int CID3v2FrameParserUnsynLyrics::ToFrameData(ID3v2UnsynchLyrics &unsynchLyrics, CID3v2Frame *pFrame)
{
	CCharBuffer &buff = pFrame->m_frameData;

	buff += (char)unsynchLyrics.m_encodingType;

	if (strlen(unsynchLyrics.m_szLanguage) != 3)
		buff += "XXX";
	else
		buff += unsynchLyrics.m_szLanguage;

	if (unsynchLyrics.m_encodingType == IET_ANSI)
	{
		// ISO-8859-1
		buff.append((const char *)unsynchLyrics.m_strAnsiContentDesc.c_str(), (unsynchLyrics.m_strAnsiContentDesc.size() + 1) * sizeof(char));

		buff.append((const char *)unsynchLyrics.m_strAnsiLyrics.c_str(), (unsynchLyrics.m_strAnsiLyrics.size() + 1) * sizeof(char));
	}
	else
	{
		// Unicode
		AppendUcs2Str(buff, unsynchLyrics.m_encodingType, unsynchLyrics.m_strUcs2ContentDesc.c_str(),
			unsynchLyrics.m_strUcs2ContentDesc.size());

		AppendUcs2Str(buff, unsynchLyrics.m_encodingType, unsynchLyrics.m_strUcs2Lyrics.c_str(),
			unsynchLyrics.m_strUcs2Lyrics.size());
	}

	return ERR_OK;
}

bool CID3v2FrameParserUnsynLyrics::IsFrameLanguageSame(CID3v2Frame *pFrame, const char *szLanguage)
{
	const char *data = pFrame->m_frameData.c_str();
	int		len = pFrame->m_frameData.size();

	if (len <= 4)
		return false;

	if (strncmp(szLanguage, data + 1, 3) == 0)
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////

void ID3v2Text::SetValue(LPCTSTR szValue)
{
#ifdef _UNICODE
	if (m_EncodingType == IET_ANSI)
		ConvertStr(szValue, -1, m_strAnsi);
	else
		m_strUnicode = szValue;
#else
	if (m_EncodingType == IET_ANSI)
		m_strAnsi = szValue;
	else
		ConvertStr(szValue, -1, m_strUnicode);
#endif
}

void ID3v2Text::SetValueAndOptimizeEncoding(LPCTSTR szValue)
{
	if (m_EncodingType == IET_ANSI && !IsAnsiStr(szValue))
		m_EncodingType = IET_UCS2LE_BOM;
	SetValue(szValue);
}

LPCTSTR ID3v2Text::GetValue()
{
#ifdef _UNICODE
	if (m_EncodingType == IET_ANSI)
		ConvertStr(m_strAnsi.c_str(), m_strAnsi.size(), m_strUnicode);
	return m_strUnicode.c_str();
#else
	if (m_EncodingType != IET_ANSI)
		ConvertStr(m_strUnicode.c_str(), m_strUnicode.size(), m_strAnsi);
	return m_strAnsi.c_str();
#endif
}

//////////////////////////////////////////////////////////////////////////

void ID3v2TextUserDefined::SetValue(LPCTSTR szDescription, LPCTSTR szValue)
{
#ifdef _UNICODE
	if (m_EncodingType == IET_ANSI)
	{
		ConvertStr(szDescription, -1, m_strDescAnsi);
		ConvertStr(szValue, -1, m_strValueAnsi);
	}
	else
	{
		m_strValueUCS2 = szDescription;
		m_strDescUCS2 = szValue;
	}
#else
	if (m_EncodingType == IET_ANSI)
	{
		m_strDescAnsi = szDescription;
		m_strValueAnsi = szValue;
	}
	else
	{
		ConvertStr(szDescription, -1, m_strDescUCS2);
		ConvertStr(szValue, -1, m_strValueUCS2);
	}
#endif
}

void ID3v2TextUserDefined::SetValueAndOptimizeEncoding(LPCTSTR szDescription, LPCTSTR szValue)
{
	if (m_EncodingType == IET_ANSI && !IsAnsiStr(szDescription) && !IsAnsiStr(szValue))
		m_EncodingType = IET_UCS2LE_BOM;
	SetValue(szDescription, szValue);
}

LPCTSTR ID3v2TextUserDefined::GetDescription()
{
#ifdef _UNICODE
	if (m_EncodingType == IET_ANSI)
		ConvertStr(m_strDescAnsi.c_str(), m_strDescAnsi.size(), m_strDescUCS2);
	return m_strDescUCS2.c_str();
#else
	if (m_EncodingType != IET_ANSI)
		ConvertStr(m_strDescUCS2.c_str(), m_strDescUCS2.size(), m_strDescAnsi);
	return m_strDescAnsi.c_str();
#endif
}

LPCTSTR ID3v2TextUserDefined::GetValue()
{
#ifdef _UNICODE
	if (m_EncodingType == IET_ANSI)
		ConvertStr(m_strValueAnsi.c_str(), m_strValueAnsi.size(), m_strValueUCS2);
	return m_strValueUCS2.c_str();
#else
	if (m_EncodingType != IET_ANSI)
		ConvertStr(m_strValueUCS2.c_str(), m_strValueUCS2.size(), m_strValueAnsi);
	return m_strValueAnsi.c_str();
#endif
}

//////////////////////////////////////////////////////////////////////////
// CID3v2FrameParserText

int CID3v2FrameParserText::Parse(const CID3v2Frame *pFrame, ID3v2Text &text)
{
	const ID3v2FrameHdr &frameHdr = pFrame->m_framehdr;
	const char *data = pFrame->m_frameData.c_str();
	int		len = pFrame->m_frameData.size();

	if (len < 2)
		return ERR_INVALID_ID3V2_FRAME;

	text.m_EncodingType = (ID3v2EncType)data[0];

	if (text.m_EncodingType == IET_ANSI)
	{
		// ISO-8859-1
		CopyAnsiStr(text.m_strAnsi, (const char *)(data + 1), len - 1);
	}
	else
	{
		// Unicode
		data++;
		len--;
		CopyUcs2StrByBom(text.m_strUnicode, text.m_EncodingType, (const byte *)data, len);
	}

	return ERR_OK;
}

int CID3v2FrameParserText::ToFrameData(ID3v2Text &text, CID3v2Frame *pFrame)
{
	CCharBuffer &buff = pFrame->m_frameData;

	buff.clear();
	buff += (char)text.m_EncodingType;
	if (text.m_EncodingType == IET_ANSI)
	{
		// ANSI
		AppendAnsiStr(buff, text.m_strAnsi);
		// buff.append(text.m_strAnsi.clear(), (text.m_strAnsi.size() + 1) * sizeof(char));
	}
	else
	{
		// unicode
		AppendUcs2Str(buff, text.m_EncodingType, text.m_strUnicode.c_str(), text.m_strUnicode.size());
	}

	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
// CID3v2FrameParserTextUserDefined

int CID3v2FrameParserTextUserDefined::Parse(const CID3v2Frame *pFrame, ID3v2TextUserDefined &text)
{
	const ID3v2FrameHdr &frameHdr = pFrame->m_framehdr;
	const char *data = pFrame->m_frameData.c_str();
	int		len = pFrame->m_frameData.size();
	int		nBytesCopied;

	if (len < 3)
		return ERR_INVALID_ID3V2_FRAME;

	text.m_EncodingType = (ID3v2EncType)data[0];

	if (text.m_EncodingType == IET_ANSI)
	{
		// ISO-8859-1
		data++;
		len--;

		nBytesCopied = CopyAnsiStr(text.m_strDescAnsi, data, len);
		data += nBytesCopied;
		len -= nBytesCopied;

		if (len <= 0)
			return ERR_INVALID_ID3V2_FRAME;

		nBytesCopied = CopyAnsiStr(text.m_strValueAnsi, data, len);
	}
	else
	{
		// Unicode
		data++;
		len--;

		nBytesCopied = CopyUcs2StrByBom(text.m_strDescUCS2, text.m_EncodingType, (const byte *)data, len);
		data += nBytesCopied;
		len -= nBytesCopied;

		CopyUcs2StrByBom(text.m_strValueUCS2, text.m_EncodingType, (const byte *)data, len);
	}

	return ERR_OK;
}

int CID3v2FrameParserTextUserDefined::ToFrameData(ID3v2TextUserDefined &text, CID3v2Frame *pFrame)
{
	CCharBuffer &buff = pFrame->m_frameData;

	buff.clear();
	buff += (char)text.m_EncodingType;
	if (text.m_EncodingType == IET_ANSI)
	{
		// ANSI
		AppendAnsiStr(buff, text.m_strDescAnsi);
		AppendAnsiStr(buff, text.m_strValueAnsi);
	}
	else
	{
		// UNICODE
		AppendUcs2Str(buff, text.m_EncodingType, text.m_strDescUCS2.c_str(), text.m_strDescUCS2.size());
		AppendUcs2Str(buff, text.m_EncodingType, text.m_strValueUCS2.c_str(), text.m_strValueUCS2.size());
	}

	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int CID3v2FrameParserComment::Parse(const CID3v2Frame *pFrame)
{
	if (pFrame->m_frameData.size() < 6)
		return ERR_INVALID_ID3V2_FRAME;

	const char *data = pFrame->m_frameData.c_str();
	size_t		nPos = 0;

	m_EncodingType = (ID3v2EncType)data[0];

	m_szLanguage[0] = data[1];
	m_szLanguage[1] = data[2];
	m_szLanguage[2] = data[3];
	m_szLanguage[3] = '\0';

	nPos += 4;
	if (m_EncodingType == IET_ANSI)
	{
		nPos += CopyAnsiStr(m_strAnsiShortDesc, data + nPos, pFrame->m_frameData.size() - nPos);
		if (nPos >= pFrame->m_frameData.size())
			return ERR_INVALID_ID3V2_FRAME;
		CopyAnsiStr(m_strAnsiText, data + nPos, pFrame->m_frameData.size() - nPos);
	}
	else
	{
		// Unicode
		nPos += CopyUcs2StrByBom(m_strUcs2ShortDesc, m_EncodingType, (const byte *)data + nPos, pFrame->m_frameData.size() - nPos);
		if (nPos >= pFrame->m_frameData.size())
			return ERR_INVALID_ID3V2_FRAME;
		CopyUcs2StrByBom(m_strUcs2Text, m_EncodingType, (const byte *)data + nPos, pFrame->m_frameData.size() - nPos);
	}

	return ERR_OK;
}

int CID3v2FrameParserComment::ToFrame(CID3v2Frame *pFrame)
{
	pFrame->m_frameData.clear();

	pFrame->m_frameData += char(m_EncodingType);
	pFrame->m_frameData.append(m_szLanguage, 3);

	if (m_EncodingType == IET_ANSI)
	{
		// ANSI
		AppendAnsiStr(pFrame->m_frameData, m_strAnsiShortDesc);
		AppendAnsiStr(pFrame->m_frameData, m_strAnsiText);
	}
	else
	{
		// unicode
		AppendUcs2Str(pFrame->m_frameData, m_EncodingType, m_strUcs2ShortDesc.c_str(), m_strUcs2ShortDesc.size());
		AppendUcs2Str(pFrame->m_frameData, m_EncodingType, m_strUcs2Text.c_str(), m_strUcs2Text.size());
	}

	return ERR_OK;
}

void CID3v2FrameParserComment::SetText(LPCTSTR szShortDesc, LPCTSTR szText)
{
	if (szShortDesc)
	{
		if (m_EncodingType == IET_ANSI)
			ConvertStr(szShortDesc, -1, m_strAnsiShortDesc);
		else
			ConvertStr(szShortDesc, -1, m_strUcs2ShortDesc);
	}

	if (szText)
	{
		if (m_EncodingType == IET_ANSI)
			ConvertStr(szText, -1, m_strAnsiText);
		else
			ConvertStr(szText, -1, m_strUcs2Text);
	}
}

//////////////////////////////////////////////////////////////////////////
int CID3v2FrameParserPic::Parse(CID3v2 *pid3v2, const CID3v2Frame *pFrame, ID3v2Pictures::ITEM *picture)
{
	if (pFrame->m_frameData.size() < 4)
		return ERR_INVALID_ID3V2_FRAME;

	const char *data = pFrame->m_frameData.c_str();
	size_t		nPos = 0;

	picture->frameUID = pFrame->m_framehdr.frameUID;

	//  Text encoding      $xx
	picture->m_text.m_EncodingType = (ID3v2EncType)data[nPos];
	nPos++;

	if (pid3v2->IsID3v2_2())
	{
		//  Image format       $xx xx xx
		ConvertStr(data + nPos, 3, picture->m_strMimeType);
		nPos += 3;
	}
	else
	{
		//  MIME type          <text string> $00
		nPos += CopyAnsiStr(picture->m_strMimeType, data + nPos, pFrame->m_frameData.size() - nPos);
		if (nPos >= pFrame->m_frameData.size())
			return ERR_INVALID_ID3V2_FRAME;
	}

	//  Picture type       $xx
	picture->m_picType = (ID3v2Pictures::PicType)(data[nPos]);
	nPos++;

	//  Description        <text string according to encoding> $00 (00)
	if (picture->m_text.m_EncodingType == IET_ANSI)
	{
		nPos += CopyAnsiStr(picture->m_text.m_strAnsi, data + nPos, pFrame->m_frameData.size() - nPos);
	}
	else
	{
		// Unicode
		nPos += CopyUcs2StrByBom(picture->m_text.m_strUnicode, picture->m_text.m_EncodingType, (const byte *)data + nPos, pFrame->m_frameData.size() - nPos);
	}
	if (nPos > pFrame->m_frameData.size())
		return ERR_INVALID_ID3V2_FRAME;

	//  Picture data       <binary data>
	picture->m_buffPic.append(data + nPos, pFrame->m_frameData.size() - nPos);

	return ERR_OK;
}

int CID3v2FrameParserPic::ToFrame(CID3v2 *pid3v2, CID3v2Frame *pFrame, ID3v2Pictures::ITEM *picture)
{
	pFrame->m_frameData.clear();

	pFrame->m_frameData += char(picture->m_text.m_EncodingType);

	if (pid3v2->IsID3v2_2())
	{
		//  Image format       $xx xx xx
		char	szPicExt[4];
		picture->MimeToPicExt(szPicExt);
		pFrame->m_frameData.append(szPicExt, 3);
	}
	else
	{
		//  MIME type          <text string> $00
		AppendAnsiStr(pFrame->m_frameData, picture->m_strMimeType);
	}

	pFrame->m_frameData += char(picture->m_picType);

	if (picture->m_text.m_EncodingType == IET_ANSI)
	{
		// ANSI
		AppendAnsiStr(pFrame->m_frameData, picture->m_text.m_strAnsi);
	}
	else
	{
		// unicode
		AppendUcs2Str(pFrame->m_frameData, picture->m_text.m_EncodingType, picture->m_text.m_strUnicode.c_str(), picture->m_text.m_strUnicode.size());
	}

	pFrame->m_frameData.append(picture->m_buffPic.c_str(), picture->m_buffPic.size());

	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int CID3v2FrameParserUserDefinedUrl::Parse(const CID3v2Frame *pFrame)
{
	if (pFrame->m_frameData.size() < 3)
		return ERR_INVALID_ID3V2_FRAME;

	const char *data = pFrame->m_frameData.c_str();
	size_t		nPos = 0;

	m_textDesc.m_EncodingType = (ID3v2EncType)data[0];

	nPos++;
	if (m_textDesc.m_EncodingType == IET_ANSI)
	{
		nPos += CopyAnsiStr(m_textDesc.m_strAnsi, data + nPos, pFrame->m_frameData.size() - nPos);
	}
	else
	{
		// Unicode
		nPos += CopyUcs2StrByBom(m_textDesc.m_strUnicode, m_textDesc.m_EncodingType, (const byte *)data + nPos, pFrame->m_frameData.size() - nPos);
	}
	if (nPos >= pFrame->m_frameData.size())
		return ERR_INVALID_ID3V2_FRAME;

	CopyAnsiStr(m_strUrl, data + nPos, pFrame->m_frameData.size() - nPos);

	return ERR_OK;
}

int CID3v2FrameParserUserDefinedUrl::ToFrame(CID3v2Frame *pFrame)
{
	pFrame->m_frameData.clear();

	pFrame->m_frameData += char(m_textDesc.m_EncodingType);

	if (m_textDesc.m_EncodingType == IET_ANSI)
	{
		// ANSI
		AppendAnsiStr(pFrame->m_frameData, m_textDesc.m_strAnsi);
	}
	else
	{
		// unicode
		AppendUcs2Str(pFrame->m_frameData, m_textDesc.m_EncodingType, m_textDesc.m_strUnicode.c_str(), m_textDesc.m_strUnicode.size());
	}

	AppendAnsiStr(pFrame->m_frameData, m_strUrl);

	return ERR_OK;
}
