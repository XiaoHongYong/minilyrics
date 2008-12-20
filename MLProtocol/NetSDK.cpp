#include "stdafx.h"
#include "netsdk.h"

NETLIB_API BOOL InitNetLib()
{
#ifdef _WIN32
    WORD sockVersion;
    WSADATA wsaData;

    sockVersion = MAKEWORD(2, 2);
    //start dll
	return (WSAStartup(sockVersion, &wsaData) == 0);
#else
	return TRUE;
#endif
}
