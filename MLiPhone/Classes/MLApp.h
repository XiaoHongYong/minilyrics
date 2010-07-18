//
//  MLApp.h
//  MiniLyrics
//
//  Created by showbrave on 6/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <MediaPlayer/MediaPlayer.h>


@protocol MLAppEventDelegate;


@interface MLApp : NSObject {

	MPMusicPlayerController		*musicPlayer;	
	UIImage						*noArtworkImage;

@private
	id<MLAppEventDelegate>	delegate;
	
}

@property (nonatomic, retain)	MPMusicPlayerController		*musicPlayer;	
@property(nonatomic, assign)	id<MLAppEventDelegate>		delegate;
@property (nonatomic, retain)	UIImage						*noArtworkImage;

- (id)init:(MPMusicPlayerController *)_musicPlayer;

-(void) onMediaChanged;

-(UIImage*) getAlbumArtWithWidth:(int)width Height:(int)height;

@end

//
// MiniLyrics notification event delegate
//
@protocol MLAppEventDelegate<NSObject>
@optional

// When lyrics has been opened, this message will be sent.
- (void)onLyricsChanged;

- (void)onMessage:(NSString*) msg;

- (void)onPopupLyrSearchView;

@end
