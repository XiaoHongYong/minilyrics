#include "stdafx.h"
#include "base.h"

OSError::~OSError()
{
	if (m_szErrMsg)
		::LocalFree(m_szErrMsg);
}

void OSError::doFormatMessage(unsigned int dwLastErr)
{
	m_dwErrCode = dwLastErr;
	if (m_szErrMsg)
	{
		::LocalFree(m_szErrMsg);
		m_szErrMsg = NULL;
	}
	::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dwLastErr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT ),
		(LPTSTR)&m_szErrMsg, 
		1,
		NULL );
}

uint32 OSError::GetLastError()
{
	return ::GetLastError();
}
