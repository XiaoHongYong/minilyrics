#pragma once

DECLAR_CPPUNIT_TEST_REG(CCSVFile)

class CXCharSeparatedValues
{
public:
	CXCharSeparatedValues(TCHAR chSeparator);
	virtual ~CXCharSeparatedValues();

	void Split(LPCTSTR szStr, VecString &vValues);
	void Split(LPCTSTR szStr, MapString &mapNameValues, TCHAR chAssign = '=');

	void AddValue(LPCTSTR szValue);
	void AddValue(int nValue);

	void AddNameValue(LPCTSTR szName, LPCTSTR szValue, TCHAR chAssign = '=');

	void clear() { m_strWriting.clear(); }

	LPCTSTR c_str() const { return m_strWriting.c_str(); }
	size_t size() const { return m_strWriting.size(); }

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
