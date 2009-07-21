#if !defined(_MEM_MAPPING_H_)
#define _MEM_MAPPING_H_

#pragma once

class CMemMapping
{
public:
	CMemMapping()
	{
		m_hMap = NULL;
		m_nSize = NULL;
		m_lpData = NULL;
	}
	virtual ~CMemMapping()
	{
		Close();
	}

	bool Create(LPCTSTR szName, int nSize)
	{
		m_hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, nSize, szName);
		assert(m_hMap);
		if (!m_hMap)
			return false;

		m_lpData = (LPBYTE)MapViewOfFile(m_hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, nSize);
		assert(m_lpData);
		if (!m_lpData)
		{
			CloseHandle(m_hMap);
			m_hMap = NULL;
			return false;
		}

		return true;
	}

	void Close()
	{
		if (m_lpData)
		{
			UnmapViewOfFile(m_lpData);
			m_lpData = NULL;
		}

		if (m_hMap)
		{
			CloseHandle(m_hMap);
			m_hMap = NULL;
		}
	}

	LPBYTE GetData() { return m_lpData; }

protected:
	HANDLE		m_hMap;
	int			m_nSize;
	LPBYTE		m_lpData;

};

#endif // !defined(_MEM_MAPPING_H_)
