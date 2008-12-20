// FileIndex3.h: interface for the CFileIndex3 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEINDEX3_H__4687E34F_591F_4321_80A8_E32035E947A8__INCLUDED_)
#define AFX_FILEINDEX3_H__4687E34F_591F_4321_80A8_E32035E947A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DB_POS_INVALID		0//(-1)

#define	FI_CUR_MAJOR		1
#define	FI_CUR_MINOR		4

#define BIF_UPDATE_IMEDIATE		0x1

#include "FileIndexBase.h"

template<class FunCmp1, class FunCmp2, class FunCmp3, class FunCmp4>
class CFileIndex4 : public CFileIndexBase
{
public:
	typedef CAVLTree_t<CFileSpaceAllocator, FunCmp1>	Index1;
	typedef CAVLTree_t<CFileSpaceAllocator, FunCmp2>	Index2;
	typedef CAVLTree_t<CFileSpaceAllocator, FunCmp3>	Index3;
	typedef CAVLTree_t<CFileSpaceAllocator, FunCmp4>	Index4;

	CFileIndex4()
	{
	}
	virtual ~CFileIndex4()
	{
	}

	virtual BOOL Init(LPCTSTR szFile, int nBinIdxCount = 1, BOOL bItemSizeSame = TRUE, BYTE byVerCustom = 0)
	{
		if (!CFileIndexBase::Init(szFile, nBinIdxCount, bItemSizeSame, byVerCustom))
			return FALSE;

		m_idx1.InitSet(&m_FileSpaceAllocator, int((LPBYTE)(m_pFileHeader()->idxHead + 0) - (LPBYTE)m_pFileHeader()));
		m_idx2.InitSet(&m_FileSpaceAllocator, int((LPBYTE)(m_pFileHeader()->idxHead + 1) - (LPBYTE)m_pFileHeader()));
		m_idx3.InitSet(&m_FileSpaceAllocator, int((LPBYTE)(m_pFileHeader()->idxHead + 2) - (LPBYTE)m_pFileHeader()));
		m_idx4.InitSet(&m_FileSpaceAllocator, int((LPBYTE)(m_pFileHeader()->idxHead + 3) - (LPBYTE)m_pFileHeader()));

		return TRUE;
	}

	DB_POS AddItem(LPVOID Item, int nLen)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		DB_POS		pos;

		pos = CFileIndexBase::AddItem(Item, nLen);
		if (pos == DB_POS_INVALID)
			return DB_POS_INVALID;

		if (m_idx1.GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			if (m_idx1.avlInsert(pos) != ERR_OK)
				return DB_POS_INVALID;
		}

		if (m_idx2.GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			if (m_idx2.avlInsert(pos) != ERR_OK)
				return DB_POS_INVALID;
		}

		if (m_idx3.GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			if (m_idx3.avlInsert(pos) != ERR_OK)
				return DB_POS_INVALID;
		}

		if (m_idx4.GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			if (m_idx4.avlInsert(pos) != ERR_OK)
				return DB_POS_INVALID;
		}

