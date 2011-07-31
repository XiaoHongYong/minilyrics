#pragma once

#include "MPMediaTree.h"
#include "SkinDroplistCtrl.h"

class CMPCmdHandlerOfMediaGuide : public ISkinCmdHandler  
{
public:
	CMPCmdHandlerOfMediaGuide();
	virtual ~CMPCmdHandlerOfMediaGuide();

	virtual void Init(CSkinWnd *pSkinWnd);

	// if the command id is processed, return true.
	virtual bool OnCommand(int nId);
	virtual bool OnCustomCommand(int nID);
	virtual bool OnUIObjNotify(IUIObjNotify *pNotify);

protected:
	void OnDblClickMediaList();

	void ReloadMediaGuideView();
	void UpdateMediaList();

	void AddHistoryPath();
	void BackHistoryPath();

protected:
	struct HistroyItem
	{
		SkinTreeStrPath_t		path;
		int						nSelChild;
	};

	CMPMediaTree				m_mediaTree;
	SkinTreeStrPath_t			m_vPathLatest;
	list<HistroyItem>			m_historyPath;

};

