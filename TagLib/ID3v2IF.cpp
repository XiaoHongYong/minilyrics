#include "StdAfx.h"
#include "ID3v2IF.h"
#include "../ID3Lib/ID3v1.h"
#include "../ID3Lib/ID3v2FrameParser.h"
#include "TagLib.h"

#define SZ_ID3V2_LYR_USER_DEFINED		_T("LYRICS")

bool IsUnsynchLyricsFrameID(UINT nFrameID)
{
	return nFrameID == ID3V2_2_USLT || nFrameID == ID3V2_3_USLT;
}

bool IsSynchLyricsFrameID(UINT nFrameID)
{
	return nFrameID == ID3V2_2_SYLT || nFrameID == ID3V2_3_SYLT;
}

void AddEmbeddedLyrics(VecString &vNames, tstring &name)
{
	int next = 0;
	for (int i = 0; i < vNames.size(); i++)
	{
		if (StrBeginWith(vNames[i].c_str(), name.c_str()))
			next++;
	}

	if (next > 0)
		name += CStrPrintf(_T("/%d"), next).c_str();
	vNames.push_back(name);
}

//////////////////////////////////////////////////////////////////////////
//
CID3v2IF::CID3v2IF(CHAR_ENCODING encoding) : CID3v2(encoding)
{
}

CID3v2IF::~CID3v2IF()
{
}

int CID3v2IF::ListLyrics(VecString &vLyrNames)
{
	int		nRet;

	for (FrameIterator it = FrameBegin(); it != FrameEnd(); ++it)
	{
		CID3v2Frame		*pFrame = *it;
		if (IsUnsynchLyricsFrameID(pFrame->m_framehdr.nFrameID))
		{
			CID3v2FrameParserUnsynLyrics	parser(m_Encoding);
			ID3v2UnsynchLyrics	unsyncLyrics;

			nRet = parser.ParseInfoOnly(pFrame, unsyncLyrics);
			if (nRet == ERR_OK)
			{
				tstring strLang;
				AnsiToTString(unsyncLyrics.m_szLanguage, -1, strLang);
				tstring name = tstring(SZ_SONG_ID3V2_USLT) + strLang;
				AddEmbeddedLyrics(vLyrNames, name);
			}
		}
		else if (IsSynchLyricsFrameID(pFrame->m_framehdr.nFrameID))
		{
			CID3v2FrameParserSynLyrics parser(m_Encoding);
			ID3v2SynchLyrics	syncLyrics;

			nRet = parser.ParseInfoOnly(pFrame, syncLyrics);
			if (nRet == ERR_OK && syncLyrics.m_byContentType == ID3v2SynchLyrics::CT_LYRICS)
			{
				tstring strLang;
				AnsiToTString(syncLyrics.m_szLanguage, -1, strLang);
				tstring name = tstring(SZ_SONG_ID3V2_SYLT) + strLang;
				AddEmbeddedLyrics(vLyrNames, name);
			}
		}
		else if (pFrame->m_framehdr.nFrameID == ID3V2_2_USER_TEXT
			|| pFrame->m_framehdr.nFrameID == ID3V2_3_USER_TEXT)
		{
			ID3v2TextUserDefined	lyrics;
			CID3v2FrameParserTextUserDefined parser(m_Encoding);

			nRet = parser.Parse(pFrame, lyrics);
			if (nRet == ERR_OK 
				&& _tcsicmp(SZ_ID3V2_LYR_USER_DEFINED, lyrics.GetDescription()) == 0)
				vLyrNames.push_back(SZ_SONG_ID3V2_LYRICS);
		}
	}

	return ERR_OK;
}

int CID3v2IF::GetSyncLyrics(ID3v2SynchLyrics &lyrics, LPCTSTR szName)
{
	FrameIterator it = GetSyncLyricsFrame(szName);
	if (it == FrameEnd())
		return ERR_NOT_FOUND;

	CID3v2FrameParserSynLyrics	parser(m_Encoding);

	int nRet = parser.Parse(*it, lyrics);
	if (nRet != ERR_OK)
		DBGLOG1(_T("Failed to parse Sync Lyrics frame: %s"), Error2Str(nRet).c_str());

	return nRet;
}

