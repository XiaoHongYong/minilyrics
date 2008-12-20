// FileIndexEx.cpp: implementation of the CFileIndexEx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SearchArith.h"
#include "FileIndexEx.h"

#define FI_FREE_RATE		1.1
#ifdef _WIN32_WCE
#define FI_MINI_FREE		(1024 * 5)
#else
#define FI_MINI_FREE		(1024 * 100)
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileIndexEx::CFileIndexEx()
{
	// m_pFileHeader() = NULL;
}

CFileIndexEx::~CFileIndexEx()
{
	Close();
}

BOOL CFileIndexEx::GetHeaderInfo(LPCTSTR szFile, FileHeader &fileHeader)
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


BOOL CFileIndexEx::Init(LPCTSTR szFile, int nBinIdxCount, BOOL bItemSizeSame, BYTE byVerCustom)
{
	size_t		nHeaderSize;
	nHeaderSize = sizeof(FileHeader) + sizeof(CAVLTree<CFileSpaceAllocator>) * (nBinIdxCount - 1);

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

		for (int i = 0; i < nBinIdxCount; i++)
		{
			m_pFileHeader()->idx[i].Clear();
		}
	}

	for (int i = 0; i < nBinIdxCount; i++)
	{
		m_pFileHeader()->idx[i].SetAllocator(&m_FileSpaceAllocator);
	}

	return TRUE;
}

void CFileIndexEx::Close()
{
	// m_pFileHeader() = NULL;
	m_FileSpaceAllocator.Close();
}

void CFileIndexEx::SetBinIndexFun(int nID, FUNCompare funCompare, DWORD dwBIFlag)
{
	if (!IsValid())
		return;

	ASSERT(nID < m_pFileHeader()->nIdxCount);

	if (nID < m_pFileHeader()->nIdxCount)
	{
		m_pFileHeader()->idx[nID].SetCompareFun(funCompare, dwBIFlag);
	}
}

DB_POS CFileIndexEx::AddItem(LPVOID Item, int nLen)
{
	LPVOID		lpTemp;
	DB_POS		pos;

	lpTemp = m_FileSpaceAllocator.Alloc(nLen);
	if (!lpTemp)
		return DB_POS_INVALID;

	pos = m_FileSpaceAllocator.AddrToPos(lpTemp);
	memcpy(lpTemp, Item, nLen);

	if (m_FileSpaceAllocator.PreAlloc(sizeof(AVLNode) * (m_pFileHeader()->nIdxCount + 1)) != ERR_OK)
		return DB_POS_INVALID;

	// 更新索引
	for (int i = 0; i < m_pFileHeader()->nIdxCount; i++)
	{
		if (m_pFileHeader()->idx[i].GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			if (m_pFileHeader()->idx[i].avlInsert(pos) != ERR_OK)
				return DB_POS_INVALID;
		}
	}

	m_pFileHeader()->dwItemCount++;

	return pos;
}

BOOL CFileIndexEx::DelItem(DB_POS pos, DWORD dwSize)
{
	BOOL		bRet;

	if (!IsValid())
		return FALSE;

	for (int i = 0; i < m_pFileHeader()->nIdxCount; i++)
	{
		CFileIndexEx::AVL_INDEX	&idx = m_pFileHeader()->idx[i];

		bRet = idx.remove(pos);
		ASSERT(bRet == ERR_OK);
	}

	m_FileSpaceAllocator.Free(pos);

	m_pFileHeader()->dwItemCount--;

	return TRUE;
}

DB_POS CFileIndexEx::ModifyItem(DB_POS nIDOld, int nLenOld, LPVOID Item, int nLen)
{
	if (nLenOld >= nLen)
	{
		LPVOID	lpItemOld;
		lpItemOld = GetItem(nIDOld);
		if (lpItemOld)
		{
			memcpy(lpItemOld, Item, nLen);
			return nIDOld;
		}
		else
			return DB_POS_INVALID;
	}
	else
	{
		DelItem(nIDOld, nLenOld);
		return AddItem(Item, nLen);
	}
}

