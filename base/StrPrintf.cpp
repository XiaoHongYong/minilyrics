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

	va_list		args;

	va_start(args, szFormat);
	va_end(args);

	vprintf(szFormat, args);

}

void CStrPrintf::printf(LPCTSTR szFormat, ...)
{
	va_list		args;

	va_start(args, szFormat);
	va_end(args);

	vprintf(szFormat, args);
}

void CStrPrintf::vprintf(LPCTSTR szFormat, va_list args)
{
	m_nSize = 0;

#ifdef _WIN32
	__try
#endif
	{
		if (!m_szBuffer)
		{
			m_nCapacity = 256;
			m_szBuffer = new TCHAR[m_nCapacity];
		}

		while (1)
		{
			m_nSize = _vsntprintf_s(m_szBuffer, m_nCapacity, m_nCapacity, szFormat, args);

			// was there an error? was the expanded string too long?
			if (m_nSize < 0)
			{
				if (m_nCapacity >= 100 * 1024)
					break;

				delete[] m_szBuffer;
				m_nCapacity *= 2;
				m_szBuffer = new TCHAR[m_nCapacity];
			}
			else
				break;
		}

		if (m_nSize < 0)
		{
			_tcscpy_safe(m_szBuffer, m_nCapacity, _T("Insufficient buffer space."));
			m_nSize = _tcslen(m_szBuffer);
		}
	}
#ifdef _WIN32
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// 有异常发生！
		_tcscpy_safe(m_szBuffer, m_nCapacity, _T("CStrPrintf, Exception catched."));
		m_nSize = _tcslen(m_szBuffer);
	}
#endif
}
