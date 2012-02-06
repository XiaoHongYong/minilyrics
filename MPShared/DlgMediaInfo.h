#pragma once


void ShowMediaInfoDialog(CSkinWnd *pParent, IMedia *pMedia);

class CDlgMediaInfoPage;

enum MediaTagType
{
	MTT_ID3V1		= 1,
	MTT_ID3V2		= 1 << 1,
	MTT_APEV1		= 1 << 2,
	MTT_APEV2		= 1 << 3,
};

UINT GetSupportedMTTByExt(LPCTSTR szMedia);


#include "PreferencePageBase.h"

class CDlgMediaInfo : public CMPSkinWnd
{
public:
	CDlgMediaInfo(IMedia *pMedia);
	~CDlgMediaInfo(void);

	void OnSkinLoaded();

	void OnDestroy();

	void OnOK();

protected:
	void Clear();
	void ReloadMediaInfo();

protected:
	friend class CDlgMediaInfoPage;
	friend class CDlgMediaInfoPageBasic;
	friend class CDlgMediaInfoPageDetail;
	friend class CDlgMediaInfoPageLyrics;
	friend class CDlgMediaInfoPagePictures;

	// basic and detail info
	tstring				m_artist,  m_title,  m_album,  m_comment;
	tstring				m_track,  m_year,  m_genre,  m_composer;
	tstring				m_encoded,  m_url,  m_copyRight,  m_origArtist,  m_publisher,  m_bpm;
	bool				m_bBasicInfoModified;
	bool				m_bDetailInfoModified;

	// lyrics
	tstring				m_strLyrName;
	tstring				m_strLyrics;
	bool				m_bUnsyncLyricsModified;

	// pictures
	ID3v2Pictures		m_pictures;
	bool				m_bPicturesModified;


protected:
	CMPAutoPtr<IMedia>	m_pMedia;

	tstring				m_strMediaFile;
	bool				m_bReadOnly;
	UINT				m_nSupportedMediaTagType;
	CHAR_ENCODING		m_nEncodingOfConvertAnsi;

	vector<class CDlgMediaInfoPage *>	m_vInfoPages;
	class CDlgMediaInfoPage				*m_pOldPage;

};
