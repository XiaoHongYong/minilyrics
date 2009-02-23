/******************************************************************************
Implement CFileSystemMonitor on Win32 Platform
*****************************************************************************/
#ifndef FILE_SYSTEM_MONITOR_WIN32_H
#define FILE_SYSTEM_MONITOR_WIN32_H


#include "filesystemmonitor.h"
//#include "mutex.h"

#define MAX_DRIVER_SUPPORT  ('Z' - 'A' + 1)
#define DRIVE_STR_BUF_SIZE	((3 + 1) * MAX_DRIVER_SUPPORT + 1)
#define DRIVE_STR_STR_LEN	(3 * MAX_DRIVER_SUPPORT)

class CFileSystemMonitorWin32: public CFileSystemMonitor
{
public:
    CFileSystemMonitorWin32():m_hWorkingThread(NULL), m_bDiskQueueIdle(TRUE), m_bMonitorOn(FALSE), m_count(0){};
    ~CFileSystemMonitorWin32(){};

public:
	virtual BOOL StartFullDiskSearch(void);
	virtual BOOL TurnOnFileSystemMonitor(BOOL bTurnOn);

protected:
    // Platform depend implementation may override below call back functions
    // for full disk search, cbOnNewFile will always be called with any files, Media libarary need to check if the file is a new one, or exisitng one
    // for file system monitor, when cbOnNewFile is called, there are new file add to Media Library; when cbOnRemoveFile is called there are file deleted.
	virtual BOOL cbIsFileIntereasted(const TCHAR* szFile);
	virtual void cbOnNewFile(const TCHAR* path, const TCHAR* file);
    virtual void cbOnEndOfFolder(const TCHAR* path);
    virtual void cbOnRemoveFile(const TCHAR* path, const TCHAR* file);



protected:
    void SearchFolder(const TCHAR* szStr, BOOL bRoot);
    static unsigned int __stdcall SeachWholeDisk(void * param);
    static unsigned int __stdcall MonitorAFolder(void * param);

protected:
    HANDLE		m_hWorkingThread;               // cbFullDiskSearchThread() thread
    BOOL        m_bDiskQueueIdle;

    BOOL        m_bMonitorOn;                   // if the monitor is on
    HANDLE      m_aHandle[MAX_DRIVER_SUPPORT];  // bounch of MonitorAFolder() thread
    int         m_count;                        // count of thread created
};



#endif //FILE_SYSTEM_MONITOR_WIN32_H
