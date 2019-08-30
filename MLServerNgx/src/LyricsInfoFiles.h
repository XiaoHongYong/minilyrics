#pragma once

DECLAR_CPPUNIT_TEST_REG(LyricsInfoFiles)

//////////////////////////////////////////////////////////////////////////
/*
File Name:	xxx.lrc.hst
Purpuse:	All the update history information, including rejected and accepted
			@ for html pages, show credits.

File Name:	xxx.lrc.hd		(hd = history data)
Purpuse:	Only accepted or pending history data.

File Name:	xxx.lrc.rj.hd	(rj = rejected)
Purpuse:	Rejected history data.

File Name:	xxx.lrc.comments
Purpuse:	The comments by user.

Format:		<xml ....>
			<Comments>
			<item id="123" user="user" reviewed="1" date="1235232"/ str="Comments here">\r\n
			<item id="122" user="user" reviewed="1" date="1235232"/ str="Comments here">\r\n
			</Comments>

*///////////////////////////////////////////////////////////////////////////


#define SZ_LYR_HST_EXT		_T(".hst")
#define SZ_LYR_HD_EXT		_T(".hd")

#define LYR_UPDATE_ID_INVALID	0
#define LYR_UPDATE_ID_ORG		1

enum ReviewStatus
{
	RS_OPTIONAL		= 1,
	RS_REQUIRED		= 2,
	RS_ACCEPTED		= 4,
	RS_REJECTED		= 8,
	RS_REVIEWED		= 16,		// It's reviewed. Do not take action on lyrics data.
};

inline bool IsUnReviewed(ReviewStatus reviewStatus)
{
	return reviewStatus == RS_OPTIONAL || reviewStatus == RS_REQUIRED;
}

struct LyrUpdateItem
{
	long			nUpdateId;
	long			nLastUpdateId;
	time_t	        dateUpdate;
	tstring			strUploader;
	ReviewStatus	reviewStatus;
	tstring			strReviewer;
	tstring			strMD5;
	bool			bUpload;

	CCharBuffer		bufLyrData;

	LyrUpdateItem(LPCTSTR szUploader, ReviewStatus reviewStatus,
		LPCTSTR szMD5, bool bUpload)
	{
		this->strUploader = szUploader;
		this->reviewStatus = reviewStatus;
		this->strMD5 = szMD5;
		this->bUpload = bUpload;
		this->nUpdateId = 0;
		this->nLastUpdateId = LYR_UPDATE_ID_INVALID;
        this->dateUpdate = time(NULL);
	}

	LyrUpdateItem()
	{
		this->nUpdateId = LYR_UPDATE_ID_INVALID;
		this->nLastUpdateId = LYR_UPDATE_ID_INVALID;
		this->reviewStatus = RS_OPTIONAL;
		this->bUpload = false;
        this->dateUpdate = time(NULL);
	}

	virtual void FromXML(SXNode *pNode);
	virtual void ToXML(CXMLWriter &xmlFile);

};

typedef list<LyrUpdateItem>		ListLyrUpdateItem;

//
// Including .ht, .hd, .rj.hd files.
//
class CLyrUpdateHistory
{
public:
	CLyrUpdateHistory(void);
	virtual ~CLyrUpdateHistory(void);

	// Specify the lyrics file name to open lyrics update history.
	int Open(LPCTSTR szFile);

	int Add(LyrUpdateItem &lyrUpdateItem, CCharBuffer &bufNewLyrData, long nDerivedFromId = 0);

	int GetUpdateHistory(ListLyrUpdateItem &listLyrUpdateItem, long &nCurrentUpdateId, bool bIncludingLyrData);

	int ReviewLyrics(long nLyricsId, long nStartUpdateId, long nUpdateIdAccepted, long nUpdateIdRejected, LPCTSTR szReviewer);

	// int GetUnreviewedUpdateHistory(ListLyrUpdateItem &listLyrUpdateItem, bool bIncludingLyrData);
	// int AcceptReview(long nUpdateId);
	// int RejectReview(long nUpdateId);

	bool IsUpdateExists(LPCTSTR szMd5);

protected:
	int FillInLyricsData(ListLyrUpdateItem &listLyrUpdateItem);

	tstring				m_strLyrFile;

};
