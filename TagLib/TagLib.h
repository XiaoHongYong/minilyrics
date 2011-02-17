#pragma once

enum TagType
{
	TT_ID3V1,
	TT_ID3V2,
	TT_APE,
};

class TagFields
{
public:
	tstring		strName;
	tstring		strValue;

};
typedef list<TagFields>		ListTagFields;

class CTag
{
public:
	ListTagFields		m_listFields;
	TagType				m_tagType;
	bool				m_bRemovable;		// Can the tag be removed
	bool				m_bEmpty;			// Is this an empty tag?

};

class CListTags: public list<CTag *>
{
public:
	CListTags();
	virtual ~CListTags();

};


class CTagLib
{
public:
	CTagLib(void);
	virtual ~CTagLib(void);

public:
	// pMediaLength: in ms
	static int GetTagFast(LPCTSTR szFile, tstring *pArtist, tstring *pTitle, tstring *pAlbum,
		tstring *pYear, tstring *pGenre, tstring *pTrackNo, uint32 *pMediaLength);

	static int GetAllTags(LPCTSTR szFile, CListTags &listTags);

	static bool IsMediaTypeSupported(LPCTSTR szFile);

};

