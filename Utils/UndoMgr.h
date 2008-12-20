#ifndef _UNDOMGR_H__
#define _UNDOMGR_H__

#pragma once


class CUndoAction
{
public:
	CUndoAction() {}
	virtual ~CUndoAction() {}
	virtual void Undo() = 0;
	virtual void Redo() = 0;

};


class CBatchUndoAction : public CUndoAction
{
public:
	typedef vector<CUndoAction*>		LIST_ACTIONS;

	CBatchUndoAction();
	virtual ~CBatchUndoAction();

	virtual void Undo();
	virtual void Redo();
	virtual void AddAction(CUndoAction *pAction);

	bool IsEmpty();

protected:
	LIST_ACTIONS			m_listActions;

};


class IUndoMgrNotify
{
public:
	enum Status
	{
		S_CAN_UNDO,
		S_CAN_REDO,
	};

	enum Action
	{
		A_UNDO,
		A_REDO,
		A_ADD,
	};

	virtual void OnStatusChanged(Status status, bool bVal) = 0;
	virtual void OnAction(Action action) = 0;

};


//
// Common undo and redo manager
//
class CUndoMgr
{
public:
	typedef vector<CUndoAction*>		LIST_ACTIONS;

	CUndoMgr();
	~CUndoMgr();

	void SetNotification(IUndoMgrNotify *pNotify) { m_pNotify = pNotify; }

	void BeginBatchAction();
	void EndBatchAction();

	virtual void AddAction(CUndoAction *pAction);

	virtual bool Undo();
	virtual bool Redo();

	virtual bool CanUndo();
	virtual bool CanRedo();

	virtual void Clear();

protected:
	void OnDetectUndoStatus();
	void OnDetectRedoStatus();

	void DoAddAction(CUndoAction *pAction);

protected:
	LIST_ACTIONS			m_listActions;
	int						m_nPosUndo;
	IUndoMgrNotify			*m_pNotify;

	CBatchUndoAction		*m_pBatchUndoAction;

	bool					m_bCanUndoPrev, m_bCanRedoPrev;

};


#endif // _UNDOMGR_H__

