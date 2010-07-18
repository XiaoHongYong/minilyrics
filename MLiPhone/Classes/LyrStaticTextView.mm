//
//  LyrStaticTextView.m
//  MiniLyrics
//
//  Created by Xiao Hongyong on 7/17/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "LyrStaticTextView.h"
#import "LyricsVerticalScrollGraph.h"
#import "MLApp.h"
#import "CMLApp.h"


@implementation LyrStaticTextView


- (id)initWithFrame:(CGRect)frame {
    if ((self = [super initWithFrame:frame])) {
        // Initialization code
    }
    return self;
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/

- (void)dealloc {
    [super dealloc];
}

#pragma mark Lyrics static text view interface

- (void) onLyricsChanged
{
	CMLData &lyrData = CMLApp::GetInstance()->GetLyricsData();
	VLyrLines &lyrLines = lyrData.GetRawLyrics();

	string	strLyrics;
	for (int i = 0; i < lyrLines.size(); i++)
	{
		PML_ROW pLine = lyrLines[i];
		if (pLine->vFrags.size() <= 0)
			continue;
		PLYRIC_FRAG pFrag = pLine->vFrags[0];
		strLyrics += pFrag->szLyric;
		strLyrics += "\r\n";
	}

	[self setText:[NSString stringWithUTF8String:strLyrics.c_str()]];
}

- (void) InitLyrView
{
	[self reloadFontSettings];
	
}

- (void) reloadFontSettings
{
	{
		int			nFontHeight;
		int			nWeight;
		BYTE		byItalic;
		tstring		strFaceName, strFaceNameOthers;
		
		ProfileGetLyricsFont(SZ_SECT_LYR_DISPLAY, nFontHeight, nWeight, byItalic, strFaceName, strFaceNameOthers);
		
		UIFont *font = [UIFont boldSystemFontOfSize:(float)nFontHeight];
		[self setFont:font];
	}
}

@end
