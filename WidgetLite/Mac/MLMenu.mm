// Menu.cpp: implementation of the CMLMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WidgetLite.h"
#include "MLMenu.h"

//////////////////////////////////////////////////////////////////////

@interface MLMenuImp : NSObject {
	CBaseWnd		*mBaseWnd;
	
}

- (void) OnCommand:(NSMenuItem *)item;
- (void) setBaseWnd:(CBaseWnd *)pWnd;

@end

@implementation MLMenuImp

- (void) OnCommand:(NSMenuItem *)item
{
	mBaseWnd->OnCommand([item tag], 0);
}

-(BOOL) validateMenuItem:(NSMenuItem *)item
{
	return YES;
}

- (void) setBaseWnd:(CBaseWnd *)pWnd
{
	mBaseWnd = pWnd;
}

@end


BOOL ToLocalMenu(CMLMenu *pMenu)
{
	return FALSE;
}

struct MLMenuInfo
{
	MLMenuImp	*menuImp;
	NSMenu		*menu;
};

//////////////////////////////////////////////////////////////////////

CMLMenu::CMLMenu()
{
	m_info = new MLMenuInfo();
	m_info->menuImp = nil;
	m_info->menu = nil;
}

CMLMenu::CMLMenu(const CMLMenu &src)
{
	m_info = new MLMenuInfo();
	m_info->menuImp = src.m_info->menuImp;
	m_info->menu = src.m_info->menu;
	[m_info->menuImp retain];
	[m_info->menu retain];
}

CMLMenu::~CMLMenu()
{
	Destroy();

	delete m_info;
}

bool CMLMenu::IsValid() const
{
	return m_info->menuImp != nil;
}

void CMLMenu::Destroy()
{
	if (m_info->menu != nil) {
		[m_info->menu release];
		m_info->menu = nil;
	}
	
	if (m_info->menuImp != nil) {
		[m_info->menuImp release];
		m_info->menuImp = nil;
	}
}

void CMLMenu::TrackPopupMenu(int x, int y, CBaseWnd *pWnd, RECT *prcNotOverlap)
{
	NSPoint pt = { x, y };
	[m_info->menuImp setBaseWnd:pWnd];
	@try {
		[m_info->menu popUpMenuPositioningItem: [m_info->menu itemAtIndex:0]
									atLocation: pt
										inView: nil];
	}
	@catch (NSException *exception) {
	}
}

void CMLMenu::TrackPopupSubMenu(int x, int y, int nSubMenu, CBaseWnd *pWnd, RECT *prcNotOverlap)
{
	NSPoint pt = { x, y };
	[m_info->menuImp setBaseWnd:pWnd];
	NSMenuItem *item = [m_info->menu itemAtIndex:nSubMenu];
	if (item == nil || [item hasSubmenu] != YES)
		return;

	[m_info->menu popUpMenuPositioningItem: [[item submenu] itemAtIndex:0]
								atLocation: pt
									inView: nil];
}

void CMLMenu::EnableItem(int nID, bool bEnable)
{
	NSMenuItem *item = [m_info->menu itemWithTag:nID];
	if (item != nil)
		[item setEnabled:YES];
}

void CMLMenu::CheckItem(int nID, bool bCheck)
{
	NSMenuItem *item = [m_info->menu itemWithTag:nID];
	if (item != nil)
		[item setState:NSOnState];
}

void CMLMenu::CheckRadioItem(int nID, int nStartID, int nEndID)
{
	for (int i = nStartID; i <= nEndID; i++)
		CheckItem(i, i == nID);
}

bool CMLMenu::CreatePopupMenu()
{
	Destroy();

	m_info->menuImp = [MLMenuImp alloc];
	m_info->menu = [[NSMenu alloc] initWithTitle:@"Menu"];
	[m_info->menu setAutoenablesItems:NO];

	return true;
}


int CMLMenu::GetItemCount()
{
	return [m_info->menu numberOfItems];
}

void CMLMenu::AppendItem(UINT nID, LPCTSTR szText, LPCTSTR szShortcutKey)
{
	NSString *title = [NSString stringWithUTF8String:szText];
	NSMenuItem *item = [m_info->menu addItemWithTitle:title
											   action:@selector(OnCommand:)
										keyEquivalent:[NSString stringWithUTF8String:szShortcutKey]];
	[item setTarget:m_info->menuImp];
	[item setTag:nID];
}

