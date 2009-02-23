/******************************************************************************
The idea here to keep media libarary are updated in time. The detail requirements are:
    1>  每次软件启动之后都做一次低优先级的后台扫描，保证媒体库的正确性
    2>  软件在运行过程中需要高优先级的监控所有文件夹的改动，保证用户最近的改动能够及时反映

    3>  后台扫描和文件夹监控都包含，所有硬盘，U盘，CD/DVD,软驱，和网络驱动器
    4>  后台扫描和文件夹监控过程中发现有新文件就立即自动添加到媒体库（这个和自动播放不同，只是将他们加入媒体库的all这个list，而不提示用户）
    5>  后台扫描和文件夹监控过程中发现文件被删除通知媒体库。（对于U盘，CD/DVD，媒体库不应该清除对应歌曲的信息，而是致上一个标志位，等待以后再次恢复）
*****************************************************************************/
#ifndef FILE_SYSTEM_MONITOR_H
#define FILE_SYSTEM_MONITOR_H





///////////////////////////////////////////////////////////////////////////////
class CFileSystemMonitor
{
public:
	virtual BOOL StartFullDiskSearch(void) = 0;
	virtual BOOL TurnOnFileSystemMonitor(BOOL bTurnOn)= 0;

protected:
    // Platform depend implementation may override below call back functions
	virtual BOOL cbIsFileIntereasted(const TCHAR* szFile){return FALSE;};
	virtual void cbOnNewFile(const TCHAR* path, const TCHAR* file){};
    virtual void cbOnEndOfFolder(const TCHAR* path){};
    virtual void cbOnRemoveFile(const TCHAR* path, const TCHAR* file){};

private:

#if 0 
	typedef enum{
		FSM_HARD_DISK,
		FSM_REMOVABLE_DISK,
		FSM_NETWORK_DISK,
	} DiskType;
	
	typedef struct{
		CXStr		name;
		CXStr		letter;
		DiskType	type;
	}Disks;
#endif
};
///////////////////////////////////////////////////////////////////////////////


#define SIZEOFARRAY(x) (sizeof(x)/sizeof((x)[0]))






#endif //FILE_SYSTEM_MONITOR_H