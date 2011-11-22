// CharEncoding.cpp: implementation of the CCharEncoding class.
//
//////////////////////////////////////////////////////////////////////


#import <Foundation/Foundation.h>

#include "stdafx.h"
#include <iconv.h>
#include <CoreFoundation/CoreFoundation.h>
#include "stringex_t.h"
#include "CharEncoding.h"
#include "safestr.h"
#include "stringex.h"


void InitSetDefaultCharEncoding()
{
	ENCODING_CODEPAGE &ecp = GetCharEncodingByID(ED_SYSDEF);
	NSLocale *local = [NSLocale currentLocale];
	NSString *lang = [local objectForKey:NSLocaleLanguageCode];
	NSString *countryCode = [local objectForKey: NSLocaleCountryCode];
	
	// Compare country code
	if ([countryCode isEqual: @"CN"] || [countryCode isEqual: @"SG"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_GB2312).nCFStringEncoding;
	else if ([countryCode isEqual: @"TW"] || [countryCode isEqual: @"HK"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_BIG5).nCFStringEncoding;
	else if ([countryCode isEqual: @"JP"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_JAPANESE_SHIFT_JIS).nCFStringEncoding;
	
	// Compare language code
	else if ([lang isEqual: @"ara"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_ARABIC).nCFStringEncoding;
	else if ([lang isEqual: @"bat"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_BALTIC_WINDOWS).nCFStringEncoding;
	else if ([lang isEqual: @"gre"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_GREEK_WINDOWS).nCFStringEncoding;
	else if ([lang isEqual: @"heb"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_HEBREW_WINDOWS).nCFStringEncoding;
	else if ([lang isEqual: @"tha"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_THAI).nCFStringEncoding;
	else if ([lang isEqual: @"tur"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_TURKISH_WINDOWS).nCFStringEncoding;
	else if ([lang isEqual: @"vie"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_VIETNAMESE).nCFStringEncoding;
	else if ([lang isEqual: @"rus"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_RUSSIAN_WINDOWS).nCFStringEncoding;
	else if ([lang isEqual: @"kor"])
		ecp.nCFStringEncoding = GetCharEncodingByID(ED_KOREAN).nCFStringEncoding;
}

ENCODING_CODEPAGE &GetSysDefaultCharEncoding()
{
	static bool defaultSet = false;
	if (!defaultSet) {
		InitSetDefaultCharEncoding();
		defaultSet = true;
	}
	return __encodingCodepage[ED_SYSDEF];
}

NSString *initNSString(const char *str, int nLen, int encodingID)
{
	NSString	*temp;
	string	strIn;
	if (str[nLen] != '\0')
	{
		strIn.append(str, nLen);
		str = strIn.c_str();
	}
	
	temp = [NSString stringWithCString:str encoding:(NSStringEncoding)
			CFStringConvertEncodingToNSStringEncoding(
													  GetCharEncodingByID((CHAR_ENCODING)encodingID).nCFStringEncoding)];
	if (!temp)
		temp = [NSString stringWithCString:str encoding:(NSStringEncoding)NSISOLatin1StringEncoding];
	
	return temp;
}

int MbcsToUtf8(const char *str, int nLen, char *strOut, int nOut, int encodingID)
{
	if (encodingID == ED_UTF8)
	{
		if (nLen == -1)
		{
			strcpy_safe(strOut, nOut, str);
			return strlen(strOut);
		}
		else
		{
			strncpysz_safe(strOut, nOut, str, nLen);
			return nLen;
		}
	}

	EmptyStr(strOut);
	
	NSString *temp = initNSString(str, nLen, encodingID);
	if (temp)
		strcpy_safe(strOut, nOut, [temp UTF8String]);
	else
		return 0;
	
	return strlen(strOut);
}

int Utf8ToMbcs(const char *str, int nLen, char *strOut, int nOut, int encodingID)
{
	if (encodingID == ED_UTF8)
	{
		if (nLen == -1)
		{
			strcpy_safe(strOut, nOut, str);
			return strlen(strOut);
		}
		else
		{
			strncpysz_safe(strOut, nOut, str, nLen);
			return nLen;
		}
	}

	EmptyStr(strOut);
	
	NSString	*temp;
	temp = [NSString stringWithUTF8String:str];
	if (temp)
	{
		[temp getCString:strOut
			  maxLength:nOut
			   encoding:CFStringConvertEncodingToNSStringEncoding(
																  GetCharEncodingByID((CHAR_ENCODING)encodingID).nCFStringEncoding)];
	} else {
		return 0;
	}

	return strlen(strOut);	
}

int MbcsToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut, int encodingID)
{
	EmptyStr(strOut);
	
	NSString *temp = initNSString(str, nLen, encodingID);
	if (temp)
	{
		if (nOut > [temp length])
			nOut = [temp length];
		[temp getCharacters:(unichar *)strOut range: NSMakeRange(0, nOut)];
		strOut[nOut] = '\0';
	} else {
		return 0;
	}

	return wcslen(strOut);
}

int UCS2ToMbcs(const WCHAR *str, int nLen, char *strOut, int nOut, int encodingID)
{
	EmptyStr(strOut);
	
	NSString	*temp;
	temp = [[NSString alloc] initWithCharacters:(const unichar*)str length:nLen];
	if (temp) {
		[temp getCString:strOut
			  maxLength:nOut
			   encoding:CFStringConvertEncodingToNSStringEncoding(
																  GetCharEncodingByID((CHAR_ENCODING)encodingID).nCFStringEncoding)];
		[temp release];
	} else {
		return 0;
	}
	
	return strlen(strOut);
}
