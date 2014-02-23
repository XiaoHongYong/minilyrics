#ifndef _ML_UTILS_H
#define _ML_UTILS_H

#include "SimpleXML.h"
#include "XMLWriter.h"
#include "MLBinXMLParser.h"
#include "UUEncode.h"
#include "Profile.h"
#include "App.h"
#include "XCharSeparatedValues.h"
#include "UndoMgr.h"
#include "digest.h"
#include "VersionNumber.h"

#ifdef _ANDROID
	#include "android/log.h"
#else
	//#include "log.h"
#endif

#ifdef _IPHONE
#include "Mac/LocalizeTool.h"
#else
#include "LocalizeTool.h"
#endif

#endif	// _ML_UTILS_H
