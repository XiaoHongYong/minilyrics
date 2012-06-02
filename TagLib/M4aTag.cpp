#include "StdAfx.h"
#include "../ID3Lib/ID3v2.h"
#include "../ID3Lib/ID3Helper.h"
#include "TagLib.h"
#include "M4aTag.h"

#define SZ_TYPE_MOOV		"moov"
#define SZ_TYPE_TRACK		"trak"
#define SZ_TYPE_UDTA		"udta"
#define SZ_TYPE_META		"meta"
#define SZ_TYPE_ILST		"ilst"
#define SZ_TYPE_LYRICS		"\xA9lyr"
#define SZ_TYPE_DATA		"data"


bool CanBoxHasChild(char szType[])
{
	LPCSTR	vList[] = { SZ_TYPE_MOOV, SZ_TYPE_TRACK, SZ_TYPE_UDTA, SZ_TYPE_META, SZ_TYPE_ILST, };

	for (int i = 0; i < CountOf(vList); i++)
	{
		if (strncmp(szType, vList[i], M4aBox::TYPE_SIZE) == 0)
			return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
class CM4aBoxParser
{
public:
	CM4aBoxParser() { }
	virtual ~CM4aBoxParser() { }

	virtual int Parse(M4aBox *box) = 0;
	virtual int ConvertTo(M4aBox *box) = 0;

};

class CM4aBoxTextParser : public CM4aBoxParser
{
public:
	virtual int Parse(M4aBox *box)
	{
		char buf[8];
		memset(buf, 0, 8);
		buf[3] = 1;
		if (memcmp(box->m_data.c_str(), buf, 8) != 0)
			return ERR_BAD_FILE_FORMAT;

		UTF8ToTString(box->m_data.c_str() + 8, box->m_nSize - 8 - M4aBox::HEADER_SIZE, m_strLyrics);
		return ERR_OK;
	}

	virtual int ConvertTo(M4aBox *box)
	{
		if (box->m_data.size() < 8)
		{
			box->m_data.resize(8);
			memset((char *)box->m_data.c_str(), 0, box->m_data.size());
			box->m_data[3] = 1;
		}
		else
			box->m_data.resize(8);
		string lyr;
		TStringToUTF8(m_strLyrics.c_str(), m_strLyrics.size(), lyr);
		box->m_data.append(lyr);
		return ERR_OK;
	}

	tstring		m_strLyrics;

};

#ifdef _DEBUG
void debugParseLyrics(M4aBox *box)
{
	CM4aBoxTextParser parser;
	parser.Parse(box);
	_tprintf(_T("Lyrics: %s\r\n"), parser.m_strLyrics.c_str());
}
#endif // #ifdef _DEBUG

//////////////////////////////////////////////////////////////////////////

M4aBox::M4aBox(LPCSTR szType, size_t offset, size_t size)
{
	m_nOffset = offset;
	m_nSize = size;
	strncpy_safe(m_szType, CountOf(m_szType), szType, M4aBox::TYPE_SIZE);

	ASSERT(sizeof(m_szType) >= TYPE_SIZE);
	m_nSizeNew = 0;
}

M4aBox::~M4aBox()
{
	ClearChildren();
}

void M4aBox::AddChild(M4aBox *box)
{
	m_vChildren.push_back(box);
}

bool M4aBox::IsType(LPCSTR szType)
{
	return strncmp(m_szType, szType, TYPE_SIZE) == 0;
}

void M4aBox::ClearChildren()
{
	for (uint i = 0; i < m_vChildren.size(); i++)
		delete m_vChildren[i];
	m_vChildren.clear();
}

M4aBox *M4aBox::FindBox(LPCSTR szBox)
{
	for (uint i = 0; i < m_vChildren.size(); i++)
	{
		M4aBox *p = m_vChildren[i];
		if (p->IsType(szBox))
			return p;
	}

	return NULL;
}

M4aBox *M4aBox::NextBox(M4aBox *brother)
{
	for (uint i = 0; i < m_vChildren.size(); i++)
	{
		M4aBox *p = m_vChildren[i];
		if (p == brother)
		{
			if (i + 1 < m_vChildren.size())
				return m_vChildren[i + 1];
		}
	}

	return NULL;
}

bool M4aBox::DeleteChild(LPCSTR szType)
{
	for (uint i = 0; i < m_vChildren.size(); i++)
	{
		M4aBox *p = m_vChildren[i];
		if (p->IsType(szType))
		{
			delete p;
			m_vChildren.erase(m_vChildren.begin() + i);
			return true;
		}
	}

	return false;
}

void M4aBox::ToBuf(string &buf)
{
	// Size
	if (m_nSizeNew == 0)
		CalculateNewSize();

	byte size[4];
	BigEndianFromUInt32(m_nSizeNew, size);
	buf.append((char *)size, M4aBox::TYPE_SIZE);

	// Type
	buf.append(m_szType, M4aBox::TYPE_SIZE);

	if (m_vChildren.size() == 0)
	{
		buf.append(m_data);
		return;
	}

	// Data
	if (m_data.size() > 0)
	{
		ASSERT(IsType(SZ_TYPE_META));
		buf.append(m_data);
	}

	// Children
	for (uint i = 0; i < m_vChildren.size(); i++)
	{
		M4aBox *p = m_vChildren[i];
		p->ToBuf(buf);
	}
}

void M4aBox::CalculateNewSize()
{
	m_nSizeNew = HEADER_SIZE + m_data.size();

	for (uint i = 0; i < m_vChildren.size(); i++)
	{
		M4aBox *p = m_vChildren[i];
		p->CalculateNewSize();
		m_nSizeNew += p->m_nSizeNew;
	}
}

//////////////////////////////////////////////////////////////////////////

class M4aLyricsBox : public M4aBox
{
public:
	M4aLyricsBox();

	tstring				m_strLyrics;

};

//////////////////////////////////////////////////////////////////////////

CM4aTag::CM4aTag() : m_root("", 0, 0)
{
}

CM4aTag::~CM4aTag()
{
	Close();
}

int CM4aTag::Open(LPCTSTR szFile, bool bModify, bool bCreate)
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

	return Open(m_fp, bCreate);
}

int CM4aTag::Open(FILE *fp, bool bCreate)
{
	ASSERT(fp);
	m_fp = fp;

	fseek(fp, 0, SEEK_END);
	size_t nFileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	m_root.m_nOffset = 0;
	m_root.m_nSize = nFileSize;
	int nRet = Parse(&m_root, 0);
	if (nRet != ERR_OK)
		return nRet;

	// Read moov box immediately
	M4aBox *boxMoov = m_root.FindBox(SZ_TYPE_MOOV);
	if (boxMoov == NULL)
		return ERR_NOT_SUPPORT_FILE_FORMAT;

	return Read(boxMoov);
}

void CM4aTag::Close()
{
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}
}

