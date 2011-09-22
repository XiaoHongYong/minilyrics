//
//  MLApp.mm
//  MiniLyrics
//
//  Created by showbrave on 6/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MLApp.h"
#import "../../base/base.h"
#import "../../Utils/Utils.h"
#import "../../MLProtocol/MLProtocol.h"
#import "../../MLProtocol/HttpClient.h"
#import "../../MLLib/MLLib.h"

#import "MLAppNotifyParam.h"

LPCTSTR TOLOCAL(LPCTSTR szString)
{
	NSString *key = [NSString stringWithUTF8String:szString];
	NSString *local = [[NSBundle mainBundle] localizedStringForKey:(key) value:@"" table:nil];
	return [local UTF8String];
}

tstring GetAppDataDir()
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(
														 NSDocumentDirectory,
														 NSUserDomainMask,
														 YES);
	NSString *documentsDir = [paths objectAtIndex:0];
	tstring	str = (const char *)[documentsDir UTF8String];
	DirStringAddSlash(str);
	return str;
}

void GetAppDataDir(TCHAR szDir[])
{
	tstring	str = GetAppDataDir();
	_tcscpy_safe(szDir, MAX_PATH, str.c_str());
}

tstring GetResourceFile(LPCTSTR szFileName)
{
	TCHAR	szTitle[MAX_PATH];
	FileGetTitle(szTitle, CountOf(szTitle), szFileName);
	LPCTSTR szExt = FileGetExt(szFileName);
	if (szExt)
		szExt++;
	else
		szExt = "";

	NSString *name = [[NSString alloc] initWithUTF8String:szTitle];
	NSString *type = [[NSString alloc] initWithUTF8String:szExt];
	NSString *dbFilePath = [[NSBundle mainBundle] pathForResource:name ofType:type];
	
	return [dbFilePath UTF8String];
}

#import "CMLApp.h"

void PostNotifySearchEnd(int nResult)
{
	MLAppNotifyParam	*pNotifyParam = [[MLAppNotifyParam alloc] initWithSearchEndResult:nResult];
	[pNotifyParam performSelectorOnMainThread: @selector (postNoification)
								   withObject: nil
								waitUntilDone: YES];
}

void PostNotifyLyricsDownloaded(LPCTSTR szMediaKey, LPCTSTR szFile)
{
	MLAppNotifyParam	*pNotifyParam = [[MLAppNotifyParam alloc] initWithLyrDownloadEnd:szMediaKey file:szFile];
	[pNotifyParam performSelectorOnMainThread: @selector (postNoification)
								   withObject: nil
								waitUntilDone: YES];
}

void PostNotifyMessage(LPCTSTR szMessage, bool bShowActivityIndicator, bool bInfoMsg)
{
	MLAppNotifyParam	*pNotifyParam = [[MLAppNotifyParam alloc] initWithMessage:szMessage showActivityIndicator:bShowActivityIndicator infoMsg:bInfoMsg];
	[pNotifyParam performSelectorOnMainThread: @selector (postNoification)
								   withObject: nil
								waitUntilDone: YES];
}

void PostNotifyLyricsChanged()
{
	MLAppNotifyParam	*pNotifyParam = [[MLAppNotifyParam alloc] initWithLyrChanged];
	[pNotifyParam performSelectorOnMainThread: @selector (postNoification)
								   withObject: nil
								waitUntilDone: YES];
}
  
UIImage *getAlbumArtWithSize(MPMusicPlayerController *musicPlayer, int width, int height)
{
	MPMediaItem *currentItem = [musicPlayer nowPlayingItem];

	UIImage *artworkImage = NULL;// = noArtworkImage;

	// Get the artwork from the current media item, if it has artwork.
	MPMediaItemArtwork *artwork = [currentItem valueForProperty: MPMediaItemPropertyArtwork];

	// Obtain a UIImage object from the MPMediaItemArtwork object
	if (artwork) {
		artworkImage = [artwork imageWithSize: CGSizeMake (width, height)];
	}

	return artworkImage;
}

@implementation MLApp

@synthesize noArtworkImage;
@synthesize musicPlayer;
@synthesize delegate;

- (id)init:(MPMusicPlayerController *)_musicPlayer
{
    if (self = [super init]) {
        // Initialization code
		self.musicPlayer = _musicPlayer;
		self.noArtworkImage = [UIImage imageNamed: @"no_artwork.png"];
		[[NSNotificationCenter defaultCenter] addObserver: self
			selector:@selector(MLNotification:)
			name:MLAPP_NOTIFY_NAME
			object:nil];

		CMLApp::GetInstance()->Init();
    }
    return self;
}

- (void)MLNotification:(NSNotification *)notification
{
	MLAppNotifyParam *param = [notification object];

	if (param.type == MLNT_LYRICS_SEARCH_END) {
		bool bPopupSearchWnd = false;
		CMLApp::GetInstance()->OnLyricsSearchEnd(param.searchEndResult, bPopupSearchWnd);
		if (bPopupSearchWnd)
			[delegate onPopupLyrSearchView];
	}
	else if (param.type == MLNT_LYRICS_DOWNLOADED) {
		CMLApp::GetInstance()->OnLyricsDownloaded([param mediaKey], [param lyricsFile]);
	}
	else if (param.type == MLNT_MESSAGE) {
		NSString	*msg = [[NSString alloc] initWithUTF8String:[param message]];
		[delegate onMessage:msg showActivityIndicator:[param showActivityIndicator] infoMsg:[param infoMsg]];
	}
	else if (param.type == MLNT_LYRICS_CHANGED) {
		[delegate onLyricsChanged];
	}
	[param release];
}

-(void) onMediaChanged
{
	MPMediaItem *currentItem = [musicPlayer nowPlayingItem];

	NSString	*title = [currentItem valueForProperty: MPMediaItemPropertyTitle];
	NSString	*artist = [currentItem valueForProperty: MPMediaItemPropertyArtist];
	NSString	*album = [currentItem valueForProperty: MPMediaItemPropertyAlbumTitle];
	NSString	*lyrics = [currentItem valueForProperty: MPMediaItemPropertyLyrics];
	NSNumber	*length = [currentItem valueForProperty: MPMediaItemPropertyPlaybackDuration];

	// Create media key
	NSString	*mediaKey = [[NSString alloc] initWithFormat:@"%@", [currentItem valueForProperty: MPMediaItemPropertyPersistentID]];

	CMLApp::GetInstance()->OnMediaChanged([mediaKey UTF8String], [artist UTF8String], [album UTF8String],
										  [title UTF8String], [lyrics UTF8String], (int)([length doubleValue] * 1000));
}

- (void)dealloc
{
	[noArtworkImage				release];
	
    [super dealloc];
}

@end
