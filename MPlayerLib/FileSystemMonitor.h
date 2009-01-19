/******************************************************************************
The idea here to keep media libarary are updated in time. The detail requirements are:

    1>  when zikiplayer is lauched, zikiplayer shall perform background search to collect all media info from file system.
    2>  While zikiplayer is active, zikiplayer shall monitor media file changes in file system.
    3>  File system here include, hard disk, USB disk, CD/DVD, remote shared folder, remote disk mounted to lock disk letter.
    4>  when seaching or monitoring find any changes, it will update zikiplayer immediately
    5>  seaching and motnitoring shall be intelligent enough to avoid conflication with other disk read/write opertion
*****************************************************************************/
#ifndef FILE_SYSTEM_MONITOR_H
#define FILE_SYSTEM_MONITOR_H






class CFileSystemMonitor
{
public:
	BOOL StartFullDiskSearch(void){};
	BOOL TurnOnFileSystemMonitor(BOOL bTurnOn){};

protected:
    // Platform depend implementation may override below call back functions
	virtual BOOL cbIsFileIntereasted(TCHAR* extension) = 0;
	virtual void cbOnNewFile(TCHAR* path, TCHAR* file){};
    virtual void cbOnEndOfFolder(TCHAR* path){};
};

#define SIZEOFARRAY(x) (sizeof(x)/sizeof(x[0]))






#endif //FILE_SYSTEM_MONITOR_H