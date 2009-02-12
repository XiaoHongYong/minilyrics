// HttpProtocol.cpp: implementation of the CHttpProtocol class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HttpProtocol.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define SZ_RETURN		"\r\n"

#define READ_BLOCK_SIZE		2048

LPCSTR	_szStdMonth[12] = 
{
	"jan", "feb", "mar", "apr", "may", "jun",
	"jul", "aug", "sep", "oct", "nov", "dec"
};

int GetMonthByStr(LPCSTR szMonth)
{
	for (int i = 0; i < 12; i++)
	{
		if (_strnicmp(_szStdMonth[i], szMonth, 3) == 0)
			return i + 1;
	}
	return -1;
}

LPCSTR	_szStdDayOfWeek[7] = 
{
	"sun", "mon", "tue", "wed", "thu", "fri", "sat"
};

int GetDayOfWeekByStr(LPCSTR szDayOfWeek)
{
	for (int i = 0; i < 7; i++)
	{
		if (_strnicmp(_szStdDayOfWeek[i], szDayOfWeek, 3) == 0)
			return i + 1;
	}
	return -1;
}

CHttpProtocol::CHttpProtocol()
{
	m_bHeadParsed = FALSE;
	m_httpVer = HTTP_V_1_1;
}

CHttpProtocol::~CHttpProtocol()
{

}

#ifdef WIN32
BOOL CHttpProtocol::HttpTimeStrToSysTime(LPCSTR szHttpTime, SYSTEMTIME &sysTime)
{
	SYSTEMTIME	SysTime;
	char		szDayOfWeek[64];
	char		szMonth[64];
	char		szGMT[64];

					// Mon, 04 Nov 2002 15:41:19 GMT
	if (8 != sscanf(szHttpTime, "%3s, %02d %3s %d %d:%d:%d %s",
				szDayOfWeek, &SysTime.wDay, szMonth, &SysTime.wYear,
				&SysTime.wHour, &SysTime.wMinute, &SysTime.wSecond, szGMT))
	{
		// GetSystemTime(&SysTime);
		return FALSE;
	}
	else
	{
		// 转换星期几？
		SysTime.wDayOfWeek = GetDayOfWeekByStr(szDayOfWeek);
		if (SysTime.wDayOfWeek == -1)
			SysTime.wDayOfWeek = 1;

		// 转换月
		SysTime.wMonth = GetMonthByStr(szMonth);
		if (SysTime.wMonth == -1)
			SysTime.wMonth = 1;
	}

	return TRUE;
}
#endif

int CHttpProtocol::ReadHead(BOOL bParseHead)
{
	int			nRead = 0;
	int			nTotalRead = 0;
	int			nLastPos = 0;
	int			i = 0;

	int			nHttpHeadEndTagLen = 4;

	while (1)
	{
		char	*szData;
		int		nRead;
		int		nRet;

		if (nLastPos > 4)
			nLastPos -= 4;

		szData = strnstr(m_buffHead.data() + nLastPos, m_buffHead.size() - nLastPos, "\r\n\r\n");
		if (!szData)
		{
			szData = strnstr(m_buffHead.data() + nLastPos, m_buffHead.size() - nLastPos, "\n\n");
			if (szData)
				nHttpHeadEndTagLen = 2;
		}
		if (szData)
		{
			m_nHeadEndPos = int(szData + nHttpHeadEndTagLen - m_buffHead.data());

			if (bParseHead)
				return ParseHead();
			return ERR_OK;
		}
		nLastPos = m_buffHead.size();

		m_buffHead.reserve(m_buffHead.size() + READ_BLOCK_SIZE);
		szData = m_buffHead.data() + m_buffHead.size();
		nRet = m_pNetFile->Read(szData, m_buffHead.capacity() - m_buffHead.size() - 4, &nRead);
		if (nRet == ERR_OK)
		{
			m_buffHead.resize(m_buffHead.size() + nRead);
			if (nRead == 0)
			{
				// 接收完成，但是没有找到HTTP头结束标志
				ERRLOG2(_T("Can't find the end pos of Http header. Buffer: %d, %S"), m_buffHead.size(), m_buffHead.c_str());
				return ERR_HTTP_HEAD_NOT_END;
			}
		}
		else
			return nRet;
		i++;
	}

	return ERR_FALSE;
}

void CHttpProtocol::SetCacheData(LPVOID lpData, int nLen)
{
	m_buffHead.clear();
	m_buffHead.append((const char *)lpData, nLen);
}

//
//int CHttpProtocol::ReadData(CCharBuffer *pBuff, int nLen)
//{
//	if (m_buffHead.size() > m_nHeadEndPos)
//		pBuff->Write(m_buffHead.GetBuffer() + m_nHeadEndPos, m_buffHead.size() - m_nHeadEndPos);
//
//	while (pBuff->GetLength() < m_dwContentLen)
//	{
//		char	*szData;
//		int		nRead;
//		int		nRet;
//		szData = pBuff->AllocMemory(pBuff->GetLength() + READ_BLOCK_SIZE) + pBuff->GetLength();
//		nRet = m_pNetFile->Read(szData, pBuff->GetCapaLength() - pBuff->GetLength() - 1, &nRead);
//		if (nRet != ERR_OK)
//		{
//			return nRet;
//		}
//		pBuff->SetLength(pBuff->GetLength() + nRead);
//	}
//
//	return ERR_OK;
//}

