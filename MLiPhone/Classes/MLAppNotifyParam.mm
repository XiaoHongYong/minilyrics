//
//  MLAppNotifyParam.m
//  MiniLyrics
//
//  Created by Hongyong Xiao on 9/21/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "../../base/base.h"
#import "../../Utils/Utils.h"

#import "MLAppNotifyParam.h"

@implementation MLAppNotifyParam

- (id)init
{
    self = [super init];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

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
