// SimpleXMLParser1.cpp: implementation of the CSimpleXMLParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utils.h"
#include "SimpleXMLParser.h"

//
// return CHAR_ENCODING of XML file.
//
CHAR_ENCODING GetXMLTxtEncoding(const char *szTxt, int nLen)
{
	// encoding='gb2312'
	const char	*szEncoding = "encoding";
	const char	*pE;
	const char	*szEnd = szTxt + (nLen < 150 ? nLen : 150);
	char		szCoding[64];
	char		*p;

	while (szTxt < szEnd)
	{
		if (*szTxt == 'e')
		{
			// 12 = strlen(encoding=' ')
			// make sure text is long enough.
			if (szTxt + 12 >= szEnd)
				return ED_SYSDEF;

			// eat up "encoding"
			pE = szEncoding;
			while (*pE && *szTxt == *pE)
			{
				pE++;
				szTxt++;
			}

			if (*pE == '\0')
			{
				// ignore space
				while (*szTxt == ' ')
					szTxt++;

				// eat up =' or ="
				if (*szTxt == '=' && (szTxt[1] == '\'' || szTxt[1] == '\"'))
				{
					szTxt += 2;
					// max encoding string length is 32.
					if (szEnd > szTxt + 32)
						szEnd = szTxt + 32;
					p = szCoding;
					while (szTxt < szEnd && *szTxt != '\'' && *szTxt != '\"')
					{
						*p = *szTxt;
						p++;
						szTxt++;
					}
					*p = '\0';
					return GetCharEncodingID(szCoding);
				}
			}
			
		}
		szTxt++;
	}

	return ED_SYSDEF;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSimpleXMLParser::CSimpleXMLParser()
{
	m_szData = NULL;
	m_nDeepth = 0;
	m_nLen = 0;
	nPos = 0;
	nPosNext = 0;
	m_nEncodingID = ED_SYSDEF;
	m_bNoProlog = false;
}

CSimpleXMLParser::~CSimpleXMLParser()
{
}


//
// UTF8: EF BB BF
// UNICODE: FF FE
// UNICODE 编码和其它编码的解析方式不同，需要单独处理
SimpleXMLParserError CSimpleXMLParser::ParseData(LPCVOID lpXmlData, size_t nLen, CSimpleXML &xmlData)
{
	SimpleXMLParserError		Error;
	BYTE						*szBuffer;
	FILE_ENCODING				fileEncoding;

	szBuffer = (BYTE *)lpXmlData;
	fileEncoding = GetStrEncodingType(szBuffer);
#ifdef _UNICODE
	if (fileEncoding == FE_UCS2)
	{
		// UNICODE
		Error = DoParse((const WCHAR *)(szBuffer + 2), (nLen - 2) / sizeof(WCHAR), xmlData);
	}
	else
	{
		wstringex		str;
		if (fileEncoding == FE_UTF8)
		{
			szBuffer += 3;
			nLen -= 3;
			Utf8ToUCS2((const char *)szBuffer, nLen, str);
			Error = DoParse(str.c_str(), str.size(), xmlData);
		}
		else
		{
			// get encoding: encoding='gb2312'
			ConvertStr((const char *)szBuffer, nLen, str, GetXMLTxtEncoding((const char *)szBuffer, nLen));
			Error = DoParse(str.c_str(), str.size(), xmlData);
		}
	}
#else
	if (fileEncoding == FE_ANSI)
	{
		Error = DoParse((const char *)szBuffer, nLen, xmlData);
	}
	else
	{
		stringex	str;
		if (fileEncoding == FE_UTF8)
		{
			szBuffer += 3;
			nLen -= 3;
			UTF8ToTString((const char *)szBuffer, nLen, str);
		}
		else
		{
			// UNICODE
			szBuffer += 2;
			nLen += 2;
			UCS2ToTString((const WCHAR *)szBuffer, nLen, str);
		}
		Error = DoParse(str.c_str(), str.size(), xmlData);
	}
#endif

	return Error;
}

SimpleXMLParserError CSimpleXMLParser::ParseFile(LPCTSTR szFile, CSimpleXML &xmlData)
{
	CCharBuffer				str;
	SimpleXMLParserError	Err;

	if (!OpenFileToBuffer(szFile, str))
		return SXPE_OPEN_FILE_FAILED;

	Err = ParseData(str.data(), str.size(), xmlData);

	return Err;
}

void CSimpleXMLParser::IgnoreSpace(int nPos, int &nPosNext)
{
	nPosNext = nPos;
	while (m_szData[nPosNext] == 0x20 || m_szData[nPosNext] == 0x9 ||
		m_szData[nPosNext] == 0xD || m_szData[nPosNext] == 0xA)
	{
		nPosNext++;
	}
}

#define SZ_L_XML_DECL	"<?xml"
#define LEN_L_XML_DECL	5

LPCSTR	__szEsc[] = { "lt;", "gt;", "amp;", "quot;", "apos;", NULL };
int		__nLen[]  = { 3,     3,     4,       5,       5      };
char	__chOrg[] = { '<',   '>',   '&',     '\"',    '\''   };

ElementType CSimpleXMLParser::ReadElemnt(ElementType desireType, _tstringex &strElemnt, int nPos, int &nPosNext)
{
	ElementType		retType = ET_UNKNOWN;
	int		k;

	if (nPos >= m_nLen)
		return ET_EOF_FILE;

	// <?xml version="1.0" encoding='gb2312'?>
	switch (desireType)
	{
	case ET_EQ:		// =
		if (m_szData[nPos] == '=')
		{
			retType = desireType;
			nPosNext = nPos + 1;
		}
		break;
	case ET_L_BRACKET:		// <
		if (m_szData[nPos] == '<')
		{
			retType = desireType;
			nPosNext = nPos + 1;
		}
		break;
	case ET_R_BRACKET:		// >
		if (m_szData[nPos] == '>')
		{
			retType = desireType;
			nPosNext = nPos + 1;
		}
		break;
	case ET_EL_BRACKET:		// </
		if (m_szData[nPos] == '<' && m_szData[nPos + 1] == '/')
		{
			retType = desireType;
			nPosNext = nPos + 2;
		}
		break;
	case ET_ER_BRACKET:		// />
		if (m_szData[nPos] == '/' && m_szData[nPos + 1] == '>')
		{
			retType = desireType;
			nPosNext = nPos + 2;
		}
		break;
	case ET_L_XML_DECL:
		// <?xml
		if (_xcsncmp(m_szData + nPos, SZ_L_XML_DECL, LEN_L_XML_DECL) == 0)
		{
			retType = ET_L_XML_DECL;
			nPosNext = nPos + LEN_L_XML_DECL;
		}
		break;
	case ET_R_XML_DECL:		// ?>
		if (m_szData[nPos] == '?' && m_szData[nPos + 1] == '>')
		{
			retType = desireType;
			nPosNext = nPos + 2;
		}
		break;
	case ET_SPACE:
		// White Space
		// [3]    S    ::=    (#x20 | #x9 | #xD | #xA)+ 
		{
			int		i = nPos;
			strElemnt.clear();
			while (m_szData[i] == 0x20 || m_szData[i] == 0x9 ||
				m_szData[i] == 0xD || m_szData[i] == 0xA)
			{
				strElemnt += m_szData[i];
				i++;
			}
			if (i > nPos)
			{
				retType = ET_SPACE;
				nPosNext = i;
			}
		}
		break;
	case ET_NAME:
		// Names and Tokens
		//	[4]    NameChar    ::=    Letter | Digit | '.' | '-' | '_' | ':' | CombiningChar | Extender 
		//	[5]    Name    ::=    (Letter | '_' | ':') (NameChar)* 
		//	[6]    Names    ::=    Name (S Name)* 
		//	[7]    Nmtoken    ::=    (NameChar)+ 
		//	[8]    Nmtokens    ::=    Nmtoken (S Nmtoken)*
		{
			int		i = nPos;
			strElemnt.clear();

			if (IsAlpha(m_szData[i]))
			{
				strElemnt += m_szData[i];
				i++;
				while (IsAlpha(m_szData[i]) || IsDigit(m_szData[i]) || m_szData[i] == '_' || m_szData[i] == '-')
				{
					strElemnt += m_szData[i];
					i++;
				}
			}
			if (i > nPos)
			{
				retType = ET_NAME;
				nPosNext = i;
			}
		}
		break;
	case ET_VALUE:
		{
			// "value"
			int		i = nPos;
			strElemnt.clear();
			TCHAR	chTag;

			chTag = m_szData[i];
			if (chTag == '\"' || chTag == '\'')
			{
				i++;
				while (m_szData[i] != chTag && i < m_nLen)
				{
					if (m_szData[i] == '&')
					{
						i++;
						for (k = 0; __szEsc[k] != NULL; k++)
						{
							if (i + __nLen[k] < m_nLen && _xcsncmp(m_szData + i, __szEsc[k], __nLen[k]) == 0)
							{
								strElemnt += __chOrg[k];
								i += __nLen[k];
								break;
							}
						}
						if (__szEsc[k] == NULL)
						{
							strElemnt += '&';
						}
					}
					else
					{
						strElemnt += m_szData[i];
						i++;
					}
				}
				if (m_szData[i] == chTag)
				{
					retType = ET_VALUE;
					nPosNext = i + 1;
				}
			}
		}
		break;
	case ET_TEXT_VALUE:
		{
			// 
			int		i = nPos;
			strElemnt.clear();

			while (m_szData[i] != '<' && i < m_nLen)
			{
				if (m_szData[i] == '&')
				{
					i++;
					for (k = 0; __szEsc[k] != NULL; k++)
					{
						if (i + __nLen[k] < m_nLen && _xcsncmp(m_szData + i, __szEsc[k], __nLen[k]) == 0)
						{
							strElemnt += __chOrg[k];
							i += __nLen[k];
							break;
						}
					}
					if (__szEsc[k] == NULL)
					{
						strElemnt += m_szData[i];
					}
				}
				else
				{
					strElemnt += m_szData[i];
					i++;
				}
			}
			if (m_szData[i] == '<')
			{
				retType = ET_TEXT_VALUE;
				nPosNext = i;
			}
		}
		break;
	case ET_ANY:
		{
			int		i = nPos;
			strElemnt.clear();

			if (m_szData[i] == '\"')
			{
				return ReadElemnt(ET_VALUE, strElemnt, nPos, nPosNext);
			}
			else if (m_szData[i] == 0x20)
			{
				return ReadElemnt(ET_SPACE, strElemnt, nPos, nPosNext);
			}
			else if (m_szData[i] == '=')
			{
				return ReadElemnt(ET_EQ, strElemnt, nPos, nPosNext);
			}
			else if (m_szData[i] == '<')
			{
				if (m_szData[i + 1] == '/')
					return ReadElemnt(ET_EL_BRACKET, strElemnt, nPos, nPosNext);
				else
					return ReadElemnt(ET_L_BRACKET, strElemnt, nPos, nPosNext);
			}
			else if (m_szData[i] == '>')
			{
				return ReadElemnt(ET_R_BRACKET, strElemnt, nPos, nPosNext);
			}
			else if (m_szData[i] == '?')
			{
				if (m_szData[i + 1] == '>')
					return ReadElemnt(ET_R_XML_DECL, strElemnt, nPos, nPosNext);
			}
			else if (m_szData[i] == '/')
			{
				if (m_szData[i + 1] == '>')
					return ReadElemnt(ET_ER_BRACKET, strElemnt, nPos, nPosNext);
			}
			else if (IsAlpha(m_szData[i]))
			{
				// name?
				return ReadElemnt(ET_NAME, strElemnt, nPos, nPosNext);
			}
		}
		break;
	default:
		break;
	}
	return retType;
}

SimpleXMLParserError CSimpleXMLParser::DoParse(const TCHAR *szData, int nLen, CSimpleXML &xmlData)
{
	string					strElement;
	SimpleXMLParserError	Error;

	m_szData = szData;
	m_nLen = nLen;

	nPos = 0; nPosNext = 0;
	Error = ParseProlog(nPos, nPosNext, xmlData);
	if (Error != SXPE_OK)
		return Error;

	SXNode	node;

	nPos = nPosNext;
	m_nDeepth = 0;
	Error = ParseEntity(nPos, nPosNext, &node);
	if (Error != SXPE_OK)
		return Error;

	ASSERT(node.listChildren.size() == 1);
	if (node.listChildren.size() != 1)
		return SXPE_FORMAT_ERROR;

	xmlData.m_pRoot = node.listChildren.front();
	node.listChildren.clear();

	return SXPE_OK;
}

SimpleXMLParserError CSimpleXMLParser::ParseProlog(int nPos, int &nPosNext, CSimpleXML &xmlData)
{
	_tstringex			strElement;
	ElementType			retType;
	ElementType			desireType;
	_tstringex			strName;
	_tstringex			strValue;

	nPos = 0; nPosNext = 0;

	// ignore all blank spaces..
	ReadElemnt(ET_SPACE, strName, nPos, nPosNext);
	nPos = nPosNext;

	if (ET_L_XML_DECL != ReadElemnt(ET_L_XML_DECL, strElement, nPos, nPosNext))
	{
		// 不是XML文件
		if (m_bNoProlog)
		{
			// No prologue
			nPosNext = 0;
			m_nEncodingID = ED_UTF8;
			return SXPE_OK;
		}

		return SXPE_NOT_XML_FORMAT;
	}
	m_bNoProlog = false;

	nPos = nPosNext;

	desireType = ET_SPACE;
	retType = ReadElemnt(desireType, strElement, nPos, nPosNext);
	if (retType != desireType)
		return SXPE_FORMAT_ERROR;
	nPos = nPosNext;

	// 读入XML declaration
	while (1)
	{
		// name
		desireType = ET_NAME;
		retType = ReadElemnt(desireType, strName, nPos, nPosNext);
		if (retType != desireType)
			return SXPE_FORMAT_ERROR;
		nPos = nPosNext;

		IgnoreSpace(nPos, nPosNext);
		nPos = nPosNext;

		desireType = ET_EQ;
		retType = ReadElemnt(desireType, strElement, nPos, nPosNext);
		if (retType != ET_EQ)
			return SXPE_FORMAT_ERROR;
		nPos = nPosNext;

		IgnoreSpace(nPos, nPosNext);
		nPos = nPosNext;

		desireType = ET_VALUE;
		retType = ReadElemnt(desireType, strValue, nPos, nPosNext);
		if (retType != desireType)
			return SXPE_FORMAT_ERROR;
		nPos = nPosNext;

		if (strName.Cmp("encoding") == 0)
		{
			// EncodingDecl    ::=    S 'encoding' Eq ('"' EncName '"' | "'" EncName "'" )  
			// EncName    ::=    [A-Za-z] ([A-Za-z0-9._] | '-')* /* Encoding name contains only Latin characters */ 
			xmlData.m_strEncoding = strValue.c_str();
			m_nEncodingID = GetCharEncodingID(xmlData.m_strEncoding.c_str());
		}

		IgnoreSpace(nPos, nPosNext);
		nPos = nPosNext;

		desireType = ET_R_XML_DECL;
		retType = ReadElemnt(desireType, strValue, nPos, nPosNext);
		if (retType == desireType)
		{
			nPos = nPosNext;
			break;
		}
	}

	return SXPE_OK;
}

SimpleXMLParserError CSimpleXMLParser::ParseEntity(int nPos, int &nPosNext, SXNode *pNode)
{
	_tstringex				strElement;
	SimpleXMLParserError	Error;

	CDeepStack		deepstack(&m_nDeepth);
	_tstringex		strNodeName;
	_tstringex		strPropName, strValue;

	while (1)
	{
		ElementType		retType;
		ElementType		desireType;
		SXNode			*pNewNode;

		IgnoreSpace(nPos, nPosNext);
		nPos = nPosNext;

		// <
		desireType = ET_L_BRACKET;
		retType = ReadElemnt(desireType, strElement, nPos, nPosNext);
		if (retType != desireType)
			return SXPE_FORMAT_ERROR;
		nPos = nPosNext;

		// <name
		desireType = ET_NAME;
		retType = ReadElemnt(desireType, strNodeName, nPos, nPosNext);
		if (retType != desireType)
			return SXPE_FORMAT_ERROR;
		nPos = nPosNext;

		pNewNode = new SXNode;
		pNewNode->strName = strNodeName.c_str();
		pNode->listChildren.push_back(pNewNode);

		// <name property, or <name> or <name/>
		desireType = ET_ANY;
		retType = ReadElemnt(desireType, strElement, nPos, nPosNext);
		nPos = nPosNext;
		if (retType == ET_SPACE)
		{
			// <name property
			while (1)
			{
				IgnoreSpace(nPos, nPosNext);
				nPos = nPosNext;

				desireType = ET_NAME;
				retType = ReadElemnt(desireType, strPropName, nPos, nPosNext);
				if (retType != desireType)
					return SXPE_FORMAT_ERROR;
				nPos = nPosNext;

				IgnoreSpace(nPos, nPosNext);
				nPos = nPosNext;

				desireType = ET_EQ;
				retType = ReadElemnt(desireType, strElement, nPos, nPosNext);
				if (retType != desireType)
					return SXPE_FORMAT_ERROR;
				nPos = nPosNext;

				IgnoreSpace(nPos, nPosNext);
				nPos = nPosNext;

				// value
				desireType = ET_VALUE;
				retType = ReadElemnt(desireType, strValue, nPos, nPosNext);
				if (retType != desireType)
					return SXPE_FORMAT_ERROR;
				nPos = nPosNext;

				pNewNode->AddProperty(strPropName.c_str(), strValue.c_str());

				IgnoreSpace(nPos, nPosNext);
				nPos = nPosNext;

				// />
				desireType = ET_ER_BRACKET;
				retType = ReadElemnt(desireType, strValue, nPos, nPosNext);
				if (retType == desireType)
				{
					nPos = nPosNext;
					break;
				}

				// ><...></name>
				desireType = ET_R_BRACKET;
				retType = ReadElemnt(desireType, strValue, nPos, nPosNext);
				if (retType == desireType)
				{
					// break 之后，由下面的代码接着处理:D
					nPos = nPosNext;
//					Error = ParseEntity(nPos, nPosNext, pNewNode);
//					if (Error != SXPE_OK)
//						return Error;
//					nPos = nPosNext;
//
//					// </name>
//					desireType = ET_EL_BRACKET;
//					retType = ReadElemnt(desireType, strElement, nPos, nPosNext);
//					if (retType != desireType)
//						return SXPE_FORMAT_ERROR;
					break;
				}
			}
		}

		if (retType == ET_R_BRACKET)
		{
			// <name>
			desireType = ET_TEXT_VALUE;
			retType = ReadElemnt(desireType, strElement, nPos, nPosNext);
			if (retType == ET_TEXT_VALUE)
			{
				nPos = nPosNext;
				pNewNode->strContent = strElement.c_str();
				TrimStr(pNewNode->strContent);
			}

			desireType = ET_ANY;
			retType = ReadElemnt(desireType, strElement, nPos, nPosNext);
			if (retType == ET_L_BRACKET)
			{
				// <....>...
				// nPos = nPosNext;
				Error = ParseEntity(nPos, nPosNext, pNewNode);
				if (Error != SXPE_OK)
					return Error;
//				nPos = nPosNext;
//				desireType = ET_ANY;
//				retType = ReadElemnt(desireType, strElement, nPos, nPosNext);
			}
			else if (retType != ET_EL_BRACKET)
				return SXPE_FORMAT_ERROR;
			nPos = nPosNext;

			desireType = ET_NAME;
			retType = ReadElemnt(desireType, strElement, nPos, nPosNext);
			if (retType != desireType)
				return SXPE_FORMAT_ERROR;
			nPos = nPosNext;

			if (strElement != strNodeName)
				return SXPE_FORMAT_ERROR;

			desireType = ET_R_BRACKET;
			retType = ReadElemnt(desireType, strElement, nPos, nPosNext);
			if (retType != desireType)
				return SXPE_FORMAT_ERROR;
			nPos = nPosNext;
		}
		else if (retType == ET_ER_BRACKET)
		{
			// <name/>
			// no property, no children
		}

		IgnoreSpace(nPos, nPosNext);
		nPos = nPosNext;

		// </
		desireType = ET_EL_BRACKET;
		retType = ReadElemnt(desireType, strElement, nPos, nPosNext);
		if (retType == desireType)
			break;

		if (m_nDeepth == 1)
		{
			desireType = ET_EOF_FILE;
			retType = ReadElemnt(desireType, strElement, nPos, nPosNext);
			if (retType == desireType)
				break;
		}
	}

	return SXPE_OK;
}
