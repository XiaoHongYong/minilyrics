//
//  MLApp.mm
//  MiniLyrics
//
//  Created by showbrave on 6/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MLApp.h"
#include "../../base/mlstdafx.h"
#include "../../base/base.h"
#include "../../Utils/Utils.h"
#include "../../OurMFCLib/OurMFClib.h"
#include "../../gfx/gfx.h"
#include "../../MLProtocol/MLProtocol.h"
#include "../../MLProtocol/HttpClient.h"
#include "../../MLLib/MLLib.h"

#import "LyricsShowGraphBase.h"

#define MLAPP_NOTIFY_NAME	@"ConverterAdded"

enum MLNotifyType {
	MLNT_LYRICS_SEARCH_END,
	MLNT_LYRICS_DOWNLOADED,
	MLNT_MESSAGE,
	MLNT_LYRICS_CHANGED,
};

tstring GetAppDataDir()
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(
														 NSDocumentDirectory,
														 NSUserDomainMask,
														 YES);
	NSString *documentsDir = [paths objectAtIndex:0];
	NSString *app = [documentsDir stringByAppendingPathComponent:@"MiniLyrics.app"];
	return (char *)[app UTF8String];
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

	NSString *name = [[NSString alloc] initWithCString:szTitle];
	NSString *type = [[NSString alloc] initWithCString:szExt];
	NSString *dbFilePath = [[NSBundle mainBundle] pathForResource:name ofType:type];
	
	return [dbFilePath UTF8String];
}

//////////////////////////////////////////////////////////////////////////////////////////

@interface MLAppNotifyParam : NSObject

MLNotifyType		type;

// Online Search End
int					searchEndResult;

// Lyrics downloaded parameter
tstring				mediaKey;
tstring				lyricsFile;

// Message parameter
tstring				message;


@property(nonatomic)	MLNotifyType		type;

@property(nonatomic)	int					searchEndResult;

- (LPCTSTR) mediaKey;
- (void) setMediaKey: (LPCTSTR) newmediaKey;

- (LPCTSTR) lyricsFile;
- (void) setLyricsFile: (LPCTSTR) file;

- (LPCTSTR) message;
- (void) setMessage: (LPCTSTR) msg;

@end

@implementation MLAppNotifyParam

@synthesize type;
@synthesize searchEndResult;

- (id) initWithSearchEndResult:(int) result
{
	if (self = [super init]) {
		self.searchEndResult = result;
		self.type = MLNT_LYRICS_SEARCH_END;
	}
	return self;
}

- (id) initWithLyrDownloadEnd:(LPCTSTR)szMediaKey file:(LPCTSTR)szFile
{
	if (self = [super init]) {
		self.type = MLNT_LYRICS_DOWNLOADED;
		[self setMediaKey:szMediaKey];
		[self setLyricsFile:szFile];
	}
	return self;
}

- (id) initWithMessage:(LPCTSTR)msg
{
	if (self = [super init]) {
		self.type = MLNT_MESSAGE;
		[self setMessage:msg];
	}
	return self;
}

- (id) initWithLyrChanged
{
	if (self = [super init]) {
		self.type = MLNT_LYRICS_CHANGED;
	}
	return self;
}

- (LPCTSTR) mediaKey { return mediaKey.c_str(); }
- (void) setMediaKey: (LPCTSTR) newmediaKey { mediaKey = newmediaKey; }

- (LPCTSTR) lyricsFile { return lyricsFile.c_str(); }
- (void) setLyricsFile: (LPCTSTR) file { lyricsFile = file; }

- (LPCTSTR) message { return message.c_str(); }
- (void) setMessage: (LPCTSTR) msg { message = msg; }

@end
//////////////////////////////////////////////////////////////////////////////////////////

#import "CMLApp.h"


void PostNotifySearchEnd(int nResult)
{
	MLAppNotifyParam	*pNotifyParam = [[MLAppNotifyParam alloc] initWithSearchEndResult:nResult];
	[[NSNotificationCenter defaultCenter] postNotificationName:MLAPP_NOTIFY_NAME object:pNotifyParam];
}

void PostNotifyLyricsDownloaded(LPCTSTR szMediaKey, LPCTSTR szFile)
{
	MLAppNotifyParam	*pNotifyParam = [[MLAppNotifyParam alloc] initWithLyrDownloadEnd:szMediaKey file:szFile];
	[[NSNotificationCenter defaultCenter]
	 postNotificationName:MLAPP_NOTIFY_NAME object:pNotifyParam];
}

void PostNotifyMessage(LPCTSTR szMessage)
{
	MLAppNotifyParam	*pNotifyParam = [[MLAppNotifyParam alloc] initWithMessage:szMessage];
	[[NSNotificationCenter defaultCenter]
	 postNotificationName:MLAPP_NOTIFY_NAME object:pNotifyParam];
}

void PostNotifyLyricsChanged()
{
	MLAppNotifyParam	*pNotifyParam = [[MLAppNotifyParam alloc] initWithLyrChanged];
	[[NSNotificationCenter defaultCenter]
	 postNotificationName:MLAPP_NOTIFY_NAME object:pNotifyParam];
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
		CMLApp::GetInstance()->OnLyricsDownloaded(param.mediaKey, param.lyricsFile);
	}
	else if (param.type == MLNT_MESSAGE) {
		NSString	*msg = [[NSString alloc] initWithCString:[param message]];
		[delegate onMessage:msg];
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

-(UIImage*) getAlbumArtWithWidth:(int)width Height:(int)height
{
	MPMediaItem *currentItem = [musicPlayer nowPlayingItem];

	UIImage *artworkImage = noArtworkImage;

	// Get the artwork from the current media item, if it has artwork.
	MPMediaItemArtwork *artwork = [currentItem valueForProperty: MPMediaItemPropertyArtwork];

	// Obtain a UIImage object from the MPMediaItemArtwork object
	if (artwork) {
		artworkImage = [artwork imageWithSize: CGSizeMake (width, height)];
	}

	return artworkImage;
}


- (void)dealloc
{
	[noArtworkImage				release];
	
    [super dealloc];
}

@end
