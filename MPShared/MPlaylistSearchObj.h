#if !defined(_MPLAYLISTSEARCHOBJ_H_)
#define _MPLAYLISTSEARCHOBJ_H_

DECLAR_CPPUNIT_TEST_REG(MPlaylistSearchObj)

class CMPlaylistSearchObj : public CSkinDataObj, public IEditNotification, public IUIObjNotifyHandler
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinDataObj)
public:
	CMPlaylistSearchObj();
	virtual ~CMPlaylistSearchObj();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	void OnCreate();

	void OnSize(int cx, int cy) { }

	void OnTimer(int nId);

public:
	virtual void OnTextChanged();

	// if this key is processed, return true.
	virtual void OnSpecialKey(SpecialKey key);

	virtual void OnUIObjNotify(IUIObjNotify *pNotify);

protected:
	void DoSearch(LPCTSTR szText);

	void UpdatePlaylist();

	void AddInRecentPlaylist(LPCTSTR szName, LPCTSTR szFile, int nPos);

	void UseResultAsNowPlaying();

protected:
	enum
	{
		TD_BEGIN_SEARCH		= 1000,
	};

	enum PLAYLIST_NAME
	{
		PN_NOWPLAYING,
		PN_ALL_MEDIALIB,
		PN_CUSTOMIZED,
	};

	tstring			m_strIDEditor;
	tstring			m_strIDPlaylist;
	tstring			m_strIDPlaylistList;

	CSkinEditCtrl	*m_pCtrlEdit;
	CSkinListCtrlEx	*m_pCtrlPlaylist;
	CSkinListCtrl	*m_pCtrlPlaylistList;

	CMPAutoPtr<IPlaylist>	m_Playlist;
	CMPAutoPtr<IPlaylist>	m_PlaylistResults;

	int				m_nIDBeginSearchTimer;

	V_Str			m_vRecentPL;

};

#endif // !defined(_MPLAYLISTSEARCHOBJ_H_)
