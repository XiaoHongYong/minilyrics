#include "StdAfx.h"
#include "../ID3Lib/ID3v2.h"
#include "../ID3Lib/ID3Helper.h"
#include "M4aTag.h"

inline uint32 BigEndianToUInt32(byte *byData)
{
	return byData[3] | (byData[2] << 8) | (byData[1] << 16) | (byData[0] << 24);
}

inline void BigEndianFromUInt32(uint32 nValue, byte *byData)
{
	byData[3] = nValue & 0xFF;
	byData[2] = (nValue >> 8) & 0xFF;
	byData[1] = (nValue >> 16) & 0xFF;
	byData[0] = (nValue >> 24) & 0xFF;
}

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

void debugParseLyrics(M4aBox *box)
{
	CM4aBoxTextParser parser;
	parser.Parse(box);
	_tprintf(_T("Lyrics: %s\r\n"), parser.m_strLyrics.c_str());
}

//////////////////////////////////////////////////////////////////////////

M4aBox::M4aBox()
{
	m_nOffset = 0;
	m_nSize = 0;
	ASSERT(sizeof(m_szType) >= TYPE_SIZE);
	m_szType[TYPE_SIZE] = '\0';
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

CM4aTag::CM4aTag()
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

int CM4aTag::Save()
{
	M4aBox *boxMoov = m_root.FindBox(SZ_TYPE_MOOV);
	if (boxMoov == NULL)
		return ERR_NOT_SUPPORT_FILE_FORMAT;

	string buf;

	boxMoov->ToBuf(buf);
	if (boxMoov->m_nSize != buf.size())
	{
		// Need to resize boxes
		M4aBox *free = m_root.NextBox(boxMoov);
		if (free == NULL)
		{
			// Resize
			int nRet = FileMoveEndData(m_fp, boxMoov->m_nOffset + boxMoov->m_nSize, boxMoov->m_nOffset + buf.size());
			if (nRet != ERR_OK)
				return nRet;
		}
		else
		{
			if (boxMoov->m_nSize + free->m_nSize + M4aBox::HEADER_SIZE < buf.size())
			{
				// Resize
				int nRet = FileMoveEndData(m_fp, boxMoov->m_nOffset + boxMoov->m_nSize, boxMoov->m_nOffset + buf.size());
				if (nRet != ERR_OK)
					return nRet;
			}
			else
			{
				// Resize free box
				free->m_nOffset = boxMoov->m_nOffset + buf.size();
				free->m_nSize -= buf.size() - boxMoov->m_nSize;
				free->m_data.resize(free->GetDataSize(), 0);

				string bufFree;
				free->ToBuf(bufFree);
				int nRet = Write(bufFree, free->m_nOffset);
				if (nRet != ERR_OK)
					return nRet;
			}
		}
	}

	int nRet = Write(buf, boxMoov->m_nOffset);
	if (nRet != ERR_OK)
		return nRet;

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

		M4aBox	*pChild = new M4aBox();
		pChild->m_nOffset = offset;
		pChild->m_nSize = size;
		memcpy(pChild->m_szType, type, M4aBox::TYPE_SIZE);

		offset += size;
		parent->AddChild(pChild);

		_tprintf(_T("Box: %S, offset: %d, size: %d\n"), pChild->m_szType, pChild->m_nOffset, pChild->m_nSize);

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

void Read(FILE *fp, string &buf, size_t offset, size_t size)
{
	fseek(fp, offset, SEEK_SET);
	buf.resize(size);
	int n = fread((char *)buf.c_str(), 1, size, fp);
	if (n != size)
		buf.resize(0);
}

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

		VerifyData(m_fp, parent);

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

	VerifyData(m_fp, parent);

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

int CM4aTag::SetLyrics(LPCTSTR szLyrics)
{
	M4aBox *pIList = GetBox("moov.udta.meta.ilst");
	if (pIList == NULL)
		return ERR_NOT_SUPPORT_FILE_FORMAT;

	GetBox("moov")->m_nSizeNew = 0;

	M4aBox *pLyrics = pIList->FindBox(SZ_TYPE_LYRICS);
	if (pLyrics == NULL)
	{
/*		// Add a lyrics box
		pLyrics = new M4aLyricsBox();
		pLyrics->m_nOffset = -1;*/
	}
	else
	{
		M4aBox *pData = pLyrics->FindBox(SZ_TYPE_DATA);
		if (!pData)
			return ERR_BAD_FILE_FORMAT;
		CM4aBoxTextParser parser;
		parser.m_strLyrics = szLyrics;
		parser.ConvertTo(pData);
	}

	return ERR_OK;
}

int CM4aTag::Write(string &buf, size_t offset)
{
	fseek(m_fp, offset, SEEK_SET);
	if (fwrite(buf.c_str(), 1, buf.size(), m_fp) != buf.size())
		return ERR_WRITE_FILE;
	return ERR_OK;
}
