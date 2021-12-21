#pragma once

class CMemmapFile  
{
public:
	CMemmapFile();
	virtual ~CMemmapFile();

	bool Open(LPCTSTR szFile, int nSizeMin, bool bMustExist = false);

	bool Resize(size_t nSize);

	size_t GetSize() { return m_nFileSize; }

	void Close();

	bool IsValid() const { return m_lpFile != NULL; }

	LPBYTE GetPtr()
	{
		return (LPBYTE)m_lpFile;
	}

protected:
	int			m_file;
	size_t		m_nFileSize;		// 文件大小
	LPVOID		m_lpFile;			// 文件映射之后的文件内容

};
