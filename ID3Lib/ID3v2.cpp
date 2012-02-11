// ID3v2.cpp: implementation of the CID3v2 class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ID3v2.h"
#include <bitset>
#include "ID3v2FrameParser.h"
#include "ID3v2Frame.h"
#include "ID3Helper.h"
#include "ID3v1.h"

void LowerString(LPSTR szString);

CHAR_ENCODING ID3v2EncTypeToCharEncoding(ID3v2EncType encType)
{
	switch (encType)
	{
	case IET_UCS2LE_BOM:
		return ED_UNICODE;
	case IET_UCS2BE_NO_BOM:
		return ED_UNICODE_BIG_ENDIAN;
	case IET_UTF8:
		return ED_UTF8;
	case IET_ANSI:
		return ED_SYSDEF;
	default:
		ASSERT(0);
		return ED_SYSDEF;
	}
}

ID3v2EncType CharEncodingToID3v2EncType(CHAR_ENCODING encoding)
{
	switch (encoding)
	{
	case ED_UNICODE:
		return IET_UCS2LE_BOM;
	case ED_UNICODE_BIG_ENDIAN:
		return IET_UCS2BE_NO_BOM;
	case ED_UTF8:
		return IET_UTF8;
	default:
		return IET_ANSI;
	}
}

LPCTSTR _ID3v2PicDescription[] = {
	_T("Other"),
	_T("32x32 pixels 'file icon' (PNG only)"),
	_T("Other file icon"),
	_T("Cover (front)"),
	_T("Cover (back)"),
	_T("Leaflet page"),
	_T("Media (e.g. label side of CD)"),
	_T("Lead artist/lead performer/soloist"),
	_T("Artist/performer"),
	_T("Conductor"),
	_T("Band/Orchestra"),
	_T("Composer"),
	_T("Lyricist/text writer"),
	_T("Recording Location"),
	_T("During recording"),
	_T("During performance"),
	_T("Movie/video screen capture"),
	_T("A bright coloured fish"),
	_T("Illustration"),
	_T("Band/artist logotype"),
	_T("Publisher/Studio logotype")
};

ID3v2SynchLyrics::ID3v2SynchLyrics()
{
	frameUID = -1;
	action = IFA_NONE;
	m_bTimeStampMs = true;
	m_byContentType = CT_LYRICS;
	strcpy(m_szLanguage, "eng");
	m_bAllSyllableIsNewLine = true;
	m_encodingType = IET_UCS2LE_BOM;
}

ID3v2UnsynchLyrics::ID3v2UnsynchLyrics()
{
	frameUID = -1;
	action = IFA_NONE;
	strcpy(m_szLanguage, "eng");
	m_encodingType = IET_ANSI;
}

void ID3v2UnsynchLyrics::SetValueAndOptimizeEncoding(LPCTSTR szDescription, LPCTSTR szLyrics)
{
	if (!IsAnsiStr(szDescription) && !IsAnsiStr(szLyrics))
		m_encodingType = IET_UCS2LE_BOM;
	else
		m_encodingType = IET_ANSI;

	m_strContentDesc = szDescription;
	m_strLyrics = szLyrics;
}

/*
image/jpeg	image/tiff	image/gif	image/bmp	image/png	
*/

#define MIME_IMAGE	"image/"
void ID3v2Pictures::ITEM::MimeToPicExt(char szPicExt[4])
{
	memset(szPicExt, 0, sizeof(szPicExt));

	int		nMimeImage = strlen(MIME_IMAGE);
	if (strncmp(m_strMimeType.c_str(), MIME_IMAGE, nMimeImage) == 0)
	{
		// 
		if (strcmp(m_strMimeType.c_str() + nMimeImage, "jpeg") == 0)
			strcpy(szPicExt, "jpg");
		else if (strcmp(m_strMimeType.c_str() + nMimeImage, "tiff") == 0)
			strcpy(szPicExt, "tif");
		else
			strncpy(szPicExt, m_strMimeType.c_str() + nMimeImage, 3);
		_strupr(szPicExt);
		return;
	}
	strncpy(szPicExt, m_strMimeType.c_str(), 3);
	_strupr(szPicExt);
}

