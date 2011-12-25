#include "stdafx.h"
#include "XCharSeparatedValues.h"
#include "App.h"

#define CHAR_ESCAPE		'\\'

CXCharSeparatedValues::CXCharSeparatedValues(TCHAR chSeparator)
{
	m_chSeparator = chSeparator;
}

CXCharSeparatedValues::~CXCharSeparatedValues()
{
}

void CXCharSeparatedValues::Split(LPCTSTR szStr, V_Str &vValues)
{
	const TCHAR *p = szStr;

	tstring		strValue;

	// Read until m_chSeparator or end of line.
	while (*p)
	{
		if (*p == CHAR_ESCAPE)
			p++;
		else if (*p == m_chSeparator)
		{
			vValues.push_back(strValue);
			strValue.clear();
			p++;
			continue;
		}

		strValue += *p;
		p++;
	}

	vValues.push_back(strValue);
}

void CXCharSeparatedValues::AddValue(LPCTSTR szValue)
{
	if (m_strWriting.size() > 0)
		m_strWriting += m_chSeparator;

	LPCTSTR szBegin = szValue;
	LPCTSTR p = szBegin;
	while (*p)
	{
		if (*p == m_chSeparator || *p == CHAR_ESCAPE)
		{
			m_strWriting.append(szBegin, (size_t)(p - szBegin));
			m_strWriting += CHAR_ESCAPE;
			m_strWriting += *p;
			szBegin = p + 1;
		}
		p++;
	}

	m_strWriting.append(szBegin);
}

void CXCharSeparatedValues::AddValue(int nValue)
{
	if (m_strWriting.size() > 0)
		m_strWriting += m_chSeparator;

	TCHAR		szBuff[32];

	_itot_s(nValue, szBuff, CountOf(szBuff), 10);
	m_strWriting += szBuff;
}

#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

IMPLEMENT_CPPUNIT_TEST_REG(CCSVFile)


class CTestCaseCVSFile : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseCVSFile);
	CPPUNIT_TEST(TestCSV);
	CPPUNIT_TEST_SUITE_END();

protected:
	void TestCSV()
	{
		CColonSeparatedValues	csv;
		CColonSeparatedValues	csv2;

		{
			V_Str vValues;
			LPCTSTR szText = _T("1:\\:2:33:");

			csv.Split(szText, vValues);
			TrimStr(vValues);

			LPCTSTR szValue[] = { _T("1"), _T(":2"), _T("33"), _T(""), };
			CPPUNIT_ASSERT(vValues.size() == CountOf(szValue));
			for (int i = 0; i < CountOf(szValue); i++)
			{
				CPPUNIT_ASSERT(vValues[i] == szValue[i]);
				csv2.AddValue(szValue[i]);
			}

			CPPUNIT_ASSERT(_tcscmp(csv2.c_str(), szText) == 0);
		}

		{
			V_Str vValues;
			LPCTSTR szText = _T("");

			csv2.clear();

			csv.Split(szText, vValues);
			TrimStr(vValues);

			LPCTSTR szValue[] = { _T("") };
			CPPUNIT_ASSERT(vValues.size() == CountOf(szValue));
			for (int i = 0; i < CountOf(szValue); i++)
			{
				CPPUNIT_ASSERT(vValues[i] == szValue[i]);
				csv2.AddValue(szValue[i]);
			}

			CPPUNIT_ASSERT(_tcscmp(csv2.c_str(), szText) == 0);
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCVSFile);

#endif // _CPPUNIT_TEST


