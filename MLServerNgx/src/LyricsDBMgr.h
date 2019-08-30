// LyricsDBMgr.h: interface for the CLyricsDBMgr class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "LyricsDB.h"
#include "MLIDEncrypt.h"

DECLAR_CPPUNIT_TEST_REG(CLyricsDBMgr);

void ConvertUtf8ToProperFileData(LPCSTR szUtf8, int nLenUtf8, CCharBuffer &bufData);


class CLyricsDBMgr  
{
public:
	CLyricsDBMgr();
	virtual ~CLyricsDBMgr();

    void init(const string &lyricsDir, const string &relatedUploadDir) {
        m_lyricsDir = lyricsDir;
        m_relatedUploadDir = relatedUploadDir;
    }

	// New uploaded lyrics.
	int AddNewUploadedLyrics(long nUploaderId, LPCTSTR szUploader, CLrcTag &lyrProp, tstring &strLyrContent, long &nLyricsId, long nDerivedFromId);

	// Update existing lyrics.
	int UpdateLyrics(RetLyrInfo &lyrInfo, CLrcTag &lyrProp, tstring &strLyrContent, LPCTSTR szUploader);

	bool IsLyricsExists(LyricsProperties &lyrProp, LPCTSTR szMD5, int nUploaderId = 0, uint32 *pOrgLyricsId = NULL);

protected:
    void getLyricsFile(const string &relatedLink, tstring &outFileName);
    int SaveUploadedLyricsFile(LPCTSTR szLyricsId, LYRICS_CONTENT_TYPE lyrContentType, stringex &strLyrContentUtf8, tstring &strFile);

	int AddLyrics(CLyricsInfo &lyrInfo, CLrcTag &lyrProp, tstring &strLyrContent, LPCTSTR szAddToDir = NULL, long nDerivedFromId = 0);

    string              m_lyricsDir;
    string              m_relatedUploadDir;
    
};

extern CLyricsDBMgr		g_LyricsDBMgr;

