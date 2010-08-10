#ifndef _OURLIB_ERROR_H
#define _OURLIB_ERROR_H

#ifdef _WIN32
#include "win32/Win32Error.h"
#include <winsock2.h>
#else
#define WSABASEERR 10000
#endif

#ifdef _LINUX
#include "linux/Win32Error.h"
#endif

#ifdef _MAC
#include "Mac/Win32Error.h"
#endif

//
//	GetLastError()可能的返回值
//
enum Error
{
	ERR_OK					= 0,
	ERR_CUSTOM_ERROR		= -1,
	ERR_FALSE				= 1,
	ERR_FAILED				= ERR_FALSE,
	ERR_INITDB_FAILED		= 2,
//	ERR_SEND_ERROR,
//	ERR_PARAM_ERROR,
	ERR_NO_MEM				= 3,
	ERR_OUTOF_BUFF			= 4,

	ERR_EOF					= 6,
	ERR_BAD_MSG				= 7,
	ERR_PARSE_XML			= 8,

	ERR_FILE_NOT_EXIST		= 9,

	ERR_EXIST				= 12,
	ERR_NOT_FOUND			= 13,

	ERR_NET_HOST_NOT_FOUND	= 17,
	ERR_NOT_RECIVED_ALL		= 18,

	//
	// Open lyrics errors!
	//
	ERR_NOT_SUPPORT_FILE_FORMAT	= 19,
	ERR_NOLYRICS_OPENED			= 20,
	ERR_UNKNOWN_LRC_FILE_TYPE	= 21,
	ERR_URL_EXIST_IN_QUEUE		= 22,				// URL 已经存在在下载队列中
	ERR_URL_EXIST_IN_HISTORY	= 23,				// URL 已经存在于下载历史中
	ERR_URL_ALREADY_DOWN_OK		= 24,				// URL 已经下载完毕

	ERR_NOT_OPENED,

	//
	ERR_CREATE_FILE_MAPPING,
	ERR_MAP_VIEWOFFILE,
	ERR_FILE_STAT,			// fstat error
	ERR_OPEN_FILE			= 35,
	ERR_READ_FILE			= 36,
	ERR_WRITE_FILE			= 37,
	ERR_SEEK_FILE			= 38,
	ERR_NOT_FIND_LRC3V2		= 39,
	ERR_BAD_LRC3V2_FORMAT	= 40,
	ERR_BAD_LS_FORMAT		= 41,		// 错误的.Ls文件
	ERR_BAD_FILE_FORMAT		= 42,

	ERR_DISABLE_PACK_XML	= 50,
	ERR_PACK_BAD_MD5		= 51,
	ERR_INVALID_PACK_VER	= 52,

	ERR_NOT_SUPPORT_ENCODING = 60,

	ERR_NOT_FOUND_ID3V2		= 70,
	ERR_NOT_SUPPORT_ID3V2_VER	= 71,
	ERR_INVALID_ID3V2_FRAME		= 72,

	ERR_HTTP_BAD_REQUEST	= 1000,
	ERR_HTTP_BAD_FORMAT,
	ERR_HTTP_BAD_HTTP_PRO_TYPE,
	ERR_HTTP_GET_NODATA,
	ERR_HTTP_INVLIAD_TRUNK,
	ERR_HTTP_BAD_URL,
	ERR_HTTP_CODE_ERROR,
	ERR_HTTP_HEAD_NOT_END,
	ERR_HTTP_DATA_NOT_FINISHED,
	ERR_HTTP_302,
	ERR_HTTP_400,		// MLERR_BAD_HTTP_REQUEST
	ERR_HTTP_401,		// MLERR_ACCESS_NOT_AUTHORIZED
	ERR_HTTP_403,		// MLERR_DOWNLOAD_DENIED
	ERR_HTTP_404,
	ERR_HTTP_407,		// MLERR_HTTP_UNATHORIZED
	ERR_HTTP_500,
	
	// ml protocal command error code
	ERR_MLPROTOCAL_ERROR_BASE	= 1035,

	ERR_PLAYER_ERROR_BASE		= 2000,

	ERR_C_ERRNO_BASE			= 3000,	// C error code
};

void SetCustomErrorDesc(LPCTSTR szErrorDesc);

class RegErr2Str
{
public:
	RegErr2Str(struct ID_TO_STRING err2Str[]);
	~RegErr2Str();

	struct ID_TO_STRING		*m_err2Str;
	RegErr2Str				*m_pNext;
};

class Error2Str
{
protected:
	int m_nError;
#ifdef _WIN32
	CWin32Error		m_win32Error;
#endif
public:
	Error2Str(int nErr) : m_nError(0)
#ifdef _WIN32
	, m_win32Error(-1)
#endif
	{
		m_nError = nErr;
#ifdef _WIN32
		if (m_nError >= WSABASEERR)
			m_win32Error.doFormatMessage((unsigned int )nErr);
#endif
	}

    operator LPCTSTR () const;
//    {
//		if (m_nError >= WSABASEERR)
//		{
//			m_win32Error.doFormatMessage(m_nError);
//			return m_win32Error.Description();
//		}
//		else
//			return IDToString(__ErrID2Str, nErr, "Unknown error!");
//    }

};

#endif // _OURLIB_ERROR_H
