// MLError.h: interface for the CMLError class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _BASE_INC_
#define _BASE_INC_

////////////////////////////////////////////////////////////
//
#include "idstring.h"

#include "Error.h"

#include "stringex_t.h"
#include "xstr.h"

#include "CharEncoding.h"
#include "stringex.h"
#include "safestr.h"

// classes
#include "StrPrintf.h"

#include "CharEncoding.h"

// apis
#include "digest.h"

#ifdef _WIN32
#include "win32/fileapi.h"
#include "win32/WndAbout.h"
#include "win32/WndHelper.h"

#include "win32/Event.h"
#include "win32/Mutex.h"
#include "win32/Semaphore.h"
#include "win32/Thread.h"
#endif

#ifdef _LINUX
#include "linux/fileapi.h"

#include "linux/Event.h"
#include "linux/Mutex.h"
#include "linux/Semaphore.h"
#include "linux/Thread.h"
#endif

#ifdef _MAC_OSX
#include "Mac/fileapi.h"

#include "Mac/Event.h"
#include "Mac/Mutex.h"
#include "Mac/Semaphore.h"
#include "Mac/Thread.h"
#endif

#include "misc.h"

#endif // _BASE_INC_
