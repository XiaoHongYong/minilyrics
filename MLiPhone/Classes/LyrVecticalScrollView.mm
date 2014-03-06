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
const int FADE_DURATION = 1500;

UIImage *getAlbumArtWithSize(MPMusicPlayerController *musicPlayer, int width, int height);

tstring getLyricsOfLine(LyricsLine *pRow)
{
	tstring str;
	for (int i = 0; i < pRow->vFrags.size(); i++) {
		str += pRow->vFrags[i]->szLyric;
	}
	
	return str;
}

int blendColor(int clr1, int clr2, int alpha) {
	int nPart1 = 255 - alpha;

	return 0;
//	return Color.argb(255, (Color.red(clr1) * nPart1 + Color.red(clr2)
//							* alpha) / 255, (Color.green(clr1) * nPart1 + Color.green(clr2)
//											 * alpha) / 255, (Color.blue(clr1) * nPart1 + Color.blue(clr2)
//															  * alpha) / 255);
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
	mHorzAlignMode = ALIGN_CENTER;
	
	m_vLyrLines = CFArrayCreateMutable(NULL, 0, NULL);
	
	return self;
}

- (void) InitLyrView
{
	UIImage *logo = [UIImage imageNamed: @"logo.png"];
	[logo release];

	[self setLyrDisplayTimer: [NSTimer scheduledTimerWithTimeInterval: 0.07
																	target: self
																	selector: @selector(onUpdateLyrDisplay)
																	userInfo: nil
																	repeats: YES]];
	
	[self reloadFontSettings];
	
	mFadeMode = FM_FADE_IN;
	m_NeedRewrap = true;
	mCurLine = -1;
	mTimePlayingCache = -1;
	mAlphaLineAbove = mAlphaLineBelow = 0;
	mMarginX = mMarginY = 10;
}


- (int) getCountWithSameTimeStamps: (int)nLine
{
	int count = CFArrayGetCount(m_vLyrLines);
	if (nLine < 0 || nLine >= count)
		return 0;
	
	LyrDisplayLine *line = (LyrDisplayLine *)CFArrayGetValueAtIndex(m_vLyrLines, nLine);
	for (int i = nLine + 1; i < count; i++)
	{
		LyrDisplayLine *p = (LyrDisplayLine *)CFArrayGetValueAtIndex(m_vLyrLines, i);
		if (line->startTime != p->startTime)
			return i - nLine;
	}
	
	return count - nLine;
}

- (int) getLineByTime:(int)time
{
	// Search from last current line.
	int i = mCurLine - 1;
	if (i >= CFArrayGetCount(m_vLyrLines))
		i = 0;
	for (; i > 0; i--) {
		if (((LyrDisplayLine *)CFArrayGetValueAtIndex(m_vLyrLines, i))->startTime < time) {
			i++;
			break;
		}
	}
	if (i < 0)
		i = 0;
	
	for (; i < CFArrayGetCount(m_vLyrLines); i++) {
		if (((LyrDisplayLine *)CFArrayGetValueAtIndex(m_vLyrLines, i))->endTime > time) {
			return i;
		}
	}
	
	if (CFArrayGetCount(m_vLyrLines) > 0)
		return CFArrayGetCount(m_vLyrLines) - 1;
	
	return -1;
}

- (int) getCurrentLinePosY: (int) timePlaying
{
	if (mCurLine < 0 || mCurLine >= CFArrayGetCount(m_vLyrLines))
		return -1;
	
	LyrDisplayLine *line = (LyrDisplayLine *)CFArrayGetValueAtIndex(m_vLyrLines, mCurLine);
	int curLineDuration = line->endTime - line->startTime;
	
	int curLinePosY = (int)self.bounds.size.height / 2;
	if (timePlaying < line->startTime || curLineDuration == 0)
		;
	else
		curLinePosY += mLineHeight * [self getCountWithSameTimeStamps: mCurLine]
			* (timePlaying - line->startTime) / curLineDuration;
	
	return curLinePosY;
}

