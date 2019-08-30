#pragma once

#include "../../MLLib/HelperFun.h"
#include "../../MLLib/LrcTag.h"
#include "SpamLyricsFilter.h"
#include "NgxOutputStream.h"
#include "mlserver_4_ngx.h"

class CVectorLrcInfo;
class CLyricsInfo;


class MLServer
{
public:
	MLServer();
	virtual ~MLServer();

public:
	virtual int Init(const MLServerNgxConf *conf);
	virtual int Process(uint8_t *data, size_t len, NgxOutputStream &outStream);

	int ProcessMLMsgCmd(CCharBuffer &buffer);

	int ProcessSearchCmd(MLMsgCmdSearch &cmdSearch, MLMsgRetSearch &retSearch);

	int ProcessBatchSearchCmd(MLMsgCmdBatchSearch &cmdSearch, MLMsgRetBatchSearch &retSearch);

	int ProcessUploadCmdV0(MLMsgCmdUploadV0 &cmdUpload, MLMsgRetUploadV0 &retMsg);

	int ProcessUploadCmd(MLMsgCmdUpload &cmdUpload, MLMsgRetUpload &retMsg);
	int ProcessUploadCmd(tstring &strLyrContent, MLMsgRetUploadV0 &retMsg, 
		long nUploaderId, LPCTSTR szUploader, tstring &strLyrId);

	int ProcessActivateLicenseCmd(MLMsgCmdActivateLicense &cmd, MLMsgRetActivateLicense &retMsg);

protected:
	int SearchMatchedLyricsOnly(LPCSTR szArCmp, LPCSTR szTiCmp, RetLyrInfoList &listLyr);

	int SearchBestMatchLyrics(LPCSTR szArtist, LPCTSTR szAlbum, LPCSTR szTitle, int nMediaLength, class MLLyricsInfoLite &infoLite);

	int SaveUploadedLyricsFile(long nLyricsId, const LyricsProperties &lyrProperties, stringex &strLyrContentUtf8, tstring &strFile);

	int VerifyDeviceLicenseInfo(CXMLMsgCmd &cmd, MLRetMsg &retMsg);

	int ProcessAppAnalyticsPut(CCharBuffer &bufData, CCharBuffer &bufRet);

	int ProcessAppAnalyticsGenReport(CCharBuffer &bufData, CCharBuffer &bufRet);

public:
    CMLPacketWrapper        m_packetWrapper;

    string                  m_lyricsDir;
    string                  m_relatedUploadDir;
    string                  m_lyricsHttpLinkBase;

    CSpamLyricsFilter       m_spamFilter;

};