bool ID3v2Pictures::ITEM::PicExtToMime(LPCSTR szPicExt)
{
	if (*szPicExt == '.')
		szPicExt++;

	m_strMimeType = MIME_IMAGE;
	if (_stricmp(szPicExt, "jpg") == 0 || _stricmp(szPicExt, "jpeg") == 0)
		m_strMimeType += "jpeg";
	else if (_stricmp(szPicExt, "tif") == 0)
		m_strMimeType += "tiff";
	else if (_stricmp(szPicExt, "png") == 0 || _stricmp(szPicExt, "gif") == 0
		 || _stricmp(szPicExt, "bmp") == 0)
	{
		m_strMimeType += szPicExt;
		LowerString((LPSTR)m_strMimeType.c_str());
	}
	else
	{
		m_strMimeType.resize(0);
		return false;
	}

	return true;
}

bool ID3v2Pictures::ITEM::SetImageFile(LPCTSTR szImage)
{
	stringex	strExt;

	ConvertStr(FileGetExt(szImage), -1, strExt);
	if (!PicExtToMime(strExt.c_str()))
		return false;

	return Tobool(OpenFileToBuffer(szImage, m_buffPic));
}

ID3v2Pictures::ITEM *ID3v2Pictures::AppendNewPic()
{
	ITEM *pic = new ITEM;

	pic->m_picType = PT_COVER_FRONT;
	pic->action = IFA_ADD;
	m_vItems.push_back(pic);

	return pic;
}

bool ID3v2Pictures::IsModified()
{
	for (size_t i = 0; i < m_vItems.size(); i++)
	{
		if (m_vItems[i]->action != IFA_NONE)
			return true;
	}

	return false;
}

LPCTSTR *ID3v2Pictures::GetAllPicDescriptions()
{
	return _ID3v2PicDescription;
}

