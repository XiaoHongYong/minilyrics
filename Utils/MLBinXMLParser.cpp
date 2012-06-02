#include "stdafx.h"
#include "MLBinXMLParser.h"
#include "Log.h"
#include "LogAlias.h"

/************************************************************************

MiniLyrics binary XML format 1.0

string encoding: UTF8.

XML = "MBXML" Version[byte] flags[uint32] total_length[uint32] [data block] [data block] ...

[data block] = block_type[2 bytes] length[uint32] [data...]

[string table data block] = "ST"[2 bytes] length[uint32] count[uint32] [string\0] [string\0] ...
[bin data block] = "BN"[2 bytes] length[uint32] count[uint32] item_length[uint32] item_data[item_length byte] item_length[uint32] item_data[item_length byte] ...
[XML content data block] = "CT" [2 bytes] length[uint32]

"<" element_name_index_str_table prop_name_index_str_table prop_value_index_str_table ">" value_name_index_str_table "</" element_name_index_str_table ">"

"<" element_name_index_str_table prop_name_index_str_table prop_value_index_str_table ">" value_name_index_str_table 
  "<" element2_name_index_str_table prop_name_index_str_table prop_value_index_str_table "/>"
"/>"

\************************************************************************/

#define SZ_BIN_XML_HEADER		"MBXML"
#define LEN_BIN_XML_HEADER		(sizeof(SZ_BIN_XML_HEADER) - 1)

#define LEN_BLOCK_NAME			2
#define LEN_BLOCK_HEADER		(LEN_BLOCK_NAME + sizeof(uint32))
#define LEN_BIN_XML_MIN			(LEN_BIN_XML_HEADER + sizeof(byte) + sizeof(uint32) + sizeof(uint32) + (LEN_BLOCK_HEADER + 2) * 2)

#define BX_BLOCK_BIN			"BN"
#define BX_BLOCK_STING_TABLE	"ST"
#define BX_BLOCK_CONTENT		"CT"


enum BinXmlVersion
{
	BXV_1				= '1'
};

enum BinXmlFlag
{
	// BXF_UCS2			= 1,		// UCS2 encoding, or it's UTF-8 encoding
	BXF_BYTE_INDEX		= 1 << 1,
	BXF_WORD_INDEX		= 1 << 2,
	BXF_DWORD_INDEX		= 1 << 3,
};

enum StringTableId
{
	BX_ID_INVALID		= 0,
	BX_ID_BINDATA		= 1,
	BX_ID_L_BRACKET		= 2,
	BX_ID_R_BRACKET		= 3,
	BX_ID_ELEMENT_END	= 4,
	BX_ID_STR_START		= 10,
};

class CBinXmlStrTable
{
public:
	bool SetTable(char *pFirst, uint32 nLength, uint32 nCount) {
		m_vStr.resize(nCount);

		const char *str = (const char *)pFirst;
		const char *end = str + nLength / sizeof(char);
		uint n;
		for (n = 0; n < nCount && str < end; n++)
		{
			size_t len = strlen(str);
			UTF8ToTString(str, len, m_vStr[n]);
			str += len + 1;
		}
		ASSERT(str == end);
		ASSERT(n == nCount);

		return n == nCount && str == end;
	}

	tstring &GetStr(int nIndex) {
		nIndex -= BX_ID_STR_START;
		ASSERT(nIndex >= 0 && nIndex < (int)m_vStr.size());
		if (nIndex < 0 || nIndex >= (int)m_vStr.size())
			return m_emptyStr;

		return m_vStr[nIndex];
	}

	V_Str			m_vStr;
	tstring			m_emptyStr;

};

//////////////////////////////////////////////////////////////////////////

class CBinXmlBinDataTable
{
public:
	bool SetTable(LPBYTE data, uint32 nLength, uint32 nCount)
	{
		m_vBinData.resize(nCount);

		LPBYTE dataEnd = data + nLength;
		for (uint32 i = 0; i < nCount && data + sizeof(uint32) + 1 < dataEnd; i++)
		{
			uint32 lenItem = LittleEndianToUInt32(data);
			data += sizeof(uint32);
			if (data + lenItem > dataEnd)
				return false;
			AnsiToTString((LPCSTR)data, lenItem, m_vBinData[i]);
			data += lenItem;
		}

		ASSERT(data == dataEnd);

		return m_vBinData.size() == nCount && data == dataEnd;
	}

