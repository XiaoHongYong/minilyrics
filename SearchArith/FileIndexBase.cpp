// FileIndexBase.cpp: implementation of the CFileIndexBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SearchArith.h"
#include "FileIndexBase.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileIndexBase::CFileIndexBase()
{

}

CFileIndexBase::~CFileIndexBase()
{
	Close();
}

BOOL CFileIndexBase::Init(LPCTSTR szFile, int nBinIdxCount, BOOL bItemSizeSame, BYTE byVerCustom)
{
	size_t		nHeaderSize;
	nHeaderSize = sizeof(FileHeader) + sizeof(CAVLTree<CFileSpaceAllocator>) * (nBinIdxCount - 1);

	Close();

	if (m_FileSpaceAllocator.Open(szFile, nHeaderSize) != ERR_OK)
		return FALSE;

	// m_pFileHeader() = (FileHeader *)m_FileSpaceAllocator.GetRemainDataPtr();

	if (!(m_pFileHeader()->szFileType[0] == 'D' && m_pFileHeader()->szFileType[1] == 'B'
		&& m_pFileHeader()->wVerMajor == FI_CUR_MAJOR && m_pFileHeader()->wVerMinor == FI_CUR_MINOR
		&& m_pFileHeader()->wVerCustom == byVerCustom
		&& m_pFileHeader()->nIdxCount == nBinIdxCount))
	{
		m_FileSpaceAllocator.Initialize();

		//
		// 重新设置文件格式
		memset(m_pFileHeader(), 0, nHeaderSize);
		m_pFileHeader()->wHeaderLength = nHeaderSize;
		m_pFileHeader()->nIdxCount = nBinIdxCount;
		m_pFileHeader()->szFileType[0] = 'D';
		m_pFileHeader()->szFileType[1] = 'B';
		m_pFileHeader()->wVerMajor = FI_CUR_MAJOR;
		m_pFileHeader()->wVerMinor = FI_CUR_MINOR;
		m_pFileHeader()->wVerCustom = byVerCustom;
		m_pFileHeader()->dwItemCount = 0;
		memset(m_pFileHeader()->idxHead, 0, sizeof(m_pFileHeader()->idxHead));
		for (int i = 0; i < CountOf(m_pFileHeader()->idxHead); i++)
			m_pFileHeader()->idxHead[i].m_dwFlag = BIF_UPDATE_IMEDIATE;
	}

	return TRUE;
}

void CFileIndexBase::Close()
{
	// m_pFileHeader() = NULL;
	m_FileSpaceAllocator.Close();
}

BOOL CFileIndexBase::GetHeaderInfo(LPCTSTR szFile, FileHeader &fileHeader)
{
	FILE	*fp;

	fp = _tfopen(szFile, _T("rb"));
	if (fp)
	{
		if (fread(&fileHeader, 1, sizeof(fileHeader), fp) == sizeof(fileHeader))
		{
			fclose(fp);
			return TRUE;
		}
		fclose(fp);
	}
	return FALSE;
}