//////////////////////////////////////////////////////////////////////////
//
CHttpRequestProtocol::CHttpRequestProtocol()
{
	m_bUseProxy = FALSE;
	m_dwAvailProp = RP_BASE_GET;
	m_RequestType = RQ_GET;

	// EmptyStr(m_szUrl);

	m_nHostPort = __HTTP_PORT;
	EmptyStr(m_szHost);
	EmptyStr(m_szAccept);
	EmptyStr(m_szRefer);
	m_dwContentLen = 0;
	EmptyStr(m_szUserPwd);
	EmptyStr(m_szContentType);
	EmptyStr(m_szUserAgent);
	m_dwRangeBeg = 0;
	m_dwRangeEnd = 0;
	EmptyStr(m_szConnection);
	EmptyStr(m_szPragma);
	EmptyStr(m_szCacheControl);
}

CHttpRequestProtocol::~CHttpRequestProtocol()
{
}

void CHttpRequestProtocol::SetUrl(const char * szUrl)
{
	m_strUrl = szUrl;
}

void CHttpRequestProtocol::SetHost(const char * szHost, int nPort)
{
	m_dwAvailProp |= RP_HOST;
	strcpy_safe(m_szHost, CountOf(m_szHost), szHost);
	m_nHostPort = nPort;
}

void CHttpRequestProtocol::SetProxyBase64Pass(BOOL bEnableProxy, const char * szBase64ProxyUserPass)
{
	m_bUseProxy = bEnableProxy;
	if (bEnableProxy)
	{
		if (szBase64ProxyUserPass && !IsEmptyString(szBase64ProxyUserPass))
		{
			m_strBase64ProxyUserPass = szBase64ProxyUserPass;
			m_dwAvailProp |= RP_PROXY_AUTHORIZATION;
		}
		else
		{
			m_strBase64ProxyUserPass = "";
			m_dwAvailProp &= ~RP_PROXY_AUTHORIZATION;
		}
	}
}

//
//int CHttpRequestProtocol::ReadHead(BOOL bParseHead)
//{
//	int			nBufferLen = 1024;
//	int			nRead = 0;
//	int			nTotalRead = 0;
//	int			nRet;
//	char		*szBegRead;
//
//	while (1)
//	{
//		szBegRead = m_buffHead.AllocMemory(nBufferLen) + nRead;
//		nRet = m_pNetFile->Read(szBegRead, nBufferLen - nRead, &nRead);
//		if (nRet == ERR_OK)
//		{
//			m_buffHead.SetLength(m_buffHead.size() + nRead);
//			szBegRead[nRead] = '\0';
//			if (szBegRead - 4 <= m_buffHead.GetBuffer())
//				szBegRead = m_buffHead.GetBuffer();
//			else
//				szBegRead -= 4;
//			szBegRead = strstr(szBegRead, "\r\n\r\n");
//			if (szBegRead)
//			{
//				m_nHeadEndPos = int(szBegRead + 4 - m_buffHead.GetBuffer());
//
//				if (bParseHead)
//					return ParseHead();
//				return ERR_OK;
//			}
//			else
//			{
//				// 继续read
//				nBufferLen += 2048;
//			}
//		}
//		else
//			return nRet;
//	}
//
//	return ERR_FALSE;
//}

