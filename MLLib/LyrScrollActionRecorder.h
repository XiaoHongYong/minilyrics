#pragma once

class CLyrScrollActionRecorder
{
public:
	struct ScrollTo
	{
		int			nTime;
		int			distance;		// The character offset of lyrics.
		double		speed;

		ScrollTo() { }
		ScrollTo(int d, int t) {
			nTime = t;
			distance = d;
			speed = 0.0;
		}
	};

	enum {
		MAX_TIME					= 0xFFFFFF
	};

	class ListScrollTo : public vector<ScrollTo>
	{
	public:

	};

public:
	CLyrScrollActionRecorder();

	static bool IsEventTag(LPCTSTR str);

	void InitCreateAutoEvents(VLyrLines &lyrLines);

	void Close();

	bool HasActions() { return m_bHasActions; }

	tstring EventsDataToString();
	void EventsDataFromString(LPCTSTR szEventsData, VLyrLines &lyrLines);

	void ScrollToLine(VLyrLines &lyrLines, int nLine, int nTime, bool bUpdateTimeTag = true);

	void UpdateTimeTagByEvents(VLyrLines &lyrLines);

protected:
	void AddScrollTo(VLyrLines &lyrLines, int nPos, int nTime, bool bUpdateTimeTag = true);

protected:
	int GetDistanceOfLine(VLyrLines &lyrLines, int nLine);

	int GetDistanceOfCurrentPos(VLyrLines &lyrLines);

	int CountLyrChar(VLyrLines &lyrLines);

	friend class CTestCaseLyrScrollActionRecorder;

protected:
	ListScrollTo			m_listEvents;
	int						m_nLyrCharCount;
	bool					m_bHasActions;

};