	tstring &GetData(int nIndex) {
		ASSERT(nIndex >= 0 && nIndex < (int)m_vBinData.size());
		if (nIndex < 0 || nIndex >= (int)m_vBinData.size())
			return m_emptyData;
		return m_vBinData[nIndex];
	}

	V_Str			m_vBinData;
	tstring			m_emptyData;

};

//////////////////////////////////////////////////////////////////////////
// "<" element_name_index_str_table prop_name_index_str_table prop_value_index_str_table ">" value_name_index_str_table "</" element_name_index_str_table ">"
// "<" element_name_index_str_table prop_name_index_str_table prop_value_index_str_table ">" value_name_index_str_table 
//   "<" element2_name_index_str_table prop_name_index_str_table prop_value_index_str_table "/>"
// "/>"
// 	BX_ID_INVALID		= 0,
// 	BX_ID_BINDATA		= 1,
// 	BX_ID_L_BRACKET		= 2,
// 	BX_ID_R_BRACKET		= 3,
// 	BX_ID_ELEMENT_END	= 4,
// 	BX_ID_STR_START		= 10

template<class index_t>
class CMLBXContentParserObj
{
public:
	CMLBXContentParserObj(CSimpleXML &xml, index_t *data, int nLen, 
		CBinXmlBinDataTable	&binDataTable, CBinXmlStrTable &strTable) 
		: m_xml(xml), m_data(data), m_nLen(nLen), m_binDataTable(binDataTable), m_strTable(strTable)
	{
	}

	CMLBinXMLParser::MLBinXmlParseError ParseContent()
	{
		ASSERT(m_xml.m_pRoot == NULL);
		m_xml.m_pRoot = new SXNode;

		int nPosNext = 0;
		CMLBinXMLParser::MLBinXmlParseError ret = ParseEntity(0, nPosNext, m_xml.m_pRoot);
		if (ret != CMLBinXMLParser::E_OK)
		{
			DBGLOG2(_T("Failed to parse MLBinary XML at: %d, error: %d"), nPosNext, ret);
		}

		return ret;
	}

protected:
	CMLBinXMLParser::MLBinXmlParseError ParseEntity(int nPos, int &nPosNext, SXNode *pNode)
	{
		if (m_data[nPos] != BX_ID_L_BRACKET)
			return CMLBinXMLParser::E_FMT_NEED_LBRACKET;
		nPos++;

		// Name
		pNode->strName = m_strTable.GetStr(m_data[nPos]);
		nPos++;

		// Properties
		while (m_data[nPos] != BX_ID_R_BRACKET && m_data[nPos] != BX_ID_ELEMENT_END)
		{
			index_t iPropName = m_data[nPos];
			index_t iPropValue = m_data[nPos + 1];
			nPos += 2;

			SXNode::Property	prop;
			prop.strName = m_strTable.GetStr(iPropName);
			if (iPropValue == BX_ID_BINDATA)
			{
				prop.strValue = m_binDataTable.GetData(m_data[nPos]);
				nPos++;
			}
			else
				prop.strValue = m_strTable.GetStr(iPropValue);
			pNode->listProperties.push_back(prop);
		}

		// Property end
		if (m_data[nPos] == BX_ID_R_BRACKET)
		{
			// Read value str
			nPos++;
			if (m_data[nPos] != BX_ID_L_BRACKET
				&& m_data[nPos] != BX_ID_ELEMENT_END)
			{
				if (m_data[nPos] == BX_ID_BINDATA)
					pNode->strContent = m_binDataTable.GetData(m_data[nPos]);
				else
					pNode->strContent = m_strTable.GetStr(m_data[nPos]);
				nPos++;
			}
		}

		// Child element
		while (m_data[nPos] == BX_ID_L_BRACKET)
		{
			SXNode *pNodeChild = new SXNode;
			CMLBinXMLParser::MLBinXmlParseError err = ParseEntity(nPos, nPosNext, pNodeChild);
			if (err != CMLBinXMLParser::E_OK)
			{
				delete pNodeChild;
				return err;
			}
			pNode->listChildren.push_back(pNodeChild);
			nPos = nPosNext;
		}

		// Element End
		nPosNext = nPos + 1;
		if (m_data[nPos] == BX_ID_ELEMENT_END)
			return CMLBinXMLParser::E_OK;
		else
			return CMLBinXMLParser::E_FMT_NEED_ELEMENT_END;
	}


protected:
	CSimpleXML			&m_xml;
	index_t				*m_data;
	int					m_nLen;

