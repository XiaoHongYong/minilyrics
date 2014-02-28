#pragma once

class CMLProtocolCmd
{
public:
	CMLProtocolCmd(void);
	~CMLProtocolCmd(void);

	void Init(LPCTSTR szClient);

	int ToSearchCommand(LPCTSTR szArtist, LPCTSTR szTitle, int nRequestPage, bool bOnlyMatched, CMLBinXMLWriter &xmlStream);

	int ToXmlCommand(CXMLMsgCmd &cmd, CMLBinXMLWriter &xmlStream);
	int DecodeSearchRetCommand(LPCTSTR szSongFile, LPCTSTR szArtist, LPCTSTR szTitle, CCharBuffer &buf, CXMLWriter &writer);

protected:
	void PrepareClientInfo(CXMLMsgCmd &cmd, bool bActivate);
	int ProcessCommonRetMsg(CCharBuffer &buf, MLRetMsg *pretMsg, MLMsgCmd cmdOrgSend);

	CMLPacketWrapper		m_packetWrapper;

	tstring			m_strLoginName;
	tstring			m_strPwdMask;
	tstring			m_strClient;

};
