//
//  LyrStaticTextView.h
//  MiniLyrics
//
//  Created by Xiao Hongyong on 7/17/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface LyrStaticTextView : UITextView {

}

- (void) onLyricsChanged;
- (void) InitLyrView;
- (void) reloadFontSettings;

@end
