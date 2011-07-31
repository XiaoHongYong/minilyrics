// MPVisAdapter.cpp: implementation of the CMPVisAdapter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "MPVisAdapter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPVisAdapter::CMPVisAdapter()
{
	OBJ_REFERENCE_INIT
}

CMPVisAdapter::~CMPVisAdapter()
{

}

MLRESULT CMPVisAdapter::Init(IMPlayer *pPlayer)
{
	return ERR_OK;
}

MLRESULT CMPVisAdapter::Quit()
{
	return ERR_OK;
}

int CMPVisAdapter::Render(VisParam *visParam)
{
	CEventVisDrawUpdate		*pEvent = new CEventVisDrawUpdate;

	// memcpy(&m_visParam, visParam, sizeof(m_visParam));

	pEvent->pVisParam = visParam; // &m_visParam;
	pEvent->eventType = ET_VIS_DRAW_UPDATE;
	CMPlayerAppBase::GetEventsDispatcher()->DispatchUnsyncEvent(pEvent);

	return ERR_OK;
}
