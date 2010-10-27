#pragma once

#include "MPSkinDialog.h"

class CDlgUpload : public CMPSkinDialog
{
public:
	CDlgUpload(tstring &strLyricsContent, LPCTSTR szMediaSource, LPCTSTR szLyrSource);
	~CDlgUpload();

protected:
	class CUploadWorkObj		*m_pUploadWorkObj;

};