int CID3v2IF::GetUnsyncLyrics(ID3v2UnsynchLyrics &lyrics, LPCTSTR szName)
{
	FrameIterator it = GetUnsyncLyricsFrame(szName);
	if (it == FrameEnd())
		return ERR_NOT_FOUND;

	CID3v2FrameParserUnsynLyrics	parser(m_Encoding);

	int nRet = parser.Parse(*it, lyrics);
	if (nRet != ERR_OK)
		DBGLOG1(_T("Failed to parse Sync Lyrics frame: %s"), Error2Str(nRet).c_str());

	return nRet;
}

int CID3v2IF::GetUserDefLyrics(ID3v2TextUserDefined &lyrics)
{
	FrameIterator it = GetUserDefLyricsFrame();
	if (it == FrameEnd())
		return ERR_NOT_FOUND;

	CID3v2FrameParserTextUserDefined	parser(m_Encoding);
	return parser.Parse(*it, lyrics);
}

int CID3v2IF::RemoveLyrics(LPCTSTR szName)
{
	if (IsEmptyString(szName))
		return ERR_NOT_FOUND;

	FrameIterator it = m_listFrames.end();
	if (StrBeginWith(szName, SZ_SONG_ID3V2_USLT))
		it = GetUnsyncLyricsFrame(szName);
	else if (StrBeginWith(szName, SZ_SONG_ID3V2_SYLT))
		it = GetSyncLyricsFrame(szName);
	else if (StrBeginWith(szName, SZ_SONG_ID3V2_LYRICS))
		it = GetUserDefLyricsFrame();

	if (it == m_listFrames.end())
		return ERR_NOT_FOUND;

	FrameRemove(it);

	return ERR_OK;
}

int CID3v2IF::SetSynchLyrics(LPCTSTR szName, ID3v2SynchLyrics &lyrics)
{
	UINT nFrameID = IsID3v2_2() ? (int)ID3V2_2_SYLT : (int)ID3V2_3_SYLT;
	CID3v2Frame *pTargFrame = NULL;
	FrameIterator it = GetSyncLyricsFrame(szName);
	if (it == m_listFrames.end())
	{
		// not found, so add it.
		pTargFrame = new CID3v2Frame(nFrameID);
		FrameAdd(pTargFrame);
	}
	else
		pTargFrame = *it;

	// do update
	pTargFrame->m_frameData.clear();
	CID3v2FrameParserSynLyrics	parser(m_Encoding);
	return parser.ToFrameData(lyrics, pTargFrame);
}

int CID3v2IF::SetUnsynchLyrics(LPCTSTR szName, LPCTSTR szDesc, LPCTSTR szLyrics)
{
	ID3v2UnsynchLyrics unsyncLyr;
	string language;
	int index;

	GetEmbeddedLyricsNameInfo(szName, language, index);

	if (language.empty())
		language = "eng";
	strcpy_safe(unsyncLyr.m_szLanguage, CountOf(unsyncLyr.m_szLanguage), language.c_str());
	unsyncLyr.SetValueAndOptimizeEncoding(szDesc, szLyrics);

	return SetUnsynchLyrics(szName, unsyncLyr);
}

int CID3v2IF::SetUnsynchLyrics(LPCTSTR szName, ID3v2UnsynchLyrics &lyrics)
{
	UINT nFrameID = IsID3v2_2() ? (int)ID3V2_2_USLT: (int)ID3V2_3_USLT;
	CID3v2Frame *pTargFrame = NULL;
	FrameIterator it = GetUnsyncLyricsFrame(szName);
	if (it == m_listFrames.end())
	{
		// not found, so add it.
		pTargFrame = new CID3v2Frame(nFrameID);
		FrameAdd(pTargFrame);
	}
	else
		pTargFrame = *it;

	// do update
	pTargFrame->m_frameData.clear();
	CID3v2FrameParserUnsynLyrics	parser(m_Encoding);
	int nRet = parser.ToFrameData(lyrics, pTargFrame);
	return nRet;
}

