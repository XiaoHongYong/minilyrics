#ifndef _WIN_TYPE_H
#define _WIN_TYPE_H

#ifdef _LINUX_GTK2
#include "linux/keymap.h"
#endif

#if !defined(WIN32) && !defined(_WIN32_WCE)

#define FW_THIN             100
#define FW_EXTRALIGHT       200
#define FW_LIGHT            300
#define FW_NORMAL           400
#define FW_MEDIUM           500
#define FW_SEMIBOLD         600
#define FW_BOLD             700
#define FW_EXTRABOLD        800
#define FW_HEAVY            900

#define FW_ULTRALIGHT       FW_EXTRALIGHT
#define FW_REGULAR          FW_NORMAL
#define FW_DEMIBOLD         FW_SEMIBOLD
#define FW_ULTRABOLD        FW_EXTRABOLD
#define FW_BLACK            FW_HEAVY

#define LPSTR_TEXTCALLBACK     ((LPTSTR)-1L)

#define _stricmp	stricmp
#define _strnicmp	strncasecmp

#define _stscanf	sscanf

#define _tfopen     fopen
#define _fgetts		fgets

#define _mbschr		strchr

/* String functions */

#define _tcschr     strchr
#define _tcscspn    strcspn
#define _tcsncat    strncat
#define _tcsncpy    strncpy
#define _tcspbrk    strpbrk
#define _tcsrchr    strrchr
#define _tcsspn     strspn
#define _tcsstr     strstr
#define _tcstok     strtok

#define _tcsnset    _strnset
#define _tcsrev     _strrev
#define _tcsset     _strset

#define _tcscmp     strcmp
#define _tcsicmp    _stricmp
#define _tcsnccmp   strncmp
#define _tcsncmp    strncmp
#define _tcsnicmp   strncasecmp
#define stricmp		strcasecmp
#define strnicmp	strncasecmp

#define _ttoi       atoi
#define _itot		itoa
#define _tstof		atof

#define _tcscat     strcat
#define _tcscpy     strcpy
#define _tcsdup     _strdup

#define _tcslen     strlen

#define _stprintf	sprintf
#define _vsntprintf vsnprintf
#define _sntprintf	snprintf

#define _tprintf printf
#define _T(x)       x

#define _istalnum	isalnum

#define _tcsftime	strftime

#define MAX_PATH          260

/////////////////////////////////////////////////////////////////////////////
// CSize - An extent, similar to Windows SIZE structure.

class CPoint;
class CRect;

typedef struct  tagSIZE
{
    long cx;
    long cy;
}SIZE, *LPSIZE;

#ifdef _MAC_OSX
typedef struct  tagPOINT
{
	long x;
	long y;
}POINT, *LPPOINT;
#endif

#ifdef _LINUX_GTK2
typedef GdkPoint	tagPOINT;
typedef GdkPoint	POINT;
typedef GdkPoint	*LPPOINT;
#endif

typedef struct  tagRECT
{
	long left;
	long top;
	long right;
	long bottom;
}RECT, *LPRECT;

class CSize : public tagSIZE
{
public:

// Constructors
	// construct an uninitialized size
	CSize();
	// create from two integers
	CSize(int initCX, int initCY);
	// create from another size
	CSize(SIZE initSize);
	// create from a point
	CSize(POINT initPt);
	// create from a DWORD: cx = LOWORD(dw) cy = HIWORD(dw)
	CSize(DWORD dwSize);

// Operations
	BOOL operator==(SIZE size) const;
	BOOL operator!=(SIZE size) const;
	void operator+=(SIZE size);
	void operator-=(SIZE size);

// Operators returning CSize values
	CSize operator+(SIZE size) const;
	CSize operator-(SIZE size) const;
	CSize operator-() const;

// Operators returning CPoint values
	CPoint operator+(POINT point) const;
	CPoint operator-(POINT point) const;

// Operators returning CRect values
	CRect operator+(const RECT* lpRect) const;
	CRect operator-(const RECT* lpRect) const;
};

/////////////////////////////////////////////////////////////////////////////
// CPoint - A 2-D point, similar to Windows POINT structure.

class CPoint : public tagPOINT
{
public:
// Constructors

