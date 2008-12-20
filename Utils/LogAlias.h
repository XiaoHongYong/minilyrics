/********************************************************************
	Created  :	2002/02/01	20:08
	FileName :	Log.h
	Author   :	xhy
	
	Purpose  :	日志文件的操作以及定义

	Comment  :  

	1）日志信息分为下面几个等级（0 － 255）：
		ID		描述
		1		致命错误或异常
		2		一般错误或异常
		3		提示错误或异常
		4		重要操作事件
		5		一般事件
		6		调试信息
		等级 0 － 16 保留，16 以后的可以自由使用，并且只有用户设置了某类型才会使用

	3）能够记录当前程序的状态

	4）

	使用的注意事项：
	1、习惯使用CString，string等类的用户来说，注意类型的转换，要转换为(LPCTSTR)类型，
		即const char *类型。否则会发生异常。
*********************************************************************/
#if !defined(AFX_LOG_H__F56B1F82_174C_11D6_B478_FFFFFF000000__INCLUDED_)
#define AFX_LOG_H__F56B1F82_174C_11D6_B478_FFFFFF000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern class CLog		g_Log;

#define LOG0(byLevel, szMessage)			\
	g_Log.WriteLog(byLevel, _T(__FILE__), __LINE__, szMessage)

#define LOG1(byLevel, sz, p1)			\
	g_Log.WriteLog(byLevel, _T(__FILE__), __LINE__, sz, p1)

#define LOG2(byLevel, sz, p1, p2)			\
	g_Log.WriteLog(byLevel, _T(__FILE__), __LINE__, sz, p1, p2)

#define LOG3(byLevel, sz, p1, p2, p3)			\
	g_Log.WriteLog(byLevel, _T(__FILE__), __LINE__, sz, p1, p2, p3)

#define LOGOUT0(byLevel, szMessage)			\
	g_Log.LogOut(byLevel, _T(__FILE__), __LINE__, szMessage)

#define LOGOUT1(byLevel, sz, p1)			\
	g_Log.LogOut(byLevel, _T(__FILE__), __LINE__, sz, p1)

#define LOGOUT2(byLevel, sz, p1, p2)			\
	g_Log.LogOut(byLevel, _T(__FILE__), __LINE__, sz, p1, p2)

#define LOGOUT3(byLevel, sz, p1, p2, p3)			\
	g_Log.LogOut(byLevel, _T(__FILE__), __LINE__, sz, p1, p2, p3)

#define ERRLOG0(szMessage)			\
	g_Log.WriteLog(LOG_LVL_ERROR, _T(__FILE__), __LINE__, szMessage)

#define ERRLOG1(sz, p1)			\
	g_Log.WriteLog(LOG_LVL_ERROR, _T(__FILE__), __LINE__, sz, p1)

#define ERRLOG2(sz, p1, p2)			\
	g_Log.WriteLog(LOG_LVL_ERROR, _T(__FILE__), __LINE__, sz, p1, p2)

#define ERRLOG3(sz, p1, p2, p3)			\
	g_Log.WriteLog(LOG_LVL_ERROR, _T(__FILE__), __LINE__, sz, p1, p2, p3)

#define ERRLOGDUMPS(sz, len)			\
	g_Log.LogDumpStr(LOG_LVL_ERROR, _T(__FILE__), __LINE__, sz, len)

#ifdef _DEBUG
#define DBGLOG0(szMessage)			\
	g_Log.WriteLog(LOG_LVL_DEBUG, _T(__FILE__), __LINE__, szMessage)

#define DBGLOG1(sz, p1)			\
	g_Log.WriteLog(LOG_LVL_DEBUG, _T(__FILE__), __LINE__, sz, p1)

#define DBGLOG2(sz, p1, p2)			\
	g_Log.WriteLog(LOG_LVL_DEBUG, _T(__FILE__), __LINE__, sz, p1, p2)

#define DBGLOG3(sz, p1, p2, p3)			\
	g_Log.WriteLog(LOG_LVL_DEBUG, _T(__FILE__), __LINE__, sz, p1, p2, p3)

#define DBGLOGDUMPS(sz, len)			\
	g_Log.LogDumpStr(LOG_LVL_DEBUG, _T(__FILE__), __LINE__, sz, len)

#else
#define DBGLOG0(szMessage)			(0)

#define DBGLOG1(sz, p1)				(0)

#define DBGLOG2(sz, p1, p2)			(0)

#define DBGLOG3(sz, p1, p2, p3)		(0)

#define DBGLOGDUMPS(sz, len)		(0)

#endif


#endif // !defined(AFX_LOG_H__F56B1F82_174C_11D6_B478_FFFFFF000000__INCLUDED_)