int CHttpRequestProtocol::WriteHead()
{
	int			nLen = 0;
	char		szUrlNew[512];
	char		szTemp[256];

	if (m_bUseProxy)
	{
		if (m_nHostPort != __HTTP_PORT)
			sprintf_s(szUrlNew, CountOf(szUrlNew), "http://%s:%d", m_szHost, m_nHostPort);
		else
			sprintf_s(szUrlNew, CountOf(szUrlNew), "http://%s", m_szHost);
		if (m_strUrl[0] != '/')
			strcat_safe(szUrlNew, CountOf(szUrlNew), "/");
		strcat_safe(szUrlNew, CountOf(szUrlNew), m_strUrl.c_str());
	}
	else
		strcpy_safe(szUrlNew, CountOf(szUrlNew), m_strUrl.c_str());

	// "GET %s HTTP/%s\r\n"
	if (m_RequestType == RQ_GET)
		m_buffHead = "GET ";
	else
		m_buffHead = "POST ";
	else
		ASSERT(0);
	m_buffHead += szUrlNew;
	m_buffHead += " HTTP/";
	m_buffHead += m_httpVer == HTTP_V_1_1 ? "1.1" : "1.0";
	m_buffHead += "\r\n";

	if (m_dwAvailProp & RP_HOST)
	{
		m_buffHead += "Host: ";
		m_buffHead += m_szHost;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_ACCEPT)
	{
		m_buffHead += "Accept: ";
		m_buffHead += m_szAccept;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_REFERER)
	{
		m_buffHead += "Referer: ";
		m_buffHead += m_szRefer;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_CONTENT_LENGTH)
	{
		sprintf(szTemp, "%d", m_dwContentLen);
		m_buffHead += "Content-Length: ";
		m_buffHead += szTemp;
		m_buffHead += "\r\n";
	}
//
//	if (m_dwAvailProp & RP_USER_PWD)
//	{
//		szBuff = m_buffHead.AllocMemory(nSize) + m_buffHead.size();
//		nLen = sprintf(szBuff, "Proxy-Authorization: %s", m_szUs);
//		m_buffHead.SetLength(m_buffHead.size() + nLen);
//		nSize += nLen;
//	}

	if (m_dwAvailProp & RP_USER_AGENT)
	{
		m_buffHead += "User-Agent: ";
		m_buffHead += m_szUserAgent;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_CONTENT_TYPE)
	{
		m_buffHead += "Content-Type: ";
		m_buffHead += m_szContentType;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_RANGE)
	{
		if (m_dwRangeEnd > m_dwRangeBeg)
			nLen = sprintf(szTemp, "Range: bytes=%d-%d\r\n", m_dwRangeBeg, m_dwRangeEnd);
		else if (m_dwRangeEnd == 0)
			nLen = sprintf(szTemp, "Range: bytes=%d-\r\n", m_dwRangeBeg);
		m_buffHead += szTemp;
	}

	if (m_dwAvailProp & RP_CONNECTION)
	{
		m_buffHead += "Connection: ";
		m_buffHead += m_szConnection;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_PRAGMA)
	{
		m_buffHead += "Pragma: ";
		m_buffHead += m_szPragma;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_CACHE_CONTROL)
	{
		m_buffHead += "Cache-Control: ";
		m_buffHead += m_szCacheControl;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_COOKIE)
	{
		m_buffHead += "Cookie: ";
		m_buffHead += m_strCookie.c_str();
		m_buffHead += "\r\n";
	}

	if ((m_dwAvailProp & RP_PROXY_AUTHORIZATION) && m_bUseProxy && !m_strBase64ProxyUserPass.empty())
	{
// 		m_buffHead += "Authorization: Basic ";
// 		m_buffHead += m_strBase64ProxyUserPass.c_str();
// 		m_buffHead += "\r\n";
		m_buffHead += "Proxy-Authorization: Basic ";
		m_buffHead += m_strBase64ProxyUserPass.c_str();
		m_buffHead += "\r\n";
	}

	m_buffHead += SZ_RETURN;

	// DBGLOGDUMPS(m_buffHead.c_str(), m_buffHead.size());

	return m_pNetFile->Write(m_buffHead.data(), m_buffHead.size(), NULL);
}

int CHttpRequestProtocol::ReadData(CCharBuffer *pBuff, int nLen)
{
	if (m_RequestType == RQ_POST)
	{
		if ((int)m_buffHead.size() > m_nHeadEndPos)
			pBuff->append(m_buffHead.data() + m_nHeadEndPos, m_buffHead.size() - m_nHeadEndPos);

		while (pBuff->size() < m_dwContentLen)
		{
			char	*szData;
			int		nRead;
			int		nRet;
			pBuff->reserve(pBuff->size() + READ_BLOCK_SIZE);
			szData = pBuff->data() + pBuff->size();
			nRet = m_pNetFile->Read(szData, pBuff->capacity() - pBuff->size() - 4, &nRead);
			if (nRet != ERR_OK)
			{
				return nRet;
			}
			if (nRead == 0)
			{
				if (m_dwContentLen == -1)
					return ERR_OK;
				else
					return ERR_HTTP_DATA_NOT_FINISHED;
			}
			pBuff->setSize(pBuff->size() + nRead);
		}
		if (pBuff->size() > m_dwContentLen)
			pBuff->setSize(m_dwContentLen);
	}
	else
		return ERR_HTTP_GET_NODATA;

	return ERR_OK;
}

int CHttpRequestProtocol::WriteData(LPVOID pData, int nLen)
{
//	DBGLOGDUMPS((char *)pData, nLen);
	return m_pNetFile->Write((const char *)pData, nLen, NULL);
}