int CID3v2IF::SetUserDefLyrics(LPCTSTR szLyrics)
{
	UINT nFrameID = IsID3v2_2() ? (int)ID3V2_2_USER_TEXT : (int)ID3V2_3_USER_TEXT;
	CID3v2Frame *pTargFrame = NULL;
	FrameIterator it = GetUserDefLyricsFrame();
	if (it == m_listFrames.end())
	{
		// not found, so add it.
		pTargFrame = new CID3v2Frame(nFrameID);
		FrameAdd(pTargFrame);
	}
	else
		pTargFrame = *it;

	ID3v2TextUserDefined text;
	text.SetValueAndOptimizeEncoding(SZ_ID3V2_LYR_USER_DEFINED, szLyrics);

	// do update
	pTargFrame->m_frameData.clear();
	CID3v2FrameParserTextUserDefined parser(m_Encoding);
	int nRet = parser.ToFrameData(text, pTargFrame);
	return nRet;
}

UINT	_vTextFrameIDV22[] = { ID3V2_2_ARTIST, ID3V2_2_TITLE, ID3V2_2_ALBUM,
	ID3V2_2_TRACK, ID3V2_2_YEAR, ID3V2_2_GENRE };

UINT	_vTextFrameIDV23[] = { ID3V2_3_ARTIST, ID3V2_3_TITLE, ID3V2_3_ALBUM,
	ID3V2_3_TRACK, ID3V2_3_YEAR, ID3V2_3_GENRE };

// TCOM	: Composer
// TPOS :
// TPE1 : Artist
// TPE2 : Album artist
// TALB : Album
// TIT2 : Title
// TRCK : Track
// TYER : Year
// COMM : Comment
// TCON : Genre
// APIC : pic
// USLT : unsyn lyrics
// TENC : encoded
// WXXX : url
// TCOP : Copy right
// TOPE : Orig. artist
// TPUB : Publisher
// TBPM : bpm
int CID3v2IF::GetTags(tstring &artist, tstring &title, tstring &album, tstring &comment, 
					  tstring &track, tstring &year, tstring &genre)
{
	int			nRet;
	ID3v2Text	txt;
	UINT		*vTextFrameID;
	UINT		nFrameID;
	CID3v2Frame	*pFrame;

	tstring		*vStr[] = { &artist, &title, &album,
		&track, &year, &genre
	};

	if (IsID3v2_2())
		vTextFrameID = _vTextFrameIDV22;
	else
		vTextFrameID = _vTextFrameIDV23;

	// read general text frames
	for (int i = 0; i < CountOf(_vTextFrameIDV22); i++)
	{
		for (FrameIterator it = FrameBegin(); it != FrameEnd(); ++it)
		{
			CID3v2Frame		*pFrame = *it;
			if (pFrame->m_framehdr.nFrameID == vTextFrameID[i])
			{
				CID3v2FrameParserText	txtParser(m_Encoding);
				nRet = txtParser.Parse(pFrame, txt);
				if (nRet == ERR_OK)
					*vStr[i] = txt.GetValue();
			}
		}
	}

	// read comment
	if (IsID3v2_2())
		nFrameID = ID3V2_2_COMMENT;
	else
		nFrameID = ID3V2_3_COMMENT;
	pFrame = FindFrame(nFrameID);
	if (pFrame)
	{
		CID3v2FrameParserComment	parser(m_Encoding);
		nRet = parser.Parse(pFrame);
		if (nRet == ERR_OK)
			comment = parser.m_strText.c_str();
	}

	// read year
	if (year.empty())
	{
		if (IsID3v2_2())
			nFrameID = ID3V2_2_YEAR2;
		else
			nFrameID = ID3V2_3_YEAR2;
		pFrame = FindFrame(nFrameID);
		if (pFrame)
		{
			CID3v2FrameParserText	txtParser(m_Encoding);
			nRet = txtParser.Parse(pFrame, txt);
			if (nRet == ERR_OK)
				year = txt.GetValue();
		}
	}

	// read genre info
	if (!genre.empty())
	{
		if (IsStrNumeric(genre.c_str()))
		{
			tstringex	str;
			ConvertStr(CID3v1::GetGenreDescription(_ttoi(genre.c_str())), -1, str);
			genre = str.c_str();
		}
		else if (genre[0] == '(')
		{
			LPCTSTR		szGenre = genre.c_str();
			szGenre++;
			int			nGenre = _ttoi(szGenre);
			while (isdigit(*szGenre))
				szGenre++;
			if (*szGenre == ')')
			{
				szGenre++;
				LPCSTR		szNewGenre;
				szNewGenre = CID3v1::GetGenreDescription(nGenre);
				if (szNewGenre)
				{
					tstringex	str;
					ConvertStr(szNewGenre, -1, str);
					if (_tcscmp(str.c_str(), szGenre) == 0)
					{
						genre = str.c_str();
					}
				}
			}
		}
	}

	return ERR_OK;
}