	// create an uninitialized point
	CPoint();
	// create from two integers
	CPoint(int initX, int initY);
	// create from another point
	CPoint(POINT initPt);
	// create from a size
	CPoint(SIZE initSize);
	// create from a dword: x = LOWORD(dw) y = HIWORD(dw)
	CPoint(DWORD dwPoint);

// Operations

// translate the point
	void Offset(int xOffset, int yOffset);
	void Offset(POINT point);
	void Offset(SIZE size);

	BOOL operator==(POINT point) const;
	BOOL operator!=(POINT point) const;
	void operator+=(SIZE size);
	void operator-=(SIZE size);
	void operator+=(POINT point);
	void operator-=(POINT point);

// Operators returning CPoint values
	CPoint operator+(SIZE size) const;
	CPoint operator-(SIZE size) const;
	CPoint operator-() const;
	CPoint operator+(POINT point) const;

// Operators returning CSize values
	CSize operator-(POINT point) const;

// Operators returning CRect values
	CRect operator+(const RECT* lpRect) const;
	CRect operator-(const RECT* lpRect) const;
};

/////////////////////////////////////////////////////////////////////////////
// CRect - A 2-D rectangle, similar to Windows RECT structure.

typedef const RECT* LPCRECT;    // pointer to read/only RECT

class CRect : public tagRECT
{
public:

// Constructors

	// uninitialized rectangle
	CRect();
	// from left, top, right, and bottom
	CRect(int l, int t, int r, int b);
	// copy constructor
	CRect(const RECT& srcRect);
	// from a pointer to another rect
	CRect(LPCRECT lpSrcRect);
	// from a point and size
	CRect(POINT point, SIZE size);
	// from two points
	CRect(POINT topLeft, POINT bottomRight);

// Attributes (in addition to RECT members)

	// retrieves the width
	int Width() const;
	// returns the height
	int Height() const;
	// returns the size
	CSize Size() const;
	// reference to the top-left point
	CPoint& TopLeft();
	// reference to the bottom-right point
	CPoint& BottomRight();
	// const reference to the top-left point
	const CPoint& TopLeft() const;
	// const reference to the bottom-right point
	const CPoint& BottomRight() const;
	// the geometric center point of the rectangle
	CPoint CenterPoint() const;
	// swap the left and right
	void SwapLeftRight();
	static void SwapLeftRight(LPRECT lpRect);

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	operator LPRECT();
	operator LPCRECT() const;

	// returns TRUE if rectangle has no area
	BOOL IsRectEmpty() const;
	// returns TRUE if rectangle is at (0,0) and has no area
	BOOL IsRectNull() const;
	// returns TRUE if point is within rectangle
	BOOL PtInRect(POINT point) const;

// Operations

	// set rectangle from left, top, right, and bottom
	void SetRect(int x1, int y1, int x2, int y2);
	void SetRect(POINT topLeft, POINT bottomRight);
	// empty the rectangle
	void SetRectEmpty();
	// copy from another rectangle
	void CopyRect(LPCRECT lpSrcRect);
	// TRUE if exactly the same as another rectangle
	BOOL EqualRect(LPCRECT lpRect) const;

	// inflate rectangle's width and height without
	// moving its top or left
	void InflateRect(int x, int y);
	void InflateRect(SIZE size);
	void InflateRect(LPCRECT lpRect);
//	void InflateRect(int l, int t, int r, int b);
	// deflate the rectangle's width and height without
	// moving its top or left
	void DeflateRect(int x, int y);
	void DeflateRect(SIZE size);
	void DeflateRect(LPCRECT lpRect);
//	void DeflateRect(int l, int t, int r, int b);

	// translate the rectangle by moving its top and left
	void OffsetRect(int x, int y);
	void OffsetRect(SIZE size);
	void OffsetRect(POINT point);
//	void NormalizeRect();

	// set this rectangle to intersection of two others
	BOOL IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2);

	// set this rectangle to bounding union of two others
	BOOL UnionRect(LPCRECT lpRect1, LPCRECT lpRect2);

	// set this rectangle to minimum of two others
//	BOOL SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2);

// Additional Operations
	void operator=(const RECT& srcRect);
	BOOL operator==(const RECT& rect) const;
	BOOL operator!=(const RECT& rect) const;
	void operator+=(POINT point);
	void operator+=(SIZE size);
