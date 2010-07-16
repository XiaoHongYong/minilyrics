//
//  LyrVecticalScrollView.h
//  MiniLyrics
//
//  Created by showbrave on 5/29/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <MediaPlayer/MediaPlayer.h>


@interface LyrVecticalScrollView : UIView {

	NSTimer						*lyrDisplayTimer;
	MPMusicPlayerController		*musicPlayer;
	BOOL						inSeekingLyrics;

}

@property (nonatomic, retain)	NSTimer						*lyrDisplayTimer;
@property (nonatomic, retain)	MPMusicPlayerController		*musicPlayer;
@property (nonatomic)			BOOL						inSeekingLyrics;

- (void) onLyricsChanged;
- (void) InitLyrView;
- (void) reloadFontSettings;
- (void) moveLyrics: (int) y;

@end
