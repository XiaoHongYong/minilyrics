//
//  BaseWnd.cpp
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/15/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "StdAfx.h"
#include "WidgetLite.h"
#import "WindowMacImp.h"
#import "ViewMacImp.h"

struct _BaseWndInfo
{
	WindowMacImp		*window;
	ViewMacImp			*view;
};

CBaseWnd::CBaseWnd()
{
	m_info = new _BaseWndInfo();
	m_info->window = NULL;
	m_info->view = NULL;
	
	m_parent = NULL;
}

CBaseWnd::~CBaseWnd()
{
	delete m_info;
}

bool CBaseWnd::CreateForSkin(LPCTSTR szClassName, LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, bool bToolWindow, bool bTopmost, bool bVisible)
{
	NSRect frame = NSMakeRect(x, y, nWidth, nHeight);
	WindowMacImp* w = [[WindowMacImp alloc] initWithContentRect:frame
											  styleMask:NSBorderlessWindowMask
												backing:NSBackingStoreBuffered
												  defer:NO];
	[w setOwnerBaseWnd:this];
	
	ViewMacImp* view = [[ViewMacImp alloc] initWithFrame:frame];
	[view setOwnerBaseWnd:this];

	m_parent = pWndParent;
	m_info->window = w;
	m_info->view = view;

	[w setContentView:view];
	[w setBackgroundColor:[NSColor blueColor]];

	if (szCaption)
		[w setTitle: [NSString stringWithUTF8String:szCaption]];

	if (pWndParent && pWndParent->m_info->window)
		[w setParentWindow:pWndParent->m_info->window];

	OnCreate();

	if (bVisible)
		[w makeKeyAndOrderFront:w];
		// [w orderFront:NSApp];

	return true;
}

bool CBaseWnd::Destroy()
{
	if (m_info->window)
		[m_info->window close];

	return true;
}

void CBaseWnd::PostDestroy()
{
	Destroy();
}

bool CBaseWnd::OnCreate()
{
	OnLanguageChanged();

	return true;
}

void CBaseWnd::OnDestroy()
{
	if (m_info->window != NULL)
	{
		m_info->window = NULL;
		m_info->view = NULL;
	}
}

void CBaseWnd::OnPaint(CMLGraphics *pGraph, CRect &rcClip)
{
}

void CBaseWnd::OnLanguageChanged()
{
}

void CBaseWnd::ActivateWindow()
{
}

BOOL CBaseWnd::ShowWindow(int nCmdShow)
{
	if (m_info->window == NULL)
		return false;
	
	switch (nCmdShow) {
	case SW_HIDE:
		[m_info->window orderOut:NSApp];
		break;
	case SW_MAXIMIZE:
		[m_info->window performZoom:NSApp];
		break;
	case SW_SHOW:
		[m_info->window orderFront:NSApp];
		break;
	case SW_MINIMIZE:
		[m_info->window performMiniaturize:NSApp];
		break;
	case SW_RESTORE:
		if (IsZoomed())
			ShowWindow(SW_MAXIMIZE);
		else
			ShowWindow(SW_MINIMIZE);
		break;
	case SW_SHOWMINIMIZED:
			[m_info->window performMiniaturize:NSApp];
		break;
	case SW_SHOWMAXIMIZED:
			[m_info->window performZoom:NSApp];
		break;
	}

	return true;
}

UINT CBaseWnd::GetDlgItemText(int nIDItem, tstring &str)
{
	str.clear();
	return 0;
}

bool CBaseWnd::GetDlgItemRect(int nIDItem, RECT* lpRect)
{
	return false;
}

void CBaseWnd::ScreenToClient(RECT &rc)
{
	if (m_info->window == NULL)
		return;

	NSRect nsrc = NSMakeRect(rc);
	NSRect rcRet = [m_info->window convertRectFromScreen:nsrc];

	NSRectToRect(rcRet, rc);
}

void CBaseWnd::ClientToScreen(RECT &rc)
{
	if (m_info->window == NULL)
		return;

	NSRect nsrc = NSMakeRect(rc);
	NSRect rcRet = [m_info->window convertRectToScreen:nsrc];

	NSRectToRect(rcRet, rc);
}

