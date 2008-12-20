#ifndef _NET_SDK_H
#define _NET_SDK_H

#include "sockadapt.h"

#pragma warning(disable:4786)

#include <list>
using namespace std;


#define NETLIB_API 

NETLIB_API BOOL InitNetLib();

#define TC_LEN_TYPE		4
#define TC_LEN_XMLLEN	7

class CComunicator;
class CServer;


#endif // _NET_SDK_H