	CBinXmlBinDataTable	&m_binDataTable;
	CBinXmlStrTable		&m_strTable;

};

//////////////////////////////////////////////////////////////////////////

CMLBinXMLParser::CMLBinXMLParser(void)
{
}

CMLBinXMLParser::~CMLBinXMLParser(void)
{
}

CMLBinXMLParser::MLBinXmlParseError CMLBinXMLParser::ParseData(LPCVOID lpXmlData, size_t nLen, CSimpleXML &xmlData)
{
	ASSERT(strlen(SZ_BIN_XML_HEADER) == LEN_BIN_XML_HEADER);

	if (nLen < LEN_BIN_XML_MIN)
		return E_INVALID_SIZE;

	xmlData.m_strEncoding = _T(SZ_UTF8);

	byte *data = (byte*)lpXmlData;
	byte *dataEnd = data;

	// Header
	if (!StrBeginWith((LPCSTR)data, SZ_BIN_XML_HEADER))
		return E_INVALID_HEADER;
	data += LEN_BIN_XML_HEADER;

	// Version
	if (*data != BXV_1)
		return E_UNKNOWN_VERSION;
	data++;

	// Flag
	uint32	nXmlFlags = LittleEndianToUInt32(data);
	data += sizeof(uint32);

	// Length
	uint32	nTotalLength = LittleEndianToUInt32(data);
	data += sizeof(uint32);
	dataEnd += nTotalLength;

	if (nTotalLength > nLen)
		return E_INVALID_SIZE;

	CBinXmlBinDataTable		binDataTable;
	CBinXmlStrTable	strTable;
	bool bStrTable = false, bBinTable = false;

	while (data + LEN_BLOCK_HEADER + sizeof(uint32) < dataEnd)
	{
		// Read block info
		LPCSTR blockName = (LPCSTR)data;
		data += LEN_BLOCK_NAME;
		uint32 lenBlock = LittleEndianToUInt32(data);
		data += sizeof(uint32);

		lenBlock -= LEN_BLOCK_HEADER;
		if (data + lenBlock > dataEnd)
			return E_INVALID_SIZE;

		if (StrBeginWith(blockName, BX_BLOCK_STING_TABLE))
		{
			uint32 countStr = LittleEndianToUInt32(data);
			data += sizeof(uint32);
			lenBlock -= sizeof(uint32);
			if (!strTable.SetTable((char*)data, lenBlock, countStr))
				return E_INVALID_STR_TABLE;

			data += lenBlock;
			bStrTable = true;
		}
		else if (StrBeginWith(blockName, BX_BLOCK_BIN))
		{
			uint32 countItem = LittleEndianToUInt32(data);
			data += sizeof(uint32);
			lenBlock -= sizeof(uint32);
			if (!binDataTable.SetTable(data, lenBlock, countItem))
				return E_INVALID_BIN_TABLE;

			data += lenBlock;
			bBinTable = true;
		}
		else if (StrBeginWith(blockName, BX_BLOCK_CONTENT))
		{
			if (IsFlagSet(nXmlFlags, BXF_BYTE_INDEX))
			{
				CMLBXContentParserObj<byte>	parser(xmlData, data, lenBlock, binDataTable, strTable);
				return parser.ParseContent();
			}
			else if (IsFlagSet(nXmlFlags, BXF_WORD_INDEX))
			{
				ASSERT(lenBlock % sizeof(uint16) == 0);
				uint16 *wData = (uint16 *)data;
				byte *dataEnd = data + lenBlock;
				uint16 *wp = wData;
				for (byte *p = data; p < dataEnd; p += sizeof(uint16), wp++)
					*wp = LittleEndianToUInt16(data);

				CMLBXContentParserObj<uint16>	parser(xmlData, wData, 
					lenBlock / sizeof(uint16), binDataTable, strTable);
				return parser.ParseContent();
			}
			else if (IsFlagSet(nXmlFlags, BXF_DWORD_INDEX))
			{
				ASSERT(lenBlock % sizeof(uint32) == 0);
				uint32 *dwData = (uint32 *)data;
				byte *dataEnd = data + lenBlock;
				uint32 *wp = dwData;
				for (byte *p = data; p < dataEnd; p += sizeof(uint32), wp++)
					*wp = LittleEndianToUInt32(data);

				CMLBXContentParserObj<uint32>	parser(xmlData, dwData, 
					lenBlock / sizeof(uint32), binDataTable, strTable);
				return parser.ParseContent();
			}
			else
				ASSERT(0);
		}
		else
		{
			ASSERT(0);
			return E_INVALID_BLOCK_TYPE;
		}
	}

	return E_OK;
}

