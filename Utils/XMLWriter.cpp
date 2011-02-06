// XMLWriter.cpp: implementation of the CXMLWriter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XMLWriter.h"
#include "UUEncode.h"

#ifdef _DEBUG
void XMLWriterTest()
{
	CXMLWriter		xml;

	xml.WriteStartElement(_T("Root"));
	xml.WriteAttribute(_T("attr1"), 1);
	xml.WriteAttribute(_T("attr2"), _T("value2"));

	xml.WriteStartElement(_T("L1_tree1"));
	xml.WriteAttribute(_T("attr2"), _T("value2"));
	xml.WriteEndElement();

	xml.WriteElementString(_T("L1_11"), _T("L1_11_value"));

	xml.WriteEndElement();

	xml.SaveAsFile(_T("test.xml"));

}
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXMLWriter::CXMLWriter(bool bWriteStartDoc)
{
	if (bWriteStartDoc)
		WriteStartDocument(_T("utf-8"));
	m_strIndent = "  ";
}

CXMLWriter::~CXMLWriter()
{
	ASSERT(m_vElementStack.empty());
}

void CXMLWriter::WriteStartDocument(LPCTSTR szEncoding)
{
	m_nEncodingID = GetCharEncodingID(szEncoding);

	m_strBuff += "<?xml version=\"1.0\" encoding='";
	WriteDirect(szEncoding);
	m_strBuff += "'?>\r\n";

	m_wp = WP_READY;
}

void CXMLWriter::WriteEndDocument()
{

}

void CXMLWriter::WriteStartElement(LPCTSTR szElement)
{
	if (m_wp == WP_ELEMENT_HEAD)
		m_strBuff += ">\r\n";

	for (int i = 0; i < (int)m_vElementStack.size(); i++)
		m_strBuff += m_strIndent.c_str();

	m_wp = WP_ELEMENT_HEAD;

	m_strBuff += "<";
	WriteDirect(szElement);

	PushElementStack(szElement);
}

void CXMLWriter::WriteAttribute(LPCTSTR szAttr, LPCTSTR szValue)
{
	ASSERT(m_wp == WP_ELEMENT_HEAD);

	m_strBuff += " ";
	WriteDirect(szAttr);
	m_strBuff += "=\"";

	WriteConvert(szValue);

	m_strBuff += "\"";
}

void CXMLWriter::WriteAttribute(LPCTSTR szAttr, int nValue)
{
	TCHAR	szBuff[64];
	_itot(nValue, szBuff, 10);
	WriteAttribute(szAttr, szBuff);
}

void CXMLWriter::WriteAttribute(LPCTSTR szAttr, LPCVOID data, size_t lenData)
{
	// Convert: Bin >> Base64
	tstring	strB64;
	encodeuu((LPCSTR)data, lenData, strB64, true, false);
	WriteAttribute(szAttr, strB64.c_str());
}

void CXMLWriter::WriteEndElement()
{
	ASSERT(m_wp == WP_ELEMENT_HEAD ||
		m_wp == WP_ELEMENT_BODY);

	if (m_wp == WP_ELEMENT_HEAD)
		m_strBuff += "/>\r\n";
	else if (m_wp == WP_ELEMENT_BODY)
	{
		if (m_strBuff[(uint)(m_strBuff.size() - 1)] == '\n')
		{
			for (int i = 1; i < (int)m_vElementStack.size(); i++)
				m_strBuff += m_strIndent.c_str();
		}

		m_strBuff += "</";
		m_strBuff += GetCurElementName();
		m_strBuff += ">\r\n";
	}

	PopElementStack();
	if (!m_vElementStack.empty())
		m_wp = WP_ELEMENT_BODY;
	else
		m_wp = WP_READY;
}

void CXMLWriter::WriteString(LPCTSTR szString)
{
	ASSERT(m_wp == WP_ELEMENT_HEAD);
	m_wp = WP_ELEMENT_BODY;

	m_strBuff += ">";

	WriteConvert(szString);
}

void CXMLWriter::WriteElementString(LPCTSTR szElement, LPCTSTR szString)
{
	for (int i = 0; i < (int)m_vElementStack.size(); i++)
		m_strBuff += m_strIndent.c_str();

	m_strBuff += "<";
	WriteDirect(szElement);
	m_strBuff += ">";

	WriteConvert(szString);

	m_strBuff += "</";
	WriteDirect(szElement);
	m_strBuff += ">\r\n";
}

void CXMLWriter::WriteElementInt(LPCTSTR szElement, int nValue)
{
	TCHAR		buf[16];
	_itot(nValue, buf, 10);
	WriteElementString(szElement, buf);
}

void CXMLWriter::WriteComment(LPCTSTR szComment)
{
	for (int i = 0; i < (int)m_vElementStack.size() + 1; i++)
		m_strBuff += m_strIndent.c_str();

	m_strBuff += "<!-- ";
	WriteConvert(szComment);
	m_strBuff += " -->\r\n";
}

void CXMLWriter::WriteData(const char *szData, int n)
{
	m_strBuff.append(szData, n);
}

BOOL CXMLWriter::SaveAsFile(LPCTSTR szFile)
{
	return SaveDataAsFile(szFile, (LPVOID)m_strBuff.c_str(), m_strBuff.size());
}

void CXMLWriter::WriteDirect(LPCTSTR szBuff)
{
#ifdef _UNICODE
	int		n = _tcslen(szBuff);
	if ((int)(m_strBuff.capacity() - m_strBuff.size()) <= n * 3)
		m_strBuff.reserve(m_strBuff.size() + n * 3 + 1);
	ConvertStr2(szBuff, n, m_strBuff.data() + m_strBuff.size(), n * 3, m_nEncodingID);
	m_strBuff.resize(m_strBuff.size() + strlen(m_strBuff.data() + m_strBuff.size()));
#else
	m_strBuff += szBuff;
#endif
}

void CXMLWriter::WriteConvert(LPCTSTR szBuff)
{
	LPCSTR		szContent = NULL;
#ifdef _UNICODE
	ConvertStr(szBuff, -1, m_strTemp, m_nEncodingID);
	szContent = m_strTemp.c_str();
#else
	szContent = szBuff;
#endif
	LPCSTR		szEnd;

	while (*szContent)
	{
		szEnd = szContent;
		while (*szEnd && *szEnd != '<' && *szEnd != '>'
			&& *szEnd != '&' && *szEnd != '\"'
			&& *szEnd != '\'')
			szEnd++;

		if ((int)(szEnd - szContent) > 0)
			m_strBuff.append(szContent, (int)(szEnd - szContent));
		if (*szEnd == '\0')
			break;

		switch (*szEnd)
		{
			case '<': m_strBuff += "&lt;"; break;
			case '>': m_strBuff += "&gt;"; break;
			case '&': m_strBuff += "&amp;"; break;
			case '\"': m_strBuff += "&quot;"; break;
			case '\'': m_strBuff += "&apos;"; break;
			default: ASSERT(0);
		}
		szEnd++;
		szContent = szEnd;
	}
}

void CXMLWriter::PushElementStack(LPCTSTR szElement)
{
#ifdef _UNICODE
	ConvertStr(szElement, -1, m_strTemp, m_nEncodingID);
	m_vElementStack.push_back(m_strTemp.c_str());
#else
	m_vElementStack.push_back(szElement);
#endif
}

LPCSTR CXMLWriter::GetCurElementName()
{
	if (m_vElementStack.empty())
		return "";
	else
		return m_vElementStack.back().c_str();
}
