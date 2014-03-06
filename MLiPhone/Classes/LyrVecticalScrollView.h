//
//  LyrVecticalScrollView.h
//  MiniLyrics
//
//  Created by showbrave on 5/29/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <MediaPlayer/MediaPlayer.h>
#import <CoreText/CoreText.h>

enum FadeMode {
	FM_FADE_IN, // Fade in
	FM_FADE_BG, // Fade to background
	FM_NO_FADE
};

enum AlignMode {
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT,
};

struct LyrDisplayLine {
	CTLineRef	lineText;
	int			startTime, endTime;
	int			width;
};


@interface LyrVecticalScrollView : UIView {

	enum FadeMode				mFadeMode;
	NSTimer						*lyrDisplayTimer;
	MPMusicPlayerController		*musicPlayer;
	
	BOOL						inSeekingLyrics;
	CFMutableArrayRef			m_vLyrLines;
	bool						m_NeedRewrap;

	CFStringRef					m_FontName;
	int							mLineHeight, mLineSpace;
	int							mMarginX, mMarginY;
	enum AlignMode				mHorzAlignMode;

	int							mCurLine;
	int							mTimePlayingCache;
	int							mAlphaLineAbove, mAlphaLineBelow;

}

@property (nonatomic, retain)	NSTimer						*lyrDisplayTimer;
@property (nonatomic, retain)	MPMusicPlayerController		*musicPlayer;
@property (nonatomic)			BOOL						inSeekingLyrics;

@property (nonatomic)	int					fontHeight;
@property (nonatomic)	int					mClrHigh, mClrLow, mClrBg, mClrText;
// @property (nonatomic)	Paint.Align mHorzAlignMode = Paint.Align.CENTER;

- (int) getLineByTime:(int)time;

- (void) RewrapLyrics:(CGContextRef) context;

- (void) onLyricsChanged;
- (void) InitLyrView;
- (void) reloadFontSettings;
- (void) onPlaybackStateChanged:(MPMusicPlaybackState)state;
- (void) onUpdateLyrDisplay;

@end
