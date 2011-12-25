#pragma once

DECLAR_CPPUNIT_TEST_REG(CCSVFile)

class CXCharSeparatedValues
{
public:
	CXCharSeparatedValues(TCHAR chSeparator);
	virtual ~CXCharSeparatedValues();

	void Split(LPCTSTR szStr, V_Str &vValues);

	void AddValue(LPCTSTR szValue);
	void AddValue(int nValue);

	void clear() { m_strWriting.clear(); }

	LPCTSTR c_str() { return m_strWriting.c_str(); }
	size_t size() { return m_strWriting.size(); }

protected:
	TCHAR			m_chSeparator;
	tstring			m_strWriting;

};

class CCommaSeparatedValues : public CXCharSeparatedValues
{
public:
	CCommaSeparatedValues() : CXCharSeparatedValues(',') { }

};

class CColonSeparatedValues : public CXCharSeparatedValues
{
public:
	CColonSeparatedValues() : CXCharSeparatedValues(':') { }

};
