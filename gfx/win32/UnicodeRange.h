// UnicodeRange.h: interface for the CUnicodeRange class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNICODERANGE_H__844E93E6_8D23_4759_A0D9_6E78F66B3F55__INCLUDED_)
#define AFX_UNICODERANGE_H__844E93E6_8D23_4759_A0D9_6E78F66B3F55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUnicodeRange  
{
public:
	CUnicodeRange();
	virtual ~CUnicodeRange();

};

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

// The following constants define unicode subranges
// values below kRangeNum must be continuous so that we can map to 
// lang group directly.
// all ranges we care about should be defined under 32, that allows 
// us to store range using bits of a PRUint32

// frequently used range definitions
const uint8   kRangeCyrillic =    ED_CYRILLIC_WINDOWS;//0;
const uint8   kRangeGreek    =    ED_GREEK_WINDOWS;//1;
const uint8   kRangeTurkish  =    ED_TURKISH_WINDOWS;//2;
const uint8   kRangeHebrew   =    ED_HEBREW_WINDOWS;//3;
const uint8   kRangeArabic   =    ED_ARABIC;//4;
const uint8   kRangeBaltic   =    ED_BALTIC_WINDOWS;//5;
const uint8   kRangeThai     =    ED_THAI;//6;
const uint8   kRangeKorean   =    ED_KOREAN;//7;
const uint8   kRangeJapanese =    ED_JAPANESE_SHIFT_JIS;//8;
const uint8   kRangeSChinese =    ED_GB2312;//9;
const uint8   kRangeTChinese =    ED_BIG5;//10;

const uint8   kRangeDevanagari =  20;	// 11
const uint8   kRangeTamil    =   21;	// 12

//const uint8   kRangeSpecificItemNum = 13;

//range/rangeSet grow to this place 13-29

const uint8   kRangeSetStart  =  30;    // range set definition starts from here
const uint8   kRangeSetLatin  =  ED_LATIN9_ISO;
const uint8   kRangeSetCJK    =  ED_GB2312;
const uint8   kRangeSetEnd    =  31;   // range set definition ends here

// less frequently used range definition
const uint8   kRangeSurrogate            = 32;
const uint8   kRangePrivate              = 33;
const uint8   kRangeMisc                 = 34;
const uint8   kRangeUnassigned           = 35;
const uint8   kRangeSyriac               = 36;
const uint8   kRangeThaana               = 37;
const uint8   kRangeBengali              = 38;
const uint8   kRangeGurmukhi             = 39;
const uint8   kRangeGujarati             = 40;
const uint8   kRangeOriya                = 41;
const uint8   kRangeTelugu               = 42;
const uint8   kRangeKannada              = 43;
const uint8   kRangeMalayalam            = 44;
const uint8   kRangeSinhala              = 45;
const uint8   kRangeLao                  = 46;
const uint8   kRangeTibetan              = 47;
const uint8   kRangeMyanmar              = 48;
const uint8   kRangeGeorgian             = 49;
const uint8   kRangeEthiopic             = 50;
const uint8   kRangeCherokee             = 51;
const uint8   kRangeAboriginal           = 52;
const uint8   kRangeOghamRunic           = 53;
const uint8   kRangeKhmer                = 54;
const uint8   kRangeMongolian            = 55;
const uint8   kRangeMathOperators        = 56;
const uint8   kRangeMiscTechnical        = 57;
const uint8   kRangeControlOpticalEnclose = 58;
const uint8   kRangeBoxBlockGeometrics   = 59;
const uint8   kRangeMiscSymbols          = 60;
const uint8   kRangeDingbats             = 61;
const uint8   kRangeBraillePattern       = 62;
const uint8   kRangeYi                   = 63;
const uint8   kRangeCombiningDiacriticalMarks = 64;
const uint8   kRangeArmenian             = 65;

const uint8   kRangeTableBase   = 128;    //values over 127 are reserved for internal use only
const uint8   kRangeTertiaryTable  = 145; // leave room for 16 subtable 
                                            // indices (kRangeTableBase + 1 ..
                                            // kRangeTableBase + 16)



extern uint32 FindCharUnicodeRange(WCHAR ch);
//extern const char* gUnicodeRangeToLangGroupTable[];
//
//inline const char* LangGroupFromUnicodeRange(uint8 unicodeRange)
//{
//  if (kRangeSpecificItemNum > unicodeRange)
//    return gUnicodeRangeToLangGroupTable[unicodeRange];
//  return NULL;
//}

extern CHAR_ENCODING FindCharEncUnicodeRange(WCHAR ch);


#endif // !defined(AFX_UNICODERANGE_H__844E93E6_8D23_4759_A0D9_6E78F66B3F55__INCLUDED_)
