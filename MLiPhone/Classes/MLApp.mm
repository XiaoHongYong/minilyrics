//
//  MLApp.mm
//  MiniLyrics
//
//  Created by showbrave on 6/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MLApp.h"
#import "../../base/mlstdafx.h"
#import "../../base/base.h"
#import "../../Utils/Utils.h"
#import "../../OurMFCLib/OurMFClib.h"
#import "../../gfx/gfx.h"
#import "../../MLProtocol/MLProtocol.h"
#import "../../MLProtocol/HttpClient.h"
#import "../../MLLib/MLLib.h"

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

void InitSetDefaultCharEncoding()
{
	ENCODING_CODEPAGE &ecp = GetCharEncodingByID(ED_SYSDEF);
	NSLocale *local = [NSLocale currentLocale];
	NSString *lang = [local objectForKey:NSLocaleLanguageCode];
	NSString *countryCode = [local objectForKey: NSLocaleCountryCode];
	
	// Compare country code
	if ([countryCode isEqual: @"CN"] || [countryCode isEqual: @"SG"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_GB2312).nCFStringEncoding;
	else if ([countryCode isEqual: @"TW"] || [countryCode isEqual: @"HK"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_BIG5).nCFStringEncoding;
	else if ([countryCode isEqual: @"JP"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_JAPANESE_SHIFT_JIS).nCFStringEncoding;
	
	// Compare language code
	else if ([lang isEqual: @"ara"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_ARABIC).nCFStringEncoding;
	else if ([lang isEqual: @"bat"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_BALTIC_WINDOWS).nCFStringEncoding;
	else if ([lang isEqual: @"gre"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_GREEK_WINDOWS).nCFStringEncoding;
	else if ([lang isEqual: @"heb"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_HEBREW_WINDOWS).nCFStringEncoding;
	else if ([lang isEqual: @"tha"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_THAI).nCFStringEncoding;
	else if ([lang isEqual: @"tur"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_TURKISH_WINDOWS).nCFStringEncoding;
	else if ([lang isEqual: @"vie"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_VIETNAMESE).nCFStringEncoding;
	else if ([lang isEqual: @"rus"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_RUSSIAN_WINDOWS).nCFStringEncoding;
	else if ([lang isEqual: @"kor"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_KOREAN).nCFStringEncoding;
}

int ConvertMultiByteToUtf8(int nEncodingID, const char *lpByteStr, int nSizeMultibyte, char *lpUtf8Str, int nSizeUtf8)
{
	EmptyStr(lpUtf8Str);

	NSString	*str;
	// kCFStringEncodingGB_18030_2000
	str = [NSString stringWithCString:lpByteStr encoding:(NSStringEncoding)
		   CFStringConvertEncodingToNSStringEncoding(
				GetCharEncodingByID((CHAR_ENCODING)nEncodingID).nCFStringEncoding)];
	if (!str)
		str = [NSString stringWithCString:lpByteStr encoding:(NSStringEncoding)NSISOLatin1StringEncoding];
	if (str)
		strcpy_safe(lpUtf8Str, nSizeUtf8, [str UTF8String]);

	return strlen(lpUtf8Str);
}

int ConvertUtf8ToMultiByte(int nEncodingID, const char *lpUtf8Str, int nSizeUtf8, char *lpMbcsStr, int nSizeMultibyte)
{
	EmptyStr(lpMbcsStr);
	
	NSString	*str;
	str = [NSString stringWithUTF8String:lpUtf8Str];
	if (str)
		[str getCString:lpMbcsStr
			  maxLength:nSizeMultibyte
			   encoding:CFStringConvertEncodingToNSStringEncoding(
																  GetCharEncodingByID((CHAR_ENCODING)nEncodingID).nCFStringEncoding)];
	
	return strlen(lpMbcsStr);	
}

int ConvertMultiByteToWideChar(int nEncodingID, const char *lpByteStr, int nSizeMultibyte, WCHAR *lpWideStr, int nSizeWideChar)
{
	EmptyStr(lpWideStr);
	
	NSString	*str;
	// kCFStringEncodingGB_18030_2000
	str = [NSString stringWithCString:lpByteStr encoding:(NSStringEncoding)
		   CFStringConvertEncodingToNSStringEncoding(
													 GetCharEncodingByID((CHAR_ENCODING)nEncodingID).nCFStringEncoding)];
	if (!str)
		str = [NSString stringWithCString:lpByteStr encoding:(NSStringEncoding)NSISOLatin1StringEncoding];
	if (str)
		[str getCharacters:(unichar *)lpWideStr range: NSMakeRange(0, [str length])];
	
	return wcslen(lpWideStr);
}

int ConvertWideCharToMultiByte(int nEncodingID, const WCHAR *lpWideStr, int nSizeWideChar, char *lpByteStr, int nSizeMultibyte)
{
	EmptyStr(lpByteStr);
	
	NSString	*str;
	str = [[NSString alloc] initWithCharacters:(const unichar*)lpWideStr length:nSizeWideChar];
	if (str) {
		[str getCString:lpByteStr
			  maxLength:nSizeMultibyte
			   encoding:CFStringConvertEncodingToNSStringEncoding(
						GetCharEncodingByID((CHAR_ENCODING)nEncodingID).nCFStringEncoding)];
	}

	return strlen(lpByteStr);
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
bool				showActivityIndicator;
bool				infoMsg;


@property(nonatomic)	MLNotifyType		type;

@property(nonatomic)	int					searchEndResult;

@property(nonatomic)	bool				showActivityIndicator;
@property(nonatomic)	bool				infoMsg;

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
@synthesize showActivityIndicator;
@synthesize infoMsg;

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

- (id) initWithMessage:(LPCTSTR)msg showActivityIndicator:(bool)bShow infoMsg:(bool)bInfoMsg
{
	if (self = [super init]) {
		self.type = MLNT_MESSAGE;
		self.showActivityIndicator = bShow;
		self.infoMsg = bInfoMsg;
		tstring	str = msg;
		str += _T("\n\n\n\n");
		[self setMessage:str.c_str()];
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

- (void) postNoification
{
	[[NSNotificationCenter defaultCenter] postNotificationName:MLAPP_NOTIFY_NAME object:self];
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
		CMLApp::GetInstance()->OnLyricsDownloaded(param.mediaKey, param.lyricsFile);
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
