// StrPrintf.cpp: implementation of the CStrPrintf class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "safestr.h"
#include "StrPrintf.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStrPrintf::CStrPrintf(LPCTSTR szFormat, ...)
{
	m_nSize = 0;
	m_nCapacity = 256;
	m_pszBuffer = NULL;

	va_list		args;

	va_start(args, szFormat);
	vprintf(szFormat, args);
	va_end(args);
}

void CStrPrintf::printf(LPCTSTR szFormat, ...)
{
	va_list		args;

	va_start(args, szFormat);
	vprintf(szFormat, args);
	va_end(args);
}

void CStrPrintf::vprintf(LPCTSTR szFormat, va_list args)
{
	m_nSize = 0;

	if (!m_pszBuffer)
	{
		m_nCapacity = 256;
		m_pszBuffer = new TCHAR[m_nCapacity];
	}

	while (1)
	{
		m_nSize = _vsntprintf(m_pszBuffer, m_nCapacity, szFormat, args);

		// was there an error? was the expanded string too long?
		if ((long)m_nSize < 0)
		{
			if (m_nCapacity >= 100 * 1024)
				break;

			delete[] m_pszBuffer;
			m_pszBuffer = NULL;
			m_nCapacity *= 2;
			m_pszBuffer = new TCHAR[m_nCapacity];
		}
		else
			break;
	}

	if ((long)m_nSize < 0)
	{
		_tcscpy_safe(m_pszBuffer, m_nCapacity, _T("Insufficient buffer space."));
		m_nSize = _tcslen(m_pszBuffer);
	}
}
