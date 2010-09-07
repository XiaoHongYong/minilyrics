// Error.cpp: implementation of the CError class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "safestr.h"
#include "Error.h"
#include "idstring.h"

static TCHAR g_szCustomErrorDesc[512];

// typedef list<ID_TO_STRING*>		LIST_ID_TO_STR;
// LIST_ID_TO_STR					g_listErr2Str;
RegErr2Str							*g_err2StrHeader = NULL;

static ID_TO_STRING	__ErrID2Str[] = 
{
	{ ERR_FALSE, _T("Failed.") },
	{ ERR_OK, _T("Succeed.") },
	{ ERR_INITDB_FAILED, _T("Init Database FAILED.") },
	{ ERR_NO_MEM, _T("Can't allocate more memory.") },
	{ ERR_OUTOF_BUFF, _T("Insufficient buffer.") },
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
	{ ERR_HTTP_GET_NODATA, _T("Can't read any data with HTTP GET command.") },		// HTTP 的GET命名是不能读取任何数据端的
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


#ifndef _WIN32
LPCTSTR TranslateCERRNO(int err)
{
	switch (err) {
		case EACCES: return _T("The requested address is protected, and the current user has inadequate permission to access it.");
		case EADDRINUSE: return "The specified address is already in use.";
		case EADDRNOTAVAIL:  return "The specified address is not available from the local machine.";
		case EAFNOSUPPORT:	return "address is not valid for the address family of socket.";
		case EBADF:	return "socket is not a valid file descriptor.";
		case EDESTADDRREQ:	return "The socket is not bound to a local address and the protocol does not support listening on an unbound socket.";
		case ENOTSOCK:	return "socket does not refer to a socket.";
		case EISDIR:	return "An empty pathname was specified.";
		case ELOOP:	return "Too many symbolic links were encountered in translat-ing the pathname.  This is taken to be indicative of a looping symbolic link.";
		case ENAMETOOLONG:	return "A component of a pathname exceeded {NAME_MAX} charac-ters, or an entire path name exceeded {PATH_MAX} char-acters.";
		case ENOENT:	return "A component of the path name does not refer to an existing file.";
		case ENOTDIR:	return "A component of the path prefix is not a directory.";
		case EROFS:	return "The name would reside on a read-only file system.";
		case EALREADY:	return "The socket is non-blocking and a previous connection attempt has not yet been completed.";
		case ECONNREFUSED:	return "The attempt to connect was ignored (because the target is not listening for connections) or explicitly rejected.";
		case EHOSTUNREACH:	return "The target host cannot be reached (e.g., down, discon-nected).";
		case EINPROGRESS:	return "The socket is non-blocking and the connection cannot be completed immediately.  It is possible to select(2) for completion by selecting the socket for writing.";
		case EINTR:	return "Its execution was interrupted by a signal.";
		case EINVAL:	return "An invalid argument was detected.";
		case EISCONN:	return "The socket is already connected.";
		case ENETDOWN:	return "The local network interface is not functioning.";
		case ENETUNREACH:	return "The network isn't reachable from this host.";
		case ENOBUFS:	return "The system call was unable to allocate a needed memory buffer.";
		case EPROTOTYPE:	return "address has a different type than the socket that is bound to the specified peer address.";
		case ECONNRESET:	return "Remote host reset the connection request.";
		case EIO:	return "An I/O error occurred while reading from or writing to the file system.";
		case EAGAIN:	return "The socket is marked non-blocking, and the receive operation would block, or a receive timeout had been set, and the timeout expired before data were received.";
		case ENOTCONN:	return "The socket is associated with a connection-oriented protocol and has not been connected.";
		case EOPNOTSUPP:	return "The type and/or protocol of socket do not support the option(s) specified in flags.";
		case ETIMEDOUT:	return "The connection timed out.";
		case EMSGSIZE:	return "The msg_iovlen member of the msghdr structure pointed to by message is less than or equal to 0, or is greater than IOV_MAX.";
		case ENOMEM:	return "Insufficient memory is available.";
		default:
			return "unknown error code.";
	}
}
#endif // #ifndef _WIN32


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

Error2Str::operator LPCTSTR () const
{
	if (m_nError == ERR_CUSTOM_ERROR)
		return g_szCustomErrorDesc;

#ifdef _WIN32
	if (m_nError >= WSABASEERR)
	{
		return m_win32Error.Description();
	}
#endif

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
		return TranslateCERRNO(m_nError - ERR_C_ERRNO_BASE);
	}
#endif

	return _T("Not translated error code.");
}
