#pragma once

#include "SimpleXML.h"

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
	};


	MLBinXmlParseError ParseData(LPCVOID lpXmlData, size_t nLen, CSimpleXML &xmlData);
	MLBinXmlParseError ParseFile(LPCTSTR szFile, CSimpleXML& xmlData);

protected:

};