- (int) getAlphaOfLine: (LyrDisplayLine *)line ofTimePlaying:(int)timePlaying
{
	int nTime;
	
	if (line->startTime > timePlaying) {
		nTime = line->startTime - timePlaying;
		if (nTime <= FADE_DURATION && nTime >= -FADE_DURATION) {
			return (FADE_DURATION - nTime) * 255 / FADE_DURATION;
		} else
			return 0;
	} else {
		nTime = timePlaying - line->endTime;
		if (nTime >= FADE_DURATION)
			return 0;
		else if (nTime > 0)
			return (FADE_DURATION - nTime) * 255 / FADE_DURATION;
		else
			return 255;
	}
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
	CTFontRef font = CTFontCreateWithName(m_FontName, fontHeight, NULL);

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
			dispLine->width = 0;
			CFArrayAppendValue(m_vLyrLines, dispLine);
			continue;
		}

		CFStringRef sLine = CFStringCreateWithCString(NULL, strLyrLine.c_str(), kCFStringEncodingUTF8);
		CFAttributedStringRef attrLineStr = CFAttributedStringCreate(NULL, sLine, attr);
		CTTypesetterRef typeSetterLine = CTTypesetterCreateWithAttributedString(attrLineStr);
		
		// Need to wrap lyrics lines.
		int start = 0, length = CFAttributedStringGetLength(attrLineStr);
		if (length == 0)
			continue;

		while (start < length) {
			int n = CTTypesetterSuggestLineBreak(typeSetterLine, start, boundsWidth);
			if (n <= 0)
				n = length - start;
			LyrDisplayLine *dispLine = new LyrDisplayLine();
			dispLine->startTime = pRow->nBegTime;
			dispLine->endTime = pRow->nEndTime;
			dispLine->lineText = CTTypesetterCreateLine(typeSetterLine, CFRangeMake(start, n));
			dispLine->width = (int)CTLineGetTypographicBounds(dispLine->lineText, NULL, NULL, NULL);
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
	if (state == MPMusicPlaybackStatePaused || state == MPMusicPlaybackStateStopped) {
		[lyrDisplayTimer invalidate];
	} else if (state == MPMusicPlaybackStatePlaying) {
		[lyrDisplayTimer fire];
	}
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

	CFDictionaryRef attributes = CFDictionaryCreate(kCFAllocatorDefault, (const void**)&keys,
					   (const void**)&values, sizeof(keys) / sizeof(keys[0]),
					   &kCFTypeDictionaryKeyCallBacks,
					   &kCFTypeDictionaryValueCallBacks);

	CFAttributedStringRef attrString = CFAttributedStringCreate(kCFAllocatorDefault, string, attributes);
	CFRelease(string);
	CFRelease(attributes);

	CTLineRef line = CTLineCreateWithAttributedString(attrString);
	
	// Set text position and draw the line into the graphics context
	CGContextSetTextPosition(context, x, y + nFontHeight);
	CTLineDraw(line, context);
	CFRelease(line);
	CFRelease(font);
}

- (int) getPlayPos
{
	if (inSeekingLyrics)
		return mTimePlayingCache;
	
	CMLData &lyrData = CMLApp::GetInstance()->GetLyricsData();
	return lyrData.GetPlayElapsedTime();
}

- (int) getDrawXPosOfLine:(LyrDisplayLine*)line
{
	if (mHorzAlignMode == ALIGN_LEFT)
		return 0;
	else if (mHorzAlignMode == ALIGN_RIGHT)
		return (int)(self.bounds.size.width - line->width);
	else
		return (int)(self.bounds.size.width - line->width) / 2;
}

