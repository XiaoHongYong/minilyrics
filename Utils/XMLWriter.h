#pragma once

bool IsValidXMLText(LPCTSTR szText);

class CXMLWriter  
{
public:
	CXMLWriter(bool bWriteStartDoc = true);
	virtual ~CXMLWriter();

	// Writes: <?xml version=\"1.0\" encoding='"szEncoding"?>
	virtual void WriteStartDocument(LPCTSTR szEncoding);

	virtual void WriteEndDocument();

	// Begin xml element
	virtual void WriteStartElement(LPCTSTR szElement);

	// Writes: szAttr="szValue"
	virtual void WriteAttribute(LPCTSTR szAttr, LPCTSTR szValue);

	// Write binary data attribute
	virtual void WriteAttribute(LPCTSTR szAttr, LPCVOID data, size_t lenData);

	// Writes: szAttr="nValue"
	virtual void WriteAttribute(LPCTSTR szAttr, int nValue);

	// Close xml element
	virtual void WriteEndElement();

	// Writes the given text content: <node>szString</node>
	virtual void WriteString(LPCTSTR szString);

	// Writes: <szElement>szString</szElement>
	virtual void WriteElementString(LPCTSTR szElement, LPCTSTR szString);

	// Writes: <szElement>nValue</szElement>
	virtual void WriteElementInt(LPCTSTR szElement, int nValue);

	// Writes: <!-- szComment -->
	virtual void WriteComment(LPCTSTR szComment);

	// Write Data directly, it's useful to joint/insert several xml node data.
	virtual void WriteData(const char *szData, int n);

	// Get xml data
	virtual CCharBuffer &GetBuffer() { return m_strBuff; }

	virtual BOOL SaveAsFile(LPCTSTR szFile);

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
