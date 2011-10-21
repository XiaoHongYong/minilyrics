
#pragma once

class IRunnable
{
public:
	IRunnable() { m_bFreeAfterRun = false; }
	virtual ~IRunnable() { }

	virtual void Run() = 0;

	// inline bool IsFreeAfterRun() const { return m_bFreeAfterRun; }
private:
	friend class CLooper;
	bool		m_bFreeAfterRun;

};

class CRunnableQueue
{
public:
	CRunnableQueue();

	void Add(IRunnable *pCallback);

	IRunnable *Next();

protected:
	typedef list<IRunnable *>		ListRunnable;

	Event			m_event;
	Mutex			m_mutex;

	ListRunnable	m_listRunnable;

};

class CLooper
{
public:
	CLooper();
	virtual ~CLooper() { }

	void Loop();

	void Post(IRunnable *pCallback);
	void PostFreeAfterRun(IRunnable *pCallback);

	void Quit();

	bool IsIdle() const { return m_bIdle; }

	bool				m_bLog;
protected:
	CRunnableQueue		m_queue;
	volatile bool		m_bIdle;

};

class CLooperThread
{
public:
	bool Start();

protected:
	static void ThreadLoop(void *lpParam);

private:
	CThread			m_thread;
	CLooper			m_looper;

};