void CBaseWnd::ScreenToClient(POINT &pt)
{
	if (m_info->window == NULL)
		return;
	
	NSRect nsrc = NSMakeRect(pt.x, pt.y, 0, 0);
	NSRect rcRet = [m_info->window convertRectFromScreen:nsrc];

	pt.x = rcRet.origin.x;
	pt.y = rcRet.origin.y;
}

void CBaseWnd::ClientToScreen(POINT &pt)
{
	if (m_info->window == NULL)
		return;
	
	NSRect nsrc = NSMakeRect(pt.x, pt.y, 0, 0);
	NSRect rcRet = [m_info->window convertRectToScreen:nsrc];
	
	pt.x = rcRet.origin.x;
	pt.y = rcRet.origin.y;
}

bool CBaseWnd::GetWindowRect(RECT* lpRect)
{
	if (m_info->window == NULL)
		return false;
	
	NSRect rcRet = [m_info->window frame];
	NSRectToRect(rcRet, *lpRect);

	return true;
}

bool CBaseWnd::GetClientRect(RECT* lpRect)
{
	if (m_info->view == NULL)
		return false;
	
	NSRect rcRet = [m_info->view frame];
	NSRectToRect(rcRet, *lpRect);

	return true;
}

void CBaseWnd::SetParent(CBaseWnd *pWndParent)
{
	m_parent = pWndParent;

	if (m_info->window == NULL)
		return;

	if (pWndParent && pWndParent->m_info->window)
		[m_info->window setParentWindow:pWndParent->m_info->window];
	else
		[m_info->window setParentWindow:NULL];
}

CBaseWnd *CBaseWnd::GetParent()
{
	return m_parent;
}

bool CBaseWnd::SetTimer(UINT nTimerId, UINT nElapse)
{
	if (m_info->window == NULL)
		return false;

	[m_info->window setTimer:nTimerId duration:nElapse];

	return true;
}

void CBaseWnd::KillTimer(UINT nTimerId)
{
	if (m_info->window)
		[m_info->window killTimer:nTimerId];
}

size_t CBaseWnd::GetWindowText(tstring &str)
{
	str.clear();

	if (m_info->window == NULL)
		return 0;
	
	NSString *s = [m_info->window title];
	if (s == nil)
		return 0;
	
	str = [s UTF8String];

	return str.size();
}

bool CBaseWnd::SetWindowText(LPCTSTR szText)
{
	if (m_info->window != NULL)
		[m_info->window setTitle: [NSString stringWithUTF8String:szText]];
	
	return true;
}

bool CBaseWnd::SetWndCursor(CCursor *pCursor)
{
	return true;
}

bool CBaseWnd::SetFocus()
{
	[m_info->window makeKeyAndOrderFront:m_info->window];
	//[m_info->window orderFront:NSApp];
	// [m_info->window makeKeyWindow];
	return true;
}

bool CBaseWnd::SetWindowRgn(CRegion *pRgn)
{
	return true;
}

bool CBaseWnd::SetCapture()
{
	return true;
}

void CBaseWnd::ReleaseCapture()
{
}

CMLGraphics *CBaseWnd::GetGraphics()
{
	if (m_info->window == NULL)
		return NULL;

	CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];

	CGContextSaveGState(context);
	
	CGContextTranslateCTM(context, 0, [m_info->view frame].size.height);
	CGContextScaleCTM(context, 1.0, -1.0);

	CMLGraphics		*pGraph;

	pGraph = new CMLGraphics;
	pGraph->Attach(context);

	return pGraph;
}

void CBaseWnd::ReleaseGraphics(CMLGraphics *pGraph)
{
	if (m_info->window == NULL)
		return;

	CGContextRestoreGState(pGraph->GetHandle());

	pGraph->Detach();

	delete pGraph;
}

BOOL CBaseWnd::InvalidateRect(const RECT *lpRect, BOOL bErase)
{
	if (m_info->window != NULL) {
		if (lpRect == NULL)
			[m_info->view setNeedsDisplay:YES];
		else {
			int h = lpRect->bottom - lpRect->top;
			[m_info->view setNeedsDisplayInRect: NSMakeRect(lpRect->left, [m_info->view frame].size.height - lpRect->top - h, lpRect->right - lpRect->left, h)];
		}
	}

	return true;
}

