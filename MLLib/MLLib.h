/********************************************************************
	Created  :	2002/01/04	21:41
	FileName :	MLLib.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/


#ifndef _MLLIB_HEADER_2005_1_26_14_20_INCLUDED_
#define _MLLIB_HEADER_2005_1_26_14_20_INCLUDED_

#include "HelperFun.h"
#include "Lyrics3v2.h"
#include "LyricsSearch.h"

#include "OurDate.h"

#include "Register.h"
#include "MLProfile.h"

#include "MLData.h"
#include "MLClientSession.h"

//
// 是否编译为英文版本？
//
//#define _ENG_EDITION
extern BOOL		g_bEngEdition;

enum LicenseType
{
	LT_EXPIRED,				// 未注册，超过试用期
	LT_CRACK,				// 被破解的
	LT_REGISTERED,			// 注册版本
	LT_TRIAL,				// 试用版本
	LT_FREE,				// 免费版本
};

extern LicenseType	g_LicenseType;

#define SZ_REGCODE			_T("C_ode_Rge")
#define SZ_REGNAME			_T("N-ame_rge")
#define SZ_INVALID_RC		_T("Inva_d_Cod-e")


const int	DOWNOKID_NONE						= 0;
const int	DOWNOKID_CHECK_VERSION				= 1;
const int	DOWNOKID_LRC						= 2;
const int	DOWNOKID_CHECK_VERSION_MANUAL		= 3;
const int	DOWNOKID_CHECK_AD					= 4;


#define NONE_LYRCS	_T("")


#endif // _MLLIB_HEADER_2005_1_26_14_20_INCLUDED_
