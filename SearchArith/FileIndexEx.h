// FileIndex.h: interface for the CFileIndexEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(FILE_INDEX_EX_INC)
#define FILE_INDEX_EX_INC

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef DB_POS_INVALID
#define DB_POS_INVALID		0//(-1)

#define	FI_CUR_MAJOR		1
#define	FI_CUR_MINOR		4

#define BIF_UPDATE_IMEDIATE		0x1
#endif

// typedef vector<CStaticAVLTree>		V_STATICAVLTREE;

//
// 文件索引模块
//
class CFileIndexEx  
{
public:
	enum
	{
		EVT_READ = 0,
		EVT_WRITE = 1,
		EVT_MAX
	};

	typedef CAVLTree<CFileSpaceAllocator>	AVL_INDEX;

	struct FileHeader
	{
		TCHAR	szFileType[2];		// 文件类型
		WORD	wHeaderLength;		// 文件头的大小: 包括了 FileHeader 和 BinIndex的大小
		WORD	wVerMajor;			// 文件版本
		BYTE	wVerMinor;			// 文件版本
		BYTE	wVerCustom;			// 用户定义的一个版本号
		DWORD	dwItemCount;		// 数据项的数目
		DWORD	dwID;				// 
		int		nIdxCount;			// 索引的数目
		CAVLTree<CFileSpaceAllocator>	idx[1];			// 索引的数据
	};

	CFileIndexEx();
	virtual ~CFileIndexEx();

	static BOOL GetHeaderInfo(LPCTSTR szFile, FileHeader &fileHeader);

	virtual BOOL Init(LPCTSTR szFile, int nBinIdxCount = 1, BOOL bItemSizeSame = TRUE, BYTE byVerCustom = 0);
	void Close();

	void SetBinIndexFun(int nID, FUNCompare funCompare, DWORD dwBIFlag = BIF_UPDATE_IMEDIATE);

	DB_POS AddItem(LPVOID Item, int nLen);

	BOOL DelItem(DB_POS nID, DWORD dwSize);
	DB_POS ModifyItem(DB_POS nIDOld, int nLenOld, LPVOID Item, int nLen);

	//
	DB_POS SearchItem(int nIdxID, LPVOID Item, int nLen);
	BOOL SearchAllItems(int nIdxID, LPVOID Item, int nLen, vector<DB_POS> &vItem);
	BOOL SearchAllItems(int nIdxID, FUNCompare funCmp, LPVOID lpItem, vector<DB_POS> &vItem);

	LPVOID GetItem(DB_POS Index);

	BOOL VisitByOrder(int nIdxID, FUNVisit funVisit, LPVOID lpUserData, int nMax = -1, BOOL bDesc = FALSE);

	DWORD GetNextID() { return ++(m_pFileHeader()->dwID); }

	BOOL IsValid() { return m_FileSpaceAllocator.IsValid(); }

	float GetSpaceUsage();
	DWORD GetUsedSpace();
	DWORD GettotalSize();

	DWORD GetItemCount();

	BOOL Resort(int nBaseIdx, int nResortIdx);

	BOOL Verify();

	AVL_INDEX *GetIndex(int nIdx)
	{
		assert((nIdx < m_pFileHeader()->nIdxCount));

		return &(m_pFileHeader()->idx[nIdx]);
	}

protected:
	static BOOL ResortFun(LPVOID lpItem, DB_POS dwPhisicalPos, LPVOID lpUserData);
//	BOOL ReAllocFile();
	inline FileHeader *m_pFileHeader()
	{
		return (FileHeader *)m_FileSpaceAllocator.GetRemainDataPtr();
	}

protected:
	CFileSpaceAllocator			m_FileSpaceAllocator;
	// FileHeader	*m_pFileHeader;

};

#endif // !defined(FILE_INDEX_EX_INC)