- (void)drawRect:(CGRect)rect {

	// [super drawRect:rect];
	CGRect	frame = [self frame];
	CGContextRef	context = UIGraphicsGetCurrentContext();
	
	CGRect viewBounds = self.bounds;
	CGContextTranslateCTM(context, 0, viewBounds.size.height);
	CGContextScaleCTM(context, 1, -1);
	CGContextSetRGBFillColor(context, 0.0, 1.0, 1.0, 1.0);
	CGContextSetLineWidth(context, 2.0);
	CGContextSelectFont(context, "Helvetica", 20.0, kCGEncodingMacRoman);
	CGContextSetCharacterSpacing(context, 1.7);
	CGContextSetTextDrawingMode(context, kCGTextFill);
	CGContextShowTextAtPoint(context, 100.0, 100.0, "SOME TEXT", 9);
	
//	CGContextSetFillColorWithColor(context, [UIColor redColor].CGColor);
	CGContextFillRect(context, frame);
	
	if (m_NeedRewrap)
		[self RewrapLyrics:context];

	if (CFArrayGetCount(m_vLyrLines) == 0)
		return;

	int timePlaying = mTimePlayingCache;
	if (!inSeekingLyrics) {
		if (timePlaying == -1)
			timePlaying = [self getPlayPos];
		else
			mTimePlayingCache = -1;
	}
	
	mCurLine = [self getLineByTime:timePlaying];
	int nCurLineCount = [self getCountWithSameTimeStamps: mCurLine];
	int mCurLinePosY = [self getCurrentLinePosY: timePlaying];

	// Draw lines above current line
	if (mCurLine > 0) {
		[self drawAboveCurLines:context timePlaying:timePlaying curLine:mCurLine y:mCurLinePosY + mLineHeight];
	}
	
	// Draw current lines
	// p.setColor(mClrHigh);
	for (int i = 0; i < nCurLineCount; i++) {
		LyrDisplayLine *line = (LyrDisplayLine*)CFArrayGetValueAtIndex(m_vLyrLines, mCurLine + i);
		
		CGContextSetTextPosition(context, [self getDrawXPosOfLine:line], mCurLinePosY);
		CTLineDraw(line->lineText, context);

		mCurLinePosY -= mLineHeight;
	}
	
	// Draw lines below current line
	if (mCurLine + 1 < CFArrayGetCount(m_vLyrLines)) {
		[self drawBelowCurLines:context timePlaying:timePlaying belowLine:mCurLine - nCurLineCount y:mCurLinePosY];
	}

//	int count = CFArrayGetCount(m_vLyrLines);
//	int y = viewBounds.size.height - mMarginY;
//	for (int i = 0; i < count; i++) {
//		// Draw every lyrics lines
//		LyrDisplayLine *line = (LyrDisplayLine*)CFArrayGetValueAtIndex(m_vLyrLines, i);
//		
//		CGContextSetTextPosition(context, mMarginX, y);
//		CTLineDraw(line->lineText, context);
//		
//		y -= mLineHeight;
//		//break;
//	}
	
	//DrawText(context, 10, 10, 50);
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
		fontHeight = 20;
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

//	CRect	rcUpdate;
//	if (rcUpdate.IsRectEmpty())
//		return;

//	[self setNeedsDisplayInRect:CGRectMake(rcUpdate.left, rcUpdate.top, rcUpdate.Width(), rcUpdate.Height())];
	[self setNeedsDisplay];
}

- (void)dealloc {
	[lyrDisplayTimer		invalidate];
	[lyrDisplayTimer		release];
	[musicPlayer release];
	CFRelease(m_vLyrLines);
    [super dealloc];
}