DB_POS CFileIndexEx::SearchItem(int nIdxID, LPVOID Item, int nLen)
{
	if (!IsValid())
		return DB_POS_INVALID;

	ASSERT((nIdxID < m_pFileHeader()->nIdxCount));
//	if (nIdxID >= m_pFileHeader()->nIdxCount)
//		return DB_POS_INVALID;

	AVL_INDEX	&Idx = m_pFileHeader()->idx[nIdxID];
	PAVLNode	node;

	node = Idx.avlSearch(Item);

	if (!node)
		return POS_NULL;
	else
		return node->data;
}

BOOL CFileIndexEx::SearchAllItems(int nIdxID, LPVOID Item, int nLen, vector<DB_POS> &vItem)
{
	if (!IsValid())
		return FALSE;

	if (nIdxID >= m_pFileHeader()->nIdxCount)
		return FALSE;

	AVL_INDEX	&idx = m_pFileHeader()->idx[nIdxID];
	vector<PAVLNode>	vNode;

	idx.avlSearchAll(Item, vNode, 
		idx.GetRootPos());

	for (int i = 0; i < (int)vNode.size(); i++)
	{
		vItem.push_back(vNode[i]->data);
	}

	return TRUE;
}

BOOL CFileIndexEx::SearchAllItems(int nIdxID, FUNCompare funCmp, LPVOID lpItem, vector<DB_POS> &vItem)
{
	if (!IsValid())
		return FALSE;

	if (nIdxID >= m_pFileHeader()->nIdxCount)
		return FALSE;

	AVL_INDEX	&idx = m_pFileHeader()->idx[nIdxID];
	vector<PAVLNode>	vNode;

	idx.avlSearchAll(funCmp, lpItem, vNode, 
		idx.GetRootPos());

	for (int i = 0; i < (int)vNode.size(); i++)
	{
		vItem.push_back(vNode[i]->data);
	}

	return TRUE;
}

LPVOID CFileIndexEx::GetItem(DB_POS Index)
{
	if (!IsValid())
		return NULL;

	return m_FileSpaceAllocator.PosToAddr(Index);
}

BOOL CFileIndexEx::VisitByOrder(int nIdxID, FUNVisit funVisit, LPVOID lpUserData, int nMax, BOOL bDesc)
{
	if (!IsValid())
		return FALSE;

	if (nIdxID >= m_pFileHeader()->nIdxCount)
		return FALSE;

	AVL_INDEX	&idx = m_pFileHeader()->idx[nIdxID];

	if (bDesc)
	{
		// 降序
		idx.VisitRight(idx.GetRootPos(), nMax, funVisit, lpUserData);
	}
	else
	{
		// 升序
//		idx.VisitLeft2(idx.GetRootPos(), nMax, funVisit, lpUserData);
		idx.VisitLeft(idx.GetRootPos(), nMax, funVisit, lpUserData);
	}

	return TRUE;
}

float CFileIndexEx::GetSpaceUsage()
{
/*
	DWORD		dwUsed;

	dwUsed = m_pFileHeader()->dwItemLength + sizeof(FileHeader);

	for (int i = 0; i < m_pFileHeader()->nIdxCount; i++)
	{
		BinIndex	&binIdx = m_pFileHeader()->idx[i];
		dwUsed += binIdx.dwIndexLength;
		dwUsed += sizeof(BinIndex);
	}

	return (float)dwUsed / m_nFileSize;
*/
	return 0;
}

DWORD CFileIndexEx::GetUsedSpace()
{
/*
	DWORD		dwUsed;

	dwUsed = m_pFileHeader()->dwItemLength + sizeof(FileHeader);

	for (int i = 0; i < m_pFileHeader()->nIdxCount; i++)
	{
		BinIndex	&binIdx = m_pFileHeader()->idx[i];
		dwUsed += binIdx.dwIndexLength;
		dwUsed += sizeof(BinIndex);
	}

	return dwUsed;
*/
	return 0;
}

DWORD CFileIndexEx::GettotalSize()
{
	return 0;//m_nFileSize;
}

DWORD CFileIndexEx::GetItemCount()
{
	if (m_pFileHeader())
		return m_pFileHeader()->dwItemCount;
	else
		return 0;
}

