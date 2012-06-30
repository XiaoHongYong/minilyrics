
@interface FontChooserDelegate : NSObject <NSWindowDelegate>

- (void)windowWillClose:(NSNotification *)notification;

@end

#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgChooseFont.h"
#import <AppKit/AppKit.h>

CDlgChooseFont::CDlgChooseFont()
{
}

CDlgChooseFont::~CDlgChooseFont()
{

}

@implementation FontChooserDelegate

- (void)windowWillClose:(NSNotification *)notification {
	printf("windowWillClose: %s", [[notification name] UTF8String]);
	[NSApp stopModalWithCode:IDOK];
}

@end

int CDlgChooseFont::DoModal(CBaseWnd *pWndParent, LPCTSTR szFontFaceName, int nFontSize, int nWeight, int nItalic)
{
	m_strFontFaceName = szFontFaceName;
	m_nFontSize = nFontSize;
	m_nWeight = nWeight;
	m_nItalic = nItalic;
	
	NSFont *oldFont = [NSFont fontWithName:[NSString stringWithUTF8String:szFontFaceName]
									   size:nFontSize];
	if (oldFont == nil)
		oldFont = [NSFont systemFontOfSize:nFontSize];
	
	NSFontPanel *panel = [NSFontPanel sharedFontPanel];
	if (pWndParent != NULL)
		[panel setParentWindow:(NSWindow*)pWndParent->GetHandle()];
	// [panel setWorksWhenModal:YES];
	[panel setDelegate:[FontChooserDelegate alloc]];
	[panel setPanelFont:oldFont isMultiple:NO];

	int nRet = [NSApp runModalForWindow:panel];
	if (nRet != IDOK)
		return nRet;
	//[panel makeKeyAndOrderFront:nil];
	
	NSFont *newFont = [panel panelConvertFont:oldFont];
	m_strFontFaceName = [[newFont fontName] UTF8String];
	m_nFontSize = [newFont pointSize];
	
	return IDOK;
}

LPCTSTR CDlgChooseFont::GetFaceName()
{
	return m_strFontFaceName.c_str();
}

int CDlgChooseFont::GetSize()
{
	return m_nFontSize;
}

int CDlgChooseFont::GetWeight()
{
	return m_nWeight;
}

int CDlgChooseFont::GetItalic()
{
	return m_nItalic;
}