int ID3v2Pictures::GetPicDescriptionsCount()
{
	return CountOf(_ID3v2PicDescription);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CID3v2::CID3v2(CHAR_ENCODING encoding)
{
	m_Encoding = encoding;
	m_bAttach = false;
	m_bCreateNew = false;
	m_fp = NULL;
	memset(&m_id3v2Header, 0, sizeof(m_id3v2Header));
	memset(&m_id3v2Footer, 0, sizeof(m_id3v2Footer));
	m_nId3v2BegPos = 0;
	m_nHeaderTotalLen = 0;
	m_nExtendedHeaderLen = 0;
	m_bModify = false;
	frameUID = 0;
}

CID3v2::~CID3v2()
{
	Close();
}

void CID3v2::Close()
{
	if (m_fp)
	{
		if (!m_bAttach)
			fclose(m_fp);
		m_fp = NULL;
	}

	for (FrameIterator it = FrameBegin(); it != FrameEnd(); ++it)
	{
		delete *it;
	}
	m_listFrames.clear();
}

long CID3v2::FindID3v2()
{
	CCharBuffer		buff;
	string			strIdentifier;
	int				nRead;

	strIdentifier = ID3_TAGID;
	buff.reserve(4096);

	while (1)
	{
		int		nLastSize;
		while (buff.size() <= strIdentifier.size())
		{
			nLastSize = buff.size();
			nRead = fread(buff.data() + buff.size(), 1, buff.capacity() - buff.size(), m_fp);
			// if (ferror(m_fp))
			//	return ERR_READ_FILE;
			if (nRead <= 0)
				return -1;
			buff.resize(buff.size() + nRead);

			// ID3v2 tag must be in the first 4k byte.
			if (ftell(m_fp) > 8 * 1024)
				return -1;
		}

		const char *data, *dataend;

		data = buff.c_str();
		dataend = buff.c_str() + buff.size() - strIdentifier.size();
		for (; data < dataend; data++)
		{
			// Here we have to loop because there could be several of the first
			// (11111111) byte, and we want to check all such instances until we find
			// a full match (11111111 111) or hit the end of the buffer.
			if (byte(*data) == 0xFF)
			{
				data++;
				if (byte(*data) != 0xFF && (byte(*data) & 0xE0) == 0xE0)	// 111xxxxx
					return -1;
			}

			if (*data == strIdentifier[0])
			{
				if (strncmp(data, strIdentifier.c_str(), strIdentifier.size()) == 0)
					return ftell(m_fp) - int(buff.c_str() + buff.size() - data);
			}
		}
		// Not found
		buff.erase(0, buff.size() - strIdentifier.size());
	}

	return -1;
}

int CID3v2::Open(LPCTSTR szFile, bool bModify, bool bCreate)
{
	if (bModify)
		m_fp = _tfopen(szFile, _T("r+b"));
	else
		m_fp = _tfopen(szFile, _T("rb"));

	if (!m_fp)
	{
		SetCustomErrorDesc(CStrPrintf(_T("%s: %s"), (LPCTSTR)OSError(), szFile).c_str());
		return ERR_CUSTOM_ERROR;
	}

	m_bAttach = false;

	return Open(m_fp, bCreate);
}

int CID3v2::Open(FILE *fp, bool bCreate)
{
#define E_RETURN(code)	{ nRet = code; goto R_ERROR; }
	ASSERT(fp);
	if (m_fp != fp)
	{
		m_fp = fp;
		m_bAttach = true;
	}

	int		nRet;

	m_bCreateNew = false;
	frameUID = 0;

	fseek(fp, 0, SEEK_SET);

	m_nId3v2BegPos = FindID3v2();
	if (m_nId3v2BegPos == -1)
	{
		if (bCreate)
		{
			// not exist id3v2 tag, create one.
			m_bCreateNew = true;

			m_nHeaderTotalLen = 0;
			m_nId3v2BegPos = 0;
			m_nExtendedHeaderLen = 0;

			memcpy(m_id3v2Header.szID, ID3_TAGID, ID3_TAGIDSIZE);
			m_id3v2Header.byMajorVer = ID3v2Header::ID3V2_V3;
			m_id3v2Header.byRevisionVer = 0;
			m_id3v2Header.byFlag = 0;

			return ERR_OK;
		}
		else
			E_RETURN(ERR_NOT_FOUND_ID3V2);
	}

	fseek(m_fp, m_nId3v2BegPos, SEEK_SET);

	// read Header (10 bytes)
	if (fread(&m_id3v2Header, 1, sizeof(m_id3v2Header), m_fp) != sizeof(m_id3v2Header))
		E_RETURN(ERR_READ_FILE);

	m_nHeaderTotalLen = SynchDataToUInt(m_id3v2Header.bySize, CountOf(m_id3v2Header.bySize))
		+ ID3_TAGHEADERSIZE;

	if (m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V2)
	{
		// DBGLOG1(_T("ID3V2 v2, Len: %d"), m_nHeaderTotalLen);
	}
	else if (m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V3)
	{
		// DBGLOG1(_T("ID3V2 v3, Len: %d"), m_nHeaderTotalLen);
	}
	else if (m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V4)
	{
		// DBGLOG1(_T("ID3V2 v4, Len: %d"), m_nHeaderTotalLen);
	}
	else
		E_RETURN(ERR_NOT_SUPPORT_ID3V2_VER);

	// read Extended Header (variable length, OPTIONAL)
	if (m_id3v2Header.IsExtendedFlagSet())
	{
		ID3v2HeaderExtended		extendedHeader;

		if (fread(&extendedHeader, 1, sizeof(extendedHeader), m_fp) != sizeof(extendedHeader))
			E_RETURN(ERR_READ_FILE);

		m_nExtendedHeaderLen = SynchDataToUInt(extendedHeader.bySize, CountOf(extendedHeader.bySize));
		fseek(m_fp, m_nExtendedHeaderLen, SEEK_CUR);
	}
	else
		m_nExtendedHeaderLen = 0;

	// read Footer (variable length, OPTIONAL)
	if (m_id3v2Header.IsFooterFlagSet())
	{
		fseek(m_fp, m_nId3v2BegPos + m_nHeaderTotalLen, SEEK_SET);
		if (fread(&m_id3v2Footer, 1, sizeof(m_id3v2Footer), m_fp) != sizeof(m_id3v2Footer))
			E_RETURN(ERR_READ_FILE);
		m_nHeaderTotalLen += ID3_TAGFOOTERSIZE;
	}

	// read all frames
	nRet = ReadAllFrames();
	if (nRet != ERR_OK)
		goto R_ERROR;

	return ERR_OK;

R_ERROR:
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}
	return nRet;
}