int CID3v2IF::SetTags(LPCTSTR szArtist, LPCTSTR szTitle, LPCTSTR szAlbum, LPCTSTR szComment, 
					  LPCTSTR szTrack, LPCTSTR szYear, LPCTSTR szGenre)
{
	UINT	nFrameID;

	LPCTSTR	vTextFrameValues[CountOf(_vTextFrameIDV22)] = { 
		szArtist, szTitle, szAlbum,
		szTrack, szYear, szGenre };

		for (int i = 0; i < CountOf(vTextFrameValues); i++)
		{
			if (vTextFrameValues[i])
			{
				if (IsID3v2_2())
					nFrameID = _vTextFrameIDV22[i];
				else
					nFrameID = _vTextFrameIDV23[i];
				if (IsEmptyString(vTextFrameValues[i]))
					RemoveFrame(nFrameID);
				else
					UpdateTextFrame(nFrameID, vTextFrameValues[i]);
			}
		}

		// update comment
		if (szComment)
		{
			if (IsEmptyString(szComment))
				RemoveGeneralCommentFrame();
			else
				UpdateGeneralCommentFrame(szComment);
		}

		return ERR_OK;
}

int CID3v2IF::UpdateUserDefinedTextFrameByDesc(ID3v2TextUserDefined &text)
{
	int					nRet;
	CID3v2Frame			*pFrame = NULL;
	FrameIterator		it;
	UINT				nFrameID;
	CID3v2FrameParserTextUserDefined	parser(m_Encoding);

	if (IsID3v2_2())
		nFrameID = ID3V2_2_USER_TEXT;
	else
		nFrameID = ID3V2_3_USER_TEXT;

	for (it = FrameBegin(); it != FrameEnd(); ++it)
	{
		pFrame = *it;
		if (pFrame->m_framehdr.nFrameID == nFrameID)
		{
			ID3v2TextUserDefined	temp;
			nRet = parser.Parse(pFrame, temp);
			if (nRet == ERR_OK)
			{
				if (_tcscmp(text.GetDescription(), temp.GetDescription()) == 0)
					break;
			}
		}
	}

	if (it == FrameEnd())
	{
		// not found, so add it.
		pFrame = new CID3v2Frame(nFrameID);
		FrameAdd(pFrame);
	}

	// do update
	ASSERT(pFrame);
	pFrame->m_frameData.clear();
	return parser.ToFrameData(text, pFrame);
}

int CID3v2IF::UpdateGeneralCommentFrame(LPCTSTR szText)
{
	CID3v2Frame			*pFrame;
	FrameIterator		it;
	UINT				nFrameID;
	CID3v2FrameParserComment	parser(m_Encoding);

	if (IsID3v2_2())
		nFrameID = ID3V2_2_COMMENT;
	else
		nFrameID = ID3V2_3_COMMENT;

	pFrame = FindFrame(nFrameID);
	if (!pFrame)
	{
		// not found, so add it.
		pFrame = new CID3v2Frame(nFrameID);
		FrameAdd(pFrame);

		if (IsAnsiStr(szText))
			parser.m_EncodingType = IET_ANSI;
		else
			parser.m_EncodingType = IET_UCS2LE_BOM;
	}

	parser.SetText(NULL, szText);

	// do update
	pFrame->m_frameData.clear();
	return parser.ToFrame(pFrame);
}