int CM4aTag::SaveClose()
{
	M4aBox *boxMoov = m_root.FindBox(SZ_TYPE_MOOV);
	if (boxMoov == NULL)
		return ERR_NOT_SUPPORT_FILE_FORMAT;

	if (GetBox("moov.trak.mdia.minf.stbl.co64") != NULL)
		return ERR_NOT_SUPPORT_FILE_FORMAT;

	boxMoov->CalculateNewSize();

	int nAdjustedOffset = 0;
	size_t nSizeNew = boxMoov->m_nSizeNew;
	if (boxMoov->m_nSize != nSizeNew)
	{
		// Need to resize boxes
		M4aBox *free = m_root.NextBox(boxMoov);
		free = NULL;
		if (free == NULL)
		{
			// Resize
			int nRet = FileMoveEndData(m_fp, boxMoov->m_nOffset + boxMoov->m_nSize, boxMoov->m_nOffset + nSizeNew);
			if (nRet != ERR_OK)
				return nRet;
			nAdjustedOffset = nSizeNew - boxMoov->m_nSize;
		}
		else
		{
			if (boxMoov->m_nSize + free->m_nSize + M4aBox::HEADER_SIZE < nSizeNew)
			{
				// Resize
				int nRet = FileMoveEndData(m_fp, boxMoov->m_nOffset + boxMoov->m_nSize, boxMoov->m_nOffset + nSizeNew);
				if (nRet != ERR_OK)
					return nRet;
				nAdjustedOffset = nSizeNew - boxMoov->m_nSize;
			}
			else
			{
				// Resize free box
				free->m_nOffset = boxMoov->m_nOffset + nSizeNew;
				free->m_nSize -= nSizeNew - boxMoov->m_nSize;
				free->m_data.resize(free->GetDataSize(), 0);

				string bufFree;
				free->ToBuf(bufFree);
				int nRet = Write(bufFree, free->m_nOffset);
				if (nRet != ERR_OK)
					return nRet;
			}
		}
	}

	if (nAdjustedOffset != 0)
		AdjustStco(nAdjustedOffset);

	string buf;
	boxMoov->ToBuf(buf);

	int nRet = Write(buf, boxMoov->m_nOffset);
	if (nRet != ERR_OK)
		return nRet;

	Close();

	return ERR_OK;
}

