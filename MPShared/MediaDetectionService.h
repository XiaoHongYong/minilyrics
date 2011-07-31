#pragma once

/*/////////////////////////////////////////////////////////////////////////

Detection the changes of media library or new media, add/update media library.

*//////////////////////////////////////////////////////////////////////////

class CMediaDetectionService
{
public:
	CMediaDetectionService(void);
	virtual ~CMediaDetectionService(void);

	void AddMediaInDir(LPCTSTR szDir);
	void AddMedia(vector<tstring> &vFiles);

};

extern CMediaDetectionService		g_mediaDetectionService;