BOOL CFileIndexEx::ResortFun(LPVOID lpItem, DB_POS dwPhisicalPos, LPVOID lpUserData)
{
	CFileIndexEx	*pThis;

	pThis = (CFileIndexEx *)lpUserData;

	AVL_INDEX	*pIdx;

	pIdx = (AVL_INDEX *)lpUserData;

	pIdx->avlInsert(pIdx->ToPos((PAVLNode)lpItem));

//	if (pThis->m_pFileHeader()->idx[i].GetFlag() & BIF_UPDATE_IMEDIATE)
//		pThis->m_pFileHeader()->idx[i].avlInsert(pos);

	return TRUE;
}

BOOL CFileIndexEx::Resort(int nBaseIdx, int nResortIdx)
{
	if (!IsValid())
		return FALSE;

	if (nBaseIdx >= m_pFileHeader()->nIdxCount)
		return FALSE;

	if (nResortIdx >= m_pFileHeader()->nIdxCount)
		return FALSE;

	if (m_FileSpaceAllocator.PreAlloc(sizeof(AVLNode) * (m_pFileHeader()->dwItemCount + 5)) != ERR_OK)
		return FALSE;

	//
	AVL_INDEX	&binIdx = m_pFileHeader()->idx[nBaseIdx];
	AVL_INDEX	&binIdx2 = m_pFileHeader()->idx[nResortIdx];

	binIdx2.removeTreeOnly();

	if (binIdx.GetRootPos() != POS_NULL)
		binIdx.VisitLeftAll(binIdx.GetRootPos(), ResortFun, &binIdx2);

/*
	AVL_INDEX	&binIdx = m_pFileHeader()->idx[nBaseIdx];
	AVL_INDEX	&binIdx2 = m_pFileHeader()->idx[nResortIdx];

	for (DWORD i = 0; i < binIdx.dwIndexLength / sizeof(DWORD); i++)
	{
		DB_POS		pos;
		LPVOID		lpItem;
		int			dwHign, dwLow, dwMid;
		int			nCmp;

		pos = binIdx.pIndexPos[i];
		lpItem = m_pItems + pos;

		dwHign = binIdx2.dwIndexLength / sizeof(DWORD) - 1;
		dwLow = 0;
		dwMid = 0;

		while (dwHign >= dwLow)
		{
			dwMid = (dwHign + dwLow) / 2;

			DWORD	dwPos = binIdx2.pIndexPos[dwMid];
			nCmp = binIdx2.funCompare(lpItem, &(m_pItems[dwPos]));

			if (nCmp == 0)
			{
				// 已经存在，则……
				// ASSERT(0);
				// return dwPos;
				break;
			}
			else if (nCmp == -1)
			{
				dwHign = dwMid - 1;
			}
			else
			{
				dwLow = dwMid + 1;
			}
		}

		if (nCmp == 1)
			dwMid++;

		int		nMoveLen;
		nMoveLen = binIdx2.dwIndexLength - dwMid * sizeof(DWORD);
		if (nMoveLen > 0)
			memmove(&(binIdx2.pIndexPos[dwMid + 1]), &(binIdx2.pIndexPos[dwMid]), nMoveLen);
		binIdx2.pIndexPos[dwMid] = pos;
		binIdx2.dwIndexLength += sizeof(DWORD);
		ASSERT(binIdx2.dwIndexLength < m_pFileHeader()->dwBinIdxAvlSize);
		if (binIdx2.dwIndexLength > m_pFileHeader()->dwBinIdxMaxSize)
			m_pFileHeader()->dwBinIdxMaxSize = binIdx2.dwIndexLength;
	}
*/	return TRUE;
}

BOOL CFileIndexEx::Verify()
{
	if (!IsValid())
		return FALSE;

	for (int i = 0; i < m_pFileHeader()->nIdxCount; i++)
	{
		CFileIndexEx::AVL_INDEX	&idx = m_pFileHeader()->idx[i];

		if (idx.GetRootPos() != POS_NULL)
			idx.VerifyBf(idx.ToAddr(idx.GetRootPos()));
	}
	return TRUE;
}
