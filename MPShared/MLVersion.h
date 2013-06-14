
#pragma once

#include "MlVersionStr.h"

#ifdef _MPLAYER
#define		ZP_MAJOR_VERSION	0
#define		ZP_MINOR_VERSION	3
#define		ZP_BUILD_NUM		1

#define AUTO_BUILD_NUM ZP_BUILD_NUM

#else
#define		ML_MAJOR_VERSION	7
#define		ML_MINOR_VERSION	6
#define		ML_BUILD_NUM		31

#define AUTO_BUILD_NUM ML_BUILD_NUM

#endif