- (void) drawAboveCurLines:(CGContextRef)context timePlaying:(int) timePlaying curLine:(int)nCurLine y:(int)y
{
	if (nCurLine < 0 || nCurLine >= CFArrayGetCount(m_vLyrLines))
		return;
//	if (mLyrData.isStaticTextLyrics()) {
//		// Draw lyrics in same color
//		for (int i = nCurLine - 1; y >= 0 && i >= 0; y -= mLineHeight, i--) {
//			canvas.drawText((LyrDisplayLine *)CFArrayGetValueAtIndex(m_vLyrLines, i).lyrics, x, y, paint);
//		}
//		return;
//	}

	// Draw lyrics above current line
	LyrDisplayLine *line = (LyrDisplayLine *)CFArrayGetValueAtIndex(m_vLyrLines, nCurLine - 1);
	mAlphaLineAbove = [self getAlphaOfLine:line ofTimePlaying:timePlaying];
	if (mAlphaLineAbove != 0 || mFadeMode != FM_FADE_BG) {
		int clr = blendColor(mFadeMode == FM_FADE_BG ? mClrBg : mClrLow, mClrHigh, mAlphaLineAbove);

		CGContextSetTextPosition(context, [self getDrawXPosOfLine:line], y);
		CTLineDraw(line->lineText, context);

		// paint.setColor(clr);
		// canvas.drawText(line.lyrics, x, y, paint);
	}
	y += mLineHeight;
	
	// Draw other lines
	for (int i = nCurLine - 2; y >= 0 && i >= 0; y += mLineHeight, i--) {
		line = (LyrDisplayLine *)CFArrayGetValueAtIndex(m_vLyrLines, i);
		int alpha = [self getAlphaOfLine:line ofTimePlaying:timePlaying];
		if (alpha == 0 && mFadeMode == FM_FADE_BG)
			return;
		
		int clr = blendColor(mFadeMode == FM_FADE_BG ? mClrBg : mClrLow, mClrHigh, alpha);
		CGContextSetTextPosition(context, [self getDrawXPosOfLine:line], y);
		CTLineDraw(line->lineText, context);

//		paint.setColor(clr);
//		canvas.drawText(line.lyrics, x, y, paint);
	}
}

- (void) drawBelowCurLines:(CGContextRef)context timePlaying:(int)timePlaying belowLine:(int)nBelowLine y:(int)y
{
//	if (mLyrData.isStaticTextLyrics()) {
//		// Draw static text lyrics
//		for (int i = nBelowLine; y - mLineHeight <= mHeight && i < mLyrLines.size(); y += mLineHeight, i++) {
//			canvas.drawText((LyrDisplayLine *)CFArrayGetValueAtIndex(m_vLyrLines, i).lyrics, x, y, p);
//		}
//		return;
//	}
	if (nBelowLine < 0 || nBelowLine >= CFArrayGetCount(m_vLyrLines))
		return;

	// Draw lyrics below current line
	LyrDisplayLine *line = (LyrDisplayLine *)CFArrayGetValueAtIndex(m_vLyrLines, nBelowLine);
	mAlphaLineBelow = [self getAlphaOfLine:line ofTimePlaying:timePlaying];
	if (mAlphaLineBelow != 0 || mFadeMode != FM_FADE_BG) {
		int clr = blendColor(mFadeMode == FM_FADE_BG ? mClrBg
							 : mClrLow, mClrHigh, mAlphaLineBelow);
		CGContextSetTextPosition(context, [self getDrawXPosOfLine:line], y);
		CTLineDraw(line->lineText, context);
//		p.setColor(clr);
//		canvas.drawText(line.lyrics, x, y, p);
	}
	y -= mLineHeight;
	
	int mHeight = (int)self.bounds.size.width;
	// Draw other lines
	for (int i = nBelowLine + 1; y - mLineHeight <= mHeight && i < CFArrayGetCount(m_vLyrLines); y -= mLineHeight, i++) {
		line = (LyrDisplayLine *)CFArrayGetValueAtIndex(m_vLyrLines, i);
		int alpha = [self getAlphaOfLine:line ofTimePlaying:timePlaying];
		if (alpha == 0 && mFadeMode == FM_FADE_BG) {
			return;
		}
		
		int clr = blendColor(mFadeMode == FM_FADE_BG ? mClrBg : mClrLow, mClrHigh, alpha);
		CGContextSetTextPosition(context, [self getDrawXPosOfLine:line], y);
		CTLineDraw(line->lineText, context);
//		p.setColor(clr);
//		canvas.drawText(line.lyrics, x, y, p);
	}
}

@end