int CID3v2::Save()
{
	CCharBuffer		buff;
	int				nRet;
	int		nNewHeaderTotalLen;

	for (FrameIterator it = m_listFrames.begin(); it != m_listFrames.end(); ++it)
	{
		CID3v2Frame		*pFrame = *it;

		nRet = pFrame->RenderFrame(&m_id3v2Header, buff);
		if (nRet != ERR_OK)
			return nRet;
	}

	nNewHeaderTotalLen = buff.size() + ID3_TAGHEADERSIZE + m_nExtendedHeaderLen;
	if (m_id3v2Header.IsFooterFlagSet())
		nNewHeaderTotalLen += ID3_TAGFOOTERSIZE;

	if (nNewHeaderTotalLen > (int)m_nHeaderTotalLen)
	{
		if (m_id3v2Header.IsFooterFlagSet())
		{
			// assert(GetPaddingLen() == 0);
			// must not have padding
		}
		else
		{
			// add more 512 to 1024 byte for backup modify or add frame...
			nNewHeaderTotalLen += 1024;
			nNewHeaderTotalLen -= nNewHeaderTotalLen % 512;
		}
	}
	else if (nNewHeaderTotalLen < (int)m_nHeaderTotalLen)
	{
		if (m_id3v2Header.IsFooterFlagSet())
		{
			// assert(GetPaddingLen() == 0);
			// must not have padding
		}
		else
		{
			// don't move mp3 data, keep padding
			nNewHeaderTotalLen = m_nHeaderTotalLen;
		}
	}

	if (nNewHeaderTotalLen != m_nHeaderTotalLen)
	{
		// move mp3 data, alloc or free header size.
		nRet = FileMoveEndData(m_fp, m_nHeaderTotalLen, nNewHeaderTotalLen);
		if (nRet != ERR_OK)
			return nRet;

		m_nHeaderTotalLen = nNewHeaderTotalLen;
	}

	int		nPaddingSize;
	nPaddingSize = nNewHeaderTotalLen - buff.size()
		- m_nExtendedHeaderLen - ID3_TAGHEADERSIZE;
	if (m_id3v2Header.IsFooterFlagSet())
		nPaddingSize -= ID3_TAGFOOTERSIZE;
	ASSERT(nPaddingSize >= 0);

	{
		// set header new size
		int		nSizeInHeader = nNewHeaderTotalLen - ID3_TAGHEADERSIZE;
		if (m_id3v2Header.IsFooterFlagSet())
			nSizeInHeader -= ID3_TAGFOOTERSIZE;
		SynchDataFromUInt(nSizeInHeader, m_id3v2Header.bySize, CountOf(m_id3v2Header.bySize));
	}

	if (!m_bCreateNew)
	{
		// compare whether the id3v2 data has been modified?
		int		nResult;
		nRet = FileDataCmp(m_fp, m_nId3v2BegPos, &m_id3v2Header, sizeof(m_id3v2Header), nResult);
		if (nRet != ERR_OK)
			return nRet;

		if (nResult == 0)
		{
			nRet = FileDataCmp(m_fp, GetBeginFramePos(),
				buff.c_str(), buff.size(), nResult);
			if (nRet != ERR_OK)
				return nRet;
			if (nResult == 0)
			{
				//
				// 如果padding data 不为0，需要reset.
				if (nPaddingSize > 0)
				{
					int		n = 4;
					char	temp[4];
					if (nPaddingSize < 4)
						n = nPaddingSize;
					fseek(m_fp, GetBeginFramePos() + buff.size(), SEEK_SET);
					if (fread(temp, 1, n, m_fp) != n)
						return ERR_READ_FILE;
					if (memcmp(temp, "\0\0\0\0", n) != 0)
						return FileDataReset(m_fp, GetBeginFramePos() + buff.size(), nPaddingSize, 0);
				}
				return ERR_OK;
			}
		}
	}

	// must have least one frame...
	if (m_bCreateNew && buff.size() == 0)
		return ERR_OK;

	// save id3info
	if (fseek(m_fp, m_nId3v2BegPos, SEEK_SET) != 0)
		return ERR_SEEK_FILE;

	// header
	if (fwrite(&m_id3v2Header, 1, sizeof(m_id3v2Header), m_fp) != sizeof(m_id3v2Header))
		return ERR_WRITE_FILE;

	// extended header
	// not modified, position not changed
	if (m_id3v2Header.IsExtendedFlagSet())
	{
		if (fseek(m_fp, m_nExtendedHeaderLen, SEEK_CUR) != 0)
			return ERR_SEEK_FILE;
	}

	// frames
	if (fwrite(buff.c_str(), 1, buff.size(), m_fp) != buff.size())
		return ERR_WRITE_FILE;

	// padding
	nRet = FileDataReset(m_fp, ftell(m_fp), nPaddingSize, 0);
	if (nRet != ERR_OK)
		return nRet;

	// footer
	if (m_id3v2Header.IsFooterFlagSet())
	{
		if (fwrite(&m_id3v2Footer, 1, sizeof(m_id3v2Footer), m_fp) != sizeof(m_id3v2Footer))
			return ERR_WRITE_FILE;
	}

	return ERR_OK;
}

