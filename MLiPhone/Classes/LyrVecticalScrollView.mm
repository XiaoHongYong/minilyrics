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
#import <CoreText/CoreText.h>
#import <CoreFoundation/CFDictionary.h>

#import "LyricsVerticalScrollGraph.h"


UIImage *getAlbumArtWithSize(MPMusicPlayerController *musicPlayer, int width, int height);

PRAW_IMAGE_DATA RawImageDataFromCGImage(CGImageRef cgimage);

@implementation LyrVecticalScrollView

@synthesize lyrDisplayTimer;
@synthesize musicPlayer;
@synthesize inSeekingLyrics;


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
	UIImage *logo = [UIImage imageNamed: @"logo.png"];
	PRAW_IMAGE_DATA pRawImage = RawImageDataFromCGImage([logo CGImage]);
	[logo release];

	m_lyrGraph.Init(self.frame.size.width, self.frame.size.height, &CMLApp::GetInstance()->GetLyricsData(), pRawImage);
	self.inSeekingLyrics = NO;

	[self setLyrDisplayTimer: [NSTimer scheduledTimerWithTimeInterval: 0.07
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
	PRAW_IMAGE_DATA pRawImage = nil;
	UIImage *image = getAlbumArtWithSize(self.musicPlayer, self.frame.size.width, self.frame.size.height);
	if (image) {
		pRawImage = RawImageDataFromCGImage([image CGImage]);
	}
	else {
		//image = [UIImage imageNamed: @"logo.png"];
		//pRawImage = RawImageDataFromCGImage([image CGImage]);
		//[image release];	
	}
	
	m_lyrGraph.OnLyricsChanged(pRawImage);
	CRect	rcUpdate;
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

CTFontRef CreateFont(CTFontDescriptorRef iFontDescriptor, CGFloat iSize);
CTFontDescriptorRef CreateFontDescriptorFromFamilyAndTraits(CFStringRef iFamilyName,
															CTFontSymbolicTraits iTraits, CGFloat iSize);

/*
CTFontDescriptorRef CreateFontDescriptorFromFamilyAndTraits(CFStringRef iFamilyName,
															CTFontSymbolicTraits iTraits, CGFloat iSize)
{
    CTFontDescriptorRef descriptor = NULL;
    CFMutableDictionaryRef attributes;
	
    assert(iFamilyName != NULL);
    // Create a mutable dictionary to hold our attributes.
    attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
										   &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    if (attributes != NULL) {
        CFMutableDictionaryRef traits;
        CFNumberRef symTraits;
		
        // Add a family name to our attributes.
        CFDictionaryAddValue(attributes, kCTFontFamilyNameAttribute, iFamilyName);
		
        // Create the traits dictionary.
        symTraits = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type,
								   &iTraits);
        
        if (symTraits != NULL) {
            // Create a dictionary to hold our traits values.
            traits = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
											   &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            
            if (traits != NULL) {
                // Add the symbolic traits value to the traits dictionary.
                CFDictionaryAddValue(traits, kCTFontSymbolicTrait, symTraits);
				
                // Add the traits attribute to our attributes.
                CFDictionaryAddValue(attributes, kCTFontTraitsAttribute, traits);
                CFRelease(traits);
            }
            CFRelease(symTraits);
        }
        // Create the font descriptor with our attributes and input size.
        descriptor = CTFontDescriptorCreateWithAttributes(attributes);
		
        CFRelease(attributes);
    }
    // Return our font descriptor.
    return descriptor;
}

CTFontRef CreateFont(CTFontDescriptorRef iFontDescriptor, CGFloat iSize)
{
    // Create the font from the font descriptor and input size. Pass
    // NULL for the matrix parameter to use the default matrix (identity).
	
    return CTFontCreateWithFontDescriptor(iFontDescriptor, iSize, NULL);
}*/

void DrawText(CGContextRef context, int x, int y, int nFontHeight)
{
	CFStringRef string = CFSTR("Hello");
	
	CTFontRef font = CreateFont(
		CreateFontDescriptorFromFamilyAndTraits(CFSTR("Helvetica"), 0, nFontHeight),
		nFontHeight);

	CGColorSpaceRef rgbColorSpace = CGColorSpaceCreateDeviceRGB();
	CGFloat components[] = { 1.0, 1.0, 1.0, 1.0 };
	CGColorRef red = CGColorCreate(rgbColorSpace, components);
	CGColorSpaceRelease(rgbColorSpace);
	
	// Initialize string, font, and context
	CFStringRef keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
	CFTypeRef values[] = { font, red };

	CFDictionaryRef attributes =
    CFDictionaryCreate(kCFAllocatorDefault, (const void**)&keys,
					   (const void**)&values, sizeof(keys) / sizeof(keys[0]),
					   &kCFTypeDictionaryKeyCallBacks,
					   &kCFTypeDictionaryValueCallBacks);
	
	CFAttributedStringRef attrString =
    CFAttributedStringCreate(kCFAllocatorDefault, string, attributes);
	CFRelease(string);
	CFRelease(attributes);
	
	CTLineRef line = CTLineCreateWithAttributedString(attrString);
	
	// Set text position and draw the line into the graphics context
	CGContextSetTextPosition(context, x, y + nFontHeight);
	CTLineDraw(line, context);
	CFRelease(line);
	CFRelease(font);
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

	// CRect rc(rect.origin.x, rect.origin.y, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height);
/*	for (int i = 0; i < 10; i++)
	{
		CRect rc(i * 10, i * 5, i * 10 + 10, i * 5 + 5);
		MLColor clr(RGB(i * 25, 128, 0));
		clr.setAlpha(128);
		m_lyrGraph.GetGraph().FillRect(&rc, clr, BPM_BLEND);
	}*/
	m_lyrGraph.GetGraph().DrawToGraphics(&graph, rect.origin.x, rect.origin.y, rect.size.width, rect.size.height,
										 rect.origin.x, rect.origin.y);
/*	{
		CRect rc(10, 200, 10 + 100, 200 + 10);
		MLColor clr(RGB(255, 0, 0));
		clr.setAlpha(128);
		m_lyrGraph.GetGraph().FillRect(&rc, clr, BPM_BLEND);
	}*/
	
/*	m_lyrGraph.GetGraph().DrawToGraphics(&graph, 0, 0, frame.size.width, frame.size.height, 0, 0);
	//m_lyrGraph.GetGraph().DrawToGraphics(&graph, rc.left, rc.top, rc.Width(), rc.Height(), rc.left, rc.top);
	{
	CRect rc(10, 200, 10 + 100, 200 + 10);
	MLColor clr(RGB(255, 0, 0));
	clr.setAlpha(128);
//	m_lyrGraph.GetGraph().FillRect(&rc, clr, BPM_BLEND);
		m_lyrGraph.GetGraph().DrawToGraphics(&graph, 0, rc.bottom, frame.size.width, 100, 0, 0);
	}	*/
	

/*	if (!m_memGraph.IsValid())
	{
		m_memGraph.Create(frame.size.width, frame.size.height, NULL, 32);
		m_font.Create("Helvetica", "Helvetica", 20, FW_NORMAL, 0);
		m_bmpFont.Create("Helvetica", "Helvetica", 50, FW_NORMAL, 0);
	}

	CRect rcText;
	
	MLColor	clr(RGB(255, 0, 0));
	m_memGraph.FillRect(0, 0, frame.size.width, frame.size.height, clr);

	MLColor	clrText(RGB(255, 255, 255));
	MLColor	clrBorder(RGB(0, 0, 0));
	m_memGraph.SetTextColor(clrText);
	
	m_memGraph.SetFont(&m_bmpFont);

	rcText.SetRect(10, 150, 10 + 200, 150 + m_bmpFont.GetHeight());
	m_memGraph.FillRect(&rcText, MLColor(RGB(0, 255, 0)), BPM_COPY);
	m_memGraph.TextOut(rcText.left, rcText.top, "Bm中文150", 7);

	rcText.SetRect(10, 250, 10 + 200, 250 + m_bmpFont.GetHeight());
	m_memGraph.FillRect(&rcText, MLColor(RGB(0, 255, 0)), BPM_COPY);
	m_memGraph.TextOutOutlined(rcText.left, rcText.top, "BmpFont300", 8, clrText, clrBorder);
	
	CMLGraphics		memGraph;
	CGContextRef	memContext = m_memGraph.GetHandle();
	memGraph.Attach(m_memGraph.GetHandle());
	CGContextSetRGBFillColor(memContext, 1.0, 1.0, 1.0, 1.0);
	memGraph.SetFont(&m_font);
	memGraph.TextOut(10, 10, "Hello200", 5);
	
	DrawTextTo(memContext, frame);

	rcText.SetRect(100, 210, 100 + 80, 210 + 30);
	m_memGraph.FillRect(&rcText, MLColor(RGB(0, 255, 0)), BPM_COPY);
	DrawText(memContext, rcText.left, rcText.top, rcText.Height());
	
	m_memGraph.DrawToGraphics(&graph, 0, 0, frame.size.width, frame.size.height, 0, 0);
 */
}

- (void) reloadFontSettings
{
	m_lyrGraph.Invalidate();
	m_lyrGraph.ReloadFontSettings();
	[self onUpdateLyrDisplay];
}

// Invoked by the lyrDisplayTimer.
- (void) onUpdateLyrDisplay {
	CMLData &lyrData = CMLApp::GetInstance()->GetLyricsData();
	if (!self.inSeekingLyrics)
		lyrData.SetPlayElapsedTime(int([musicPlayer currentPlaybackTime] * 1000));

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
