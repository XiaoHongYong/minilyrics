#include "../../MLProtocol/StdAfx.h"
#include "../../MLProtocol/MLProtocol.h"
#include "../../MLLib/MLLib.h"

#include "MLProtocolCmd.h"

#define CUR_ACCOUNT_ID		4
#define CUR_ACCOUNT_NAME	"Mlv1clt4.0"

void RetLyrInfoListToVResult(CLyricsSearchParameter &searchParam, RetLyrInfoList &filelist, V_LRCSEARCHRESULT &vResult)
{
	for (RetLyrInfoList::iterator it = filelist.begin(); it != filelist.end(); it++)
	{
		RetLyrInfo		&file = *it;
		LrcSearchResult		item;
		item.strUrl = file.strLink;
		item.strSaveFileName = file.GetFileName();
		item.strArtist = file.strArtist;
		item.strTitle = file.strTitle;
		item.strAlbum = file.strAlbum;
		item.strUploader = file.strUploader;
		item.fRate = file.fRate;
		item.nRateCount = file.nRateCount;
		item.nDownloads = file.nDownloads;
		item.nMatchValue = (float)searchParam.CalMatchValueByTitle(item.strArtist.c_str(), item.strTitle.c_str(), MLGetFileType(item.strSaveFileName.c_str()));
		// DBGLOG2(_T("file: %s, value: %d"), item.strSaveFileName.c_str(), item.nMatchValue);

		// Sort English result before other language.
		if (IsAnsiStr(item.strSaveFileName.c_str()))
			item.nMatchValue += 0.5;

		// increase rating
		if (item.nRateCount > 0)
		{
			item.nMatchValue += item.fRate - 3;
			if (item.nRateCount > 5)
			{
				if (item.fRate >= 4.8)
					item.nMatchValue++;
				else if (item.fRate <= 3.0)
					item.nMatchValue--;
			}
			item.nMatchValue += item.nRateCount / (float)1000.0;
		}

		vResult.AddResult(item);
	}

	vResult.Sort();
}

void ToString(V_LRCSEARCHRESULT &vResult, CXMLWriter &writer);

CMLProtocolCmd::CMLProtocolCmd(void)
{
}

CMLProtocolCmd::~CMLProtocolCmd(void)
{
}

void CMLProtocolCmd::Init(LPCTSTR szClient)
{
	CMLPacketAccountMgr *pMgr = CMLPacketAccountMgr::GetInstance();
	pMgr->AddAccount(CUR_ACCOUNT_ID, CUR_ACCOUNT_NAME);
	m_packetWrapper.SetId(CUR_ACCOUNT_ID);
	pMgr->SetMode(FALSE, MPV_V1_MD5_ID);

	m_strClient = szClient;
}

int CMLProtocolCmd::ToSearchCommand(LPCTSTR szArtist, LPCTSTR szTitle, int nRequestPage, bool bOnlyMatched, CMLBinXMLWriter &xmlStream)
{
	int			nRet;
	MLMsgCmdSearch	cmdSearch;

	cmdSearch.strTitle = szTitle;
	cmdSearch.strArtist = szArtist;
	cmdSearch.nRequestPage = nRequestPage;
	cmdSearch.bOnlyMatchedLyrics = bOnlyMatched;

	PrepareClientInfo(cmdSearch, false);

	return ToXmlCommand(cmdSearch, xmlStream);
}

void CMLProtocolCmd::PrepareClientInfo(CXMLMsgCmd &cmd, bool bActivate)
{
	cmd.strClient = m_strClient;
	cmd.nProductId = PRODUCT_ID_MINILYRICS;
#ifndef _ANDROID
	if (g_licenseManager.GetLicenseStatus() == LS_ACTIVE || bActivate)
	{
		CLicenseManager::LicenseInfo	licenseInfo;
		g_licenseManager.GetLicenseInfo(licenseInfo);

		cmd.strRegCode = licenseInfo.strCode;
		strrep(cmd.strRegCode, _T("-"), _T(""));
		cmd.strDeviceID = licenseInfo.strDeviceID;
		cmd.bOnlineVerified = licenseInfo.bOnlineVerified;
	}
#endif // #ifdef _IPHONE
}

int CMLProtocolCmd::ToXmlCommand(CXMLMsgCmd &cmd, CMLBinXMLWriter &xmlStream)
{
	int nRet = cmd.ToXML(xmlStream);
	if (nRet != ERR_OK)
		return nRet;

	return m_packetWrapper.Wrapp(xmlStream.GetBuffer());
}

int CMLProtocolCmd::ProcessCommonRetMsg(CCharBuffer &buf, MLRetMsg *pretMsg, MLMsgCmd cmdOrgSend)
{
	int nRet = m_packetWrapper.Unwrapp(buf);
	if (nRet != ERR_OK)
		return nRet;

	if (buf.c_str()[0] != 'M')
		return ERR_BAD_MSG;

	CSimpleXML		xml;
	if (!xml.ParseData(buf.data(), buf.size()))
		return ERR_PARSE_XML;

	if (_tcscmp(xml.m_pRoot->strName.c_str(), MZ_RETURN) != 0)
		return ERR_CMD_NOT_MATCH;

	nRet = pretMsg->FromXML(xml.m_pRoot);
	if (nRet != ERR_OK)
		return nRet;

	MLMsgCmd		cmdOrg = MLMsgStr2Cmd(pretMsg->strOrgCmd.c_str());
	if (cmdOrg != cmdOrgSend)
		return ERR_CMD_NOT_MATCH;

	return pretMsg->result;
}

int CMLProtocolCmd::DecodeSearchRetCommand(LPCTSTR szSongFile, LPCTSTR szArtist, LPCTSTR szTitle, CCharBuffer &buf, CXMLWriter &writer)
{
	MLMsgRetSearch retSearch;
	int nRet = ProcessCommonRetMsg(buf, &retSearch, MC_SEARCH);

	V_LRCSEARCHRESULT vResult;
	CLyricsSearchParameter searchParam(szSongFile, szArtist, szTitle);
	RetLyrInfoListToVResult(searchParam, retSearch.listResultFiles, vResult);

	writer.WriteStartElement("result");

	if (retSearch.nCurPage > 0)
		writer.WriteAttribute("cur_page", retSearch.nCurPage);
	if (retSearch.nPageCount > 1)
		writer.WriteAttribute("page_count", retSearch.nPageCount);
	writer.WriteAttribute("server_url", retSearch.strServerUrl.c_str());

	if (nRet != ERR_OK)
		writer.WriteAttribute("message", Error2Str(nRet).c_str());
	else if (retSearch.strMessage.size() > 0)
		writer.WriteAttribute("message", retSearch.strMessage.c_str());	

	ToString(vResult, writer);
	writer.WriteEndElement();

	return ERR_OK;
}
