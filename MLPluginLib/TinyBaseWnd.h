
#pragma once

class CTinyBaseWnd
{
public:
	CTinyBaseWnd();
	virtual ~CTinyBaseWnd();

	bool Create(HINSTANCE hInstance, LPCTSTR szGetPosWndClass = NULL, LPCTSTR szGetPosWndName = NULL);
	void Destroy() { ::DestroyWindow(m_hWnd); m_hWnd = NULL; }

	HWND GetHandle() const { return m_hWnd; }

	virtual void OnCreate() { }
	virtual void OnDestroy() { }
	virtual void OnTimer(int nTimerID) { }

	virtual void OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds) { }

	virtual void SetTimer(DWORD dwTimerID, UINT uElapse) { ::SetTimer(m_hWnd, dwTimerID, uElapse, NULL); }
	virtual void KillTimer(DWORD dwTimerID) { ::KillTimer(m_hWnd, dwTimerID); }

	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	HWND			m_hWnd;

};