bool CBaseWnd::IsChild()
{
	return false;
}

bool CBaseWnd::IsMouseCaptured()
{
	return true;
}

bool CBaseWnd::IsIconic()
{
	if (m_info->window == NULL)
		return false;

	return [m_info->window isMiniaturized] == YES;
}

bool CBaseWnd::IsZoomed()
{
	if (m_info->window == NULL)
		return false;
	
	return [m_info->window isZoomed] == YES;
}

bool CBaseWnd::IsWindow()
{
	return m_info->window != NULL;
}

bool CBaseWnd::IsVisible()
{
	if (m_info->window == NULL)
		return false;
	
	return [m_info->window isVisible] == YES;
}

bool CBaseWnd::IsTopmost()
{
	if (m_info->window == NULL)
		return false;

	return [m_info->window level] == NSMainMenuWindowLevel;
}

bool CBaseWnd::IsSameWnd(CBaseWnd *pWnd)
{
	return this == pWnd;
}

void CBaseWnd::SetTopmost(bool bTopmost)
{
	if (m_info->window == NULL)
		return;

	if (bTopmost)
		[m_info->window setLevel: NSMainMenuWindowLevel];
	else
		[m_info->window setLevel: NSNormalWindowLevel];
}

bool CBaseWnd::IsToolWindow()
{
	if (m_info->window == NULL)
		return false;
	
	return [m_info->window level] == NSFloatingWindowLevel;
}

void CBaseWnd::SetToolWindow(bool bToolWindow)
{
	if (m_info->window == NULL)
		return;
	
	if (bToolWindow)
		[m_info->window setLevel: NSFloatingWindowLevel];
	else
		[m_info->window setLevel: NSNormalWindowLevel];
}

bool CBaseWnd::SetForeground()
{
	if (m_info->window == NULL)
		return false;
	
	[m_info->window orderFront:NSApp];

	return true;
}

void CBaseWnd::SetWindowPos(int x, int y)
{
	if (m_info->window == NULL)
		return;

	[m_info->window setFrameOrigin:NSMakePoint(x, y)];
}

void CBaseWnd::SetWindowPosSafely(int x, int y)
{
	SetWindowPos(x, y);
}

bool CBaseWnd::MoveWindow(int X, int Y, int nWidth, int nHeight, bool bRepaint)
{
	if (m_info->window == NULL)
		return false;
	
	[m_info->window setFrame:NSMakeRect(X, Y, nWidth, nHeight) display: bRepaint ? YES : NO];
	
	return true;
}

bool CBaseWnd::MoveWindowSafely(int X, int Y, int nWidth, int nHeight, bool bRepaint)
{
	return MoveWindow(X, Y, nWidth, nHeight, bRepaint);
}

int CBaseWnd::MessageOut(LPCTSTR lpText, UINT uType, LPCTSTR lpCaption)
{
	NSAlert *alert = [NSAlert alertWithMessageText:[m_info->window title]
									defaultButton:nil
								  alternateButton:nil
									  otherButton:nil
						informativeTextWithFormat:[NSString stringWithUTF8String:lpText]];
	
	[alert runModal];
	
	return IDOK;
}

bool CBaseWnd::ReplaceChildPos(int nIDChildSrcPos, CBaseWnd *pChildNew)
{
	return true;
}

void CBaseWnd::PostUserMessage(int nMessageID, LPARAM param)
{
	if (m_info->window == NULL)
		return;
	
	NSArray *msg = [NSArray arrayWithObjects:[NSNumber numberWithInt:nMessageID], [NSNumber numberWithLong:param], nil];
	
	[m_info->window performSelectorOnMainThread:@selector(onUserMsg)
									 withObject:msg
								  waitUntilDone:false];
}

void CBaseWnd::SetTransparent(BYTE nAlpha, bool bClickThrough)
{
	m_nAlpha = nAlpha;
	m_bClickThrough = bClickThrough;
}

BOOL CBaseWnd::UpdateLayeredWindowUsingMemGraph(CRawGraph *pGraph)
{
	return false;
}