		return pos;
	}

	BOOL DelItem(DB_POS pos, DWORD dwSize)
	{
		int		nRet;

		nRet = m_idx1.remove(pos);
		ASSERT(nRet == ERR_OK);

		nRet = m_idx2.remove(pos);
		ASSERT(nRet == ERR_OK);

		nRet = m_idx3.remove(pos);
		ASSERT(nRet == ERR_OK);

		nRet = m_idx4.remove(pos);
		ASSERT(nRet == ERR_OK);

		CFileIndexBase::DelItem(pos, dwSize);

		return TRUE;
	}

	void SetCmpFun1(FunCmp1 &funCmp, DWORD dwBIFlag = BIF_UPDATE_IMEDIATE)
	{
		m_idx1.SetCompareFun(funCmp, dwBIFlag);
	}

	void SetCmpFun2(FunCmp2 &funCmp, DWORD dwBIFlag = BIF_UPDATE_IMEDIATE)
	{
		m_idx2.SetCompareFun(funCmp, dwBIFlag);
	}

	void SetCmpFun3(FunCmp3 &funCmp, DWORD dwBIFlag = BIF_UPDATE_IMEDIATE)
	{
		m_idx3.SetCompareFun(funCmp, dwBIFlag);
	}

	void SetCmpFun4(FunCmp4 &funCmp, DWORD dwBIFlag = BIF_UPDATE_IMEDIATE)
	{
		m_idx4.SetCompareFun(funCmp, dwBIFlag);
	}

	void RemoveIndex(DB_POS pos)
	{
		int		nRet;

		nRet = m_idx1.remove(pos);
		ASSERT(nRet == ERR_OK);

		nRet = m_idx2.remove(pos);
		ASSERT(nRet == ERR_OK);

		nRet = m_idx3.remove(pos);
		ASSERT(nRet == ERR_OK);

		nRet = m_idx4.remove(pos);
		ASSERT(nRet == ERR_OK);
	}

	int AddIndex(DB_POS pos)
	{
		int	nRet;
		if (m_idx1.GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			nRet = m_idx1.avlInsert(pos);
			if (nRet != ERR_OK)
				return nRet;
		}

		if (m_idx2.GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			nRet = m_idx2.avlInsert(pos);
			if (nRet != ERR_OK)
				return nRet;
		}

		if (m_idx3.GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			nRet = m_idx3.avlInsert(pos);
			if (nRet != ERR_OK)
				return nRet;
		}

		if (m_idx4.GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			nRet = m_idx4.avlInsert(pos);
			if (nRet != ERR_OK)
				return nRet;
		}

		return ERR_OK;
	}

	DB_POS ModifyItem(DB_POS nIDOld, int nLenOld, LPVOID Item, int nLen)
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

	DB_POS SearchItemOfIndex1(LPVOID Item)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		PAVLNode	node;

		node = m_idx1.avlSearch(Item);

		if (!node)
			return POS_NULL;
		else
			return node->data;
	}

	DB_POS SearchItemOfIndex2(LPVOID Item)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		PAVLNode	node;

		node = m_idx2.avlSearch(Item);

		if (!node)
			return POS_NULL;
		else
			return node->data;
	}

	DB_POS SearchItemOfIndex3(LPVOID Item)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		PAVLNode	node;

		node = m_idx3.avlSearch(Item);

		if (!node)
			return POS_NULL;
		else
			return node->data;
	}

	DB_POS SearchItemOfIndex4(LPVOID Item)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		PAVLNode	node;

		node = m_idx4.avlSearch(Item);

		if (!node)
			return POS_NULL;
		else
			return node->data;
	}

	BOOL SearchAllItemsOfIndex1(LPVOID Item, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx1.avlSearchAll(Item, vNode, 
			m_idx1.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	BOOL SearchAllItemsOfIndex2(LPVOID Item, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx2.avlSearchAll(Item, vNode, 
			m_idx2.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	BOOL SearchAllItemsOfIndex3(LPVOID Item, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx3.avlSearchAll(Item, vNode, 
			m_idx3.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	BOOL SearchAllItemsOfIndex4(LPVOID Item, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx4.avlSearchAll(Item, vNode, 
			m_idx4.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	template<class __FunCmp1>
	BOOL SearchAllItemsOfIndex1(__FunCmp1 &funCmp, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx1.avlSearchAll(funCmp, vNode, 
			m_idx1.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	template<class __FunCmp2>
	BOOL SearchAllItemsOfIndex2(__FunCmp2 &funCmp, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx2.avlSearchAll(funCmp, vNode, 
			m_idx2.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	template<class __FunCmp3>
	BOOL SearchAllItemsOfIndex3(__FunCmp3 &funCmp, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx3.avlSearchAll(funCmp, vNode, 
			m_idx3.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	template<class __FunCmp4>
	BOOL SearchAllItemsOfIndex4(__FunCmp4 &funCmp, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx4.avlSearchAll(funCmp, vNode, 
			m_idx4.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	CAVLTree_t<CFileSpaceAllocator, FunCmp1> &GetIndex1()
	{
		return m_idx1;
	}

	CAVLTree_t<CFileSpaceAllocator, FunCmp2> &GetIndex2()
	{
		return m_idx2;
	}

	CAVLTree_t<CFileSpaceAllocator, FunCmp3> &GetIndex3()
	{
		return m_idx3;
	}

	CAVLTree_t<CFileSpaceAllocator, FunCmp4> &GetIndex4()
	{
		return m_idx4;
	}

protected:
	CAVLTree_t<CFileSpaceAllocator, FunCmp1>	m_idx1;			// 索引的数据
	CAVLTree_t<CFileSpaceAllocator, FunCmp2>	m_idx2;			// 索引的数据
	CAVLTree_t<CFileSpaceAllocator, FunCmp3>	m_idx3;			// 索引的数据
	CAVLTree_t<CFileSpaceAllocator, FunCmp4>	m_idx4;			// 索引的数据

};

template<class FunCmp1, class FunCmp2, class FunCmp3>
class CFileIndex3 : public CFileIndexBase
{
public:
	typedef CAVLTree_t<CFileSpaceAllocator, FunCmp1>	Index1;
	typedef CAVLTree_t<CFileSpaceAllocator, FunCmp2>	Index2;
	typedef CAVLTree_t<CFileSpaceAllocator, FunCmp3>	Index3;

	CFileIndex3()
	{
	}
	virtual ~CFileIndex3()
	{
	}

	virtual BOOL Init(LPCTSTR szFile, int nBinIdxCount = 1, BOOL bItemSizeSame = TRUE, BYTE byVerCustom = 0)
	{
		if (!CFileIndexBase::Init(szFile, nBinIdxCount, bItemSizeSame, byVerCustom))
			return FALSE;

		m_idx1.InitSet(&m_FileSpaceAllocator, int((LPBYTE)(m_pFileHeader()->idxHead + 0) - (LPBYTE)m_pFileHeader()));
		m_idx2.InitSet(&m_FileSpaceAllocator, int((LPBYTE)(m_pFileHeader()->idxHead + 1) - (LPBYTE)m_pFileHeader()));
		m_idx3.InitSet(&m_FileSpaceAllocator, int((LPBYTE)(m_pFileHeader()->idxHead + 2) - (LPBYTE)m_pFileHeader()));

		return TRUE;
	}

	DB_POS AddItem(LPVOID Item, int nLen)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		DB_POS		pos;

		pos = CFileIndexBase::AddItem(Item, nLen);
		if (pos == DB_POS_INVALID)
			return DB_POS_INVALID;

		if (m_idx1.GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			if (m_idx1.avlInsert(pos) != ERR_OK)
				return DB_POS_INVALID;
		}

		if (m_idx2.GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			if (m_idx2.avlInsert(pos) != ERR_OK)
				return DB_POS_INVALID;
		}

		if (m_idx3.GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			if (m_idx3.avlInsert(pos) != ERR_OK)
				return DB_POS_INVALID;
		}

		return pos;
	}

	BOOL DelItem(DB_POS pos, DWORD dwSize)
	{
		int		nRet;

		nRet = m_idx1.remove(pos);
		ASSERT(nRet == ERR_OK);

		nRet = m_idx2.remove(pos);
		ASSERT(nRet == ERR_OK);

		nRet = m_idx3.remove(pos);
		ASSERT(nRet == ERR_OK);

		CFileIndexBase::DelItem(pos, dwSize);

		return TRUE;
	}

	void SetCmpFun1(FunCmp1 &funCmp, DWORD dwBIFlag = BIF_UPDATE_IMEDIATE)
	{
		m_idx1.SetCompareFun(funCmp, dwBIFlag);
	}

	void SetCmpFun2(FunCmp2 &funCmp, DWORD dwBIFlag = BIF_UPDATE_IMEDIATE)
	{
		m_idx2.SetCompareFun(funCmp, dwBIFlag);
	}

	void SetCmpFun3(FunCmp3 &funCmp, DWORD dwBIFlag = BIF_UPDATE_IMEDIATE)
	{
		m_idx3.SetCompareFun(funCmp, dwBIFlag);
	}

	DB_POS ModifyItem(DB_POS nIDOld, int nLenOld, LPVOID Item, int nLen)
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

	DB_POS SearchItemOfIndex1(LPVOID Item)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		PAVLNode	node;

		node = m_idx1.avlSearch(Item);

		if (!node)
			return POS_NULL;
		else
			return node->data;
	}

	DB_POS SearchItemOfIndex2(LPVOID Item)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		PAVLNode	node;

		node = m_idx2.avlSearch(Item);

		if (!node)
			return POS_NULL;
		else
			return node->data;
	}

	DB_POS SearchItemOfIndex3(LPVOID Item)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		PAVLNode	node;

		node = m_idx3.avlSearch(Item);

		if (!node)
			return POS_NULL;
		else
			return node->data;
	}

	BOOL SearchAllItemsOfIndex1(LPVOID Item, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx1.avlSearchAll(Item, vNode, 
			m_idx1.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	BOOL SearchAllItemsOfIndex2(LPVOID Item, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx2.avlSearchAll(Item, vNode, 
			m_idx2.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	BOOL SearchAllItemsOfIndex3(LPVOID Item, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx3.avlSearchAll(Item, vNode, 
			m_idx3.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	template<class __FunCmp1>
	BOOL SearchAllItemsOfIndex1(__FunCmp1 &funCmp, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx1.avlSearchAll(funCmp, vNode, 
			m_idx1.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	template<class __FunCmp2>
	BOOL SearchAllItemsOfIndex2(__FunCmp2 &funCmp, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx2.avlSearchAll(funCmp, vNode, 
			m_idx2.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	template<class __FunCmp3>
	BOOL SearchAllItemsOfIndex3(__FunCmp3 &funCmp, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx3.avlSearchAll(funCmp, vNode, 
			m_idx3.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	CAVLTree_t<CFileSpaceAllocator, FunCmp1> &GetIndex1()
	{
		return m_idx1;
	}

	CAVLTree_t<CFileSpaceAllocator, FunCmp2> &GetIndex2()
	{
		return m_idx2;
	}

	CAVLTree_t<CFileSpaceAllocator, FunCmp3> &GetIndex3()
	{
		return m_idx3;
	}

protected:
	CAVLTree_t<CFileSpaceAllocator, FunCmp1>	m_idx1;			// 索引的数据
	CAVLTree_t<CFileSpaceAllocator, FunCmp2>	m_idx2;			// 索引的数据
	CAVLTree_t<CFileSpaceAllocator, FunCmp3>	m_idx3;			// 索引的数据

};

template<class FunCmp1, class FunCmp2>
class CFileIndex2 : public CFileIndexBase
{
public:
	typedef CAVLTree_t<CFileSpaceAllocator, FunCmp1>	Index1;
	typedef CAVLTree_t<CFileSpaceAllocator, FunCmp2>	Index2;

	CFileIndex2()
	{
	}
	virtual ~CFileIndex2()
	{
	}

	virtual BOOL Init(LPCTSTR szFile, int nBinIdxCount = 1, BOOL bItemSizeSame = TRUE, BYTE byVerCustom = 0)
	{
		if (!CFileIndexBase::Init(szFile, nBinIdxCount, bItemSizeSame, byVerCustom))
			return FALSE;

		m_idx1.InitSet(&m_FileSpaceAllocator, int((LPBYTE)(m_pFileHeader()->idxHead + 0) - (LPBYTE)m_pFileHeader()));
		m_idx2.InitSet(&m_FileSpaceAllocator, int((LPBYTE)(m_pFileHeader()->idxHead + 1) - (LPBYTE)m_pFileHeader()));

		return TRUE;
	}

	DB_POS AddItem(LPVOID Item, int nLen)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		DB_POS		pos;

		pos = CFileIndexBase::AddItem(Item, nLen);
		if (pos == DB_POS_INVALID)
			return DB_POS_INVALID;

		if (m_idx1.GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			if (m_idx1.avlInsert(pos) != ERR_OK)
				return DB_POS_INVALID;
		}

		if (m_idx2.GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			if (m_idx2.avlInsert(pos) != ERR_OK)
				return DB_POS_INVALID;
		}

		return pos;
	}

	BOOL DelItem(DB_POS pos, DWORD dwSize)
	{
		int		nRet;

		nRet = m_idx1.remove(pos);
		ASSERT(nRet == ERR_OK);

		nRet = m_idx2.remove(pos);
		ASSERT(nRet == ERR_OK);

		CFileIndexBase::DelItem(pos, dwSize);

		return TRUE;
	}

	void SetCmpFun1(FunCmp1 &funCmp, DWORD dwBIFlag = BIF_UPDATE_IMEDIATE)
	{
		m_idx1.SetCompareFun(funCmp, dwBIFlag);
	}

	void SetCmpFun2(FunCmp2 &funCmp, DWORD dwBIFlag = BIF_UPDATE_IMEDIATE)
	{
		m_idx2.SetCompareFun(funCmp, dwBIFlag);
	}

	DB_POS ModifyItem(DB_POS nIDOld, int nLenOld, LPVOID Item, int nLen)
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

	DB_POS SearchItemOfIndex1(LPVOID Item)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		PAVLNode	node;

		node = m_idx1.avlSearch(Item);

		if (!node)
			return POS_NULL;
		else
			return node->data;
	}

	DB_POS SearchItemOfIndex2(LPVOID Item)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		PAVLNode	node;

		node = m_idx2.avlSearch(Item);

		if (!node)
			return POS_NULL;
		else
			return node->data;
	}

	BOOL SearchAllItemsOfIndex1(LPVOID Item, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx1.avlSearchAll(Item, vNode, 
			m_idx1.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	BOOL SearchAllItemsOfIndex2(LPVOID Item, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx2.avlSearchAll(Item, vNode, 
			m_idx2.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

#ifndef _LINUX
	BOOL ResortIndex2()
	{
		if (!IsValid())
			return FALSE;

		m_FileSpaceAllocator.PreAlloc(sizeof(AVLNode) * (m_pFileHeader()->dwItemCount + 5));

		m_idx2.removeTreeOnly();

		if (m_idx1.GetRootPos() != POS_NULL)
		{
			Index1::descend_iterator	it, itEnd;
			itEnd = m_idx1.desc_end();
			for (it = m_idx1.desc_begin(); it != itEnd; ++it)
			{
				m_idx2.avlInsert(it.Data());
			}
		}

		return TRUE;
	}
#endif

	CAVLTree_t<CFileSpaceAllocator, FunCmp1> &GetIndex1()
	{
		return m_idx1;
	}

	CAVLTree_t<CFileSpaceAllocator, FunCmp2> &GetIndex2()
	{
		return m_idx2;
	}

protected:
	Index1					m_idx1;			// 索引的数据
	Index2					m_idx2;			// 索引的数据

};

template<class FunCmp1>
class CFileIndex1 : public CFileIndexBase
{
public:
	typedef CAVLTree_t<CFileSpaceAllocator, FunCmp1>	Index1;

	CFileIndex1()
	{
	}
	virtual ~CFileIndex1()
	{
	}

	virtual BOOL Init(LPCTSTR szFile, int nBinIdxCount = 1, BOOL bItemSizeSame = TRUE, BYTE byVerCustom = 0)
	{
		if (!CFileIndexBase::Init(szFile, nBinIdxCount, bItemSizeSame, byVerCustom))
			return FALSE;

		m_idx1.InitSet(&m_FileSpaceAllocator, int((LPBYTE)(m_pFileHeader()->idxHead + 0) - (LPBYTE)m_pFileHeader()));

		return TRUE;
	}

	DB_POS AddItem(LPVOID Item, int nLen)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		DB_POS		pos;

		pos = CFileIndexBase::AddItem(Item, nLen);
		if (pos == DB_POS_INVALID)
			return DB_POS_INVALID;

		if (m_idx1.GetFlag() & BIF_UPDATE_IMEDIATE)
		{
			if (m_idx1.avlInsert(pos) != ERR_OK)
				return DB_POS_INVALID;
		}

		return pos;
	}

	BOOL DelItem(DB_POS pos, DWORD dwSize)
	{
		int		nRet;

		nRet = m_idx1.remove(pos);
		ASSERT(nRet == ERR_OK);

		CFileIndexBase::DelItem(pos, dwSize);

		return TRUE;
	}

	void SetCmpFun1(FunCmp1 &funCmp, DWORD dwBIFlag = BIF_UPDATE_IMEDIATE)
	{
		m_idx1.SetCompareFun(funCmp, dwBIFlag);
	}

	DB_POS ModifyItem(DB_POS nIDOld, int nLenOld, LPVOID Item, int nLen)
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

	DB_POS SearchItemOfIndex1(LPVOID Item)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		PAVLNode	node;

		node = m_idx1.avlSearch(Item);

		if (!node)
			return POS_NULL;
		else
			return node->data;
	}

	template<class __FunCmp2>
	DB_POS SearchItemOfIndex1(__FunCmp2 &funCmp)
	{
		if (!IsValid())
			return DB_POS_INVALID;

		PAVLNode	node;

		node = m_idx1.avlSearch(funCmp);

		if (!node)
			return POS_NULL;
		else
			return node->data;
	}

	BOOL SearchAllItemsOfIndex1(LPVOID Item, vector<DB_POS> &vItem)
	{
		if (!IsValid())
			return FALSE;

		vector<PAVLNode>	vNode;

		m_idx1.avlSearchAll(Item, vNode, 
			m_idx1.GetRootPos());

		for (int i = 0; i < (int)vNode.size(); i++)
		{
			vItem.push_back(vNode[i]->data);
		}

		return TRUE;
	}

	CAVLTree_t<CFileSpaceAllocator, FunCmp1> &GetIndex1()
	{
		return m_idx1;
	}

protected:
	CAVLTree_t<CFileSpaceAllocator, FunCmp1>	m_idx1;			// 索引的数据

};

#endif // !defined(AFX_FILEINDEX3_H__4687E34F_591F_4321_80A8_E32035E947A8__INCLUDED_)
