// MemmapFile.cpp: implementation of the CMemmapFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "fileapi.h"
#include "MemmapFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemmapFile::CMemmapFile()
{
	m_file = 0;
	m_nFileSize = 0;
	m_lpFile = NULL;
}

CMemmapFile::~CMemmapFile()
{
	Close();
}

bool CMemmapFile::Open(LPCTSTR szFile, int nSizeMin, bool bMustExist)
{
	if (IsFileExist(szFile))
	{
		struct stat		filestat;

		m_file = open(szFile, O_RDWR);
		if (m_file < 0)
			return false;

		memset(&filestat, 0, sizeof(filestat));
		if (fstat(m_file, &filestat) < 0)
		{
			Close();
			return false;
		}

		m_nFileSize = filestat.st_size;
		if (m_nFileSize < nSizeMin)
		{
			m_nFileSize = nSizeMin;
			if (lseek(m_file, m_nFileSize - 1, SEEK_SET) == -1)
			{
				Close();
				return false;
			}

			if (write(m_file, "", 1) != 1)
			{
				Close();
				return false;
			}
		}
	}
	else
	{
		if (bMustExist)
			return false;

		m_file = open(szFile, O_RDWR);
		if (m_file < 0)
		{
			return false;
		}

		m_nFileSize = nSizeMin;
		if (lseek(m_file, m_nFileSize - 1, SEEK_SET) == -1)
		{
			Close();
			return false;
		}

		if (write(m_file, "", 1) != 1)
		{
			Close();
			return false;
		}
	}

	m_lpFile = mmap(0, m_nFileSize, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, m_file, 0);
	if (m_lpFile == (LPVOID)-1)
	{
		m_lpFile = mmap(0, m_nFileSize, PROT_READ | PROT_WRITE, MAP_FILE | MAP_PRIVATE, m_file, 0);
		if (m_lpFile == (LPVOID)-1)
		{
		m_lpFile = NULL;
		Close();
		return false;
		}
	}

	return true;
}

void CMemmapFile::Close()
{
	if (m_lpFile)
	{
		munmap(m_lpFile, m_nFileSize);
		m_lpFile = NULL;
	}

	if (m_file >= 0)
	{
		close(m_file);
		m_file = -1;
	}
}
