#include "stdafx.h"
#include "MLProtocolAdmin.h"

//////////////////////////////////////////////////////////////////////////
MLMsgCmdGetLyrUpdateHistory::MLMsgCmdGetLyrUpdateHistory()
{
	startDate.dwTime = 0;
	startUpdateId = 0;
	reviewStatusFilter = 0;
}

int MLMsgCmdGetLyrUpdateHistory::FromXML(SXNode *pNode)
{
	strLoginName = pNode->GetPropertySafe(_T("user"));
	strPwdMask = pNode->GetPropertySafe(_T("password"));
	startDate.dwTime = pNode->GetPropertyInt(_T("startDate"));
	startUpdateId = pNode->GetPropertyInt(_T("startId"));
	reviewStatusFilter = pNode->GetPropertyInt(_T("reviewStatusFilter"));
	return ERR_OK;
}

int MLMsgCmdGetLyrUpdateHistory::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_GET_LYR_UPDATE);
	xmlFile.WriteAttribute(_T("user"), strLoginName.c_str());
	xmlFile.WriteAttribute(_T("password"), strPwdMask.c_str());
	xmlFile.WriteAttribute(_T("startDate"), startDate.dwTime);
	xmlFile.WriteAttribute(_T("startId"), startUpdateId);
	xmlFile.WriteAttribute(_T("reviewStatusFilter"), reviewStatusFilter);
	xmlFile.WriteEndElement();
	return ERR_OK;
}

#define MZ_UPDATE_ITEM	_T("item")

//////////////////////////////////////////////////////////////////////////
int MLMsgRetLyrUpdateHistory::UpdateItem::FromXML(SXNode *pNode)
{
	nUpdateIdStart = pNode->GetPropertyInt(_T("UpdateIdStart"));
	nLyrId = pNode->GetPropertyInt(_T("LyrId"));
	strLyrLink = pNode->GetPropertySafe(_T("Link"));
	strUploaders = pNode->GetPropertySafe(_T("Uploaders"));
	return ERR_OK;
}

int MLMsgRetLyrUpdateHistory::UpdateItem::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_UPDATE_ITEM);
	xmlFile.WriteAttribute(_T("UpdateIdStart"), nUpdateIdStart);
	xmlFile.WriteAttribute(_T("LyrId"), nLyrId);
	xmlFile.WriteAttribute(_T("Link"), strLyrLink.c_str());
	xmlFile.WriteAttribute(_T("Uploaders"), strUploaders.c_str());
	xmlFile.WriteEndElement();
	return ERR_OK;
}

int MLMsgRetLyrUpdateHistory::FromXML(SXNode *pNode)
{
	int nRet = MLRetMsg::FromXML(pNode);
	if (nRet != ERR_OK)
		return nRet;

	nNextStartUpdateId = pNode->GetPropertyInt(_T("NextUpdateIdStart"));

	SXNode::iterator	it, itEnd;
	itEnd = pNode->listChildren.end();
	for (it = pNode->listChildren.begin(); it != itEnd; ++it)
	{
		UpdateItem item;

		ASSERT(_tcscmp((*it)->strName.c_str(), MZ_UPDATE_ITEM) == 0);
		item.FromXML(*it);
		listUpdates.push_back(item);
	}

	return ERR_OK;
}

int MLMsgRetLyrUpdateHistory::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_RETURN);
	MLRetMsg::ToXMLAttribute(xmlFile);

	xmlFile.WriteAttribute(_T("NextUpdateIdStart"), nNextStartUpdateId);

	for (ListUpdate::iterator it = listUpdates.begin(); it != listUpdates.end(); ++it)
	{
		(*it).ToXML(xmlFile);
	}
	xmlFile.WriteEndElement();
	return ERR_OK;
}