//	void operator+=(LPCRECT lpRect);
	void operator-=(POINT point);
	void operator-=(SIZE size);
//	void operator-=(LPCRECT lpRect);
//	void operator&=(const RECT& rect);
//	void operator|=(const RECT& rect);

// Operators returning CRect values
	CRect operator+(POINT point) const;
	CRect operator-(POINT point) const;
	CRect operator+(LPCRECT lpRect) const;
	CRect operator+(SIZE size) const;
	CRect operator-(SIZE size) const;
	CRect operator-(LPCRECT lpRect) const;
//	CRect operator&(const RECT& rect2) const;
//	CRect operator|(const RECT& rect2) const;
//	CRect MulDiv(int nMultiplier, int nDivisor) const;
};

inline BOOL IsRectEmpty(LPCRECT lpRect)
	{ return lpRect->left >= lpRect->right || lpRect->top >= lpRect->bottom; }

inline CSize::CSize()
	{  }
inline CSize::CSize(int initCX, int initCY)
	{ cx = initCX; cy = initCY; }
inline CSize::CSize(SIZE initSize)
	{ *(SIZE*)this = initSize; }
inline CSize::CSize(POINT initPt)
	{ *(POINT*)this = initPt; }
inline CSize::CSize(DWORD dwSize)
	{
		cx = (short)LOWORD(dwSize);
		cy = (short)HIWORD(dwSize);
	}
inline BOOL CSize::operator==(SIZE size) const
	{ return (cx == size.cx && cy == size.cy); }
inline BOOL CSize::operator!=(SIZE size) const
	{ return (cx != size.cx || cy != size.cy); }
inline void CSize::operator+=(SIZE size)
	{ cx += size.cx; cy += size.cy; }
inline void CSize::operator-=(SIZE size)
	{ cx -= size.cx; cy -= size.cy; }
inline CSize CSize::operator+(SIZE size) const
	{ return CSize(cx + size.cx, cy + size.cy); }
inline CSize CSize::operator-(SIZE size) const
	{ return CSize(cx - size.cx, cy - size.cy); }
inline CSize CSize::operator-() const
	{ return CSize(-cx, -cy); }
inline CPoint CSize::operator+(POINT point) const
	{ return CPoint(cx + point.x, cy + point.y); }
inline CPoint CSize::operator-(POINT point) const
	{ return CPoint(cx - point.x, cy - point.y); }
inline CRect CSize::operator+(const RECT* lpRect) const
	{ return CRect(lpRect) + *this; }
inline CRect CSize::operator-(const RECT* lpRect) const
	{ return CRect(lpRect) - *this; }

// CPoint
inline CPoint::CPoint()
	{ }
inline CPoint::CPoint(int initX, int initY)
	{ x = initX; y = initY; }
inline CPoint::CPoint(POINT initPt)
	{ x = initPt.x; y = initPt.y; }
inline CPoint::CPoint(SIZE initSize)
	{ *(SIZE*)this = initSize; }
inline CPoint::CPoint(DWORD dwPoint)
	{
		x = (short)LOWORD(dwPoint);
		y = (short)HIWORD(dwPoint);
	}
inline void CPoint::Offset(int xOffset, int yOffset)
	{ x += xOffset; y += yOffset; }
inline void CPoint::Offset(POINT point)
	{ x += point.x; y += point.y; }
inline void CPoint::Offset(SIZE size)
	{ x += size.cx; y += size.cy; }
inline BOOL CPoint::operator==(POINT point) const
	{ return (x == point.x && y == point.y); }
inline BOOL CPoint::operator!=(POINT point) const
	{ return (x != point.x || y != point.y); }
inline void CPoint::operator+=(SIZE size)
	{ x += size.cx; y += size.cy; }
inline void CPoint::operator-=(SIZE size)
	{ x -= size.cx; y -= size.cy; }
inline void CPoint::operator+=(POINT point)
	{ x += point.x; y += point.y; }
inline void CPoint::operator-=(POINT point)
	{ x -= point.x; y -= point.y; }
inline CPoint CPoint::operator+(SIZE size) const
	{ return CPoint(x + size.cx, y + size.cy); }
