#pragma once

#include "LyricsInfoFiles.h"

//////////////////////////////////////////////////////////////////////////
/*

Upload lyrics(must be logged in users):
1. New lyrics:			a) Add in lyrics db;	b)Add in lyrics update manager for review (Optinal for review)
2. Existing, same user:	a) Update in lyrics db;	b)Add in lyrics update manager for review (Optinal for review)
3. Existing, another u:							a)Add in lyrics update manager for review (Optinal for required)


Review lyrics (administrator)
1. Accept:				a) Set flag to accepted.	b) Add in lyrics db, if NOT added.
2. Reject:				a) Set flag to rejected.	b) Revert changes in lyrics db and lyrics file, if added/updated.

*//////////////////////////////////////////////////////////////////////////
/*
struct LyrUpdateItem
{
	long			nId;
	long			nLyrId;
	ReviewStatus	reviewStatus;
	LONG_DATETIME2	nDateUpdate;
	tstring			strUploader;
};

typedef list<LyrUpdateItem>			ListLyrUpdate;
*/
class CLyricsUpdateMgr
{
public:
	CLyricsUpdateMgr(void);
	virtual ~CLyricsUpdateMgr(void);

	int AddUpdate(long nLyricsId, LPCTSTR szLyrFile, LyrUpdateItem &lyrUpdateItem, CCharBuffer &bufLyrData, long nDerivedFromId = 0);

};

extern CLyricsUpdateMgr		g_lyrUpdateMgr;
