#pragma once

#include "avltree.h"
#include "avltree2.h"

#ifdef _WIN32
#include "win32/FileSpaceAllocator.h"
#endif

#ifdef _LINUX
#include "linux/FileSpaceAllocator.h"
#endif

#ifdef _MAC_OSX
#include "Mac/FileSpaceAllocator.h"
#endif

#include "fileindexex.h"
#include "FileIndex3.h"

#include "StringDB.h"
