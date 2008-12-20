// SimpleXML.h: interface for the CSimpleXML class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLEXML_H__C6DFE0B7_DFB0_4DF5_937A_406F7FB35C5D__INCLUDED_)
#define AFX_SIMPLEXML_H__C6DFE0B7_DFB0_4DF5_937A_406F7FB35C5D__INCLUDED_

DECLAR_CPPUNIT_TEST_REG(SimpleXML)


class CSimpleXML;
class CXMLWriter;

class SXNode;

SXNode *DupSXNode(SXNode *pOrgNode);

#define SURETEXT(p1)	(p1 ? p1 : _T(""))

//////////////////////////////////////////////////////////////////////////
//
// Only keep basic XML data structure, discard other tags (exp: comments...)
//
// <strName property="value">
//   <strName>Content</strName>
// </StrName>
class SXNode
{
public:
	typedef list<SXNode*>	LIST_CHILDREN;
	typedef list<tstring>	LIST_STRING;

	struct Property
	{
		tstring			strName;
		tstring			strValue;
	};
	typedef list<Property>	LIST_PROPERTIES;
	typedef LIST_PROPERTIES::iterator	iterProperties;
	typedef LIST_CHILDREN::iterator	iterator;

	tstring				strName;
	tstring				strContent;		// the content <nodeName>content</nodeName>
	LIST_PROPERTIES		listProperties;
	LIST_CHILDREN		listChildren;

public:
	// Can't be inherited.
	~SXNode();

public:
	// If not exists, NULL will be returned
	LPCTSTR GetProperty(LPCTSTR szPropName);

	// If not exists, _T("") will be returned.
	LPCTSTR GetPropertySafe(LPCTSTR szPropName);

	int GetPropertyInt(LPCTSTR szPropName) { return _ttoi(GetPropertySafe(szPropName)); }

	void AddProperty(LPCTSTR szPropName, LPCTSTR szPropValue);
	void SetProperty(LPCTSTR szPropName, LPCTSTR szPropValue);

	void EraseProperty(LPCTSTR szPropName);

public:
	// Get first child same with the node name
	SXNode *GetChild(LPCTSTR szName);

	void ToXML(CXMLWriter &xmlStream);

};

class CSimpleXML  
{
public:
	CSimpleXML();

	// Can't be inherited.
	~CSimpleXML();

	BOOL ParseFile(LPCTSTR szXmlFile);

	BOOL ParseData(LPCVOID lpXmlData, size_t nLen);

	BOOL SaveFile(LPCTSTR szXmlFile);

	void Free();

public:
	SXNode		*m_pRoot;
	tstring		m_strEncoding;

};

#endif // !defined(AFX_SIMPLEXML_H__C6DFE0B7_DFB0_4DF5_937A_406F7FB35C5D__INCLUDED_)
