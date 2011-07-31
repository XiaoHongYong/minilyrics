// MPMediaLibCmdHandler.h: interface for the CMPMediaLibCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPMEDIALIBCMDHANDLER_H__1A67AACB_A410_4FA7_B631_4107B96C2050__INCLUDED_)
#define AFX_MPMEDIALIBCMDHANDLER_H__1A67AACB_A410_4FA7_B631_4107B96C2050__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SkinDroplistCtrl.h"

class CMediaLibTreeProvider
{
public:
	enum FolderType
	{
		FT_MEDIA_LIB,
		FT_NOW_PLAYING,
		FT_ALL_MUSIC,
		FT_ALL_ARTIST,
		FT_ALL_SONGS_BY_ARTIST,
		FT_ARTIST,
		FT_ARTIST_UNKNOWN,

		FT_ALL_ALBUM,
		FT_ALBUM,
		FT_ALBUM_UNKNOWN,
		FT_ALBUM_TOP_RATING,

		FT_ARTIST_ALBUM,
		FT_ARTIST_ALBUM_UNKNOWN,
		FT_ARTIST_ALL_MUSIC,
		FT_ARTIST_TOP_RATING,

		FT_ALL_PLAYLIST_FILES,
		FT_PLAYLIST_FILE,
		FT_MEDIA_FILE,
		FT_TOP_RATING,
		FT_TOP_PLAYED,
		FT_RECENT_PLAYED,
		FT_RECENT_ADDED,
	};

	enum ImageIndex
	{
		II_MEDIA_LIB,
		II_PLAYLISTS,
		II_ARTIST,
		II_ALBUM,
		II_GENRE,
		II_MUSIC,
		II_NOW_PLAYING,
	};

	struct Item
	{
		tstring		strName;
		BYTE		nImageIndex;
		bool		bUptodate;
		FolderType	folderType;
		CMPAutoPtr<IPlaylist>	playlist;
		LPCTSTR GetValue() const
		{
			if (folderType == FT_ARTIST_UNKNOWN
				|| folderType == FT_ARTIST_ALBUM_UNKNOWN
				|| folderType == FT_ALBUM_UNKNOWN)
				return _T("");
			else
				return strName.c_str();
		}
	};
	typedef vector<Item>		V_ITEMS;

	CMediaLibTreeProvider() { }
	~CMediaLibTreeProvider() { }

	void Init();
	void Close();

	void Update();

	bool EnumChildren(V_ITEMS &vItems);

	bool GetChildPlaylist(int nIndex, IPlaylist **playlist);

	bool ChToChild(int nIndex);
	bool ChToParent();
	bool ChToPath(V_ITEMS &vPath);
	bool ChToPath(LPCTSTR szPath);

	bool GetPath(V_ITEMS &vPath);

	Item GetChildData(int n);

	bool IsCurNodePlaylist();
	bool GetCurNodePlaylist(IPlaylist **playlist);

	bool IsCurNodePlaylistFile();
	bool GetCurNodePlaylistFile(int nChildPos, tstring &strPlaylistFile);

	void EraseChild(int nIndex) { m_tree.EraseChild(nIndex); }

protected:
	void AddPlaylistToTree(IPlaylist *playlist);

protected:
	CMPAutoPtr<IMediaLibrary>		m_mediaLib;
	CTree<Item>						m_tree;

};

class CMPMediaLibCmdHandler : public ISkinCmdHandler  
{
public:
	CMPMediaLibCmdHandler();
	virtual ~CMPMediaLibCmdHandler();

	virtual void Init(CSkinWnd *pSkinWnd);

	// if the command id is processed, return true.
	virtual bool OnCommand(int nId);
	virtual bool OnCustomCommand(int nID);
	virtual bool OnUIObjNotify(IUIObjNotify *pNotify);

protected:
	void OnDblClickMediaList();

	void ReloadMedialibView();
	void UpdateMediaList();

	void AddHistoryPath();
	void BackHistoryPath();

protected:
	struct HistroyItem
	{
		CMediaLibTreeProvider::V_ITEMS		path;
		int									nSelChild;
	};

	CMediaLibTreeProvider		m_mediaLibTree;
	CMediaLibTreeProvider::V_ITEMS		m_vPathLatest;
	list<HistroyItem>			m_historyPath;


};

#endif // !defined(AFX_MPMEDIALIBCMDHANDLER_H__1A67AACB_A410_4FA7_B631_4107B96C2050__INCLUDED_)
