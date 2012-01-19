// idstring.cpp: implementation of the CLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseDate.h"
#include "base.h"

static const int _MonthDays[12] =
{
	0,
	31, 
	31 + 28, 
	31 + 28 + 31, 
	31 + 28 + 31 + 30, 
	31 + 28 + 31 + 30 + 31, 
	31 + 28 + 31 + 30 + 31 + 30, 
	31 + 28 + 31 + 30 + 31 + 30 + 31, 
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31, 
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30, 
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31, 
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30
};

inline int GetDaysToYear(int year)
{
	year--;
	return year * 365 + year / 4 - year / 100 + year / 400;
}

inline int GetDaysToMonth(int year, int month)
{
	ASSERT(month >= 1 && month <= 12);
	int n = _MonthDays[month - 1];
	if (CBaseDate::IsLeapYear(year) && month > 2)
		n++;
	return n;
}

CBaseDate::CBaseDate()
{
	this->day = 0;
	this->month = 0;
	this->year = 0;
	this->hour = 0;
	this->minute = 0;
	this->second = 0;
	this->ms = 0;
}

CBaseDate::CBaseDate(int year, int month, int day)
{
	this->day = (byte)day;
	this->month = (byte)month;
	this->year = (WORD)year;
	this->hour = 0;
	this->minute = 0;
	this->second = 0;
	this->ms = 0;
}

CBaseDate::CBaseDate(int year, int month, int day, int hour, int minute, int second, int ms)
{
	this->day = (byte)day;
	this->month = (byte)month;
	this->year = (WORD)year;
	this->hour = (byte)hour;
	this->minute = (byte)minute;
	this->second = (byte)second;
	this->ms = (WORD)ms;
}

uint32 CBaseDate::GetTime()
{
	if (month <= 0 || month > 12)
		return 0;

	uint32 nAllDays = GetDaysToYear(year) + day - 1;

	nAllDays += GetDaysToMonth(year, month);

	return (nAllDays - GetDaysToYear(1970)) * SECOND_IN_ONE_DAY + ((hour * 60 + minute) * 60 + second);
}

void CBaseDate::FromTime(uint32 timeInMillis)
{
	ms = 0;

	second = timeInMillis % 60;
	timeInMillis /= 60;

	minute = timeInMillis % 60;
	timeInMillis /= 60;

	hour = timeInMillis % 24;
	timeInMillis /= 24;

	// Now, timeInMillis is Days.
	uint32 daysTotal = timeInMillis + GetDaysToYear(1970);

	// estimate the start year.
	year = timeInMillis / 365 + 1970;
	uint32 daysToYear = GetDaysToYear(year);
	while (daysToYear <= daysTotal)
	{
		daysToYear += 365;
		if (IsLeapYear(year))
			daysToYear++;
		year++;
	}
	year--;

	int dayOfYear = daysTotal - GetDaysToYear(year);
	ASSERT(dayOfYear >= 0);
	for (month = 1; month <= 12; month++)
	{
		if ((int)dayOfYear < GetDaysToMonth(year, month))
			break;
	}
	month--;

	day = dayOfYear - GetDaysToMonth(year, month);
	day++;
}

uint32 CBaseDate::GetMLDate()
{
	return (year << 16) | (month << 8) | day;
}

void CBaseDate::FromMLDate(uint32 mlDate)
{
	year = (WORD)(mlDate >> 16);
	month = (byte)((mlDate >> 8) & 0xFF);
	day = (byte)(mlDate & 0xFF);

	ms = hour = minute = second = 0;
}

enum
{
	S_MINUTE	= 60,
	S_HOUR		= (60 * 60),
	S_DAY		= (60 * 60 * 24),
	S_MONTH		= (60 * 60 * 24 * 31),
	S_YEAR		= (60 * 60 * 24 * 31 * 12),

	BEGIN_OF_DATE	= 1970
};

uint32 CBaseDate::GetMLDateTime()
{
	return hour * S_HOUR + minute * 60 + second
		+ (year - BEGIN_OF_DATE) * S_YEAR + (month - 1) * S_MONTH + (day - 1) * S_DAY;
}

void CBaseDate::FromMLDateTime(uint32 mlDateTime)
{
	year = (WORD)(mlDateTime / S_YEAR + BEGIN_OF_DATE);
	month = (byte)((mlDateTime % S_YEAR) / S_MONTH) + 1;
	day = (byte)((mlDateTime % S_MONTH) / S_DAY) + 1;
	hour = (byte)((mlDateTime % S_DAY) / S_HOUR);
	minute = (byte)((mlDateTime % S_HOUR) / S_MINUTE);
	second = (byte)(mlDateTime % S_MINUTE);
	ms = 0;
}

tstring CBaseDate::ToUtcDateString()
{
	TCHAR szDate[64];
	_stprintf(szDate, _T("%04d-%02d-%02d"), year, month, day);
	return szDate;
}

tstring CBaseDate::ToUtcDateTimeString()
{
	TCHAR szDate[64];
	_stprintf(szDate, _T("%04d-%02d-%02d %02d:%02d.%02d"), year, month, day, hour, minute, second);
	return szDate;
}

