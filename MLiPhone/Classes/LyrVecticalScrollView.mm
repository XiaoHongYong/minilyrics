//
//  LyrVecticalScrollView.m
//  MiniLyrics
//
//  Created by showbrave on 5/29/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "../../base/base.h"
#import "../../Utils/Utils.h"
#import "../../MLLib/MLLib.h"
#import "LyrVecticalScrollView.h"
#import "CMLApp.h"
#import <CoreText/CoreText.h>
#import <CoreFoundation/CFDictionary.h>

const int MARGIN_X = 10;
const int MARGIN_Y = 10;
const int END_LYR_LINE_DURATION = 10 * 1000;

UIImage *getAlbumArtWithSize(MPMusicPlayerController *musicPlayer, int width, int height);

tstring getLyricsOfLine(LyricsLine *pRow)
{
	tstring str;
	for (int i = 0; i < pRow->vFrags.size(); i++) {
		str += pRow->vFrags[i]->szLyric;
	}
	
	return str;
}

@implementation LyrVecticalScrollView

@synthesize lyrDisplayTimer;
@synthesize musicPlayer;
@synthesize inSeekingLyrics;
@synthesize fontHeight;
@synthesize mClrHigh, mClrLow, mClrBg, mClrText;


- (id)initWithCoder:(NSCoder *)decoder
{
    if (self = [super initWithCoder:decoder]) {
		[self init];
    }
    return self;
}

- (id)initWithFrame:(CGRect)frame
{
    if (self = [super initWithFrame:frame]) {
		[self init];
    }
    return self;
}

- (id) init
{
	// Initialization code
	self.inSeekingLyrics = NO;
	
	m_vLyrLines = CFArrayCreateMutable(NULL, 0, NULL);
	
	return self;
}

- (void) InitLyrView
{
	UIImage *logo = [UIImage imageNamed: @"logo.png"];
	[logo release];

	[self setLyrDisplayTimer: [NSTimer scheduledTimerWithTimeInterval: 0.07
																	target: self
																  selector: @selector (onUpdateLyrDisplay)
																  userInfo: nil
																   repeats: YES]];
	
	[self reloadFontSettings];
	
	mFadeMode = FM_FADE_IN;
	m_NeedRewrap = true;
	mCurLine = -1;
	mTimePlayingCache = -1;
	mAlphaLineAbove = mAlphaLineBelow = 0;
	mMarginX = mMarginY = 5;
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
						  rect.origin.x, rect.origin.y, (int)rect.size.width, 120];
	
	// And now we actually draw some text. This screen will demonstrate the typical drawing modes used.
	CGContextSetTextDrawingMode(context, kCGTextFill);
	const char *str = [drawArea UTF8String];
	CGContextShowTextAtPoint(context, 10.0, 80.0, str, strlen(str));
}

- (int) getLineByTime:(int)time
{
	// todo:/....
	return 0;
}

- (void) freeLyricsLines
{
	for (int i = CFArrayGetCount(m_vLyrLines) - 1; i >= 0; i--) {
		delete (LyrDisplayLine *)CFArrayGetValueAtIndex(m_vLyrLines, i);
	}
	CFArrayRemoveAllValues(m_vLyrLines);
}

