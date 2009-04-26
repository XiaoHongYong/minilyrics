 // SimpleXML.cpp: implementation of the CSimpleXML class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SimpleXML.h"
#include "simplexmlParser.h"
#include "XMLWriter.h"
#include "App.h"

IMPLEMENT_CPPUNIT_TEST_REG(SimpleXML)

SXNode *DupSXNode(SXNode *pOrgNode)
{
	SXNode		*pNewNode;

	pNewNode = new SXNode;
	ASSERT(pNewNode);
	if (!pNewNode)
		return NULL;

	for (SXNode::iterator it = pOrgNode->listChildren.begin(); 
		it != pOrgNode->listChildren.end(); ++it)
	{
		SXNode		*pChild = DupSXNode(*it);
		pNewNode->listChildren.push_back(pChild);
	}

	pNewNode->strName = pOrgNode->strName;
	pNewNode->strContent = pOrgNode->strContent;

	pNewNode->listProperties.insert(pNewNode->listProperties.begin(), 
		pOrgNode->listProperties.begin(), pOrgNode->listProperties.end());

	return pNewNode;
}

SXNode::~SXNode()
{
	iterator	it, itEnd;

	itEnd = listChildren.end();
	for (it = listChildren.begin(); it != itEnd; ++it)
	{
		SXNode		*p = *it;
		delete p;
	}
	listChildren.clear();

	listProperties.clear();
}

// If not exists, NULL will be returned
LPCTSTR SXNode::GetProperty(LPCTSTR szPropName)
{
	LIST_PROPERTIES::iterator	it, itEnd;

	itEnd = listProperties.end();

	for (it = listProperties.begin(); it != itEnd; ++it)
	{
		Property	&prop = *it;
		if (_tcscmp(szPropName, prop.strName.c_str()) == 0)
		{
			return prop.strValue.c_str();
		}
	}

	return NULL;
}

// If not exists, _T("") will be returned.
LPCTSTR SXNode::GetPropertySafe(LPCTSTR szPropName)
{
	LPCTSTR		szValue;

	szValue = GetProperty(szPropName);
	if (szValue)
		return szValue;
	else
		return _T("");
}

void SXNode::AddProperty(LPCTSTR szPropName, LPCTSTR szPropValue)
{
	Property	prop;
	prop.strName = szPropName;
	prop.strValue = szPropValue;
	listProperties.push_back(prop);
}

void SXNode::SetProperty(LPCTSTR szPropName, LPCTSTR szPropValue)
{
	LIST_PROPERTIES::iterator	it, itEnd;

	itEnd = listProperties.end();

	for (it = listProperties.begin(); it != itEnd; ++it)
	{
		Property	&prop = *it;
		if (_tcscmp(szPropName, prop.strName.c_str()) == 0)
		{
			prop.strValue = szPropValue;
			return;
		}
	}

	AddProperty(szPropName, szPropValue);
}


void SXNode::EraseProperty(LPCTSTR szPropName)
{
	LIST_PROPERTIES::iterator	it;

	for (it = listProperties.begin(); it != listProperties.end(); ++it)
	{
		Property	&prop = *it;
		if (_tcscmp(szPropName, prop.strName.c_str()) == 0)
		{
			listProperties.erase(it);
			return;
		}
	}
}


// Get first child same with the node name
SXNode *SXNode::GetChild(LPCTSTR szName)
{
	iterator	it, itEnd;

	itEnd = listChildren.end();
	for (it = listChildren.begin(); it != itEnd; ++it)
	{
		SXNode		*p = *it;
		if (_tcscmp(p->strName.c_str(), szName) == 0)
			return p;
	}

	return NULL;
}

void SXNode::ToXML(CXMLWriter &xmlStream)
{
	xmlStream.WriteStartElement(strName.c_str());

	// write properties
	{
		LIST_PROPERTIES::iterator	it, itEnd;

		itEnd = listProperties.end();
		for (it = listProperties.begin(); it != itEnd; ++it)
		{
			Property	&prop = *it;
			xmlStream.WriteAttribute(prop.strName.c_str(), prop.strValue.c_str());
		}
	}

	// write content
	if (!strContent.empty())
		xmlStream.WriteString(strContent.c_str());

	// write children
	{
		iterator	it, itEnd;

		itEnd = listChildren.end();
		for (it = listChildren.begin(); it != itEnd; ++it)
		{
			SXNode		*pNode = *it;
			pNode->ToXML(xmlStream);
		}
	}

	xmlStream.WriteEndElement();
}

