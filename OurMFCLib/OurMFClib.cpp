///////////////////////////////////////////////////
// the implementation of mylib

#include "stdafx.h"
#include <assert.h>
#include <tchar.h>
#include "OurMFClib.h"


//////////////////////////////////////////////////
// functions

/*///////////////////////////////
// The implimentation of CRect
CRect::CRect()
{
}

CRect::	CRect(int x1, int y1, int x2, int y2)
{
	left = x1;
	top = y1;
	right = x2;
	bottom = y2;
}

CRect::~CRect()
{
}

inline int CRect::Width() const
{
	int iWidth;
	if (right >= left)
		iWidth = right - left;
	else
		iWidth = left - right;
	return iWidth;
}

inline int CRect::Height() const
{
	int iHeight;
	if (bottom >= top)
		iHeight = bottom - top;
	else
		iHeight = top - bottom;
	return iHeight;
}

inline void CRect::operator =(const RECT &srcRect)
{
	left = srcRect.left;
	right = srcRect.right;
	top = srcRect.top;
	bottom = srcRect.bottom;
}

inline BOOL CRect::PtInRect(POINT point) const
{
	if (left <= point.x && point.x <= right
		&& top <= point.y && point.y <= bottom)
		return TRUE;
	return FALSE;
}

inline void CRect::SetRect(int x1, int y1, int x2, int y2)
{
	left = x1;
	top = y1;
	right = x2;
	bottom = y2;
}

inline void CRect::CopyRect(LPCRECT lpSrcRect)
{
	if (lpSrcRect)
	{
		left = lpSrcRect->left;
		right = lpSrcRect->right;
		top = lpSrcRect->top;
		bottom = lpSrcRect->bottom;
	}
}

inline void CRect::SetRectEmpty()
{
	left = 0;
	right = 0;
	top = 0;
	bottom = 0;
}

inline CRect::operator LPRECT()
{
	return this;
}

inline CRect::operator LPCRECT() const
{
	return this;
}
*/

/////////////////////////////////////////////////////////////////////////////
/*

CPtrArray::CPtrArray()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CPtrArray::~CPtrArray()
{
	delete[] (BYTE*)m_pData;
}

void CPtrArray::SetSize(int nNewSize, int nGrowBy)
{
//	ASSERT(nNewSize >= 0);

	if (nGrowBy != -1)
		m_nGrowBy = nGrowBy;  // set new size

	if (nNewSize == 0)
	{
		// shrink to nothing
		delete[] (BYTE*)m_pData;
		m_pData = NULL;
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		// create one with exact size
#ifdef SIZE_T_MAX
//		ASSERT(nNewSize <= SIZE_T_MAX/sizeof(void*));    // no overflow
#endif
		m_pData = (void**) new BYTE[nNewSize * sizeof(void*)];

		memset(m_pData, 0, nNewSize * sizeof(void*));  // zero fill

		m_nSize = m_nMaxSize = nNewSize;
	}
	else if (nNewSize <= m_nMaxSize)
	{
		// it fits
		if (nNewSize > m_nSize)
		{
			// initialize the new elements

			memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*));

		}

		m_nSize = nNewSize;
	}
	else
	{
		// otherwise, grow array
		int nGrowBy = m_nGrowBy;
		if (nGrowBy == 0)
		{
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowBy = min(1024, max(4, m_nSize / 8));
		}
		int nNewMax;
		if (nNewSize < m_nMaxSize + nGrowBy)
			nNewMax = m_nMaxSize + nGrowBy;  // granularity
		else
			nNewMax = nNewSize;  // no slush

//		ASSERT(nNewMax >= m_nMaxSize);  // no wrap around
#ifdef SIZE_T_MAX
//		ASSERT(nNewMax <= SIZE_T_MAX/sizeof(void*)); // no overflow
#endif
		void** pNewData = (void**) new BYTE[nNewMax * sizeof(void*)];

		// copy new data from old
		memcpy(pNewData, m_pData, m_nSize * sizeof(void*));

		// construct remaining elements
//		ASSERT(nNewSize > m_nSize);

		memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*));


		// get rid of old stuff (note: no destructors called)
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

int CPtrArray::Append(const CPtrArray& src)
{
//	ASSERT(this != &src);   // cannot append to itself

	int nOldSize = m_nSize;
	SetSize(m_nSize + src.m_nSize);

	memcpy(m_pData + nOldSize, src.m_pData, src.m_nSize * sizeof(void*));

	return nOldSize;
}

void CPtrArray::Copy(const CPtrArray& src)
{
//	ASSERT_VALID(this);
//	ASSERT(this != &src);   // cannot append to itself

	SetSize(src.m_nSize);

	memcpy(m_pData, src.m_pData, src.m_nSize * sizeof(void*));

}

void CPtrArray::FreeExtra()
{
//	ASSERT_VALID(this);

	if (m_nSize != m_nMaxSize)
	{
		// shrink to desired size
#ifdef SIZE_T_MAX
//		ASSERT(m_nSize <= SIZE_T_MAX/sizeof(void*)); // no overflow
#endif
		void** pNewData = NULL;
		if (m_nSize != 0)
		{
			pNewData = (void**) new BYTE[m_nSize * sizeof(void*)];
			// copy new data from old
			memcpy(pNewData, m_pData, m_nSize * sizeof(void*));
		}

		// get rid of old stuff (note: no destructors called)
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CPtrArray::SetAtGrow(int nIndex, void* newElement)
{

	if (nIndex >= m_nSize)
		SetSize(nIndex+1);
	m_pData[nIndex] = newElement;
}

void CPtrArray::InsertAt(int nIndex, void* newElement, int nCount)
{

	if (nIndex >= m_nSize)
	{
		// adding after the end of the array
		SetSize(nIndex + nCount);  // grow so nIndex is valid
	}
	else
	{
		// inserting in the middle of the array
		int nOldSize = m_nSize;
		SetSize(m_nSize + nCount);  // grow it to new size
		// shift old data up to fill gap
		memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
			(nOldSize-nIndex) * sizeof(void*));

		// re-init slots we copied from

		memset(&m_pData[nIndex], 0, nCount * sizeof(void*));

	}

	// insert new value in the gap
//	ASSERT(nIndex + nCount <= m_nSize);



	// copy elements into the empty space
	while (nCount--)
		m_pData[nIndex++] = newElement;

}



void CPtrArray::RemoveAt(int nIndex, int nCount)
{
	// just remove a range
	int nMoveCount = m_nSize - (nIndex + nCount);

	if (nMoveCount)
		memmove(&m_pData[nIndex], &m_pData[nIndex + nCount],
			nMoveCount * sizeof(void*));
	m_nSize -= nCount;
}

void CPtrArray::InsertAt(int nStartIndex, CPtrArray* pNewArray)
{
	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (int i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}
*/
