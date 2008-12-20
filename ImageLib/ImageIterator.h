// ImageIterator.h: interface for the CImageIterator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGEITERATOR_H__9809A1BB_21FC_4EA5_854E_4DDAB59AC607__INCLUDED_)
#define AFX_IMAGEITERATOR_H__9809A1BB_21FC_4EA5_854E_4DDAB59AC607__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CImageIterator
{
protected:
	int			m_xbyte, m_y;		// Counters
	int			m_stepx, m_stepy;
	BYTE*		m_pRow;	//  Image pointer
	PRAW_IMAGE_DATA	m_pimage;

public:
	// Constructors
	CImageIterator(void)
	{
		m_xbyte = 0;
		m_y = 0;
		m_stepx = 0;
		m_stepy = 0;
		m_pimage = NULL;
		m_pRow = NULL;
	}
	CImageIterator(PRAW_IMAGE_DATA img)
	{
		m_xbyte = 0;
		m_y = 0;
		m_stepx = 0;
		m_stepy = 0;
		m_pimage = img;
		m_pRow = m_pimage->RowPtr(0);
	}
	operator PRAW_IMAGE_DATA()
	{
		return m_pimage;
	}

	// Iterators
	bool ItOK()
	{
		if (m_pimage && m_pimage->buff
			&& m_xbyte >= 0 && m_xbyte < m_pimage->width
			&& m_y >= 0 && m_y < m_pimage->height)
			return true;
		return false;
	}
	void Reset()
	{
		m_xbyte = 0;
		m_y = 0;
		if (m_pimage)
			m_pRow = m_pimage->RowPtr(0);
		else
			m_pRow = NULL;
	}
	void Upset()
	{
		m_xbyte = 0;
		if (m_pimage)
		{
			m_y = m_pimage->height - 1;
			m_pRow = m_pimage->RowPtr(m_y);
		}
		else
		{
			m_y = 0;
			m_pRow = NULL;
		}
	}
	void SetRow(BYTE *buf, int n)
	{
		if (n < 0)
			n = m_pimage->AbsStride();
		else
			n = min(n, (int)m_pimage->AbsStride());

		if (m_pRow)
			memcpy(m_pRow, buf, n);
	}
	void GetRow(BYTE *buf, int n)
	{
		ASSERT(n <= m_pimage->AbsStride());
		if (buf && n > 0 && n <= m_pimage->AbsStride())
		{
			memcpy(buf, m_pRow, n);
		}
	}
	BYTE GetByte( ) { return m_pRow[m_xbyte]; }
	void SetByte(BYTE b) { m_pRow[m_xbyte] = b; }
	BYTE* GetRow(void) { return m_pRow; }
	BYTE* GetRow(int nRow)
	{
		SetY(nRow);
		return m_pRow;
	}
	BOOL NextRow()
	{
		m_y++;
		if (m_y >= m_pimage->height)
			return false;
		m_pRow += m_pimage->stride;

		return true;
	}
	BOOL PrevRow()
	{
		m_y--;
		if (m_y < 0)
			return false;
		m_pRow -= m_pimage->stride;

		return true;
	}
	BOOL NextByte()
	{
		m_xbyte++;
		if (m_xbyte < (int)m_pimage->AbsStride())
			return true;
		else 
		{
			m_y++;
			if (m_y < (int)m_pimage->height)
			{
				m_pRow += m_pimage->stride;
				m_xbyte = 0;
				return true;
			}
			else
				return false;
		}
	}
	BOOL PrevByte()
	{
		m_xbyte--;
		if (m_xbyte >= 0)
			return true;
		else
		{
			m_y--;
			if (m_y >= 0 && m_y < (int)m_pimage->height)
			{
				m_pRow -= m_pimage->stride;
				m_xbyte = 0;
				return true;
			}
			else
				return false;
		}
	}

	void SetSteps(int x, int y=0) {  m_stepx = x; m_stepy = y; }
	void GetSteps(int &x, int &y) {  x = m_stepx; y = m_stepy; }
	BOOL NextStep()
	{
		m_xbyte += m_stepx;
		if (m_xbyte < (int)m_pimage->AbsStride())
			return true;
		else
		{
			m_y += m_stepy;
			if (m_y < (int)m_pimage->height)
			{
				m_pRow += m_pimage->stride;
				m_xbyte = 0;
				return true;
			}
			else
				return false;
		}
	}
	BOOL PrevStep()
	{
		m_xbyte -= m_stepx;
		if (m_xbyte >= 0)
			return true;
		else
		{
			m_y -= m_stepy;
			if (m_y >= 0 && m_y < (int)m_pimage->height)
			{
				m_pRow -= m_pimage->stride;
				m_xbyte = 0;
				return true;
			}
			else
				return false;
		}
	}
	void SetY(int y)
	{
		if (y < 0 || y >= m_pimage->height)
			return;
		m_y = y;
		m_pRow = m_pimage->RowPtr(y);
	}
	int  GetY() { return m_y; }
};

#endif // !defined(AFX_IMAGEITERATOR_H__9809A1BB_21FC_4EA5_854E_4DDAB59AC607__INCLUDED_)
