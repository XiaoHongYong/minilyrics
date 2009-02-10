// StrPrintf.h: interface for the CStrPrintf class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRPRINTF_H__71471822_3722_4AF3_9FF3_3E99A3EE8635__INCLUDED_)
#define AFX_STRPRINTF_H__71471822_3722_4AF3_9FF3_3E99A3EE8635__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CStrPrintf
{
public:
	CStrPrintf(LPCTSTR szFormat, ...);

	CStrPrintf()
	{
		m_nSize = 0;
		m_nCapacity = 0;
		m_szBuffer = NULL;
	}

	~CStrPrintf()
	{
		if (m_szBuffer)
			delete[] m_szBuffer;
	}

	void printf(LPCTSTR szFormat, ...);

	void vprintf(LPCTSTR szFormat, va_list args);

	LPCTSTR c_str() { if (m_szBuffer) return m_szBuffer; else return _T(""); }
	size_t size() { return m_nSize; }

protected:
	LPTSTR		m_szBuffer;
	int			m_nSize;
	int			m_nCapacity;

};

#endif // !defined(AFX_STRPRINTF_H__71471822_3722_4AF3_9FF3_3E99A3EE8635__INCLUDED_)
