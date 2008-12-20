// MemmapFile.h: interface for the CMemmapFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMMAPFILE_H__B225B484_E3D9_43C4_9331_869F82768C0F__INCLUDED_)
#define AFX_MEMMAPFILE_H__B225B484_E3D9_43C4_9331_869F82768C0F__INCLUDED_

#include <sys/mman.h>

class CMemmapFile  
{
public:
	CMemmapFile();
	virtual ~CMemmapFile();

	bool Open(LPCTSTR szFile, int nSizeMin, bool bMustExist = false);

	int GetSize() { return m_nFileSize; }

	void Close();

	LPBYTE GetPtr()
	{
		return (LPBYTE)m_lpFile;
	}

protected:
	int			m_file;
	int			m_nFileSize;		// 文件大小
	LPVOID		m_lpFile;			// 文件映射之后的文件内容

};

#endif // !defined(AFX_MEMMAPFILE_H__B225B484_E3D9_43C4_9331_869F82768C0F__INCLUDED_)