bool CMLBinXMLParser::IsHeaderType(LPCSTR szHeader)
{
	return StrBeginWith(szHeader, SZ_BIN_XML_HEADER);
}

//////////////////////////////////////////////////////////////////////////

inline void AppendUint32(stringex &buf, uint32 n)
{
	byte byLen[sizeof(uint32)];
	LittleEndianFromUInt32(n, byLen);
	buf.append((LPCSTR)byLen, sizeof(uint32));
}

CMLBinXMLWriter::CMLBinXMLWriter() : CXMLWriter(false)
{
	m_nBinItemCount = 0;
	m_nElementDeep = 0;
}

CMLBinXMLWriter::~CMLBinXMLWriter()
{
}

// Begin xml element
void CMLBinXMLWriter::WriteStartElement(LPCTSTR szElement)
{
	if (m_wp == WP_ELEMENT_HEAD)
		m_vContentBlock.push_back(BX_ID_R_BRACKET);
	m_wp = WP_ELEMENT_HEAD;

	m_vContentBlock.push_back(BX_ID_L_BRACKET);
	m_vContentBlock.push_back(GetStrIndex(szElement));
	m_nElementDeep++;
}

// Writes: szAttr="szValue"
void CMLBinXMLWriter::WriteAttribute(LPCTSTR szAttr, LPCTSTR szValue)
{
	ASSERT(m_wp == WP_ELEMENT_HEAD);

	m_vContentBlock.push_back(GetStrIndex(szAttr));
	m_vContentBlock.push_back(GetStrIndex(szValue));
}

void CMLBinXMLWriter::WriteAttribute(LPCTSTR szAttr, LPCVOID data, size_t lenData)
{
	ASSERT(m_wp == WP_ELEMENT_HEAD);

	// length
	AppendUint32(m_bufBinTable, lenData);

	// data
	m_bufBinTable.append((LPCSTR)data, lenData);
	m_nBinItemCount++;

	// attribute
	m_vContentBlock.push_back(GetStrIndex(szAttr));
	m_vContentBlock.push_back(BX_ID_BINDATA);
	m_vContentBlock.push_back(m_nBinItemCount - 1);
}

// Close xml element
void CMLBinXMLWriter::WriteEndElement()
{
	ASSERT(m_wp == WP_ELEMENT_HEAD || m_wp == WP_ELEMENT_BODY);

	m_nElementDeep--;
	ASSERT(m_nElementDeep >= 0);

	m_vContentBlock.push_back(BX_ID_ELEMENT_END);

	if (m_nElementDeep > 0)
		m_wp = WP_ELEMENT_BODY;
	else
		m_wp = WP_READY;
}

// Writes the given text content: <node>szString</node>
void CMLBinXMLWriter::WriteString(LPCTSTR szString)
{
	ASSERT(m_wp == WP_ELEMENT_HEAD);
	m_wp = WP_ELEMENT_BODY;

	m_vContentBlock.push_back(BX_ID_R_BRACKET);
	m_vContentBlock.push_back(GetStrIndex(szString));
}

// Writes: <szElement>szString</szElement>
void CMLBinXMLWriter::WriteElementString(LPCTSTR szElement, LPCTSTR szString)
{
	m_vContentBlock.push_back(BX_ID_L_BRACKET);
	m_vContentBlock.push_back(GetStrIndex(szElement));
	m_vContentBlock.push_back(BX_ID_R_BRACKET);
	m_vContentBlock.push_back(GetStrIndex(szString));
	m_vContentBlock.push_back(BX_ID_ELEMENT_END);
}

// Writes: <!-- szComment -->
void CMLBinXMLWriter::WriteComment(LPCTSTR szComment)
{
}

