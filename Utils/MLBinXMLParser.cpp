#include "stdafx.h"
#include "MLBinXMLParser.h"

/************************************************************************

MiniLyrics binary XML format 1.0

XML = "MBXML" Version[byte] flags[uint32] total_length[uint32] [data block] [data block] ...

[data block] = block_type[2 bytes] length[uint32] [data...]

[string table data block] = "ST"[2 bytes] length[uint32] count[uint32] [string\0] [string\0] ...
[bin data block] = "BN"[2 bytes] length[uint32] item_length[uint32] item_data[item_length byte] item_length[uint32] item_data[item_length byte] ...
[XML content data block] = "CT" [2 bytes] length[uint32] 

"<" element_name_index_str_table prop_name_index_str_table prop_value_index_str_table ">" value_name_index_str_table "</" element_name_index_str_table ">"

"<" element_name_index_str_table prop_name_index_str_table prop_value_index_str_table ">" value_name_index_str_table 
  "<" element2_name_index_str_table prop_name_index_str_table prop_value_index_str_table "/>"
"/>"

/************************************************************************/

#define SZ_BIN_XML_HEADER		"MBXML"
#define LEN_BIN_XML_HEADER		(sizeof(SZ_BIN_XML_HEADER) - 1)

#define LEN_BLOCK_HEADER		2
#define LEN_BIN_XML_MIN			(LEN_BIN_XML_HEADER + sizeof(byte) + sizeof(uint32) + sizeof(uint32) + (LEN_BLOCK_HEADER + sizeof(uint32) + 2) * 2)

#define BX_BLOCK_BIN			"BN"
#define BX_BLOCK_STING_TABLE	"ST"
#define BX_BLOCK_CONTENT		"CT"


enum BinXmlVersion
{
	BXV_1				= '1'
};

enum BinXmlFlag
{
	BXF_UCS2			= 1,		// UCS2 encoding, or it's UTF-8 encoding
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

template<class _TCHAR>
class CBinXmlStrTable
{
public:
	typedef basic_string<_TCHAR> _string;

	CBinXmlStrTable(_TCHAR *p, uint32 nLength) {
		typedef vector<_TCHAR *>	VecPStr;
		VecPStr			vStr;

		ASSERT(StrBeginWith((LPCSTR)p, BX_BLOCK_STING_TABLE));
		p += LEN_BLOCK_HEADER;

		m_length += NetbyteToUInt32(p);
		p += sizeof(uint32);

		const _TCHAR *str = (const _TCHAR *)p;
		const _TCHAR *end = str + m_length / sizeof(uint32);
		while (str < end)
		{
			vStr.push_back(str);
			size_t n = _xcslen(str);
			str += n + 1;
		}
		ASSERT(str == end);

		m_vStr.reserve(vStr.size());
		for (size_t i = 0; i < vStr.size(); i++)
			m_vStr.push_back(vStr[i]);
	}

	_string GetStr(int nIndex) {
		nIndex -= BX_ID_STR_START;
		ASSERT(nIndex >= 0 && nIndex < m_vStr.size());
		if (nIndex < 0 || nIndex >= m_vStr.size())
		{
			_TCHAR	str[1];
			str[0] = '\0';
			return str;
		}

		return m_vStr[nIndex];
	}

	typedef vector<_string>	VecStr;

	VecStr			m_vStr;
	uint32			m_length;

};

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
	uint32	nXmlFlags = NetbyteToUInt32(data);
	data += sizeof(uint32);

	// Length
	uint32	nTotalLength = NetbyteToUInt32(data);
	data += sizeof(uint32);
	dataEnd += nTotalLength;

	if (nTotalLength > nLen)
		return E_INVALID_SIZE;

	while (data + LEN_BLOCK_HEADER + sizeof(uint32) < dataEnd)
	{
		if (StrBeginWith((LPCSTR)data, BX_BLOCK_STING_TABLE))
		{
			data += LEN_BLOCK_HEADER;
			uint32 lenStrTable = NetbyteToUInt32(data);
		}
		else if (StrBeginWith((LPCSTR)data, BX_BLOCK_BIN))
		{
			data += LEN_BLOCK_HEADER;
			uint32 lenStrTable = NetbyteToUInt32(data);
		}
	}

	return E_OK;
}

CMLBinXMLParser::MLBinXmlParseError CMLBinXMLParser::ParseFile(LPCTSTR szFile, CSimpleXML& xmlData)
{
	return E_OK;
}
