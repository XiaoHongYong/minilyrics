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

protected:
	void UpdatePlaylist(CEventPlaylistChanged *pEventPlaylistChanged = NULL);

};
