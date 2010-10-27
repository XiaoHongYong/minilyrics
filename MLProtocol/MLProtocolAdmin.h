#pragma once

#include "MLProtocol.h"
#include "../MLServer/LongDate.h"

//////////////////////////////////////////////////////////////////////////
// Protocols for Admin.

//
// 1) Get all lyrics update history
//
class MLMsgCmdGetLyrUpdateHistory : public CXMLMsgCmd
{
public:
	tstring			strLoginName;		// Admin login name and password
	tstring			strPwdMask;

	LONG_DATETIME	startDate;
	int				startUpdateId;
	int				reviewStatusFilter;	// ReviewStatus

public:
	MLMsgCmdGetLyrUpdateHistory();

	virtual int FromXML(SXNode *pNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};


//
// Returned Corresponding messages of 1) MLMsgCmdGetLyrUpdateHistory.
//
class MLMsgRetLyrUpdateHistory : public MLRetMsg
{
public:
	class UpdateItem
	{
	public:
		int				nUpdateIdStart;
		long			nLyrId;
		tstring			strLyrLink;
		tstring			strUploaders;

		virtual int FromXML(SXNode *pNode);
		virtual int ToXML(CXMLWriter &xmlFile);

	};
	typedef list<UpdateItem>			ListUpdate;

public:
	int						nNextStartUpdateId;
	ListUpdate				listUpdates;

public:
	MLMsgRetLyrUpdateHistory()
	{
		nNextStartUpdateId = 0;
	}

	virtual int FromXML(SXNode *pNode);
	virtual int ToXML(CXMLWriter &xmlFile);

	void AddUpdate(int updateId, int lyrId, LPCTSTR szUploader);

};

//
// Submit review of lyrics.
//
class MLMsgCmdSubmitReviewOfUpdateOfLyr : public CXMLMsgCmd
{
public:
	tstring			strLoginName;		// Admin login name and password
	tstring			strPwdMask;

	long			nLyricsId;
	long			nStartUpdateId;
	long			nRejectedUpdateId;
	long			nAcceptedUpdateId;

public:
	MLMsgCmdSubmitReviewOfUpdateOfLyr()
	{
		nLyricsId = nStartUpdateId = nRejectedUpdateId = nAcceptedUpdateId = 0;
	}

	virtual int FromXML(SXNode *pNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

class MLMsgRetSubmitReviewOfUpdateOfLyr : public MLRetMsg
{
};

//
// Execute command on MLServer
//
class MLMsgCmdExecute : public CXMLMsgCmd
{
public:
	enum Command
	{
		C_DEL_LYRICS,			// Parameter1: lyrics id
		C_ADD_BATCH_UPLOAD,		// Parameter1: related server side file path name.(Exp. upx3\\20101024.7z )
	};
public:
	tstring			strLoginName;		// Admin login name and password
	tstring			strPwdMask;
	Command			command;
	tstring			strParam1, strParam2;

public:
	virtual int FromXML(SXNode *pNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

class MLMsgRetExecute : public MLRetMsg
{
public:
	tstring			strMessage;

	virtual int FromXML(SXNode *pNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

