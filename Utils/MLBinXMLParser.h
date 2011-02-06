#pragma once

#include "SimpleXML.h"
#include "XMLWriter.h"

DECLAR_CPPUNIT_TEST_REG(CMLBinXMLParser)

class CMLBinXMLParser
{
public:
	CMLBinXMLParser(void);
	virtual ~CMLBinXMLParser(void);

	enum MLBinXmlParseError
	{
		E_OK,
		E_INVALID_HEADER,
		E_UNKNOWN_VERSION,
		E_INVALID_SIZE,
		E_INVALID_STR_TABLE,
		E_INVALID_BIN_TABLE,
		E_INVALID_BLOCK_TYPE,
		E_NO_STR_TABLE,
		E_FMT_NEED_LBRACKET,
		E_FMT_NEED_ELEMENT_END,
	};

	MLBinXmlParseError ParseData(LPCVOID lpXmlData, size_t nLen, CSimpleXML &xmlData);

	static bool IsHeaderType(LPCSTR szHeader);

};

class CMLBinXMLWriter : public CXMLWriter
{
public:
	CMLBinXMLWriter();
	virtual ~CMLBinXMLWriter();

	virtual void WriteStartDocument(LPCTSTR szEncoding) { }

	virtual void WriteEndDocument() { }

	// Begin xml element
	void WriteStartElement(LPCTSTR szElement);

	// Writes: szAttr="szValue"
	void WriteAttribute(LPCTSTR szAttr, LPCTSTR szValue);

	// Write binary data attribute
	void WriteAttribute(LPCTSTR szAttr, LPCVOID data, size_t lenData);

	// Close xml element
	void WriteEndElement();

	// Writes the given text content: <node>szString</node>
	void WriteString(LPCTSTR szString);

	// Writes: <szElement>szString</szElement>
	void WriteElementString(LPCTSTR szElement, LPCTSTR szString);

	// Writes: <!-- szComment -->
	void WriteComment(LPCTSTR szComment);

	CCharBuffer &GetBuffer();

	BOOL SaveAsFile(LPCTSTR szFile);

protected:
	int GetStrIndex(LPCTSTR str);

	void WriteStrTable();
	void WriteContentBlock(int indexSize);

protected:
	typedef map<tstring, int>	MapStrIndex;
	MapStrIndex			m_mapStrTable;

	int					m_nElementDeep;
	vector<int>			m_vContentBlock;

	stringex			m_bufBinTable;
	size_t				m_nBinItemCount;

};
