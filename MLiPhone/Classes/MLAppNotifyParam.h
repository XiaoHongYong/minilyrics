//
//  MLAppNotifyParam.h
//  MiniLyrics
//
//  Created by Hongyong Xiao on 9/21/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

enum MLNotifyType {
	MLNT_LYRICS_SEARCH_END,
	MLNT_LYRICS_DOWNLOADED,
	MLNT_MESSAGE,
	MLNT_LYRICS_CHANGED,
};

#define MLAPP_NOTIFY_NAME	@"ConverterAdded"


@interface MLAppNotifyParam : NSObject {

enum MLNotifyType		type;

// Online Search End
int					searchEndResult;

// Lyrics downloaded parameter
tstring				mediaKey;
tstring				lyricsFile;

// Message parameter
tstring				message;
bool				showActivityIndicator;
bool				infoMsg;

}

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