// what is a general comment frame?
// 1) language is empty.
// 2) description is empty.
int CID3v2IF::RemoveGeneralCommentFrame()
{
	int					nRet;
	CID3v2Frame			*pFrame;
	FrameIterator		it;
	UINT				nFrameID;
	CID3v2FrameParserComment	parser(m_Encoding);

	if (IsID3v2_2())
		nFrameID = ID3V2_2_COMMENT;
	else
		nFrameID = ID3V2_3_COMMENT;

	for (it = FrameBegin(); it != FrameEnd(); ++it)
	{
		pFrame = *it;

		if (pFrame->m_framehdr.nFrameID == nFrameID)
		{
			nRet = parser.Parse(pFrame);
			if (nRet == ERR_OK)
			{
				if (IsEmptyString(parser.m_szLanguage) && 
					parser.m_strShortDesc.empty())
				{
					// find it, and remove it.
					FrameRemove(it);
					return ERR_OK;
				}
			}
		}
	}

	return ERR_NOT_FOUND;
}

int CID3v2IF::UpdateGeneralUrlFrame(LPCTSTR szText)
{
	int					nRet;
	CID3v2Frame			*pFrame = NULL;
	CID3v2FrameParserUserDefinedUrl	parser(m_Encoding);
	UINT				nFrameID;

	if (IsID3v2_2())
		nFrameID = ID3V2_2_URL;
	else
		nFrameID = ID3V2_3_URL;

	pFrame = FindFrame(nFrameID);
	if (!pFrame)
	{
		// not found, so add it.
		pFrame = new CID3v2Frame(nFrameID);
		FrameAdd(pFrame);
	}

	ConvertStr(szText, -1, parser.m_strUrl);
	parser.m_textDesc.m_EncodingType = IET_ANSI;

	// do update
	pFrame->m_frameData.clear();
	nRet = parser.ToFrame(pFrame);
	return nRet;
}

int CID3v2IF::RemoveGeneralUrlFrame()
{
	if (IsID3v2_2())
		return RemoveFrame(ID3V2_2_URL);
	else
		return RemoveFrame(ID3V2_3_URL);
}

int CID3v2IF::GetPictures(ID3v2Pictures &pictures)
{
	int						nRet;
	FrameIterator			it;
	CID3v2Frame				*pFrame = NULL;
	CID3v2FrameParserPic	parser(m_Encoding);
	UINT					nFrameID;
	ID3v2Pictures::ITEM		*pic;

	if (IsID3v2_2())
		nFrameID = ID3V2_2_PICTURE;
	else
		nFrameID = ID3V2_3_PICTURE;

	pictures.Free();

	for (it = FrameBegin(); it != FrameEnd(); ++it)
	{
		pFrame = *it;
		if (pFrame->m_framehdr.nFrameID == nFrameID)
		{
			pic = new ID3v2Pictures::ITEM;
			nRet = parser.Parse(this, pFrame, pic);
			if (nRet == ERR_OK)
				pictures.m_vItems.push_back(pic);
			else
			{
				ERRLOG1(_T("Parse ID3v2 picture frame failed: %s"), (LPCTSTR)Error2Str(nRet));
				delete pic;
			}
		}
	}

	return ERR_OK;
}

int CID3v2IF::UpdatePictures(ID3v2Pictures &pictures)
{
	int		nRet;

	for (ID3v2Pictures::V_ITEMS::iterator it = pictures.m_vItems.begin();
		it != pictures.m_vItems.end(); ++it)
	{
		ID3v2Pictures::ITEM		*pic = *it;
		if (pic->action == IFA_DEL)
			nRet = RemoveFrameByUID(pic->frameUID);
		else if (pic->action == IFA_MODIFY || pic->action == IFA_ADD)
			nRet = UpdatePicFrame(pic);
		else
			continue;
		if (nRet != ERR_OK)
			return nRet;
	}

	return ERR_OK;
}

int CID3v2IF::UpdatePicFrame(ID3v2Pictures::ITEM *pic)
{
	int						nRet;
	CID3v2Frame				*pFrame = NULL;
	CID3v2FrameParserPic	parser(m_Encoding);
	UINT					nFrameID;

	if (IsID3v2_2())
		nFrameID = ID3V2_2_PICTURE;
	else
		nFrameID = ID3V2_3_PICTURE;

	if (pic->frameUID != -1)
		pFrame = FindFrameByUID(pic->frameUID, nFrameID);
	if (!pFrame)
	{
		// not found, so add it.
		pFrame = new CID3v2Frame(nFrameID);
		FrameAdd(pFrame);
	}

	// do update
	pFrame->m_frameData.clear();
	nRet = parser.ToFrame(this, pFrame, pic);
	return nRet;
}

