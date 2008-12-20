#include "stdafx.h"
#include "UndoMgr.h"


//////////////////////////////////////////////////////////////////////////

CUndoMgr::CUndoMgr()
{
	m_nPosUndo = -1;
	m_pBatchUndoAction = NULL;
	m_pNotify = NULL;

	m_bCanUndoPrev = false;
	m_bCanRedoPrev = false;
}


CUndoMgr::~CUndoMgr()
{
	if (m_pBatchUndoAction)
	{
		delete m_pBatchUndoAction;
		m_pBatchUndoAction = NULL;
	}
	Clear();
}


void CUndoMgr::BeginBatchAction()
{
	ASSERT(!m_pBatchUndoAction);
	if (m_pBatchUndoAction)
	{
		EndBatchAction();
	}
	m_pBatchUndoAction = new CBatchUndoAction;
}


void CUndoMgr::EndBatchAction()
{
	ASSERT(m_pBatchUndoAction);
	if (m_pBatchUndoAction)
	{
		if (m_pBatchUndoAction->IsEmpty())
			delete m_pBatchUndoAction;
		else
			DoAddAction(m_pBatchUndoAction);
		m_pBatchUndoAction = NULL;
	}
}


void CUndoMgr::AddAction(CUndoAction *pAction)
{
	if (m_pBatchUndoAction)
		m_pBatchUndoAction->AddAction(pAction);
	else
		DoAddAction(pAction);
}


bool CUndoMgr::Undo()
{
	ASSERT(!m_pBatchUndoAction);
	if (m_pBatchUndoAction)
		EndBatchAction();

	ASSERT(m_nPosUndo >= -1 && m_nPosUndo < (int)m_listActions.size());
	if (CanUndo())
	{
		m_listActions[m_nPosUndo]->Undo();
		m_nPosUndo--;

		if (m_pNotify)
			m_pNotify->OnAction(IUndoMgrNotify::A_UNDO);

		OnDetectRedoStatus();
		OnDetectUndoStatus();
		return true;
	}
	return false;
}


bool CUndoMgr::Redo()
{
	ASSERT(!m_pBatchUndoAction);
	if (m_pBatchUndoAction)
		EndBatchAction();

	// ASSERT(CanRedo);
	if (CanRedo())
	{
		m_listActions[m_nPosUndo + 1]->Redo();
		m_nPosUndo++;

		if (m_pNotify)
			m_pNotify->OnAction(IUndoMgrNotify::A_REDO);

		OnDetectRedoStatus();
		OnDetectUndoStatus();
		return true;
	}
	return false;
}


bool CUndoMgr::CanUndo()
{
	return m_nPosUndo >= 0 && m_listActions.size();
}


bool CUndoMgr::CanRedo()
{
	return m_listActions.size() && m_nPosUndo < int(m_listActions.size() - 1);
}


void CUndoMgr::Clear()
{
	ASSERT(!m_pBatchUndoAction);
	if (m_pBatchUndoAction)
		EndBatchAction();

	if (m_listActions.size())
	{
		OnDetectRedoStatus();
		OnDetectUndoStatus();
	}

	for (LIST_ACTIONS::iterator it = m_listActions.begin(); it != m_listActions.end(); it++)
	{
		delete *it;
	}
	m_listActions.clear();

	m_nPosUndo = -1;
}


void CUndoMgr::OnDetectUndoStatus()
{
	bool		bNew = CanUndo();
	if (m_bCanUndoPrev != bNew)
	{
		m_bCanUndoPrev = bNew;
		if (m_pNotify)
			m_pNotify->OnStatusChanged(IUndoMgrNotify::S_CAN_UNDO, bNew);
	}
}


void CUndoMgr::OnDetectRedoStatus()
{
	bool		bNew = CanRedo();
	if (m_bCanRedoPrev != bNew)
	{
		m_bCanRedoPrev = bNew;
		if (m_pNotify)
			m_pNotify->OnStatusChanged(IUndoMgrNotify::S_CAN_REDO, bNew);
	}
}


void CUndoMgr::DoAddAction(CUndoAction *pAction)
{
	ASSERT(m_nPosUndo >= -1);
	if (m_nPosUndo < (int)(m_listActions.size() - 1))
	{
		for (LIST_ACTIONS::iterator it = m_listActions.begin() + (m_nPosUndo + 1);
			it != m_listActions.end(); it++)
		{
			delete *it;
		}
		m_listActions.erase(m_listActions.begin() + (m_nPosUndo + 1), m_listActions.end());
	}

	m_listActions.push_back(pAction);
	m_nPosUndo = m_listActions.size() - 1;

	if (m_pNotify)
		m_pNotify->OnAction(IUndoMgrNotify::A_ADD);

	OnDetectRedoStatus();
	OnDetectUndoStatus();
}
