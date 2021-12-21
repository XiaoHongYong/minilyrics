// FileIndexBase.h: interface for the CFileIndexBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEINDEXBASE_H__8AB8E991_B569_4651_B9E6_51B8CB8638EC__INCLUDED_)
#define AFX_FILEINDEXBASE_H__8AB8E991_B569_4651_B9E6_51B8CB8638EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef DB_POS_INVALID
#define DB_POS_INVALID		0//(-1)

#define	FI_CUR_MAJOR		1
#define	FI_CUR_MINOR		4

#define BIF_UPDATE_IMEDIATE		0x1
#endif


class CFileIndexBase  
{
public:

	struct FileHeader
	{
		TCHAR	szFileType[2];			// 文件类型
		WORD	wHeaderLength;			// 文件头的大小: 包括了 FileHeader 和 BinIndex的大小
		WORD	wVerMajor;				// 文件版本
		BYTE	wVerMinor;				// 文件版本
		BYTE	wVerCustom;				// 用户定义的一个版本号
		DWORD	dwItemCount;			// 数据项的数目
		DWORD	dwID;					// 
		int		nIdxCount;				// 索引的数目
		CAVLTree_t_Head	idxHead[4];		// 索引的数据
	};

	CFileIndexBase();
	virtual ~CFileIndexBase();

	static BOOL GetHeaderInfo(LPCTSTR szFile, FileHeader &fileHeader);

	virtual BOOL Init(LPCTSTR szFile, int nBinIdxCount = 1, BOOL bItemSizeSame = TRUE, BYTE byVerCustom = 0);
	void Close();

	DB_POS AddItem(LPVOID Item, int nLen)
	{
		LPVOID		lpTemp;
		DB_POS		pos;

		lpTemp = m_FileSpaceAllocator.Alloc(nLen);
		if (!lpTemp)
			return DB_POS_INVALID;

		pos = m_FileSpaceAllocator.AddrToPos(lpTemp);
		memcpy(lpTemp, Item, nLen);

		if (m_FileSpaceAllocator.PreAlloc(sizeof(AVLNode) * 6) != ERR_OK)
			return DB_POS_INVALID;

		m_pFileHeader()->dwItemCount++;

		return pos;
	}

	BOOL DelItem(DB_POS pos, DWORD dwSize)
	{
		if (!IsValid())
			return false;
		m_FileSpaceAllocator.Free(pos);

		m_pFileHeader()->dwItemCount--;

		return TRUE;
	}
//
//	DB_POS ModifyItem(DB_POS nIDOld, int nLenOld, LPVOID Item, int nLen)
//	{
//		if (nLenOld >= nLen)
//		{
//			LPVOID	lpItemOld;
//			lpItemOld = GetItem(nIDOld);
//			if (lpItemOld)
//			{
//				memcpy(lpItemOld, Item, nLen);
//				return nIDOld;
//			}
//			else
//				return DB_POS_INVALID;
//		}
//		else
//		{
//			DelItem(nIDOld, nLenOld);
//			return AddItem(Item, nLen);
//		}
//	}

	LPVOID GetItem(DB_POS Index)
	{
		if (!IsValid())
			return NULL;

		return m_FileSpaceAllocator.PosToAddr(Index);
	}

	DWORD GetNextID() { if (IsValid()) return m_pFileHeader()->dwID++; else return 0; }

	BOOL IsValid() { return m_FileSpaceAllocator.IsValid(); }

	DWORD GetItemCount()
	{
		if (m_pFileHeader())
			return m_pFileHeader()->dwItemCount;
		else
			return 0;
	}

	CFileSpaceAllocator *GetAllocator()
	{
		return &m_FileSpaceAllocator;
	}

protected:
	inline FileHeader *m_pFileHeader()
	{
		return (FileHeader *)m_FileSpaceAllocator.GetRemainDataPtr();
	}

protected:
	CFileSpaceAllocator			m_FileSpaceAllocator;
	// FileHeader	*m_pFileHeader;

};

#endif // !defined(AFX_FILEINDEXBASE_H__8AB8E991_B569_4651_B9E6_51B8CB8638EC__INCLUDED_)
