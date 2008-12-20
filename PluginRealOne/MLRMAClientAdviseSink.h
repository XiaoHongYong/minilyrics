// MLRMAClientAdviseSink.h: interface for the CMLRMAClientAdviseSink class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MLRMACLIENTADVISESINK_H__0703BD57_9C93_4295_8458_2364CD982441__INCLUDED_)
#define AFX_MLRMACLIENTADVISESINK_H__0703BD57_9C93_4295_8458_2364CD982441__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMLRMAClientAdviseSink : public IRMAClientAdviseSink  
{
public:
	CMLRMAClientAdviseSink();
	virtual ~CMLRMAClientAdviseSink();

    /*
     *	IUnknown methods
     */
    STDMETHOD(QueryInterface)		(THIS_
					REFIID riid,
					void** ppvObj);

    STDMETHOD_(ULONG32,AddRef)		(THIS);

    STDMETHOD_(ULONG32,Release)		(THIS);

    /*
     *	IRMAClientAdviseSink methods
     */

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnPosLength
     *	Purpose:
     *	    Called to advise the client that the position or length of the
     *	    current playback context has changed.
     */
    STDMETHOD(OnPosLength)		(THIS_
					UINT32	  ulPosition,
					UINT32	  ulLength);

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnPresentationOpened
     *	Purpose:
     *	    Called to advise the client a presentation has been opened.
     */
    STDMETHOD(OnPresentationOpened)	(THIS);

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnPresentationClosed
     *	Purpose:
     *	    Called to advise the client a presentation has been closed.
     */
    STDMETHOD(OnPresentationClosed)	(THIS);

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnStatisticsChanged
     *	Purpose:
     *	    Called to advise the client that the presentation statistics
     *	    have changed. 
     */
    STDMETHOD(OnStatisticsChanged)	(THIS);

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnPreSeek
     *	Purpose:
     *	    Called by client engine to inform the client that a seek is
     *	    about to occur. The render is informed the last time for the 
     *	    stream's time line before the seek, as well as the first new
     *	    time for the stream's time line after the seek will be completed.
     *
     */
    STDMETHOD(OnPreSeek)	(THIS_
				ULONG32		    ulOldTime,
				ULONG32		    ulNewTime);

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnPostSeek
     *	Purpose:
     *	    Called by client engine to inform the client that a seek has
     *	    just occurred. The render is informed the last time for the 
     *	    stream's time line before the seek, as well as the first new
     *	    time for the stream's time line after the seek.
     *
     */
    STDMETHOD(OnPostSeek)	(THIS_
				ULONG32		    ulOldTime,
				ULONG32		    ulNewTime);

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnStop
     *	Purpose:
     *	    Called by client engine to inform the client that a stop has
     *	    just occurred. 
     *
     */
    STDMETHOD(OnStop)		(THIS);

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnPause
     *	Purpose:
     *	    Called by client engine to inform the client that a pause has
     *	    just occurred. The render is informed the last time for the 
     *	    stream's time line before the pause.
     *
     */
    STDMETHOD(OnPause)		(THIS_
				ULONG32		    ulTime);

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnBegin
     *	Purpose:
     *	    Called by client engine to inform the client that a begin or
     *	    resume has just occurred. The render is informed the first time 
     *	    for the stream's time line after the resume.
     *
     */
    STDMETHOD(OnBegin)		(THIS_
				ULONG32		    ulTime);

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnBuffering
     *	Purpose:
     *	    Called by client engine to inform the client that buffering
     *	    of data is occuring. The render is informed of the reason for
     *	    the buffering (start-up of stream, seek has occurred, network
     *	    congestion, etc.), as well as percentage complete of the 
     *	    buffering process.
     *
     */
    STDMETHOD(OnBuffering)	(THIS_
				ULONG32		    ulFlags,
				UINT16		    unPercentComplete);

    /************************************************************************
     *	Method:
     *	    IRMAClientAdviseSink::OnContacting
     *	Purpose:
     *	    Called by client engine to inform the client is contacting
     *	    hosts(s).
     *
     */
    STDMETHOD(OnContacting)	(THIS_
				 const char*	pHostName);

	private:
		long	m_RefCount;
};

#endif // !defined(AFX_MLRMACLIENTADVISESINK_H__0703BD57_9C93_4295_8458_2364CD982441__INCLUDED_)
