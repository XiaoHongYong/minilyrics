/****************************************************************************
 *
 *  $Id: MLRealPlugin.h,v 1.14 1997/10/21 20:42:32 christof Exp $
 *
 *  Copyright (C) 1995,1996,1997 RealNetworks, Inc.
 *  All rights reserved.
 *  
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary information of RealNetworks, Inc.,
 *  and is licensed subject to restrictions on use and distribution.
 * 
 *  MLRealPlugin.h
 *
 *  Example plug-in which illustrates basic features of RMA.
 */
#ifndef _MLREALPLUGIN_H_
#define _MLREALPLUGIN_H_


/****************************************************************************
 * Constants
 */
#define MY_PLUGIN_VERSION 0
#define MY_DESCRIPTION   "MiniLyrics for realone Plug-in"
#define MY_COPYRIGHT     "(c) 2004 MiniLyrics, All rights reserved."
#define MY_MORE_INFO_URL "http://www.MiniLyrics.com"


/****************************************************************************
 *
 *  CMLRealPlugin Class
 *
 *  This class inherits the interfaces necessary to implement the familiar
 *  "Hello World" example. At a minimum, any plug-in must implement the
 *  IRMAPlugin interface to handle fundamental plug-in operations. Since
 *  this is a COM object, this class also inherits COM's IUnknown interface
 *  to handle object reference counting and interface query.
 */

class CMLRealPlugin :  public IRMAPlugin
{
	public:
	/****** Public Class Methods *******************************************/
	CMLRealPlugin();


	/************************************************************************
	 *  IRMAPlugin Interface Methods                         ref:  rmaplugn.h
	 */
	STDMETHOD(GetPluginInfo)
		(THIS_ 
		  REF(BOOL)        bLoadMultiple,
		  REF(const char*) pDescription,
		  REF(const char*) pCopyright,
		  REF(const char*) pMoreInfoURL,
		  REF(UINT32)      versionNumber
		);
	STDMETHOD(InitPlugin) (THIS_ IUnknown* pRMACore);


	/************************************************************************
	 *  IUnknown COM Interface Methods                          ref:  pncom.h
	 */
	STDMETHOD (QueryInterface ) (THIS_ REFIID ID, void** ppInterfaceObj);
	STDMETHOD_(UINT32, AddRef ) (THIS);
	STDMETHOD_(UINT32, Release) (THIS);


	private:
	/****** Private Class Variables ****************************************/
	INT32               m_RefCount; // Object's reference count
	
	/****** Private Static Class Variables *********************************/
	static const char* zm_pDescription;
	static const char* zm_pCopyright;
	static const char* zm_pMoreInfoURL;
};

#endif  /* ifndef _MLRealPlugin_H_ */