int CM4aTag::Parse(M4aBox *parent, int nExtraHeaderOffset)
{
	size_t	totalSize = nExtraHeaderOffset;
	size_t	offset = parent->m_nOffset + nExtraHeaderOffset;

	while (true)
	{
		fseek(m_fp, offset, SEEK_SET);

		// Read size
		byte	buf[M4aBox::TYPE_SIZE];
		int read = fread(buf, 1, M4aBox::TYPE_SIZE, m_fp);
		if (read != M4aBox::TYPE_SIZE)
			return ERR_BAD_FILE_FORMAT;

		size_t size = BigEndianToUInt32(buf);
		if (size < M4aBox::HEADER_SIZE)
			return ERR_BAD_FILE_FORMAT;

		// Read type
		char	type[M4aBox::TYPE_SIZE];
		read = fread(type, 1, M4aBox::TYPE_SIZE, m_fp);
		if (read != M4aBox::TYPE_SIZE)
			return ERR_BAD_FILE_FORMAT;

		totalSize += size;
		if (totalSize > parent->m_nSize)
			return ERR_BAD_FILE_FORMAT;

		M4aBox	*pChild = new M4aBox(type, offset, size);

		offset += size;
		parent->AddChild(pChild);

		DBGLOG3(_T("Box: %S, offset: %d, size: %d\n"), type, offset, size);

		if (totalSize == parent->m_nSize)
			break;
	}

	for (uint i = 0; i < parent->m_vChildren.size(); i++)
	{
		M4aBox	*pChild = parent->m_vChildren[i];

		// if (CanBoxHasChild(pChild->m_szType))
		{
			int nExtraHeaderOffset = M4aBox::HEADER_SIZE;
			if (pChild->IsType(SZ_TYPE_META))
				nExtraHeaderOffset += 4;
			int nRet = Parse(pChild, nExtraHeaderOffset);
			if (nRet != ERR_OK)
				pChild->ClearChildren();
		}
	}

	return ERR_OK;
}

/**
stco = sample table chunk offset

QT Atom Preamble
1 byte    version
3 bytes   flags
4 bytes   total entries in offset table (n)
4 bytes   chunk offset 0
4 bytes   chunk offset 1
..
..
4 bytes   chunk offset n-1
*/
int CM4aTag::AdjustStco(int nAdjustedOffset)
{
	M4aBox *pBoxStco = GetBox("moov.trak.mdia.minf.stbl.stco");
	if (pBoxStco != NULL)
	{
		const int offsetTrunk = 8;
		int count = (pBoxStco->m_data.size() - offsetTrunk) / 4;
		int countShould = BigEndianToUInt32((byte *)(pBoxStco->m_data.c_str() + 4));
		ASSERT(count == countShould);
		if (count > countShould)
			count = countShould;

		int end = offsetTrunk + count * 4;
		for (int i = offsetTrunk; i < end; i += 4)
		{
			uint32 n = BigEndianToUInt32((byte *)(pBoxStco->m_data.c_str() + i));
			BigEndianFromUInt32(n + nAdjustedOffset, (byte *)(pBoxStco->m_data.c_str() + i));
		}
	}

	return ERR_OK;
}

void Read(FILE *fp, string &buf, size_t offset, size_t size)
{
	fseek(fp, offset, SEEK_SET);
	buf.resize(size);
	int n = fread((char *)buf.c_str(), 1, size, fp);
	if (n != size)
		buf.resize(0);
}