int CID3v2::RemoveTagAndClose()
{
	if (m_nHeaderTotalLen <= 0)
		return ERR_NOT_FOUND_ID3V2;

	int nRet = FileMoveEndData(m_fp, m_nId3v2BegPos + m_nHeaderTotalLen, m_nId3v2BegPos);
	if (nRet == ERR_OK)
		Close();

	return nRet;	
}

int CID3v2::ReadAllFrames()
{
	int			nRet;
	long		nNextFramePos;
	CID3v2Frame	*pFrame;

	nNextFramePos = GetBeginFramePos();
	fseek(m_fp, nNextFramePos, SEEK_SET);

	pFrame = new CID3v2Frame(0);
	while ((nRet = ReadFrame(pFrame)) == ERR_OK)
	{
#ifdef DEBUG
//		pFrame->DebugOutput();
#endif
		m_listFrames.push_back(pFrame);
		pFrame = new CID3v2Frame(0);
	}

	// free latest allocated mem
	delete pFrame;

	if (nRet == ERR_EOF)
		return ERR_OK;

	return nRet;
}

int CID3v2::ReadFrame(CID3v2Frame *pFrame)
{
	int				nRet, nNextFramePos;
	ID3v2FrameHdr	&frame = pFrame->m_framehdr;
	CCharBuffer		&buffData = pFrame->m_frameData;

	frame.frameUID = frameUID;
	frameUID++;

	nNextFramePos = ftell(m_fp);
	if (nNextFramePos > (int)m_nHeaderTotalLen)
	{
		// DBGLOG2(_T("Read out of the len, %d, %d"), ftell(m_fp), m_nHeaderTotalLen);
		return ERR_EOF;
	}
	else if (nNextFramePos == m_nHeaderTotalLen)
	{
		// DBGLOG1(_T("End of read frame %d"), m_nHeaderTotalLen);
		return ERR_EOF;
	}

	if (m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V2)
	{
		ID3v2FrameHeaderV2		fh2;
		nRet = fread(&fh2, 1, sizeof(fh2), m_fp);
		if (nRet != sizeof(fh2))
			return ERR_READ_FILE;
		frame.nFrameID = fh2.ToFrameUintID();
		frame.nLen = ByteDataToUInt(fh2.bySize, CountOf(fh2.bySize));
	}
	else if (m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V3)
	{
		ID3v2FrameHeaderV3		fh3;
		nRet = fread(&fh3, 1, sizeof(fh3), m_fp);
		if (nRet != sizeof(fh3))
			return ERR_READ_FILE;
		frame.nFrameID = fh3.ToFrameUintID();
		frame.nLen = ByteDataToUInt(fh3.bySize, CountOf(fh3.bySize));

		ASSERT(CountOf(frame.byFlags) == CountOf(fh3.byFlags));
		memcpy(frame.byFlags, fh3.byFlags, CountOf(fh3.byFlags));

		{
			// first byte of flags
			std::bitset<8>	flags(fh3.byFlags[0]);
			frame.bTagAlterPreservation = flags[7];
			frame.bFileAlterPreservation = flags[6];
			frame.bReadOnly = flags[5];
		}
		{
			// second byte of flags
			std::bitset<8>	flags(fh3.byFlags[1]);
			frame.bCompression = flags[7];
			frame.bEncryption = flags[6];
			frame.bGroupingIdentity = flags[5];
		}
	}
	else if (m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V4)
	{
		ID3v2FrameHeaderV3		fh3;
		nRet = fread(&fh3, 1, sizeof(fh3), m_fp);
		if (nRet != sizeof(fh3))
			return ERR_READ_FILE;
		frame.nFrameID = fh3.ToFrameUintID();
		frame.nLen = SynchDataToUInt(fh3.bySize, CountOf(fh3.bySize));

		ASSERT(CountOf(frame.byFlags) == CountOf(fh3.byFlags));
		memcpy(frame.byFlags, fh3.byFlags, CountOf(fh3.byFlags));

		{
			// first byte of flags
			std::bitset<8>	flags(fh3.byFlags[0]);
			frame.bTagAlterPreservation = flags[6];
			frame.bFileAlterPreservation = flags[5];
			frame.bReadOnly = flags[4];
		}
		{
			// second byte of flags
			std::bitset<8>	flags(fh3.byFlags[1]);
			frame.bGroupingIdentity = flags[6];
			frame.bCompression = flags[3];
			frame.bEncryption = flags[2];
			frame.bUnsyncronisation = flags[1];
			frame.bDataLengthIndicator = flags[0];
		}
	}
	else
		return ERR_NOT_SUPPORT_ID3V2_VER;

	if (frame.nFrameID <= 0xFFFFFF)
	{
		// DBGLOG2(_T("Encounter End Frame: %d, %d"), ftell(m_fp), m_nHeaderTotalLen);
		return ERR_EOF;
	}

	// Check for IsValidFrame Length?
	if (frame.nLen < 0 || ftell(m_fp) + frame.nLen > GetBeginFramePos() + (int)m_nHeaderTotalLen)
		return ERR_INVALID_ID3V2_FRAME;

	buffData.clear();
	buffData.reserve(frame.nLen);
	if (buffData.capacity() < (size_t)frame.nLen)
		return ERR_OUTOF_BUFF;
	if (fread(buffData.data(), 1, frame.nLen, m_fp) != frame.nLen)
		return ERR_READ_FILE;

	buffData.resize(frame.nLen);

	return ERR_OK;
}

