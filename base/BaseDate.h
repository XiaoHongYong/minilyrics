
#pragma once

DECLAR_CPPUNIT_TEST_REG(CBaseDate)

class CBaseDate
{
public:
	byte	day, month;
	WORD	year;
	byte	hour;
	byte	minute, second;
	WORD	ms;

	enum {
		SECOND_IN_ONE_DAY = 60 * 60 * 24,
		MILLIS_IN_ONE_DAY = 1000 * SECOND_IN_ONE_DAY,
	};

public:
	CBaseDate();
	CBaseDate(int year, int month, int day);
	CBaseDate(int year, int month, int day, int hour, int minute, int second, int ms);

	// Returns this Date as a second value. 
	// The value is the number of seconds since Jan. 1, 1970, midnight GMT.
	uint32 GetTime();
	void FromTime(uint32 timeInMillis);

	// Returns the MiniLyrics server date format as the uint32 value.
	uint32 GetMLDate();
	void FromMLDate(uint32 mlDate);

	// Returns the MiniLyrics server date time format as the uint32 value.
	uint32 GetMLDateTime();
	void FromMLDateTime(uint32 mlDateTime);

	tstring ToUtcDateString();
	tstring ToUtcDateTimeString();

	bool FromString(LPCTSTR szDate);

	bool IsLeapYear() const { return IsLeapYear(year); }

	static bool IsLeapYear(int year);

	static CBaseDate GetCurrentDate();

};