- (void) RewrapLyrics:(CGContextRef) context
{
	m_NeedRewrap = false;
	[self freeLyricsLines];

	CMLData &lyrData = CMLApp::GetInstance()->GetLyricsData();
	CLyricsLines &lyrLines = lyrData.GetRawLyrics();
	float boundsWidth = self.bounds.size.width - mMarginX * 2;

	if (m_FontName == NULL)
		m_FontName = CFStringCreateWithCString(NULL, "Times", kCFStringEncodingMacRoman);

	// Prepare font
	CTFontRef font = CTFontCreateWithName(m_FontName, 48, NULL);

	// Create attributed dictionary
	CFStringRef keys[] = { kCTFontAttributeName };
	CFTypeRef values[] = { font };
	CFDictionaryRef attr = CFDictionaryCreate(NULL, 
											  (const void **)&keys, (const void **)&values,
											  sizeof(keys) / sizeof(keys[0]), 
											  &kCFTypeDictionaryKeyCallBacks, 
											  &kCFTypeDictionaryValueCallBacks);

	for (int i = 0; i < lyrLines.size(); i++) {
		LyricsLine *pRow = lyrLines[i];
		tstring strLyrLine = getLyricsOfLine(pRow);

		if (strLyrLine.size() == 0) {
			// Empty Line
			LyrDisplayLine *dispLine = new LyrDisplayLine();
			dispLine->startTime = pRow->nBegTime;
			dispLine->endTime = pRow->nEndTime;
			dispLine->lineText = NULL;
			CFArrayAppendValue(m_vLyrLines, dispLine);
			continue;
		}

		CFStringRef sLine = CFStringCreateWithCString(NULL, strLyrLine.c_str(), kCFStringEncodingUTF8);
		CFAttributedStringRef attrLineStr = CFAttributedStringCreate(NULL, sLine, attr);
		CTTypesetterRef typeSetterLine = CTTypesetterCreateWithAttributedString(attrLineStr);
		
		// Need to wrap lyrics lines.
		int start = 0, length = CFAttributedStringGetLength(attrLineStr);
		int startTime = pRow->nBegTime;
		if (length == 0)
			continue;
		float fTimeOfChar = ((float) (pRow->nEndTime - startTime)) / length;

		while (start < length) {
			int n = CTTypesetterSuggestLineBreak(typeSetterLine, start, boundsWidth);
			if (n <= 0)
				n = length - start;
			LyrDisplayLine *dispLine = new LyrDisplayLine();
			dispLine->startTime = startTime;
			dispLine->endTime = startTime + fTimeOfChar * n;
			dispLine->lineText = CTTypesetterCreateLine(typeSetterLine, CFRangeMake(start, n));
			CFArrayAppendValue(m_vLyrLines, dispLine);
			start += n;
		}

		CFRelease(typeSetterLine);
		CFRelease(attrLineStr);
		CFRelease(sLine);
	}

	CFRelease(font);
	CFRelease(attr);
}

-(void) onLyricsChanged
{
	UIImage *image = getAlbumArtWithSize(self.musicPlayer, self.frame.size.width, self.frame.size.height);
	if (image) {
	}
	else {
		//image = [UIImage imageNamed: @"logo.png"];
		//pRawImage = RawImageDataFromCGImage([image CGImage]);
		//[image release];	
	}

	m_NeedRewrap = true;
	[self setNeedsDisplay];
}

- (void) onPlaybackStateChanged:(MPMusicPlaybackState)state
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
}

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
	CGContextRef	context = UIGraphicsGetCurrentContext();
	
	CGContextSetFillColorWithColor(context, [UIColor redColor].CGColor);
	CGContextFillRect(context, frame);
	
	if (m_NeedRewrap)
		[self RewrapLyrics:context];

	int count = CFArrayGetCount(m_vLyrLines);
	int y = mMarginY;
	for (int i = 0; i < count; i++) {
		// Draw every lyrics lines
		LyrDisplayLine *line = (LyrDisplayLine*)CFArrayGetValueAtIndex(m_vLyrLines, i);
		
		CGContextSetTextPosition(context, mMarginX, y);
		CTLineDraw(line->lineText, context);
		
		y += mLineHeight;
	}
	
	//[self DrawText:context 10, 10, 100];
	// CRect rc(rect.origin.x, rect.origin.y, rect.origin.x + rect.size.width, rect.origin.y + rect.size.height);
/*	for (int i = 0; i < 10; i++)
	{
		CRect rc(i * 10, i * 5, i * 10 + 10, i * 5 + 5);
		MLColor clr(RGB(i * 25, 128, 0));
		clr.setAlpha(128);
		m_lyrGraph.GetGraph().FillRect(&rc, clr, BPM_BLEND);
	}*/
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
	NSUserDefaults *settings = [NSUserDefaults standardUserDefaults];
	fontHeight = [settings integerForKey:@"font_height"];
	if (fontHeight < 10)
		fontHeight = 10;
	else if (fontHeight > 60)
		fontHeight = 60;
	mLineSpace = fontHeight / 10;
	mLineHeight = fontHeight + mLineSpace;

	m_FontName = NULL;
	// mFontName = [[settings stringForKey:@"font_name"] ;

	// [self onUpdateLyrDisplay];
}

// Invoked by the lyrDisplayTimer.
- (void) onUpdateLyrDisplay {
	CMLData &lyrData = CMLApp::GetInstance()->GetLyricsData();
	if (!self.inSeekingLyrics)
		lyrData.SetPlayElapsedTime(int([musicPlayer currentPlaybackTime] * 1000));

	CRect	rcUpdate;
	if (rcUpdate.IsRectEmpty())
		return;

	[self setNeedsDisplayInRect:CGRectMake(rcUpdate.left, rcUpdate.top, rcUpdate.Width(), rcUpdate.Height())];
}

- (void)dealloc {
	[lyrDisplayTimer		invalidate];
	[lyrDisplayTimer		release];
	[musicPlayer release];
	CFRelease(m_vLyrLines);
    [super dealloc];
}


@end
