#pragma once

class CMemmapFile  
{
public:
	CMemmapFile();
	virtual ~CMemmapFile();

	bool Open(LPCTSTR szFile, int nSizeMin, bool bMustExist = false);

	bool Resize(size_t nSize);

	int GetSize() { return m_nFileSize; }

	void Close();

	bool IsValid() const { return m_lpFile != NULL; }

	LPBYTE GetPtr()
	{
		return m_lpFile;
	}

protected:
	HANDLE		m_hFile;			// 文件句柄
	HANDLE		m_hFilemap;			// 文件映射句柄
	int			m_nFileSize;		// 文件大小
	LPBYTE		m_lpFile;			// 文件映射之后的文件内容

};
