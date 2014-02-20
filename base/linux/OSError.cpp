#include "stdafx.h"
#include "../base.h"

OSError::~OSError()
{
}

void OSError::doFormatMessage(unsigned int dwLastErr)
{
	m_dwErrCode = dwLastErr;
	m_szErrMsg = strerror((int)dwLastErr);
}

uint32 OSError::GetLastError()
{
	return errno;
}
