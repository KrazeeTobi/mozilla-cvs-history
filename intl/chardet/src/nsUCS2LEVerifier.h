/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */
/* 
 * DO NOT EDIT THIS DOCUMENT MANUALLY !!!
 * THIS FILE IS AUTOMATICALLY GENERATED BY THE TOOLS UNDER
 *    mozilla/intl/chardet/tools/
 * Please contact ftang@netscape.com or mozilla-i18n@mozilla.org
 * if you have any question. Thanks
 */
#include "nsVerifier.h"
static PRUint32 UCS2LE_cls [ 256 / 8 ] = {
PCK4BITS(0,0,0,0,0,0,0,0),  // 00 - 07 
PCK4BITS(0,0,1,0,0,2,0,0),  // 08 - 0f 
PCK4BITS(0,0,0,0,0,0,0,0),  // 10 - 17 
PCK4BITS(0,0,0,3,0,0,0,0),  // 18 - 1f 
PCK4BITS(0,0,0,0,0,0,0,0),  // 20 - 27 
PCK4BITS(0,3,3,3,3,3,0,0),  // 28 - 2f 
PCK4BITS(0,0,0,0,0,0,0,0),  // 30 - 37 
PCK4BITS(0,0,0,0,0,0,0,0),  // 38 - 3f 
PCK4BITS(0,0,0,0,0,0,0,0),  // 40 - 47 
PCK4BITS(0,0,0,0,0,0,0,0),  // 48 - 4f 
PCK4BITS(0,0,0,0,0,0,0,0),  // 50 - 57 
PCK4BITS(0,0,0,0,0,0,0,0),  // 58 - 5f 
PCK4BITS(0,0,0,0,0,0,0,0),  // 60 - 67 
PCK4BITS(0,0,0,0,0,0,0,0),  // 68 - 6f 
PCK4BITS(0,0,0,0,0,0,0,0),  // 70 - 77 
PCK4BITS(0,0,0,0,0,0,0,0),  // 78 - 7f 
PCK4BITS(0,0,0,0,0,0,0,0),  // 80 - 87 
PCK4BITS(0,0,0,0,0,0,0,0),  // 88 - 8f 
PCK4BITS(0,0,0,0,0,0,0,0),  // 90 - 97 
PCK4BITS(0,0,0,0,0,0,0,0),  // 98 - 9f 
PCK4BITS(0,0,0,0,0,0,0,0),  // a0 - a7 
PCK4BITS(0,0,0,0,0,0,0,0),  // a8 - af 
PCK4BITS(0,0,0,0,0,0,0,0),  // b0 - b7 
PCK4BITS(0,0,0,0,0,0,0,0),  // b8 - bf 
PCK4BITS(0,0,0,0,0,0,0,0),  // c0 - c7 
PCK4BITS(0,0,0,0,0,0,0,0),  // c8 - cf 
PCK4BITS(0,0,0,0,0,0,0,0),  // d0 - d7 
PCK4BITS(0,0,0,0,0,0,0,0),  // d8 - df 
PCK4BITS(0,0,0,0,0,0,0,0),  // e0 - e7 
PCK4BITS(0,0,0,0,0,0,0,0),  // e8 - ef 
PCK4BITS(0,0,0,0,0,0,0,0),  // f0 - f7 
PCK4BITS(0,0,0,0,0,0,4,5)   // f8 - ff 
};


static PRUint32 UCS2LE_st [ 7] = {
PCK4BITS(     6,     6,     7,     6,     4,     3,eError,eError),//00-07 
PCK4BITS(eError,eError,eError,eError,eItsMe,eItsMe,eItsMe,eItsMe),//08-0f 
PCK4BITS(eItsMe,eItsMe,     5,     5,     5,eError,eItsMe,eError),//10-17 
PCK4BITS(     5,     5,     5,eError,     5,eError,     6,     6),//18-1f 
PCK4BITS(     7,     6,     8,     8,     5,     5,     5,eError),//20-27 
PCK4BITS(     5,     5,     5,eError,eError,eError,     5,     5),//28-2f 
PCK4BITS(     5,     5,     5,eError,     5,eError,eStart,eStart) //30-37 
};


static nsVerifier nsUCS2LEVerifier = {
     "UTF-16LE",
    {
       eIdxSft4bits, 
       eSftMsk4bits, 
       eBitSft4bits, 
       eUnitMsk4bits, 
       UCS2LE_cls 
    },
    6,
    {
       eIdxSft4bits, 
       eSftMsk4bits, 
       eBitSft4bits, 
       eUnitMsk4bits, 
       UCS2LE_st 
    }
};