void MLMsgRetLyrUpdateHistory::AddUpdate(int updateId, int lyrId, LPCTSTR szUploader)
{
	for (ListUpdate::iterator it = listUpdates.begin(); it != listUpdates.end(); ++it)
	{
		UpdateItem &item = *it;
		if (item.nLyrId == lyrId)
		{
			item.strUploaders += _T(",");
			item.strUploaders += szUploader;
			return;
		}
	}

	UpdateItem item;
	item.nUpdateIdStart = updateId;
	item.nLyrId = lyrId;
	item.strUploaders = szUploader;
	listUpdates.push_back(item);
}

//////////////////////////////////////////////////////////////////////////

void GetCommaIntArray(LPCTSTR str, vector<int> &vValues)
{
	vValues.clear();
	if (!str)
		return;

	V_Str	vStr;
	StrSplit(str, ',', vStr);

	for (int i = 0; i < (int)vStr.size(); i++)
	{
		vValues.push_back(_ttoi(vStr[i].c_str()));
	}
}

tstring IntArrayToCommaStr(vector<int> &vValues)
{
	tstring		str;

	for (int i = 0; i < (int)vValues.size(); i++)
	{
		if (i != 0)
			str += _T(",");
		str += CStrPrintf(_T("%d"), vValues[i]).c_str();
	}

	return str;
}

int MLMsgCmdSubmitReviewOfUpdateOfLyr::FromXML(SXNode *pNode)
{
	strLoginName = pNode->GetPropertySafe(_T("user"));
	strPwdMask = pNode->GetPropertySafe(_T("password"));
	nLyricsId = pNode->GetPropertyInt(_T("LyricsId"));
	nStartUpdateId = pNode->GetPropertyInt(_T("StartUpdateId"));
	nRejectedUpdateId = pNode->GetPropertyInt(_T("RejectedUpdateId"));
	nAcceptedUpdateId = pNode->GetPropertyInt(_T("AcceptedUpdateId"));

	return ERR_OK;
}

int MLMsgCmdSubmitReviewOfUpdateOfLyr::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_SUBMIT_LYR_REVIEW);
	xmlFile.WriteAttribute(_T("user"), strLoginName.c_str());
	xmlFile.WriteAttribute(_T("password"), strPwdMask.c_str());
	xmlFile.WriteAttribute(_T("LyricsId"), nLyricsId);
	xmlFile.WriteAttribute(_T("StartUpdateId"), nStartUpdateId);
	xmlFile.WriteAttribute(_T("RejectedUpdateId"), nRejectedUpdateId);
	xmlFile.WriteAttribute(_T("AcceptedUpdateId"), nAcceptedUpdateId);
	xmlFile.WriteEndElement();

	return ERR_OK;

}

int MLMsgCmdExecute::FromXML(SXNode *pNode)
{
	strLoginName = pNode->GetPropertySafe(_T("user"));
	strPwdMask = pNode->GetPropertySafe(_T("password"));
	command = (Command)pNode->GetPropertyInt(_T("cmd"));
	strParam1 = pNode->GetPropertySafe(_T("p1"));
	strParam2 = pNode->GetPropertySafe(_T("p2"));

	return ERR_OK;
}

int MLMsgCmdExecute::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_EXE_SVR_CMD);
	xmlFile.WriteAttribute(_T("user"), strLoginName.c_str());
	xmlFile.WriteAttribute(_T("password"), strPwdMask.c_str());
	xmlFile.WriteAttribute(_T("cmd"), command);
	xmlFile.WriteAttribute(_T("p1"), strParam1.c_str());
	xmlFile.WriteAttribute(_T("p2"), strParam2.c_str());
	xmlFile.WriteEndElement();

	return ERR_OK;

}

int MLMsgRetExecute::FromXML(SXNode *pNode)
{
	int nRet = MLRetMsg::FromXML(pNode);
	if (nRet != ERR_OK)
		return nRet;

	strMessage = pNode->GetPropertySafe(_T("msg"));

	return ERR_OK;
}

int MLMsgRetExecute::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_RETURN);
	MLRetMsg::ToXMLAttribute(xmlFile);

	xmlFile.WriteAttribute(_T("msg"), strMessage.c_str());

	xmlFile.WriteEndElement();
	return ERR_OK;
}
