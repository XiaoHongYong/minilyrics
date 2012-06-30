#include "StdAfx.h"
#include "FontInfo.h"
#include "MLFont.h"

CMLFont::CMLFont()
{
	m_font = NULL;
}

CMLFont::~CMLFont(void)
{
	Destroy();
}


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

bool CMLFont::Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline)
{
	tstring	strFontName;

	CFontInfo::Set(szFaceNameLatin9, szFaceNameOthers, nHeight, nWeight, nItalic, bUnderline);

	//if (szFaceNameLatin9 == NULL || IsEmptyString(szFaceNameLatin9))
	szFaceNameLatin9 = "Helvetica";
	CFStringRef	name = CFStringCreateWithCString(NULL, szFaceNameLatin9, kCFStringEncodingUTF8);
	CTFontSymbolicTraits fontTraits = 0;
	if (nWeight == FW_BOLD)
		fontTraits |= kCTFontBoldTrait;
	if (nItalic)
		fontTraits |= kCTFontItalicTrait;
	m_font = CreateFont(
						CreateFontDescriptorFromFamilyAndTraits(name, fontTraits, nHeight),
						nHeight);
	CFRelease(name);

	return true;
}

bool CMLFont::Create(const CFontInfo &font)
{
	return Create(font.GetName(), font.GetNameOthers(), font.GetSize(), font.GetWeight(), font.GetItalic(), font.IsUnderline());
}

void CMLFont::Destroy()
{
	if (m_font != NULL)
	{
		CFRelease(m_font);
		m_font = NULL;
	}
}
