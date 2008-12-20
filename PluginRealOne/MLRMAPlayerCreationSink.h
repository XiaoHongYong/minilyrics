// MLRMAPlayerCreationSink.h: interface for the CMLRMAPlayerCreationSink class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MLRMAPLAYERCREATIONSINK_H__94464039_FC55_4C99_9D2A_21552885E661__INCLUDED_)
#define AFX_MLRMAPLAYERCREATIONSINK_H__94464039_FC55_4C99_9D2A_21552885E661__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMLRMAPlayerCreationSink : public IRMAPlayerCreationSink  
{
public:
	CMLRMAPlayerCreationSink();
	virtual ~CMLRMAPlayerCreationSink();
    /*
     * IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_ REFIID riid, void** ppvObj);
    STDMETHOD_(UINT32,AddRef)		(THIS);
    STDMETHOD_(UINT32,Release)		(THIS);

    /*
     * IRMAPlayerCreationSink Methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAPlayerCreationSink::PlayerCreated
     *	Purpose:
     *	    Notification when a new player is created
     *
     */
    STDMETHOD(PlayerCreated)		(THIS_
					IRMAPlayer* pPlayer);
    
    /************************************************************************
     *	Method:
     *	    IRMAPlayerCreationSink::PlayerClosed
     *	Purpose:
     *	    Notification when an exisitng player is closed
     *
     */
    STDMETHOD(PlayerClosed)		(THIS_
					IRMAPlayer* pPlayer);

private:
	/****** Private Class Variables ****************************************/
	INT32               m_RefCount; // Object's reference count

};

#endif // !defined(AFX_MLRMAPLAYERCREATIONSINK_H__94464039_FC55_4C99_9D2A_21552885E661__INCLUDED_)