#ifdef _DEBUG
/*void PrintNode(SXNode *node, int &nDeep, tstringex &str)
{
	nDeep++;

	str += _T("<");
	str += node->GetName();
	str += _T(">");

	str += node->GetContent();

	for (list<SXNode *>::iterator it = node->listChildren.begin();
		it != node->listChild.end(); ++it)
	{
		printNode(*it, nDeep, str);
	}
	if (node->type == SX_NODE_ELEMENT)
	{
		str += _T("</");
		str += node->name;
		str += _T(">\r\n");
	}
	nDeep--;
}*/
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSimpleXML::CSimpleXML()
{
	m_pRoot = NULL;
}

CSimpleXML::~CSimpleXML()
{
	Free();
}

void CSimpleXML::New(LPCTSTR szRootName)
{
	m_pRoot = new SXNode;
	m_pRoot->strName = szRootName;
}

BOOL CSimpleXML::ParseFile(LPCTSTR szXmlFile, bool bNoProlog)
{
	CSimpleXMLParser		parser;
	SimpleXMLParserError	ret;

	Free();

	parser.m_bNoProlog = bNoProlog;
	ret = parser.ParseFile(szXmlFile, *this);

	if (ret != SXPE_OK)
	{
		DBGLOG2(_T("parse xml file: %s FAILED at pos : %d"), szXmlFile, parser.nPosNext);
		return FALSE;
	}

	return TRUE;
}

BOOL CSimpleXML::ParseData(LPCVOID lpXmlData, size_t nLen, bool bNoProlog)
{
	CSimpleXMLParser		parser;
	SimpleXMLParserError	ret;

	Free();

	parser.m_bNoProlog = bNoProlog;
	ret = parser.ParseData(lpXmlData, nLen, *this);

	if (ret != SXPE_OK)
	{
		// ERRLOG2(_T("parse xml string:%s FAILED at pos : %d"), ((const char *)lpXmlData) + parser.nPosNext, parser.nPosNext);
		// ERRLOGDUMPS(szXmlStr, nLen);
		return FALSE;
	}

	return TRUE;
}

BOOL CSimpleXML::SaveFile(LPCTSTR szXmlFile)
{
	if (!m_pRoot)
		return false;

	CXMLWriter		xmlStream(m_strEncoding.empty());

	if (!m_strEncoding.empty())
		xmlStream.WriteStartDocument(m_strEncoding.c_str());

	m_pRoot->ToXML(xmlStream);

	xmlStream.WriteEndDocument();

	return xmlStream.SaveAsFile(szXmlFile);
}

void CSimpleXML::Free()
{
	if (m_pRoot)
	{
		delete m_pRoot;
		m_pRoot = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
//

#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

class CTestCaseSimpleXML : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseSimpleXML);
	CPPUNIT_TEST(TestSXNode);
	CPPUNIT_TEST_SUITE_END();

protected:

public:
	void setUp()
	{
	}
	void tearDown() 
	{
	}

protected:
	void TestSXNode()
	{
		SXNode		node;

		LPCTSTR		SZ_NODE_NAME = _T("node");
		LPCTSTR		SZ_CONTENT_NAME = _T("content");
		LPCTSTR		SZ_PROP_NAME = _T("PropName_%d");
		LPCTSTR		SZ_VALUE_NAME = _T("Value_%d");
		int			i;

		node.strName = SZ_NODE_NAME;
		node.strContent = SZ_CONTENT_NAME;

		// Add property
		for (i = 0; i < 5; i++)
		{
			TCHAR		szName[64], szValue[64];
			LPCTSTR		p;

			_stprintf(szName, SZ_PROP_NAME, i);
			_stprintf(szValue, SZ_VALUE_NAME, i);
			node.AddProperty(szName, szValue);

			p = node.GetProperty(szName);
			if (!p || !(_tcscmp(p, szValue) == 0))
				CPPUNIT_FAIL_T(CStrPrintf(_T("SXNode.GetProperty(), case: %d, %s"), i, szName).c_str());
		}

		// Erase property
		for (i = 0; i < 1; i++)
		{
			TCHAR		szName[64], szValue[64];
			LPCTSTR		p;

			_stprintf(szName, SZ_PROP_NAME, i);
			_stprintf(szValue, SZ_VALUE_NAME, i);
			// node.AddProperty(szName, szValue);

			node.EraseProperty(szName);
			p = node.GetProperty(szName);
			if (p)
				CPPUNIT_FAIL_T(CStrPrintf(_T("SXNode.EraseProperty(), case: %d, %s"), i, szName).c_str());
		}

		// To xml, and parse from xml
		CXMLWriter		xmlWriter;
		CSimpleXML		xml;

		node.ToXML(xmlWriter);
		CPPUNIT_ASSERT(xml.ParseData(xmlWriter.GetData(), xmlWriter.GetLength()));

		//
		// compare two xml data struct...
		//
		{
			// SXNode			*pNode;

			// CPPUNIT_ASSERT(_tcsicmp(pNode->strName.c_str(), node.strName.c_str()) == 0);
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseSimpleXML);


#endif // _CPPUNIT_TEST