CCharBuffer &CMLBinXMLWriter::GetBuffer()
{
	if (m_strBuff.size() > 0)
		return m_strBuff;

	uint32	flagXml = 0;
	int indexSize = 0;
	if (m_mapStrTable.size() + BX_ID_STR_START <= 255)
	{
		indexSize = 1;
		flagXml |= BXF_BYTE_INDEX;
	}
	else if (m_mapStrTable.size() + BX_ID_STR_START <= (uint16)-1)
	{
		indexSize = sizeof(uint16);
		flagXml |= BXF_WORD_INDEX;
	}
	else
	{
		indexSize = sizeof(uint32);
		flagXml |= BXF_DWORD_INDEX;
	}

	m_strBuff.append(SZ_BIN_XML_HEADER);
	m_strBuff.append(BXV_1);
	AppendUint32(m_strBuff, flagXml);
	AppendUint32(m_strBuff, 0);

	// Write string table
	WriteStrTable();

	// Write binary data block
	if (m_bufBinTable.size())
	{
		m_strBuff.append(BX_BLOCK_BIN, LEN_BLOCK_NAME);
		AppendUint32(m_strBuff, LEN_BLOCK_HEADER + sizeof(uint32) + m_bufBinTable.size());
		AppendUint32(m_strBuff, m_nBinItemCount);
		m_strBuff.append(m_bufBinTable.c_str(), m_bufBinTable.size());
	}

	// Write content block
	WriteContentBlock(indexSize);

	// Rewrite total length
	LittleEndianFromUInt32(m_strBuff.size(), (byte *)m_strBuff.data() + LEN_BIN_XML_HEADER + 1 + sizeof(uint32));

	return m_strBuff;
}

BOOL CMLBinXMLWriter::SaveAsFile(LPCTSTR szFile)
{
	GetBuffer();

	return CXMLWriter::SaveAsFile(szFile);
}

int CMLBinXMLWriter::GetStrIndex(LPCTSTR str)
{
	MapStrIndex::iterator it = m_mapStrTable.find(str);
	if (it == m_mapStrTable.end())
	{
		int n = m_mapStrTable.size() + BX_ID_STR_START;
		m_mapStrTable[str] = n;
		ASSERT(n + 1 - BX_ID_STR_START == m_mapStrTable.size());
		return n;
	}
	else
	{
		return (*it).second;
	}
}

void CMLBinXMLWriter::WriteStrTable()
{
	ASSERT(strlen(BX_BLOCK_CONTENT) == LEN_BLOCK_NAME);
	typedef vector<const tstring *> VPStr;
	VPStr vPStr;

	// Sort map str as an array.
	vPStr.resize(m_mapStrTable.size());
	for (MapStrIndex::iterator it = m_mapStrTable.begin(); it != m_mapStrTable.end(); ++it)
	{
		ASSERT((*it).second >= BX_ID_STR_START && (*it).second < BX_ID_STR_START + (int)m_mapStrTable.size());
		vPStr[(*it).second - BX_ID_STR_START] = &((*it).first);
	}

	// Write header
	uint32 nStrTableStartPos = m_strBuff.size();
	m_strBuff.append(BX_BLOCK_STING_TABLE, LEN_BLOCK_NAME);
	AppendUint32(m_strBuff, 0);
	AppendUint32(m_strBuff, m_mapStrTable.size());

	// Write string
	stringex strUtf8;
	for (uint i = 0; i < vPStr.size(); i++)
	{
		const tstring *pStr = vPStr[i];
		if (m_strBuff.capacity() < m_strBuff.size() + pStr->size() * 3)
			m_strBuff.reserve(m_strBuff.size() + 1024 * 4);

		TStringToUTF8(pStr->c_str(), pStr->size(), strUtf8);
		m_strBuff.append(strUtf8.c_str(), strUtf8.size() + 1);
	}

	// Rewrite length
	LittleEndianFromUInt32(m_strBuff.size() - nStrTableStartPos, (byte*)m_strBuff.data() + nStrTableStartPos + LEN_BLOCK_NAME);
}

