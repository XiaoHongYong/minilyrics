#pragma once

class CDlgUpload : public CSkinDialog
{
public:
	CDlgUpload(tstring &strLyricsContent, LPCTSTR szMediaSource, LPCTSTR szLyrSource);
	~CDlgUpload();

protected:
	class CUploadWorkObj		*m_pUploadWorkObj;

};

