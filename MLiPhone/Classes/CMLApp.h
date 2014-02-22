#pragma once

#import "../../base/base.h"
#import "../../Utils/Utils.h"
#import "../../MLLib/MLLib.h"
#import "IMLAppNotify.h"

class CMLApp
{
public:
	CMLApp(void);
	virtual ~CMLApp(void);

	static CMLApp *GetInstance();

	int Init();
	void Quit();

	void OnMediaChanged(LPCTSTR szMediaKey, LPCTSTR szArtist, LPCTSTR szAlbum, LPCTSTR szTitle, LPCTSTR szEmbeddedLyr, int nMediaLength);
	
public:
	LPCTSTR GetArtist() const { return m_strArtist.c_str(); }
	LPCTSTR GetTitle() const { return m_strTitle.c_str(); }
	LPCTSTR GetMediaKey() const { return m_strCurMediaKey.c_str(); }

public:
	CMLData &GetLyricsData() { return m_lyrData; }

protected:
	void AddMediaInfo();

protected:
	CMLData					m_lyrData;

	// Media information
	tstring					m_strCurMediaKey;
	tstring					m_strArtist, m_strTitle, m_strAlbum;
	int						m_nMediaLength;

	static CMLApp			*m_pAppInstance;

};
