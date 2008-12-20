// UnicodeRangeCharSet.h: interface for the CUnicodeRangeCharSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNICODERANGECHARSET_H__22099824_B309_4F6E_9CE4_403781358BB0__INCLUDED_)
#define AFX_UNICODERANGECHARSET_H__22099824_B309_4F6E_9CE4_403781358BB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _WINCE_
#ifndef HANGUL_CHARSET
#define HANGUL_CHARSET	DEFAULT_CHARSET
#endif
#endif

class CUnicodeRangeCharSet  
{
public:
	CUnicodeRangeCharSet();
	virtual ~CUnicodeRangeCharSet();

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
const uint8   kRangeCyrillic =		DEFAULT_CHARSET;//0;
const uint8   kRangeGreek    =    GREEK_CHARSET;//1;
const uint8   kRangeTurkish  =    TURKISH_CHARSET;//2;
const uint8   kRangeHebrew   =    HEBREW_CHARSET;//3;
const uint8   kRangeArabic   =    ARABIC_CHARSET;//4;
const uint8   kRangeBaltic   =    BALTIC_CHARSET;//5;
const uint8   kRangeThai     =    THAI_CHARSET;//6;
const uint8   kRangeKorean   =    HANGUL_CHARSET;//7;
const uint8   kRangeJapanese =    SHIFTJIS_CHARSET;//8;
const uint8   kRangeSChinese =    GB2312_CHARSET;//9;
const uint8   kRangeTChinese =    CHINESEBIG5_CHARSET;//10;

const uint8   kRangeDevanagari =  DEFAULT_CHARSET;	// 11
const uint8   kRangeTamil    =   DEFAULT_CHARSET;	// 12

//const uint8   kRangeSpecificItemNum = 13;

//range/rangeSet grow to this place 13-29

const uint8   kRangeSetStart  =  30;    // range set definition starts from here
const uint8   kRangeSetLatin  =		DEFAULT_CHARSET;
const uint8   kRangeSetCJK    =  GB2312_CHARSET;
const uint8   kRangeSetEnd    =  31;   // range set definition ends here

// less frequently used range definition
const uint8   kRangeSurrogate            = DEFAULT_CHARSET;
const uint8   kRangePrivate              = DEFAULT_CHARSET;
const uint8   kRangeMisc                 = DEFAULT_CHARSET;
const uint8   kRangeUnassigned           = DEFAULT_CHARSET;
const uint8   kRangeSyriac               = DEFAULT_CHARSET;
const uint8   kRangeThaana               = DEFAULT_CHARSET;
const uint8   kRangeBengali              = DEFAULT_CHARSET;
const uint8   kRangeGurmukhi             = DEFAULT_CHARSET;
const uint8   kRangeGujarati             = DEFAULT_CHARSET;
const uint8   kRangeOriya                = DEFAULT_CHARSET;
const uint8   kRangeTelugu               = DEFAULT_CHARSET;
const uint8   kRangeKannada              = DEFAULT_CHARSET;
const uint8   kRangeMalayalam            = DEFAULT_CHARSET;
const uint8   kRangeSinhala              = DEFAULT_CHARSET;
const uint8   kRangeLao                  = DEFAULT_CHARSET;
const uint8   kRangeTibetan              = DEFAULT_CHARSET;
const uint8   kRangeMyanmar              = DEFAULT_CHARSET;
const uint8   kRangeGeorgian             = DEFAULT_CHARSET;
const uint8   kRangeEthiopic             = DEFAULT_CHARSET;
const uint8   kRangeCherokee             = DEFAULT_CHARSET;
const uint8   kRangeAboriginal           = DEFAULT_CHARSET;
const uint8   kRangeOghamRunic           = DEFAULT_CHARSET;
const uint8   kRangeKhmer                = DEFAULT_CHARSET;
const uint8   kRangeMongolian            = DEFAULT_CHARSET;
const uint8   kRangeMathOperators        = DEFAULT_CHARSET;
const uint8   kRangeMiscTechnical        = DEFAULT_CHARSET;
const uint8   kRangeControlOpticalEnclose = DEFAULT_CHARSET;
const uint8   kRangeBoxBlockGeometrics   = DEFAULT_CHARSET;
const uint8   kRangeMiscSymbols          = DEFAULT_CHARSET;
const uint8   kRangeDingbats             = DEFAULT_CHARSET;
const uint8   kRangeBraillePattern       = DEFAULT_CHARSET;
const uint8   kRangeYi                   = DEFAULT_CHARSET;
const uint8   kRangeCombiningDiacriticalMarks = DEFAULT_CHARSET;
const uint8   kRangeArmenian             = DEFAULT_CHARSET;

const uint8   kRangeTableBase   = 128;    //values over 127 are reserved for internal use only
const uint8   kRangeTertiaryTable  = 145; // leave room for 16 subtable 
                                            // indices (kRangeTableBase + 1 ..
                                            // kRangeTableBase + 16)



extern uint8 FindUCharRangCharSet(WCHAR ch);

#endif // !defined(AFX_UNICODERANGECHARSET_H__22099824_B309_4F6E_9CE4_403781358BB0__INCLUDED_)
