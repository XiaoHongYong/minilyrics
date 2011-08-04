#pragma once

class CMPlaylistCtrl : public CSkinListCtrl, public IEventHandler
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinListCtrl)
public:
	CMPlaylistCtrl();
	virtual ~CMPlaylistCtrl();

	void OnCreate();
	void OnEvent(const IEvent *pEvent);

protected:
	void UpdatePlaylist(CEventPlaylistChanged *pEventPlaylistChanged = NULL);

};