int CHttpRequestProtocol::ParseHead()
{
	LPCSTR		szHead;
	LPCSTR		szPos;
	LPCSTR		szEnd;
	LPCSTR		szEnd2;
	szPos = szHead = m_buffHead.data();

	m_dwAvailProp = 0;

	if (_strnicmp(szHead, "GET", 3) == 0)
	{
		m_RequestType = RQ_GET;
		szPos += 4;
	}
	else if (_strnicmp(szHead, "POST", 4) == 0)
	{
		m_RequestType = RQ_POST;
		szPos += 5;
	}
	else
		return ERR_HTTP_BAD_REQUEST;

	//
	// /a.rar HTTP/1.1
	szEnd = strstr(szPos, SZ_RETURN);
	if (szEnd == NULL)
		return ERR_HTTP_BAD_FORMAT;
	szEnd2 = strchr(szPos, 0x20);
	if (szEnd2 == NULL || szEnd2 >= szEnd)
		return ERR_HTTP_BAD_FORMAT;
	// strncpysz_safe(m_szUrl, 512, szPos, (int)(szEnd2 - szPos));
	m_strUrl.erase();
	m_strUrl.append(szPos, szEnd2);

	szEnd2++;
	if (strncmp(szEnd2, "HTTP/", 5) != 0)
		return ERR_HTTP_BAD_HTTP_PRO_TYPE;
	szEnd2 += 5;

	if (strncmp(szEnd2, "1.1", 2) == 0)
		m_httpVer = HTTP_V_1_1;
	else if (strncmp(szEnd2, "1.0", 2) == 0)
		m_httpVer = HTTP_V_1_0;
	else
		m_httpVer = HTTP_V_UK;

	if (m_httpVer != HTTP_V_UK)
	{
		szEnd2+=3;
		if (strncmp(szEnd2, SZ_RETURN, 2) != 0)
			return ERR_HTTP_BAD_HTTP_PRO_TYPE;
	}

	//
	// 
	szPos = szEnd + 2;
	while (1)
	{
		if (strncmp(szPos, SZ_RETURN, 2) == 0)
			break;
		szEnd = strstr(szPos, SZ_RETURN);
		if (!szEnd)
			return ERR_HTTP_BAD_FORMAT;
		if (strncmp(szPos, "Host: ", 6) == 0)
		{
			szPos += 6;
			strncpysz_safe(m_szHost, CountOf(m_szHost), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_HOST;
		}
		else if (strncmp(szPos, "Accept: ", 8) == 0)
		{
			szPos += 8;
			strncpysz_safe(m_szAccept, CountOf(m_szAccept), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_ACCEPT;
		}
		else if (strncmp(szPos, "Referer: ", 9) == 0)
		{
			szPos += 9;
			strncpysz_safe(m_szRefer, CountOf(m_szRefer), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_REFERER;
		}
		else if (strncmp(szPos, "Range: ", 7) == 0)
		{
			//assert(0);
			szPos += 7;
			// strncpysz_safe(m_szHost, CountOf(m_szHost), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_RANGE;
		}
		else if (strncmp(szPos, "Content-Length: ", 16) == 0)
		{
			szPos += 16;
			m_dwContentLen = atoi(szPos);
			m_dwAvailProp |= RP_CONTENT_LENGTH;
		}
		else if (strncmp(szPos, "User-Agent: ", 12) == 0)
		{
			szPos += 12;
			strncpysz_safe(m_szUserAgent, CountOf(m_szUserAgent), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_USER_AGENT;
		}
		else if (strncmp(szPos, "Cache-Control: ", 15) == 0)
		{
			szPos += 15;
			strncpysz_safe(m_szCacheControl, CountOf(m_szCacheControl), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_CACHE_CONTROL;
		}
		else if (strncmp(szPos, "Connection: ", 12) == 0)
		{
			szPos += 12;
			strncpysz_safe(m_szConnection, CountOf(m_szConnection), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_CONNECTION;
		}
		else if (strncmp(szPos, "Cookie: ", 8) == 0)
		{
			szPos += 8;
			m_strCookie = "";
			m_strCookie.append(szPos, szEnd);
			// strncpysz_safe(m_szCookie, CountOf(m_szCookie), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_COOKIE;
		}
		else
		{
			// char		szName[128];
			// strncpysz_safe(szName, CountOf(szName), szPos, int(szEnd - szPos));
			// ERRLOG1("unknown http request property: %s", szName);
		}
		szPos = szEnd + 2;
	}

	return ERR_OK;
}

void CHttpRequestProtocol::SetAvailProp(DWORD dwProp)
{
	m_dwAvailProp = dwProp;
}

CHttpRequestProtocol::REQUEST_TYPE CHttpRequestProtocol::GetRequestType()
{
	return m_RequestType;
}

void CHttpRequestProtocol::SetRequestType(REQUEST_TYPE rqType)
{
	m_RequestType = rqType;
}

int CHttpRequestProtocol::GetContentLength(long &nLength)
{
	return m_dwContentLen;
}

void CHttpRequestProtocol::SetContentLength(long nLength)
{
	m_dwContentLen = nLength;
}

void CHttpRequestProtocol::SetRange(DWORD dwStart, DWORD dwEnd)
{
	m_dwRangeBeg = dwStart;
	m_dwRangeEnd = dwEnd;
}

LPCSTR CHttpRequestProtocol::GetPropValue(REQUEST_PROPERTY requestProperty)
{
	switch (requestProperty)
	{
	case RP_HOST:
		return m_szHost;
		break;
	case RP_ACCEPT:
		return m_szAccept;
		break;
	case RP_REFERER:
		return m_szRefer;
		break;
//	case RP_CONTENT_LENGTH:
//		m_dwContentLen = atoi(szValue);
//		break;
	case RP_USER_PWD:
		return m_szUserPwd;
		break;
	case RP_CONTENT_TYPE:
		return m_szContentType;
		break;
	case RP_USER_AGENT:
		return m_szUserAgent;
		break;
//	case RP_RANGE:
//		break;
	case RP_CONNECTION:
		return m_szConnection;
		break;
	case RP_PRAGMA:
		return m_szPragma;
		break;
	case RP_CACHE_CONTROL:
		return m_szCacheControl;
		break;
	case RP_COOKIE:
		return m_strCookie.c_str();
		break;
	default:
		DBGLOG1(_T("CHttpRequestProtocol::GetPropValue() does not support get the property: %d"), requestProperty);
		break;
	}

	return NULL;
}

void CHttpRequestProtocol::SetPropValue(REQUEST_PROPERTY requestProperty, const char * szValue)
{
	switch (requestProperty)
	{
	case RP_HOST:
		strcpy_safe(m_szHost, CountOf(m_szHost), szValue);
		break;
	case RP_ACCEPT:
		strcpy_safe(m_szAccept, CountOf(m_szAccept), szValue);
		break;
	case RP_REFERER:
		strcpy_safe(m_szRefer, CountOf(m_szRefer), szValue);
		break;
	case RP_CONTENT_LENGTH:
		m_dwContentLen = atoi(szValue);
		break;
	case RP_USER_PWD:
		strcpy_safe(m_szUserPwd, CountOf(m_szUserPwd), szValue);
		break;
	case RP_CONTENT_TYPE:
		strcpy_safe(m_szContentType, CountOf(m_szContentType), szValue);
		break;
	case RP_USER_AGENT:
		strcpy_safe(m_szUserAgent, CountOf(m_szUserAgent), szValue);
		break;
//	case RP_RANGE:
//		break;
	case RP_CONNECTION:
		strcpy_safe(m_szConnection, CountOf(m_szConnection), szValue);
		break;
	case RP_PRAGMA:
		strcpy_safe(m_szPragma, CountOf(m_szPragma), szValue);
		break;
	case RP_CACHE_CONTROL:
		strcpy_safe(m_szCacheControl, CountOf(m_szCacheControl), szValue);
		break;
	case RP_COOKIE:
		m_strCookie = szValue;
		// strcpy_safe(m_szCookie, CountOf(m_szCookie), szValue);
		break;
	case RP_PROXY_AUTHORIZATION:
		m_strBase64ProxyUserPass = szValue;
		break;
	default:
		DBGLOG2(_T("CHttpRequestProtocol::SetPropValue() does not support set the property: %d, %s"), requestProperty, szValue);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
//
CHttpReturnProtocol::CHttpReturnProtocol()
{
	m_dwAvailProp = RP_BASE_RET;

	m_dwResultCode = 200;
	EmptyStr(m_szResult);

	EmptyStr(m_szServer);
	EmptyStr(m_szConnection);
	EmptyStr(m_szDate);
	EmptyStr(m_szContentType);
	EmptyStr(m_szLastModified);
	EmptyStr(m_szEtag);
	m_dwContentLen = (DWORD)-1;
	m_dwRangeBeg = 0;
	m_dwRangeEnd = 0;
	// EmptyStr(m_szCookie);
	EmptyStr(m_szLocation);
	EmptyStr(m_szCacheControl);
	EmptyStr(m_szContentFileName);		// Content-Disposition: attachment;filename=l黎明 - That's Life.lrc
	m_bTransfEncChunked = FALSE;
}

CHttpReturnProtocol::~CHttpReturnProtocol()
{
}

int CHttpReturnProtocol::WriteHead()
{
	char		szTemp[256];

	m_buffHead.clear();

	if (m_httpVer == HTTP_V_1_1)
		m_buffHead += "HTTP/1.1 200 OK\r\n";
	else
		m_buffHead += "HTTP/1.0 200 OK\r\n";

	if (m_dwAvailProp & RP_SERVER)
	{
		m_buffHead += "Server: ";
		m_buffHead += m_szServer;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_DATE)
	{
		m_buffHead += "Date: ";
		m_buffHead += m_szDate;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_LOCATION)
	{
		m_buffHead += "Location: ";
		m_buffHead += m_szLocation;
		m_buffHead += "\r\n";
	}
//
//	if (m_dwAvailProp & RP_USER_PWD)
//	{
//		szBuff = m_buffHead.AllocMemory(nSize) + m_buffHead.size();
//		nLen = sprintf(szBuff, "Proxy-Authorization: %s", m_szUs);
//		m_buffHead.SetLength(m_buffHead.size() + nLen);
//		nSize += nLen;
//	}

	if (m_dwAvailProp & RP_CONTENT_LENGTH)
	{
		sprintf(szTemp, "%d", m_dwContentLen);
		m_buffHead += "Content-Length: ";
		m_buffHead += szTemp;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_CONTENT_TYPE)
	{
		m_buffHead += "Content-Type: ";
		m_buffHead += m_szContentType;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_CONTENT_RANGE)
	{
		if (m_dwRangeEnd > m_dwRangeBeg)
			sprintf(szTemp, "Range: bytes=%d-%d\r\n", m_dwRangeBeg, m_dwRangeEnd);
		else if (m_dwRangeEnd == 0)
			sprintf(szTemp, "Range: bytes=%d-\r\n", m_dwRangeBeg);
		m_buffHead += szTemp;
	}

	if (m_dwAvailProp & RP_LAST_MODIFIED)
	{
		m_buffHead += "Last-Modified: ";
		m_buffHead += m_szLastModified;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_ETAG)
	{
		m_buffHead += "ETag: ";
		m_buffHead += m_szEtag;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_SET_COOKIE)
	{
		m_buffHead += "Set-Cookie: ";
		m_buffHead += m_strCookie.c_str();
		m_buffHead += "\r\n";
//		m_buffHead += "Set-Cookie: ";
//		m_buffHead += "username=10";
//		m_buffHead += "\r\n";
//		m_buffHead += "Set-Cookie: ";
//		m_buffHead += "Value1=va2";
//		m_buffHead += "\r\n";
//		m_buffHead += "Set-Cookie: ";
//		m_buffHead += "Value3=va352";
//		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_CONTENT_DISP_FILE)
	{
		m_buffHead += "Content-Disposition: attachment;filename=: ";
		m_buffHead += m_szContentFileName;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_CACHE_CONTROL)
	{
		m_buffHead += "Cache-control: ";
		m_buffHead += m_szCacheControl;
		m_buffHead += "\r\n";
	}

	if (m_dwAvailProp & RP_CONNECTION)
	{
		m_buffHead += "Connection: ";
		m_buffHead += m_szConnection;
		m_buffHead += "\r\n";
//		m_buffHead += "X-Powered-By: ";
//		m_buffHead += "PHP";
//		m_buffHead += "\r\n";
	}

	m_buffHead += "\r\n";

	// DBGLOGDUMPS(m_buffHead.c_str(), m_buffHead.size());
//	DBGLOGDUMPS(m_buffHead.GetBuffer(), m_buffHead.size());

	return m_pNetFile->Write(m_buffHead.data(), m_buffHead.size(), NULL);
}

//
// nSize = 0 结束
// nSize = -1 需要读取更多的数据
// nSize = -2 数据块错误
LPSTR ReadTrunkSize(LPSTR szBuffer, int &nCount, int &nSize)
{
	LPSTR		szPos = szBuffer;
	LPSTR		szEnd = szBuffer + nCount;

	nSize = 0;

	if (szPos + 1 >= szEnd)
		goto BUFFER_TOO_SMALL;
	if (*szPos == '\r' && *(szPos + 1) == '\n')
	{
		szPos += 2;
		if (szPos + 1 >= szEnd)
			goto BUFFER_TOO_SMALL;
		if (*szPos == '\r' && *(szPos + 1) == '\n')
		{
			szPos += 2;
			nCount -= 2;
		}
	}

	if (szPos + 3 >= szEnd)
		goto BUFFER_TOO_SMALL;
	while (*szPos != ' ' && *szPos != '\r' && *szPos != '\n')
	{
		nSize *= 0x10;
		if (isdigit(*szPos))
		{
			nSize += *szPos - '0';
		}
		else if (*szPos >= 'a' && *szPos <= 'f')
			nSize += (*szPos - 'a' + 10);
		else if (*szPos >= 'A' && *szPos <= 'F')
			nSize += (*szPos - 'A' + 10);
		else if (*szPos == 0x20)
		{
			szPos++;
			while (szPos < szEnd && *szPos == 0x20)
				szPos++;
			break;
		}
		else
		{
			// 无效
			nSize = -2;
			return szBuffer;
		}
		szPos++;
		if (szPos >= szEnd)
			goto BUFFER_TOO_SMALL;
	}
	while (*szPos == ' ')
		szPos++;

	if (szPos + 2 >= szEnd)
		goto BUFFER_TOO_SMALL;

	if (*szPos == '\r' && *(szPos + 1) == '\n')
		szPos += 2;
	else
	{
		nSize = -2;
		return szBuffer;
	}

	nCount -= int(szPos - szBuffer);
	ASSERT(nCount >= 0);
	return szPos;

BUFFER_TOO_SMALL:
	nSize = -1;
	return szBuffer;
}

int CHttpReturnProtocol::ReadData(CCharBuffer *pBuff, int nLen)
{
	if (m_bTransfEncChunked)
	{
		DWORD		dwTrunk = 0;

		//
		// 分析m_buffHead 中部分数据
		LPSTR		szData;
		int			nCount = 0;
		int			nTrunkSize = 0;
		char		szBuffer[2048];
		if ((int)m_buffHead.size() > m_nHeadEndPos)
		{
			szData = m_buffHead.data() + m_nHeadEndPos;
			nCount = m_buffHead.size() - m_nHeadEndPos;
			while (nCount > 3)
			{
				szData = ReadTrunkSize(szData, nCount, nTrunkSize);
				if (nTrunkSize == -2)
				{
					// Invalid trunck size!
					DBGLOG1(_T("Invalid Trunk size: %s"), szData - 5);
					return ERR_HTTP_INVLIAD_TRUNK;
				}
				else if (nTrunkSize == -1)
				{
					nTrunkSize = 0;
					break;
				}
				else if (nTrunkSize == 0)
					return ERR_OK;
				if (nTrunkSize <= nCount)
				{
					pBuff->append(szData, nTrunkSize);
					szData += nTrunkSize;
					nCount -= nTrunkSize;
					nTrunkSize = 0;
				}
				else
				{
					pBuff->append(szData, nCount);
					szData += nCount;
					nTrunkSize -= nCount;
					nCount = 0;
					break;
				}
			}
			if (nCount > 0)
			{
				memcpy(szBuffer, szData, nCount);
			}
		}

		//
		// 接收并分析数据
		while (pBuff->size() < m_dwContentLen)
		{
			char	*szData;
			int		nRet;
			int		nRead;
			szData = szBuffer;

			nRet = m_pNetFile->Read(szBuffer + nCount, CountOf(szBuffer) - nCount, &nRead);
			if (nRet != ERR_OK)
			{
				return nRet;
			}
			if (nRead == 0)
			{
				if (m_dwContentLen == -1 && nCount == 0)
					return ERR_OK;
				else
					return ERR_HTTP_DATA_NOT_FINISHED;
			}
			nCount += nRead;
			do
			{
				if (nTrunkSize == 0)
				{
					szData = ReadTrunkSize(szData, nCount, nTrunkSize);
					if (nTrunkSize == -2)
					{
						// Invalid trunck size!
						DBGLOG1(_T("Invalid Trunk size: %s"), szData);
						return ERR_HTTP_INVLIAD_TRUNK;
					}
					else if (nTrunkSize == -1)
					{
						nTrunkSize = 0;
						break;
					}
					else if (nTrunkSize == 0)
						return ERR_OK;
				}
				if (nTrunkSize <= nCount)
				{
					pBuff->append(szData, nTrunkSize);
					szData += nTrunkSize;
					nCount -= nTrunkSize;
					nTrunkSize = 0;
				}
				else
				{
					pBuff->append(szData, nCount);
					szData += nCount;
					nTrunkSize -= nCount;
					nCount = 0;
					break;
				}
			}
			while (nCount > 3);

			if (nCount > 0)
				memmove(szBuffer, szData, nCount);
		}
	}
	else
	{
		if ((int)m_buffHead.size() > m_nHeadEndPos)
			pBuff->append(m_buffHead.data() + m_nHeadEndPos, m_buffHead.size() - m_nHeadEndPos);

		while (pBuff->size() < m_dwContentLen)
		{
			char	*szData;
			int		nRead;
			int		nRet;

			pBuff->reserve(pBuff->size() + READ_BLOCK_SIZE);
			szData = pBuff->data() + pBuff->size();
			nRet = m_pNetFile->Read(szData, pBuff->capacity() - pBuff->size() - 4, &nRead);
			if (nRet != ERR_OK)
			{
				return nRet;
			}
			if (nRead == 0)
			{
				if (m_dwContentLen == -1)
					return ERR_OK;
				else
					return ERR_HTTP_DATA_NOT_FINISHED;
			}
			pBuff->setSize(pBuff->size() + nRead);
		}
	}

	if (pBuff->size() > m_dwContentLen)
		pBuff->setSize(m_dwContentLen);

	return ERR_OK;
}

int CHttpReturnProtocol::WriteData(LPVOID pData, int nLen)
{
	// DBGLOGDUMPS((char *)pData, nLen);
	return m_pNetFile->Write((const char *)pData, nLen, NULL);
}

#define SZ_CONTENT_DISP_FILENAME	"Content-Disposition: attachment;filename="
#define SZ_TRANSF_ENC_CHUNKED		"Transfer-Encoding: chunked"

int CHttpReturnProtocol::ParseHead()
{
	LPCSTR		szHead;
	LPCSTR		szPos;
	LPCSTR		szEnd;
	szPos = szHead = m_buffHead.data();

	// DBGLOGDUMPS(m_buffHead.c_str(), m_buffHead.size());
	m_dwAvailProp = 0;

	// HTTP/1.1 200 OK
	if (strncmp(szHead, "HTTP/", 5) == 0)
	{
		szPos += 5;
		if (strncmp(szPos, "1.1", 3) == 0)
			m_httpVer = HTTP_V_1_1;
		else if (strncmp(szPos, "1.0", 3) == 0)
			m_httpVer = HTTP_V_1_0;
		else
			m_httpVer = HTTP_V_UK;
		szPos = strchr(szPos, 0x20);
		if (szPos == NULL)
			return ERR_HTTP_BAD_FORMAT;
		szPos++;

		m_dwResultCode = atoi(szPos);
		for (int i = 0; i < 3; i++)
		{
			if (!isdigit(*szPos))
				return ERR_HTTP_BAD_FORMAT;
			szPos++;
		}

		szPos++;
		szEnd = strstr(szPos, SZ_RETURN);
		if (szEnd == NULL)
			return ERR_HTTP_BAD_FORMAT;

		strncpysz_safe(m_szResult, CountOf(m_szResult), szPos, int(szEnd - szPos));
	}
	else
		return ERR_HTTP_BAD_HTTP_PRO_TYPE;

	//
	// 
	szPos = szEnd + 2;
	while (1)
	{
		if (strncmp(szPos, SZ_RETURN, 2) == 0 || *szPos == '\n')
			break;
		szEnd = strstr(szPos, SZ_RETURN);
		if (!szEnd)
			szEnd = strchr(szPos, '\n');
		if (!szEnd)
			return ERR_HTTP_BAD_FORMAT;
		if (strncmp(szPos, "Server: ", 8) == 0)
		{
			szPos += 8;
			strncpysz_safe(m_szServer, CountOf(m_szServer), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_SERVER;
		}
		else if (strncmp(szPos, "Connection: ", 12) == 0)
		{
			szPos += 12;
			strncpysz_safe(m_szConnection, CountOf(m_szConnection), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_CONNECTION;
		}
		else if (strncmp(szPos, "Date: ", 6) == 0)
		{
			szPos += 6;
			strncpysz_safe(m_szDate, CountOf(m_szDate), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_DATE;
		}
		else if (strncmp(szPos, "Location: ", 10) == 0)
		{
			szPos += 10;
			strncpysz_safe(m_szLocation, CountOf(m_szLocation), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_LOCATION;
		}
		else if (strncmp(szPos, "Content-Type: ", 14) == 0)
		{
			szPos += 14;
			strncpysz_safe(m_szContentType, CountOf(m_szContentType), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_CONTENT_TYPE;
		}
		else if (strncmp(szPos, "Last-Modified: ", 15) == 0)
		{
			szPos += 15;
			strncpysz_safe(m_szLastModified, CountOf(m_szLastModified), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_LAST_MODIFIED;
		}
		else if (strncmp(szPos, "ETag: ", 6) == 0)
		{
			szPos += 6;
			strncpysz_safe(m_szEtag, CountOf(m_szEtag), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_ETAG;
		}
		else if (strncmp(szPos, "Content-Length: ", 16) == 0)
		{
			szPos += 16;
			m_dwContentLen = (DWORD)atoi(szPos);
			m_dwAvailProp |= RP_CONNECTION;
		}
		else if (strncmp(szPos, "Content-Range: ", 15) == 0)
		{
			szPos += 15;
			// strncpysz_safe(m_szConnection, CountOf(m_szContentRange), szPos, size_t(szEnd - szPos));
			ERRLOG0(_T("Content-Range: Not parsed"));
			m_dwAvailProp |= RP_CONNECTION;
		}
		else if (strncmp(szPos, "Set-Cookie: ", 12) == 0)
		{
			szPos += 12;
			m_strCookie = "";
			m_strCookie.append(szPos, szEnd);
			m_dwAvailProp |= RP_SET_COOKIE;
		}
		else if (strncmp(szPos, "Cache-control: ", 15) == 0)
		{
			szPos += 15;
			strncpysz_safe(m_szCacheControl, CountOf(m_szCacheControl), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_CACHE_CONTROL;
		}
		else if (strncmp(szPos, SZ_TRANSF_ENC_CHUNKED, strlen(SZ_TRANSF_ENC_CHUNKED)) == 0)
		{
			szPos += strlen(SZ_TRANSF_ENC_CHUNKED);
			m_bTransfEncChunked = TRUE;
			m_dwAvailProp |= RP_CACHE_CONTROL;
		}
		else if (strncmp(szPos, SZ_CONTENT_DISP_FILENAME, strlen(SZ_CONTENT_DISP_FILENAME)) == 0)
		{
			szPos += strlen(SZ_CONTENT_DISP_FILENAME);
			strncpysz_safe(m_szCacheControl, CountOf(m_szCacheControl), szPos, size_t(szEnd - szPos));
			m_dwAvailProp |= RP_CONTENT_DISP_FILE;
		}
		else
		{
			// char		szName[128];
			// strncpysz_safe(szName, CountOf(szName), szPos, int(szEnd - szPos));
			// ERRLOG1("unknown http return property: %s", szName);
		}
		szPos = szEnd + 2;
	}

	return ERR_OK;
}

void CHttpReturnProtocol::SetAvailProp(DWORD dwProp)
{
	m_dwAvailProp = dwProp;
}

int CHttpReturnProtocol::GetContentLength(long &nLength)
{
	return ERR_OK;
}

void CHttpReturnProtocol::SetContentLength(long nLength)
{
	m_dwContentLen = nLength;
	m_dwAvailProp |= RP_CONTENT_LENGTH;
}

void CHttpReturnProtocol::SetRange(DWORD dwStart, DWORD dwEnd)
{
}

void CHttpReturnProtocol::SetServer(const char * szServer)
{
	strcpy_safe(m_szServer, CountOf(m_szServer), szServer);
}

void CHttpReturnProtocol::SetConnection(const char * szConnection)
{
	strcpy_safe(m_szConnection, CountOf(m_szConnection), szConnection);
}

const char * CHttpReturnProtocol::GetPropValue(RET_PROPERTY retProperty)
{
	switch (retProperty)
	{
	case RP_SERVER:
		return m_szServer;
		break;
	case RP_CONNECTION:
		return m_szConnection;
		break;
	case RP_DATE:
		return m_szDate;
		break;
//	case RP_CONTENT_LENGTH:
//		m_dwContentLen = atoi(szValue);
//		break;
	case RP_LOCATION:
		return m_szLocation;
		break;
	case RP_CONTENT_TYPE:
		return m_szContentType;
		break;
//	case RP_CONTENT_RANGE:
//		return m_szUserAgent;
//		break;
	case RP_LAST_MODIFIED:
		return m_szLastModified;
		break;
	case RP_ETAG:
		return m_szEtag;
		break;
	case RP_SET_COOKIE:
		return m_strCookie.c_str();
		break;
	case RP_CACHE_CONTROL:
		return m_szCacheControl;
		break;
//	case RP_CONTENT_DISPOSITION:
//		return m_szLastModified;
//		break;
	default:
		DBGLOG1(_T("CHttpRequestProtocol::GetPropValue() does not support set the property: %d"), retProperty);
		break;
	}

	return NULL;
}

void CHttpReturnProtocol::SetPropValue(RET_PROPERTY retProperty, const char * szValue)
{
}

const char * CHttpReturnProtocol::GetResult(int &nCode)
{
	nCode = m_dwResultCode;
	return m_szResult;
}

int CHttpReturnProtocol::ResultCodeToErrorCode(int nResultCode)
{
	if (nResultCode >= 200 && nResultCode < 300)
		return ERR_OK;
	else if (nResultCode == 400)
		return ERR_HTTP_400;
	else if (nResultCode == 401)
		return ERR_HTTP_401;
	else if (nResultCode == 403)
		return ERR_HTTP_403;
	else if (nResultCode == 404)
		return ERR_HTTP_404;
	else if (nResultCode == 407)
		return ERR_HTTP_407;
	else if (nResultCode == 302)
		return ERR_HTTP_302;
	else if (nResultCode == 500)
		return ERR_HTTP_500;
	else
		return ERR_HTTP_CODE_ERROR;
}