int CID3v2IF::RemovePicFrame(ID3v2FrameUID_t nFrameUID, UINT picType)
{
	CID3v2Frame			*pFrame = NULL;
	FrameIterator		it;
	UINT				nFrameID;

	if (IsID3v2_2())
		nFrameID = ID3V2_2_PICTURE;
	else
		nFrameID = ID3V2_3_PICTURE;

	for (it = FrameBegin(); it != FrameEnd(); ++it)
	{
		pFrame = *it;

		if (pFrame->m_framehdr.nFrameID == nFrameID)
		{
			FrameRemove(it);
			return ERR_OK;
		}
	}

	return ERR_NOT_FOUND;
}

int CID3v2IF::UpdateTextFrame(UINT nFrameID, LPCTSTR szText)
{
	int					nRet;
	CID3v2Frame			*pFrame = NULL;
	ID3v2Text			text;
	CID3v2FrameParserText	parser(m_Encoding);

	pFrame = FindFrame(nFrameID);
	if (!pFrame)
	{
		// not found, so add it.
		pFrame = new CID3v2Frame(nFrameID);

#ifdef _DEBUG
		if (m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V2)
			ASSERT((nFrameID & 0xFF) == 0);
#endif
		FrameAdd(pFrame);
	}

	text.SetValueAndOptimizeEncoding(szText);

	// do update
	pFrame->m_frameData.clear();
	nRet = parser.ToFrameData(text, pFrame);
	return nRet;
}

CID3v2IF::FrameIterator CID3v2IF::GetSyncLyricsFrame(LPCTSTR szName)
{
	string language;
	int index;
	if (!GetEmbeddedLyricsNameInfo(szName, language, index))
		return m_listFrames.end();

	int nFrameID = IsID3v2_2() ? (int)ID3V2_2_SYLT : (int)ID3V2_3_SYLT;

	if (language.empty())
		return FindFrameIter(nFrameID);

	FrameIterator itLastFound = m_listFrames.end();
	for (FrameIterator it = m_listFrames.begin(); it != m_listFrames.end(); ++it)
	{
		CID3v2Frame *pFrame = *it;
		if (pFrame->GetFrameID() == nFrameID
			&& CID3v2FrameParserSynLyrics::IsFrameLanguageSame(pFrame, language.c_str()))
		{
			itLastFound = it;
			if (index == 0)
				return it;
			else
				index--;
		}
	}

	return itLastFound;
}

CID3v2IF::FrameIterator CID3v2IF::GetUnsyncLyricsFrame(LPCTSTR szName)
{
	string language;
	int index;
	if (!GetEmbeddedLyricsNameInfo(szName, language, index))
		return m_listFrames.end();

	int nFrameID = IsID3v2_2() ? (int)ID3V2_2_USLT : (int)ID3V2_3_USLT;

	if (language.empty())
		return FindFrameIter(nFrameID);

	FrameIterator itLastFound = m_listFrames.end();
	for (FrameIterator it = m_listFrames.begin(); it != m_listFrames.end(); ++it)
	{
		CID3v2Frame *pFrame = *it;
		if (pFrame->GetFrameID() == nFrameID
			&& CID3v2FrameParserUnsynLyrics::IsFrameLanguageSame(pFrame, language.c_str()))
		{
			itLastFound = it;
			if (index == 0)
				return it;
			else
				index--;
		}
	}

	return itLastFound;
}

CID3v2IF::FrameIterator CID3v2IF::GetUserDefLyricsFrame()
{
	UINT nFrameID = IsID3v2_2() ? (int)ID3V2_2_USER_TEXT : (int)ID3V2_3_USER_TEXT;

	CID3v2FrameParserTextUserDefined parser(m_Encoding);
	for (FrameIterator it = m_listFrames.begin(); it != m_listFrames.end(); ++it)
	{
		CID3v2Frame *pFrame = *it;
		if (pFrame->GetFrameID() != nFrameID)
			continue;

		ID3v2TextUserDefined temp;
		int nRet = parser.Parse(*it, temp);
		if (nRet == ERR_OK)
		{
			if (_tcscmp(SZ_ID3V2_LYR_USER_DEFINED, temp.GetDescription()) == 0)
				return it;
		}
	}

	return m_listFrames.end();
}
