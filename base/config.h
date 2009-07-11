#ifndef _ML_CONFIG_H
#define _ML_CONFIG_H

// #define _LINUX_GTK2
// #define _LINUX
// #define _MAC_OSX

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


#endif