#ifdef _DEBUG
// This is used to verify the data read can composed are OK.
void VerifyData(FILE *fp, M4aBox *box)
{
	string buf;
	Read(fp, buf, box->m_nOffset, box->m_nSize);
	ASSERT(buf.size() == box->m_nSize);

	string buf2;
	box->ToBuf(buf2);
	ASSERT(buf.size() == buf2.size());
	ASSERT(memcmp(buf.c_str(), buf2.c_str(), buf.size()) == 0);
}
#endif // #ifdef _DEBUG

int CM4aTag::Read(M4aBox *parent)
{
	// Assure the box is correct, we don't verify again.
	if (parent->m_vChildren.size() == 0)
	{
		parent->m_data.resize(parent->GetDataSize());
		fseek(m_fp, parent->m_nOffset + M4aBox::HEADER_SIZE, SEEK_SET);
		int n = fread((char *)parent->m_data.c_str(), 1, parent->m_data.size(), m_fp);
		if (n != parent->m_data.size())
			return ERR_READ_FILE;

		// VerifyData(m_fp, parent);

		return ERR_OK;
	}

	if (parent->IsType(SZ_TYPE_META))
	{
		parent->m_data.resize(4);
		fseek(m_fp, parent->m_nOffset + M4aBox::HEADER_SIZE, SEEK_SET);
		int n = fread((char *)parent->m_data.c_str(), 1, 4, m_fp);
		if (n != 4)
			return ERR_READ_FILE;
	}

	for (uint i = 0; i < parent->m_vChildren.size(); i++)
	{
		M4aBox	*pChild = parent->m_vChildren[i];
		int nRet = Read(pChild);
		if (nRet != ERR_OK)
			return nRet;
	}

	// VerifyData(m_fp, parent);

	return ERR_OK;
}

M4aBox *CM4aTag::GetBox(LPCSTR szPath)
{
	tstring	strPath;
	AnsiToTString(szPath, -1, strPath);
	VecString	vPath;
	StrSplit(strPath.c_str(), '.', vPath);

	M4aBox *parent = &m_root;
	for (uint i = 0; i < vPath.size(); i++)
	{
		string ansiName;
		TStringToAnsi(vPath[i].c_str(), -1, ansiName);
		parent = parent->FindBox(ansiName.c_str());
		if (!parent)
			return NULL;
	}

	return parent;
}

int CM4aTag::ListLyrics(VecString &vLyrNames)
{
	if (HasLyrics())
		vLyrNames.push_back(SZ_SONG_M4A_LYRICS);

	return ERR_OK;
}

int CM4aTag::GetLyrics(tstring &strLyrics)
{
	M4aBox *box = GetBox("moov.udta.meta.ilst.\xA9lyr.data");
	if (!box)
		return ERR_NOT_FOUND;

	CM4aBoxTextParser parser;
	int nRet = parser.Parse(box);
	if (nRet == ERR_OK)
		strLyrics = parser.m_strLyrics;

	return nRet;
}

bool CM4aTag::HasLyrics()
{
	return GetBox("moov.udta.meta.ilst.\xA9lyr.data") != NULL;
}

int CM4aTag::SetLyrics(LPCTSTR szLyrics)
{
	M4aBox *pIList = GetBox("moov.udta.meta.ilst");
	if (pIList == NULL)
		return ERR_NOT_SUPPORT_FILE_FORMAT;

	GetBox("moov")->m_nSizeNew = 0;

	M4aBox *pLyrics = pIList->FindBox(SZ_TYPE_LYRICS);
	if (pLyrics == NULL)
	{
		pLyrics = new M4aBox(SZ_TYPE_LYRICS, 0, 0);
		pIList->AddChild(pLyrics);

		pLyrics->AddChild(new M4aBox(SZ_TYPE_DATA, 0, 0));
	}

	M4aBox *pData = pLyrics->FindBox(SZ_TYPE_DATA);
	if (!pData)
		return ERR_BAD_FILE_FORMAT;

	CM4aBoxTextParser parser;
	parser.m_strLyrics = szLyrics;
	parser.ConvertTo(pData);

	return ERR_OK;
}

int CM4aTag::RemoveLyrics()
{
	return ERR_NOT_SUPPORT_FILE_FORMAT;
}

int CM4aTag::Write(string &buf, size_t offset)
{
	fseek(m_fp, offset, SEEK_SET);
	if (fwrite(buf.c_str(), 1, buf.size(), m_fp) != buf.size())
		return ERR_WRITE_FILE;
	return ERR_OK;
}