CMLMenu CMLMenu::AppendSubmenu(LPCTSTR szText)
{
	NSString *title = [NSString stringWithUTF8String:szText];
	NSMenuItem *item = [m_info->menu addItemWithTitle:title
												  action:@selector(OnCommand:)
										   keyEquivalent:@""];
	[item setTarget:m_info->menuImp];
	NSMenu *submenu = [[NSMenu alloc] initWithTitle:@""];
	[item setSubmenu:submenu];

	CMLMenu menu;
	menu.m_info->menu = submenu;
	menu.m_info->menuImp = m_info->menuImp;
	[menu.m_info->menuImp retain];
	
	return menu;
}

void CMLMenu::AppendSeperator()
{
	[m_info->menu addItem:[NSMenuItem separatorItem]];
}

void CMLMenu::InsertItem(int nPos, UINT nID, LPCTSTR szText, LPCTSTR szShortcutKey)
{
	NSString *title = [NSString stringWithUTF8String:szText];
	NSMenuItem *item = [m_info->menu insertItemWithTitle:title
												  action:@selector(OnCommand:)
										   keyEquivalent:[NSString stringWithUTF8String:szShortcutKey]
												 atIndex:nPos];
	[item setTarget:m_info->menuImp];
	[item setTag:nID];
}

void CMLMenu::InsertItemByID(UINT nPosID, UINT nID, LPCTSTR szText, LPCTSTR szShortcutKey)
{
	int pos = [m_info->menu indexOfItemWithTag:nID];
	if (pos == -1)
		AppendItem(nID, szText, szShortcutKey);
	else
		InsertItem(pos, nID, szText, szShortcutKey);
}

CMLMenu CMLMenu::InsertSubmenu(int nPos, LPCTSTR szText)
{
	NSString *title = [NSString stringWithUTF8String:szText];
	NSMenuItem *item = [m_info->menu insertItemWithTitle:title
												  action:@selector(OnCommand:)
										   keyEquivalent:@""
												 atIndex:nPos];
	[item setTarget:m_info->menuImp];
	NSMenu *submenu = [[NSMenu alloc] initWithTitle:@""];
	[item setSubmenu:submenu];

	CMLMenu menu;
	menu.m_info->menu = submenu;
	menu.m_info->menuImp = m_info->menuImp;
	[menu.m_info->menuImp retain];

	return menu;
}

void CMLMenu::InsertSeperator(int nPos)
{
	[m_info->menu insertItem: [NSMenuItem separatorItem] atIndex: nPos];
}

void CMLMenu::RemoveItem(int nPos)
{
	[m_info->menu removeItemAtIndex:nPos];
}

bool CMLMenu::GetMenuItemText(UINT nItem, tstring &strText, bool bByPosition)
{
	UINT		nID;
	bool		bSubMenu;

	return GetMenuItemInfo(nItem, strText, nID, bSubMenu, bByPosition);
}

bool CMLMenu::GetMenuItemInfo(UINT nItem, tstring &strText, UINT &nID, bool &bSubMenu, bool bByPosition)
{
	NSMenuItem *item = nil;
	@try {
		if (bByPosition)
			item = [m_info->menu itemAtIndex:nItem];
		else
			item = [m_info->menu itemWithTag:nItem];
	}
	@catch (NSException *exception) {
		return false;
	}
	if (item == nil)
		return false;
	
	strText = [[item title] UTF8String];
	nID = [item tag];
	bSubMenu = [item hasSubmenu] == YES;

	return true;
}

bool CMLMenu::HasItem(int nPos)
{
	@try {
		return [m_info->menu itemAtIndex:nPos] != nil;
	}
	@catch (NSException *exception) {
		return false;
	}
}


bool CMLMenu::HasSubmenu(int nPos)
{
	@try {
		NSMenuItem *item = [m_info->menu itemAtIndex:nPos];
		if (item == nil)
			return false;

		return [item hasSubmenu] == YES;
	}
	@catch (NSException *exception) {
		return false;
	}
}

CMLMenu CMLMenu::GetSubmenu(int nPos)
{
	CMLMenu menu;

	NSMenuItem *item = [m_info->menu itemAtIndex:nPos];
	if (item == nil)
		return menu;

	menu.m_info->menu = [item submenu];
	[menu.m_info->menu retain];
	menu.m_info->menuImp = m_info->menuImp;
	[menu.m_info->menuImp retain];

	return menu;
}

CMLMenu & CMLMenu::operator = (CMLMenu &menu)
{
	m_info->menu = menu.m_info->menu;
	m_info->menuImp = menu.m_info->menuImp;

	[m_info->menu retain];
	[m_info->menuImp retain];

	return *this;
}

