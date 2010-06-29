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

}

@property (nonatomic, retain)	NSTimer						*lyrDisplayTimer;
@property(nonatomic, retain)	MPMusicPlayerController		*musicPlayer;

-(void) onLyricsChanged;
- (void) InitLyrView;

@end