inline CPoint CPoint::operator-(SIZE size) const
	{ return CPoint(x - size.cx, y - size.cy); }
inline CPoint CPoint::operator-() const
	{ return CPoint(-x, -y); }
inline CPoint CPoint::operator+(POINT point) const
	{ return CPoint(x + point.x, y + point.y); }
inline CSize CPoint::operator-(POINT point) const
	{ return CSize(x - point.x, y - point.y); }
inline CRect CPoint::operator+(const RECT* lpRect) const
	{ return CRect(lpRect) + *this; }
inline CRect CPoint::operator-(const RECT* lpRect) const
	{ return CRect(lpRect) - *this; }

// CRect
inline CRect::CRect()
	{  }
inline CRect::CRect(int l, int t, int r, int b)
	{ left = l; top = t; right = r; bottom = b; }
inline CRect::CRect(const RECT& srcRect)
	{ CopyRect(&srcRect); }
inline CRect::CRect(LPCRECT lpSrcRect)
	{ CopyRect(lpSrcRect); }
inline CRect::CRect(POINT point, SIZE size)
	{ right = (left = point.x) + size.cx; bottom = (top = point.y) + size.cy; }
inline CRect::CRect(POINT topLeft, POINT bottomRight)
	{ left = topLeft.x; top = topLeft.y;
		right = bottomRight.x; bottom = bottomRight.y; }
inline int CRect::Width() const
	{ return right - left; }
inline int CRect::Height() const
	{ return bottom - top; }
inline CSize CRect::Size() const
	{ return CSize(right - left, bottom - top); }
inline CPoint& CRect::TopLeft()
	{ return *((CPoint*)this); }
inline CPoint& CRect::BottomRight()
	{ return *((CPoint*)this+1); }
inline const CPoint& CRect::TopLeft() const
	{ return *((CPoint*)this); }
inline const CPoint& CRect::BottomRight() const
	{ return *((CPoint*)this+1); }
inline CPoint CRect::CenterPoint() const
	{ return CPoint((left+right)/2, (top+bottom)/2); }
inline void CRect::SwapLeftRight()
	{ SwapLeftRight(LPRECT(this)); }
inline void CRect::SwapLeftRight(LPRECT lpRect)
	{ long temp = lpRect->left; lpRect->left = lpRect->right; lpRect->right = temp; }
inline CRect::operator LPRECT()
	{ return this; }
inline CRect::operator LPCRECT() const
	{ return this; }
inline BOOL CRect::IsRectEmpty() const
	{ return left >= right || top >= bottom; }
inline BOOL CRect::IsRectNull() const
	{ return (left == 0 && right == 0 && top == 0 && bottom == 0); }
inline BOOL CRect::PtInRect(POINT point) const
	{ return point.x >= left && point.x < right && point.y >= top && point.y < bottom; }
inline void CRect::SetRect(int x1, int y1, int x2, int y2)
	{ left = x1; top = y1; right = x2; bottom = y2; }
inline void CRect::SetRect(POINT topLeft, POINT bottomRight)
	{ SetRect(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y); }
inline void CRect::SetRectEmpty()
	{ left = right = top = bottom = 0; }
inline void CRect::CopyRect(LPCRECT lpSrcRect)
	{
		left = lpSrcRect->left; top = lpSrcRect->top;
		right = lpSrcRect->right; bottom = lpSrcRect->bottom;
	}
inline BOOL CRect::EqualRect(LPCRECT lpRect) const
	{ return left == lpRect->left && top == lpRect->top && right == lpRect->right && bottom == lpRect->bottom; }
inline void CRect::InflateRect(int x, int y)
	{ left -= x; right += x; top -= y; bottom += y; }
inline void CRect::InflateRect(SIZE size)
	{ InflateRect(size.cx, size.cy); }
inline void CRect::DeflateRect(int x, int y)
	{ InflateRect(-x, -y); }
inline void CRect::DeflateRect(SIZE size)
	{ InflateRect(-size.cx, -size.cy); }
inline void CRect::OffsetRect(int x, int y)
	{ left += x; right += x; top += y; bottom += y; }
inline void CRect::OffsetRect(POINT point)
	{ OffsetRect(point.x, point.y); }
inline void CRect::OffsetRect(SIZE size)
	{ OffsetRect(size.cx, size.cy); }
