//
//  DlgSaveEmbeddedLyricsController.m
//  MiniLyricsMac
//
//  Created by Xiao Hongyong on 6/7/13.
//
//

#import "DlgSaveEmbeddedLyricsController.h"

#import "stdafx.h"
#import "MPlayerApp.h"
#import "DlgSaveEmbeddedLyrics.h"

tstring				m_strMediaUrl;
CCharBuffer			m_bufLyrics;

uint32				m_nDefaultEmbededLST;
bool				m_bHasEmbeddedLyrAlready;

@implementation DlgSaveEmbeddedLyricsController


- (void) doModal:(CDlgSaveEmbeddedLyrics *)dialog
{
	mDialg = dialog;
	[NSApp beginSheet: [self window]
	   modalForWindow: (NSWindow*)CMPlayerAppBase::GetMainWnd()->GetHandle()// nil//(NSWindow*)dialog->GetHandle()
		modalDelegate: nil
	   didEndSelector: nil
		  contextInfo: nil];
	
	[NSApp runModalForWindow: [self window]];
    // Dialog is up here.
    [NSApp endSheet: [self window]];
	[[self window] orderOut: self];
}

- (IBAction)buttonOK:(id)sender
{
	[NSApp stopModal];
	mDialg->OnOK();
}

- (IBAction)buttonCancel:(id)sender
{
	[NSApp stopModal];
}

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
		
		_tableDesc = [NSMutableArray new];
		_tableName = [NSMutableArray new];
		_tableCheck = [NSMutableArray new];
    }
    
    return self;
}

- (void)addItem:(const char *)text name:(const char *)name isChecked:(bool)isChecked
{
	[_tableDesc addObject:[NSString stringWithUTF8String:text]];
	[_tableName addObject:[NSString stringWithUTF8String:name]];
	[_tableCheck addObject:[NSNumber numberWithBool: isChecked]];
}

- (int)getItemCount
{
	return (int)[_tableDesc count];
}

- (const char*)getNameAtIndex:(long)index
{
	return [[_tableName objectAtIndex:index] UTF8String];
}

- (bool)getCheckAtIndex:(long)index
{
	return [[_tableCheck objectAtIndex:index] boolValue];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
	
	NSTableColumn *column = [[[self tableView] tableColumns] objectAtIndex:0];
	[[column headerCell] setStringValue: [NSString stringWithUTF8String: _TLT("Embedded Lyrics Type")]];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return [_tableName count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
	if ([[tableColumn identifier] isEqualTo:@"check"])
		return [_tableCheck objectAtIndex:row];
	else if ([[tableColumn identifier] isEqualTo:@"desc"])
		return [_tableDesc objectAtIndex:row];
	else
		// Return the same value for any table column -- it is just a string
		return [_tableName objectAtIndex:row];
}

- (void)tableView:(NSTableView *)tableView setObjectValue:(id)value forTableColumn:(NSTableColumn *)column row:(NSInteger)row {
	[_tableCheck replaceObjectAtIndex:row withObject:value];
    [_tableView reloadData];
}

@end
