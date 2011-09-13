#pragma once

class CMPlaylistCtrl : public CSkinListCtrl, public IEventHandler
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinListCtrl)
public:
	CMPlaylistCtrl();
	virtual ~CMPlaylistCtrl();

	void OnCreate();
	void OnEvent(const IEvent *pEvent);

	void OnKeyDown(UINT nChar, UINT nFlags);

	void SendNotifyEvent(CSkinListCtrlEventNotify::Command cmd, int nClickedRow, int nClickedCol);

	void DeleteSelectedItems();
	void OffsetAllSelectedItems(bool bMoveDown);

	void SetNowPlaying();

	enum
	{
		IMAGE_NONE			= 0,
		IMAGE_NOW_PLAYING	= 5,
	};

protected:
	void UpdatePlaylist(CEventPlaylistChanged *pEventPlaylistChanged = NULL);

	int GetItemImageIndex(int nItem);

	int			m_nNowPlaying;

};