int CID3v2::FrameAdd(CID3v2Frame *pFrame)
{
	m_listFrames.push_back(pFrame);

	return ERR_OK;
}

CID3v2::FrameIterator CID3v2::FrameRemove(FrameIterator itToRemove)
{
	return m_listFrames.erase(itToRemove);
}

CID3v2::FrameIterator CID3v2::FindFrameIter(UINT nFrameID)
{
	for (FrameIterator it = m_listFrames.begin(); it != m_listFrames.end(); ++it)
	{
		CID3v2Frame *pFrame = *it;
		if (pFrame->m_framehdr.nFrameID == nFrameID)
			return it;
	}

	return m_listFrames.end();
}

CID3v2Frame *CID3v2::FindFrame(UINT nFrameID)
{
	FrameIterator it = FindFrameIter(nFrameID);
	if (it == m_listFrames.end())
		return NULL;
	return *it;
}

CID3v2Frame *CID3v2::FindFrameByUID(ID3v2FrameUID_t frameUID, UINT nFrameID)
{
	if (frameUID == -1)
		return NULL;

	FrameIterator	it;
	for (it = m_listFrames.begin(); it != m_listFrames.end(); ++it)
	{
		CID3v2Frame *pFrame = *it;
		if (pFrame->m_framehdr.frameUID == frameUID && pFrame->m_framehdr.nFrameID == nFrameID)
			return pFrame;
	}

	return NULL;
}

int CID3v2::RemoveFrame(UINT nFrameID)
{
	CID3v2Frame			*pFrame = NULL;
	FrameIterator		it;

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

int CID3v2::RemoveFrameByUID(ID3v2FrameUID_t frameUID)
{
	CID3v2Frame			*pFrame = NULL;
	FrameIterator		it;

	for (it = FrameBegin(); it != FrameEnd(); ++it)
	{
		pFrame = *it;

		if (pFrame->m_framehdr.frameUID == frameUID)
		{
			FrameRemove(it);
			return ERR_OK;
		}
	}

	return ERR_NOT_FOUND;
}

int CID3v2::GetBeginFramePos()
{
	return m_nId3v2BegPos + ID3_TAGHEADERSIZE + m_nExtendedHeaderLen;
}
