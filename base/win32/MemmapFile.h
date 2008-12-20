// MemmapFile.h: interface for the CMemmapFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMMAPFILE_H__B225B484_E3D9_43C4_9331_869F82768C0F__INCLUDED_)
#define AFX_MEMMAPFILE_H__B225B484_E3D9_43C4_9331_869F82768C0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
		return m_lpFile;
	}

protected:
	HANDLE		m_hFile;			// 文件句柄
	HANDLE		m_hFilemap;			// 文件映射句柄
	int			m_nFileSize;		// 文件大小
	LPBYTE		m_lpFile;			// 文件映射之后的文件内容

};

#endif // !defined(AFX_MEMMAPFILE_H__B225B484_E3D9_43C4_9331_869F82768C0F__INCLUDED_)
