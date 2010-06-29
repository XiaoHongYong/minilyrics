//
//  LyrVecticalScrollView.m
//  MiniLyrics
//
//  Created by showbrave on 5/29/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "LyrVecticalScrollView.h"
#import "../../base/mlstdafx.h"
#import "../../base/base.h"
#import "../../Utils/Utils.h"
#import "../../gfx/gfx.h"
#import "../../GfxRaw/GfxRaw.h"
#import "../../MLLib/MLLib.h"
#import "CMLApp.h"

#import "LyricsVerticalScrollGraph.h"


@implementation LyrVecticalScrollView

@synthesize lyrDisplayTimer;
@synthesize musicPlayer;


CRawGraph m_memGraph;
CMLFont		m_font;
CRawBmpFont	m_bmpFont;

CLyricsVerticalScrollGraph	m_lyrGraph;

- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        // Initialization code
		m_memGraph.Create(frame.size.width, frame.size.height, NULL, 32);
    }
    return self;
}

- (void) InitLyrView
{
	m_lyrGraph.Init(self.frame.size.width, self.frame.size.height, &CMLApp::GetInstance()->GetLyricsData());

	[self setLyrDisplayTimer: [NSTimer scheduledTimerWithTimeInterval: 0.04
																	target: self
																  selector: @selector (onUpdateLyrDisplay)
																  userInfo: nil
																   repeats: YES]];
}

void DrawTextTo(CGContextRef context, CGRect rect)
{
	CGContextSetRGBFillColor(context, 1.0, 1.0, 1.0, 1.0);
	// CGContextSetRGBStrokeColor(context, 1.0, 0.0, 0.0, 1.0);
	
	// Some initial setup for our text drawing needs.
	// First, we will be doing our drawing in Helvetica-36pt with the MacRoman encoding.
	// This is an 8-bit encoding that can reference standard ASCII characters
	// and many common characters used in the Americas and Western Europe.
	CGContextSelectFont(context, "Helvetica", 15, kCGEncodingMacRoman);
	// Next we set the text matrix to flip our text upside down. We do this because the context itself
	// is flipped upside down relative to the expected orientation for drawing text (much like the case for drawing Images & PDF).
	CGContextSetTextMatrix(context, CGAffineTransformMakeScale(1.0, -1.0));
	
	NSString *drawArea = [[NSString alloc] initWithFormat:@"Left: %d, Top: %d, Width: %d, height: %d", 
						  rect.origin.x, rect.origin.y, rect.size.width, 120];
	
	// And now we actually draw some text. This screen will demonstrate the typical drawing modes used.
	CGContextSetTextDrawingMode(context, kCGTextFill);
	const char *str = [drawArea UTF8String];
	CGContextShowTextAtPoint(context, 10.0, 80.0, str, strlen(str));
}

-(void) onLyricsChanged
{
	m_lyrGraph.OnLyricsChanged();
	CRect	rcUpdate;
	m_lyrGraph.Invalidate();
	m_lyrGraph.UpdateGraph(rcUpdate);
	[self setNeedsDisplay];
}

-(void) onPlaybackStateChanged:(MPMusicPlaybackState)state
{
/*	if (state == MPMusicPlaybackStatePaused || state == MPMusicPlaybackStateStopped) {
		[lyrDisplayTimer invalidate];
	}
	else if (state == MPMusicPlaybackStatePlaying) {
		[lyrDisplayTimer fire];
	}*/
}

- (void)drawRect:(CGRect)rect {

	// [super drawRect:rect];
	CGRect	frame = [self frame];
	CMLGraphics		graph;
	CGContextRef	context = UIGraphicsGetCurrentContext();
	graph.Attach(context);
	
	// Draw in memGraph
	ASSERT(m_lyrGraph.GetGraph().IsValid());
	{
		// m_memGraph.Create(frame.size.width, frame.size.height, NULL, 32);
		// m_font.Create("Helvetica", "Helvetica", 20, FW_NORMAL, 0);
		// m_bmpFont.Create("Helvetica", "Helvetica", 50, FW_NORMAL, 0);
	}

	m_lyrGraph.GetGraph().DrawToGraphics(&graph, 0, 0, frame.size.width, frame.size.height, 0, 0);

/*	MLColor	clr(RGB(255, 0, 0));
	m_memGraph.FillRect(0, 0, frame.size.width, frame.size.height, clr);

	MLColor	clrText(RGB(255, 255, 255));
	MLColor	clrBorder(RGB(0, 0, 0));
	m_memGraph.SetTextColor(clrText);
	
	m_memGraph.SetFont(&m_bmpFont);
	m_memGraph.TextOut(10, 150, "BmpFont", 7);
	m_memGraph.TextOutOutlined(10, 200, "BmpFont2", 8, clrText, clrBorder);
	
	CMLGraphics		memGraph;
	CGContextRef	memContext = m_memGraph.GetHandle();
	memGraph.Attach(m_memGraph.GetHandle());
	CGContextSetRGBFillColor(memContext, 1.0, 1.0, 1.0, 1.0);
	memGraph.SetFont(&m_font);
	memGraph.TextOut(10, 10, "Hello", 5);
	
	DrawTextTo(memContext, frame);
	
	m_memGraph.DrawToGraphics(&graph, 0, 0, frame.size.width, frame.size.height, 0, 0);
 */
}

// Invoked by the lyrDisplayTimer.
- (void) onUpdateLyrDisplay {
	CMLApp::GetInstance()->GetLyricsData().SetPlayElapsedTime(int([musicPlayer currentPlaybackTime] * 1000));

	CRect	rcUpdate;
	m_lyrGraph.UpdateGraph(rcUpdate);
	if (rcUpdate.IsRectEmpty())
		return;

	[self setNeedsDisplayInRect:CGRectMake(rcUpdate.left, rcUpdate.top, rcUpdate.Width(), rcUpdate.Height())];
}

- (void)dealloc {
	[lyrDisplayTimer		invalidate];
	[lyrDisplayTimer		release];
	[musicPlayer release];
    [super dealloc];
}


@end
