// XMLWriter.h: interface for the CXMLWriter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLWRITER_H__763FEC52_F9EE_4CC5_9682_C79A601FE98E__INCLUDED_)
#define AFX_XMLWRITER_H__763FEC52_F9EE_4CC5_9682_C79A601FE98E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CXMLWriter  
{
public:
	CXMLWriter(bool bWriteStartDoc = true);
	virtual ~CXMLWriter();

	// Writes: <?xml version=\"1.0\" encoding='"szEncoding"?>
	void WriteStartDocument(LPCTSTR szEncoding);

	void WriteEndDocument();

	// Begin xml element
	void WriteStartElement(LPCTSTR szElement);

	// Writes: szAttr="szValue"
	void WriteAttribute(LPCTSTR szAttr, LPCTSTR szValue);

	// Writes: szAttr="nValue"
	void WriteAttribute(LPCTSTR szAttr, int nValue);

	// Close xml element
	void WriteEndElement();

	// Writes the given text content: <node>szString</node>
	void WriteString(LPCTSTR szString);

	// Writes: <szElement>szString</szElement>
	void WriteElementString(LPCTSTR szElement, LPCTSTR szString);

	// Writes: <szElement>nValue</szElement>
	void WriteElementInt(LPCTSTR szElement, int nValue);

	// Writes: <!-- szComment -->
	void WriteComment(LPCTSTR szComment);

	// Write Data directly, it's useful to joint/insert several xml node data.
	void WriteData(const char *szData, int n);

	// Get xml data
	LPCSTR GetData() { return m_strBuff.data(); }
	size_t GetLength() { return m_strBuff.size(); }
	CCharBuffer &GetBuffer() { return m_strBuff; }

	BOOL SaveAsFile(LPCTSTR szFile);

	void SetIndentStr(LPSTR szIndentStr) { m_strIndent = szIndentStr; }

protected:
	void WriteDirect(LPCTSTR szBuff);
	void WriteConvert(LPCTSTR szBuff);

	void PushElementStack(LPCTSTR szElement);
	void PopElementStack() { ASSERT(!m_vElementStack.empty()); if (!m_vElementStack.empty()) m_vElementStack.pop_back(); }
	LPCSTR GetCurElementName();

protected:
	string				m_strIndent;
	CHAR_ENCODING		m_nEncodingID;
	vector<string>		m_vElementStack;
	stringex			m_strBuff;
	stringex			m_strTemp;	// temp buffer to save new/delete time.

	enum WritePos
	{
		WP_READY,
		WP_ELEMENT_HEAD,
		WP_ELEMENT_BODY,
	};

	WritePos			m_wp;

};

#endif // !defined(AFX_XMLWRITER_H__763FEC52_F9EE_4CC5_9682_C79A601FE98E__INCLUDED_)
