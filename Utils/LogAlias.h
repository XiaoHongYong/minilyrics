#pragma once

extern class CLog		g_Log;

#define LOG0(byLevel, szMessage)			\
	g_Log.WriteLog(byLevel, _T(__FILE__), __LINE__, szMessage)

#define LOG1(byLevel, sz, p1)			\
	g_Log.WriteLog(byLevel, _T(__FILE__), __LINE__, sz, p1)

#define LOG2(byLevel, sz, p1, p2)			\
	g_Log.WriteLog(byLevel, _T(__FILE__), __LINE__, sz, p1, p2)

#define LOG3(byLevel, sz, p1, p2, p3)			\
	g_Log.WriteLog(byLevel, _T(__FILE__), __LINE__, sz, p1, p2, p3)

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
