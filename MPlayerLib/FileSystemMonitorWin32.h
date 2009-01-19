/******************************************************************************
Implement CFileSystemMonitor on Win32 Platform
*****************************************************************************/
#ifndef FILE_SYSTEM_MONITOR_WIN32_H
#define FILE_SYSTEM_MONITOR_WIN32_H


#include "filesystemmonitor.h"



class CFileSystemMonitorWin32: public CFileSystemMonitor
{
public:
    CFileSystemMonitorWin32():m_bMonitorOn(FALSE), m_hWorkingThread(NULL),m_bDiskQueueIdle(FALSE){};

public:
	BOOL StartFullDiskSearch(void);
	BOOL TurnOnFileSystemMonitor(BOOL bTurnOn);

protected:
    // Platform depend implementation may override below call back functions
	virtual BOOL cbIsFileIntereasted(TCHAR* extension);
	virtual void cbOnNewFile(TCHAR* path, TCHAR* file);
    virtual void cbOnEndOfFolder(TCHAR* path);

protected:
    static unsigned int __stdcall cbFullDiskSearchThread(void * param);

protected:
    BOOL        m_bMonitorOn;
    uintptr_t   m_hWorkingThread;
    BOOL        m_bDiskQueueIdle;
};



#endif //FILE_SYSTEM_MONITOR_WIN32_H
