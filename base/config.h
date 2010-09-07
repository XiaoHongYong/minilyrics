#ifndef _ML_CONFIG_H
#define _ML_CONFIG_H

// #define _LINUX_GTK2
// #define _LINUX
// #define _MAC_OSX
// #define _IPHONE

#ifdef __MAC_OS_X_VERSION_MAX_ALLOWED
#define _MAC_OSX
#endif

#if defined(_IPHONE) || defined(_MAC_OSX)
#define _MAC
#define UTF8 // in iPhone and Mac OS X, default character encoding is UTF8
#endif


#ifdef _LINUX
#define UTF8
#ifndef _DEBUG
#define _DEBUG
#endif
#ifndef _LINUX_GTK2
#define _LINUX_GTK2
#endif
#endif

#ifdef WIN32
#ifndef _WIN32_WCE
#ifndef _WIN32_DESKTOP
#define _WIN32_DESKTOP
#endif
#endif
#endif

#ifdef _WIN32_WCE
#ifndef _WINCE_
#define _WINCE_
#endif
#endif

#ifdef UTF8
#define DEFAULT_ENCODING		ED_UTF8
#endif

#ifdef UNICODE
#define DEFAULT_ENCODING		ED_UNICODE
#endif

#ifndef DEFAULT_ENCODING
#define DEFAULT_ENCODING		ED_SYSDEF
#endif


#endif
