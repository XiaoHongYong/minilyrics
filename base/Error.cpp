// Error.cpp: implementation of the CError class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "safestr.h"
#include "Error.h"
#include "idstring.h"

static TCHAR g_szCustomErrorDesc[512];

RegErr2Str							*g_err2StrHeader = NULL;

static ID_TO_STRING	__ErrID2Str[] = 
{
	{ ERR_FALSE, _T("Failed.") },
	{ ERR_OK, _T("Succeed.") },
	{ ERR_INITDB_FAILED, _T("Init Database FAILED.") },
	{ ERR_NO_MEM, _T("Can't allocate more memory.") },
	{ ERR_OUTOF_BUFF, _T("Insufficient buffer.") },
	{ ERR_NOT_IMPLEMENTED, _T("Not implemented.") },
	{ ERR_EOF, _T("Reached the end of the file.") },
	{ ERR_BAD_MSG, _T("Corrupted message.") },
	{ ERR_PARSE_XML, _T("Parse xml FAILED.") },

	{ ERR_FILE_NOT_EXIST, _T("File does not exist.") },

	{ ERR_EXIST, _T("Exist") },
	{ ERR_NOT_FOUND, _T("Not found.") },

	{ ERR_NET_HOST_NOT_FOUND, _T("Ineternet connection may be down, no such host is known.") },
	{ ERR_NOT_RECIVED_ALL, _T("The data packet didn't receive the rest data.") },

	{ ERR_NOT_SUPPORT_FILE_FORMAT, _T("The file may not be supported.") },
	{ ERR_NOLYRICS_OPENED, _T("No lyrics file was opened.") },

	{ ERR_OPEN_FILE, _T("Open file FAILED.")},
	{ ERR_READ_FILE, _T("Read file FAILED.")},
	{ ERR_WRITE_FILE, _T("Write file FAILED.") },
	{ ERR_SEEK_FILE, _T("Seek file FAILED.") },
	{ ERR_NOT_FIND_LRC3V2, _T("Not found Lyrics3v2 tag") },
	{ ERR_BAD_LRC3V2_FORMAT, _T("Invalid Lyrics3v2 format.") },
	{ ERR_BAD_FILE_FORMAT, _T("Invalid file format.") },

	{ ERR_DISABLE_PACK_XML, _T("XML packet was disabled, please upgrade to the latest version.") },
	{ ERR_PACK_BAD_MD5, _T("Invalid Signature, please contact us for more info.") },
	{ ERR_INVALID_PACK_VER, _T("Invalid package version, please contact us for more info.") },

	{ ERR_NOT_SUPPORT_ENCODING, _T("Not supported character encoding, please contact us for more info.") },

	{ ERR_NOT_FOUND_ID3V2, _T("Not found id3v2 tag.") },
	{ ERR_NOT_SUPPORT_ID3V2_VER, _T("Not supported id3v2 tag version.") },
	{ ERR_INVALID_ID3V2_FRAME, _T("Invalid id3v2 tag frame.") },

	{ ERR_HTTP_BAD_REQUEST, _T("Invalid HTTP request.") },		// 无效的HTTP请求
	{ ERR_HTTP_BAD_FORMAT, _T("Invalid Http format.") },		// 无效的HTTP格式
	{ ERR_HTTP_BAD_HTTP_PRO_TYPE, _T("Unrecognized HTTP protocol type.") },		// 无法识别的HTTP协议类型
	{ ERR_HTTP_INVLIAD_TRUNK, _T("Invalid HTTP trunck size.") },		// 解析HTTP return 数据失败，无法识别的trunck大小
	{ ERR_HTTP_BAD_URL, _T("Invalid HTTP URL format.") },		// 无效的HTTP URL格式
	{ ERR_HTTP_CODE_ERROR, _T("NOT dealed HTTP return code.") },		// 未处理的HTTP return 代码
	{ ERR_HTTP_HEAD_NOT_END, _T("Can't read the whole HTTP head.") },		// 无法完整的读取HTTP头
	{ ERR_HTTP_DATA_NOT_FINISHED, _T("Can't read all the data of the HTTP message.") },		// 无法完全读取HTTP消息的数据段
	{ ERR_HTTP_302, _T("HTTP status code: 302, Redirect the request.") },		// HTTP状态代码：302，重定向请求
	{ ERR_HTTP_400, _T("HTTP status code: 400, The request could not be understood by the server due to malformed syntax.") },		// HTTP状态代码：400，服务器无法识别的请求
	{ ERR_HTTP_401, _T("HTTP status code: 401, The request requires user authentication.") },		// HTTP状态代码：401，此请求需要用户名和密码验证
	{ ERR_HTTP_403, _T("HTTP status code: 403, Forbidden.") },		// HTTP状态代码：403，访问被禁止
	{ ERR_HTTP_404, _T("HTTP status code: 404, The server has not found anything matching the Request-URI.") },		// HTTP状态代码：404，请求的URI在服务器端不能找到
	{ ERR_HTTP_407, _T("HTTP status code: 407, Proxy Authentication Required.") },		// HTTP状态代码：407，使用代理时用户名和密码验证失败！
	{ ERR_HTTP_500, _T("HTTP status code: 500, Internal Server Error.") },		// HTTP状态代码：500，服务器内部错误

	{ 0, NULL}
};

static RegErr2Str	__addErr2Str(__ErrID2Str);

void SetCustomErrorDesc(LPCTSTR szErrorDesc)
{
	_tcscpy_safe(g_szCustomErrorDesc, CountOf(g_szCustomErrorDesc), szErrorDesc);
}

RegErr2Str::RegErr2Str(struct ID_TO_STRING err2Str[])
{
	m_err2Str = err2Str;
	m_pNext = NULL;

	if (!g_err2StrHeader)
		g_err2StrHeader = this;
	else
	{
		RegErr2Str		*p = g_err2StrHeader;
		while (p->m_pNext)
		{
			p = p->m_pNext;
		}

		p->m_pNext = this;
	}
}

RegErr2Str::~RegErr2Str()
{
	ASSERT(g_err2StrHeader != NULL);

	if (g_err2StrHeader)
	{
		if (g_err2StrHeader == this)
			g_err2StrHeader = m_pNext;
		else
		{
			RegErr2Str *p = g_err2StrHeader;
			while (p)
			{
				if (p->m_pNext == this)
				{
					p->m_pNext = m_pNext;
					return;
				}
				p = p->m_pNext;
			}
			ASSERT(0 && _T("Can't reach here"));
		}
	}
}

LPCTSTR Error2Str::c_str() const
{
	if (m_nError == ERR_CUSTOM_ERROR)
		return g_szCustomErrorDesc;

	if (m_nError >= ERR_MAX)
		return m_osError.Description();

	LPCTSTR							szError;
	RegErr2Str						*it;

	for (it = g_err2StrHeader; it != NULL; it = it->m_pNext)
	{
		ID_TO_STRING		*pid2str = it->m_err2Str;
		szError = IDToString(pid2str, m_nError, NULL);
		if (szError)
			return szError;
	}

#ifndef _WIN32
	if (m_nError >= ERR_C_ERRNO_BASE && m_nError - ERR_C_ERRNO_BASE <= 100)
	{
		// C error no, translate it.
		m_osError.doFormatMessage(m_nError - ERR_C_ERRNO_BASE);
		return m_osError.Description();
	}
#endif

	return _T("Not translated error code.");
}