void CMLBinXMLWriter::WriteContentBlock(int indexSize)
{
	m_strBuff.reserve(m_strBuff.size() + m_vContentBlock.size() * indexSize
		+ LEN_BLOCK_HEADER + sizeof(uint32));
	m_strBuff.append(BX_BLOCK_CONTENT, LEN_BLOCK_NAME);
	AppendUint32(m_strBuff, m_vContentBlock.size() * indexSize);

	size_t start = m_strBuff.size();
	m_strBuff.resize(m_strBuff.size() + m_vContentBlock.size() * indexSize);
	byte *data = (byte *)m_strBuff.data() + start;
	if (indexSize == sizeof(byte))
	{
		for (int i = 0; i < (int)m_vContentBlock.size(); i++)
			data[i] = (byte)m_vContentBlock[i];
	}
	else if (indexSize == sizeof(uint16))
	{
		for (int i = 0; i < (int)m_vContentBlock.size(); i++)
		{
			LittleEndianFromUInt16(m_vContentBlock[i], data);
			data += sizeof(uint16);
		}
	}
	else if (indexSize == sizeof(uint32))
	{
		for (int i = 0; i < (int)m_vContentBlock.size(); i++)
		{
			LittleEndianFromUInt32(m_vContentBlock[i], data);
			data += sizeof(uint32);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(CMLBinXMLParser)

class CTestCaseCMLBinXMLParser : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseCMLBinXMLParser);
	CPPUNIT_TEST(TestParseBinXml);
	CPPUNIT_TEST(TestParseBinXml2);
	CPPUNIT_TEST(TestParseBinXml3);
	CPPUNIT_TEST_SUITE_END();

protected:
	void TestParseBinXml()
	{
		CMLBinXMLWriter xmlWriter;

		xmlWriter.WriteStartElement(_T("Node"));
		xmlWriter.WriteAttribute(_T("attr"), _T("value"));
		xmlWriter.WriteAttribute(_T("attr2"), _T("value2"));
		xmlWriter.WriteStartElement(_T("Node2"));
		xmlWriter.WriteEndElement();
		xmlWriter.WriteEndElement();

		CCharBuffer &buf = xmlWriter.GetBuffer();

		CMLBinXMLParser parser;
		CSimpleXML		xmlData;

		CMLBinXMLParser::MLBinXmlParseError ret = parser.ParseData(buf.c_str(), buf.size(), xmlData);
		CPPUNIT_ASSERT(ret == CMLBinXMLParser::E_OK);

		CMLBinXMLWriter xmlWriter2;
		xmlData.m_pRoot->ToXML(xmlWriter2);
		CCharBuffer &buf2 = xmlWriter2.GetBuffer();
		CPPUNIT_ASSERT(buf.size() == buf2.size());
		CPPUNIT_ASSERT(memcmp(buf.c_str(), buf2.c_str(), buf.size()) == 0);
	}

	void TestParseBinXml2()
	{
		CMLBinXMLWriter xmlWriter;

		xmlWriter.WriteStartElement(_T("Node"));
		xmlWriter.WriteAttribute(_T("attr"), _T("value"));
		xmlWriter.WriteAttribute(_T("attr2"), _T("value2"));
		xmlWriter.WriteString(_T("str"));
		xmlWriter.WriteStartElement(_T("Node2"));
		xmlWriter.WriteEndElement();
		xmlWriter.WriteEndElement();

		CCharBuffer &buf = xmlWriter.GetBuffer();

		CSimpleXML		xmlData;

		CPPUNIT_ASSERT(xmlData.ParseData(buf.c_str(), buf.size()));

		CMLBinXMLWriter xmlWriter2;
		xmlData.m_pRoot->ToXML(xmlWriter2);
		CCharBuffer &buf2 = xmlWriter2.GetBuffer();
		CPPUNIT_ASSERT(buf.size() == buf2.size());
		CPPUNIT_ASSERT(memcmp(buf.c_str(), buf2.c_str(), buf.size()) == 0);
	}

	void TestParseBinXml3()
	{
		CMLBinXMLWriter xmlWriter;

#define SZ_BIN_DATA		"\0x1\0x2"

		xmlWriter.WriteStartElement(_T("Node"));
		xmlWriter.WriteAttribute(_T("attr"), _T("value"));
		xmlWriter.WriteAttribute(_T("attr2"), SZ_BIN_DATA, CountOf(SZ_BIN_DATA)); // bin data
		xmlWriter.WriteEndElement();

		CCharBuffer &buf = xmlWriter.GetBuffer();

		CMLBinXMLParser parser;
		CSimpleXML		xmlData;

		CMLBinXMLParser::MLBinXmlParseError ret = parser.ParseData(buf.c_str(), buf.size(), xmlData);
		CPPUNIT_ASSERT(ret == CMLBinXMLParser::E_OK);

		CPPUNIT_ASSERT(xmlData.m_pRoot->strName == _T("Node"));
		CPPUNIT_ASSERT(_tcscmp(xmlData.m_pRoot->GetProperty(_T("attr")), _T("value")) == 0);
		string bufAttr;
		xmlData.m_pRoot->GetPropertyBinData(_T("attr2"), bufAttr);
		CPPUNIT_ASSERT(bufAttr.size() == CountOf(SZ_BIN_DATA));
		CPPUNIT_ASSERT(memcmp(SZ_BIN_DATA, bufAttr.c_str(), bufAttr.size()) == 0);
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCMLBinXMLParser);

#endif // _CPPUNIT_TEST