bool CBaseDate::FromString(LPCTSTR szDate)
{
	// year
	szDate = ReadInt_t(szDate, year);
	if (*szDate != '-' && *szDate != '/')
		return false;
	szDate++;

	// month
	szDate = ReadInt_t(szDate, month);
	if (*szDate != '-' && *szDate != '/')
		return false;
	szDate++;

	// day
	szDate = ReadInt_t(szDate, day);

	while (*szDate == ' ')
		szDate++;

	if (!*szDate)
	{
		ms = hour = minute = second = 0;
		return true;
	}

	// hour
	szDate = ReadInt_t(szDate, hour);
	if (*szDate != ':')
		return false;
	szDate++;

	// minute
	szDate = ReadInt_t(szDate, minute);
	if (*szDate != ':' && *szDate != '.')
		return false;
	szDate++;

	// second
	szDate = ReadInt_t(szDate, second);

	ms = 0;
	return true;
}

CBaseDate CBaseDate::GetCurrentDate()
{
#ifdef _WIN32
	SYSTEMTIME	sysTime;

	GetSystemTime(&sysTime);

	return CBaseDate(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
#else
	time_t		ltime;
	tm			*ptm;

	time(&ltime);
	ptm = gmtime(&ltime);

	return CBaseDate(ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, 0);
#endif
}

CBaseDate CBaseDate::GetLocalTime()
{
#ifdef _WIN32
	SYSTEMTIME	sysTime;

	::GetLocalTime(&sysTime);

	return CBaseDate(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
#else
	time_t		ltime;
	tm			*ptm;

	time(&ltime);
	ptm = localtime(&ltime);

	return CBaseDate(ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, 0);
#endif
}

bool CBaseDate::IsLeapYear(int year)
{
	if (year % 4 != 0)
		return false;

	if (year % 400 == 0)
		return true;
	if (year % 100 == 0)
		return false;

	return true;
}


//////////////////////////////////////////////////////////////////////////
// CPPUnit test
#ifdef _CPPUNIT_TEST

#include <time.h>

IMPLEMENT_CPPUNIT_TEST_REG(CBaseDate)

class CTestCaseCBaseDate : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseCBaseDate);
	CPPUNIT_TEST(TestFromTime);
	CPPUNIT_TEST(TestFromString);
	CPPUNIT_TEST_SUITE_END();

protected:
	void TestFromTime()
	{
		time_t	timeCur = 0;
		time(&timeCur);
		tm *tmCur = gmtime(&timeCur);
		CBaseDate t;
		t.FromTime((uint32)timeCur);

		CPPUNIT_ASSERT(tmCur->tm_year + 1900 == t.year);
		CPPUNIT_ASSERT(tmCur->tm_mon + 1 == t.month);
		CPPUNIT_ASSERT(tmCur->tm_mday == t.day);
		CPPUNIT_ASSERT(tmCur->tm_hour == t.hour);
		CPPUNIT_ASSERT(tmCur->tm_min == t.minute);
		CPPUNIT_ASSERT(tmCur->tm_sec == t.second);
		CPPUNIT_ASSERT(0 == t.ms);
		CPPUNIT_ASSERT(timeCur == t.GetTime());

		t.month = 12;
		CBaseDate t2;
		t2.FromTime(t.GetTime());
		CPPUNIT_ASSERT(t2.month = t.month);
		CPPUNIT_ASSERT(t2.GetTime() == t.GetTime());

		for (int year = 1999; year < 2001; year++)
		{
			CBaseDate	date(year, 1, 1);
			uint32 time = date.GetTime();

			CBaseDate	date2;
			date2.FromTime(time);

			CPPUNIT_ASSERT(date2.year == year);
		}

		int vYear[] = { 1970, 1988, 2000, 2100 };

		for (int i = 0; i < CountOf(vYear); i++)
		{
			int year = vYear[i];
			for (int month = 1; month <= 12; month++)
			{
				int dayToMonthStart = GetDaysToMonth(year, month);
				int dayToMOnthEnd;
				if (month == 12)
					dayToMOnthEnd = dayToMonthStart + 31;
				else
					dayToMOnthEnd = GetDaysToMonth(year, month + 1);
				for (int day = 1; day <= dayToMOnthEnd - dayToMonthStart; day++)
				{
					CBaseDate	date(year, month, day);
					uint32 time = date.GetTime();

					CBaseDate	date2;
					date2.FromTime(time);

					CPPUNIT_ASSERT(date2.year == year);
					CPPUNIT_ASSERT(date2.month == month);
					CPPUNIT_ASSERT(date2.day == day);
				}
			}
		}
	}

	void TestFromString()
	{
		LPCTSTR vDate[] = { _T("2010-01-27"), _T("1970-12-31"), _T("2010-10-27") };

		for (int i = 0; i < CountOf(vDate); i++)
		{
			CBaseDate	date;

			date.FromString(vDate[i]);
			tstring str = date.ToUtcDateString();
			CPPUNIT_ASSERT(str == vDate[i]);
		}

		LPCTSTR vDateTime[] = { _T("2010-01-27 10:12.01"), _T("1970-12-31 10:02.15"), _T("2010-10-27 10:22.59") };

		for (int i = 0; i < CountOf(vDateTime); i++)
		{
			CBaseDate	date;

			date.FromString(vDateTime[i]);
			tstring str = date.ToUtcDateTimeString();
			CPPUNIT_ASSERT(str == vDateTime[i]);
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCBaseDate);

#endif // _CPPUNIT_TEST