inline BOOL CRect::IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2)
	{
		if (::IsRectEmpty(lpRect1) || ::IsRectEmpty(lpRect2) ||
			(lpRect1->left >= lpRect2->right) || (lpRect2->left >= lpRect1->right) ||
			(lpRect1->top >= lpRect2->bottom) || (lpRect2->top >= lpRect1->bottom))
		{
			SetRectEmpty();
			return FALSE;
		}
		left   = max(lpRect1->left, lpRect2->left);
		right  = min(lpRect1->right, lpRect2->right);
		top    = max(lpRect1->top, lpRect2->top);
		bottom = min(lpRect1->bottom, lpRect2->bottom);
		return TRUE;
	}
inline BOOL CRect::UnionRect(LPCRECT lpRect1, LPCRECT lpRect2)
	{
		if (::IsRectEmpty(lpRect1))
		{
			if (::IsRectEmpty(lpRect2))
			{
				SetRectEmpty();
				return FALSE;
			}
			else
				*this = *lpRect2;
		}
		else
		{
			if (::IsRectEmpty(lpRect2))
				*this = *lpRect1;
			else
			{
				left   = min(lpRect1->left, lpRect2->left);
				right  = max(lpRect1->right, lpRect2->right);
				top    = min(lpRect1->top, lpRect2->top);
				bottom = max(lpRect1->bottom, lpRect2->bottom);
			}
		}
		return TRUE;
	}
inline void CRect::operator=(const RECT& srcRect)
	{ CopyRect(&srcRect); }
inline BOOL CRect::operator==(const RECT& rect) const
	{ return EqualRect(&rect); }
inline BOOL CRect::operator!=(const RECT& rect) const
	{ return !EqualRect(&rect); }
inline void CRect::operator+=(POINT point)
	{ OffsetRect(point.x, point.y); }
inline void CRect::operator+=(SIZE size)
	{ OffsetRect(size.cx, size.cy); }
/*inline void CRect::operator+=(LPCRECT lpRect)
	{
		left += lpRect->left;
		right += lpRect->right;
		top += lpRect->top;
		bottom += lpRect->bottom;
	}*/
inline void CRect::operator-=(POINT point)
	{ OffsetRect(-point.x, -point.y); }
inline void CRect::operator-=(SIZE size)
	{ OffsetRect(-size.cx, -size.cy); }
//inline void CRect::operator-=(LPCRECT lpRect)
//	{
//		left -= lpRect->left;
//		right -= lpRect->right;
//		top -= lpRect->top;
//		bottom -= lpRect->bottom;
//	}
//inline void CRect::operator&=(const RECT& rect)
//	{ ::IntersectRect(this, this, &rect); }
//inline void CRect::operator|=(const RECT& rect)
//	{ ::UnionRect(this, this, &rect); }
inline CRect CRect::operator+(POINT pt) const
	{ CRect rect(*this); rect.OffsetRect(pt.x, pt.y); return rect; }
inline CRect CRect::operator-(POINT pt) const
	{ CRect rect(*this); rect.OffsetRect(-pt.x, -pt.y); return rect; }
inline CRect CRect::operator+(SIZE size) const
	{ CRect rect(*this); rect.OffsetRect(size.cx, size.cy); return rect; }
inline CRect CRect::operator-(SIZE size) const
	{ CRect rect(*this); rect.OffsetRect(-size.cx, -size.cy); return rect; }
//inline CRect CRect::operator+(LPCRECT lpRect) const
//	{ CRect rect(this); rect.InflateRect(lpRect); return rect; }
//inline CRect CRect::operator-(LPCRECT lpRect) const
//	{ CRect rect(this); rect.DeflateRect(lpRect); return rect; }
//inline CRect CRect::operator&(const RECT& rect2) const
//	{ CRect rect; ::IntersectRect(&rect, this, &rect2);
//		return rect; }
//inline CRect CRect::operator|(const RECT& rect2) const
//	{ CRect rect; ::UnionRect(&rect, this, &rect2);
//		return rect; }
//inline BOOL CRect::SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2)
//	{ return ::SubtractRect(this, lpRectSrc1, lpRectSrc2); }

#endif

#endif // _WIN_TYPE_H
