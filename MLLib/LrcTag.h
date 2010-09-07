
#pragma once

DECLAR_CPPUNIT_TEST_REG(LrcTag)

enum LyrTagType
{
	LTT_ARTIST			= 1,
	LTT_TITLE			= 1 << 1,
	LTT_ALBUM			= 1 << 2,
	LTT_BY				= 1 << 3,
	LTT_OFFSET			= 1 << 4,
	LTT_MEDIA_LENGTH	= 1 << 5,
	LTT_ID				= 1 << 6,
	LTT_ENCODING		= 1 << 7,
	LTT_ALL				= 0xFFFFFFFF,
};

bool IsTxtTagAtTailOfFile(LyrTagType tagType);

#define	SZ_LRC_AR				"ar:"
#define	SZ_LRC_TI				"ti:"
#define	SZ_LRC_AL				"al:"
#define	SZ_LRC_BY				"by:"
#define	SZ_LRC_ENCODING			"Encoding:"
#define	SZ_LRC_OFFSET			"offset:"
#define	SZ_LRC_LENGTH			"length:"
#define	SZ_LRC_ID				"id:"

#define	SZ_TXT_AR				"Artist:"
#define	SZ_TXT_TI				"Title:"
#define	SZ_TXT_AL				"Album:"
#define	SZ_TXT_ENCODING			"encoding:"
#define	SZ_TXT_OFFSET			"Offset:"
#define	SZ_TXT_LENGTH			"Length:"
#define	SZ_TXT_ID				"ID:"

struct LyricsProperties
{
	LyricsProperties()
	{
		m_nTimeOffset = 0;
		m_nEncodingID = ED_SYSDEF;
	}

	bool Equal(const LyricsProperties &other) const;
	void clear();

	CHAR_ENCODING GetEncodingIndex() { return m_nEncodingID; }
	LPCTSTR GetEncoding() const { return m_strEncoding.c_str(); }
	void SetEncoding(CHAR_ENCODING nEncodingID);
	void SetEncoding(LPCTSTR szEncoding);

	void SetOffsetTime(int nOffsetTime);
	void SetOffsetTime(LPCTSTR szOfffsetTime) { m_strTimeOffset = szOfffsetTime; m_nTimeOffset = _ttoi(szOfffsetTime); }
	LPCTSTR GetOffsetTimeStr() { return m_strTimeOffset.c_str(); }
	int GetOffsetTime() { return m_nTimeOffset; }

	void SetTxtLyrScrollEvents(LPCTSTR szTxtLyrScrollEvents) { m_strTimeOffset = szTxtLyrScrollEvents; }
	LPCTSTR GetTxtLyrScrollEvents() { return m_strTimeOffset.c_str(); }

	void SetMediaLength(int nMediaLength);

	tstring					m_strArtist;
	tstring					m_strTitle;
	tstring					m_strAlbum;
	tstring					m_strBy;
	tstring					m_strMediaLength;
	tstring					m_strId;

protected:
	// Character encoding
	tstring					m_strEncoding;
	CHAR_ENCODING			m_nEncodingID;

	// offset time of timestamps in lyrics, in ms
	int						m_nTimeOffset;
	tstring					m_strTimeOffset;	// Also, it will store TxtLyrScrollEvents

};

class CLyrTagNameValueMap
{
public:
	CLyrTagNameValueMap(LyricsProperties &prop, uint32 tagMask = LTT_ALL);

	int GetCount() const { return m_vItems.size(); }
	LyrTagType GetTagType(int nIndex);
	const string &GetLrcName(int nIndex);
	const string &GetTxtName(int nIndex);
	void SetValue(int nIndex, LPCWSTR szValue, int nLen);
	void SetValue(int nIndex, LPCSTR szValue, int nLen);
	const tstring &GetValue(int nIndex);

	tstring GetLrcTagString(int nIndex, bool bIncLineRet = true);
	tstring GetTxtTagString(int nIndex, bool bIncLineRet = true);

	void Remove(int nIndex);
	CHAR_ENCODING GetEncoding() { return m_encoding; }

	struct Item
	{
		string			strLrcName;
		string			strTxtName;
		LyrTagType		tagType;
		tstring			*pstrValue;
	};

protected:
	void OnSetValue(int nIndex);

protected:
	vector<Item>		m_vItems;
	LyricsProperties	&m_prop;
	CHAR_ENCODING		m_encoding;
	tstring				m_strOffset, m_strEncoding;

};

class CLrcTag : public LyricsProperties
{
public:
	CLrcTag(uint32 tagMask = LTT_ALL) { m_tagMask = tagMask; }

	// Used for writing, compare for changed tags, and save them.
	CLrcTag(LyricsProperties &orgProp, LyricsProperties &newProp, uint32 tagMask = LTT_ALL);

	int ReadFromFile(LPCTSTR szFile);
	bool ReadFromText(LPBYTE str, int nLen, bool bUseSpecifiedEncoding = false, CHAR_ENCODING encoding = ED_SYSDEF);
	bool ReadFromText(const char *str, int nLen);
	bool ReadFromText(LPCWSTR str, int nLen);

	int WriteToFile(LPCTSTR szFile, bool bLrcTag);
	bool WriteToText(stringex &str, bool bLrcTag);
	bool WriteToText(wstringex &str, bool bLrcTag);

	bool IsTagChanged() { return m_tagMask != 0; }

protected:

	uint32				m_tagMask;

};
