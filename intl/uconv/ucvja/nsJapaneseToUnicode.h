/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */
#ifndef nsShiftJISToUnicode_h__
#define nsShiftJISToUnicode_h__
#include "nsISupports.h"
#include "nsUCvJaSupport.h"


class nsShiftJISToUnicode : public nsBasicDecoderSupport
{
public:

 nsShiftJISToUnicode() 
     { 
         mState=0; mData=0; 
     };
 virtual ~nsShiftJISToUnicode() {};

 NS_IMETHOD Convert(const char * aSrc, PRInt32 * aSrcLength,
     PRUnichar * aDest, PRInt32 * aDestLength) ;
 NS_IMETHOD GetMaxLength(const char * aSrc, PRInt32 aSrcLength,
     PRInt32 * aDestLength) 
     {
        *aDestLength = aSrcLength;
        return NS_OK;
     };
 NS_IMETHOD Reset()
     {
        mState = 0;
        return NS_OK;
     };

private:

private:
 PRInt32  mState;
 PRInt32 mData;
};

class nsEUCJPToUnicodeV2 : public nsBasicDecoderSupport
{
public:

 nsEUCJPToUnicodeV2() 
     { 
          mState=0; mData=0; 
     };
 virtual ~nsEUCJPToUnicodeV2() {};

 NS_IMETHOD Convert(const char * aSrc, PRInt32 * aSrcLength,
     PRUnichar * aDest, PRInt32 * aDestLength) ;
 NS_IMETHOD GetMaxLength(const char * aSrc, PRInt32 aSrcLength,
     PRInt32 * aDestLength) 
     {
        *aDestLength = aSrcLength;
        return NS_OK;
     };
 NS_IMETHOD Reset()
     {
        mState = 0;
        return NS_OK;
     };

private:
 PRInt32  mState;
 PRInt32 mData;
};
 
class nsISO2022JPToUnicodeV2 : public nsBasicDecoderSupport
{
public:

 nsISO2022JPToUnicodeV2() 
     { 
        mState = mState_ASCII;
        mLastLegalState = mState_ASCII;
        mData = 0;
        G2charset = G2_unknown;
        mGB2312Decoder = nsnull;
        mEUCKRDecoder = nsnull;
        mISO88597Decoder = nsnull;
     };
 virtual ~nsISO2022JPToUnicodeV2()
     {
        NS_IF_RELEASE(mGB2312Decoder);
        NS_IF_RELEASE(mEUCKRDecoder);
        NS_IF_RELEASE(mISO88597Decoder);
     };

 NS_IMETHOD Convert(const char * aSrc, PRInt32 * aSrcLength,
     PRUnichar * aDest, PRInt32 * aDestLength) ;
 NS_IMETHOD GetMaxLength(const char * aSrc, PRInt32 aSrcLength,
     PRInt32 * aDestLength) 
     {
        *aDestLength = aSrcLength;
        return NS_OK;
     };
 NS_IMETHOD Reset()
     {
        mState = mState_ASCII;
        mLastLegalState = mState_ASCII;
        return NS_OK;
     };

private:
 enum {
   mState_ASCII,
   mState_ESC,
   mState_ESC_28,
   mState_ESC_24,
   mState_ESC_24_28,
   mState_JISX0201_1976Roman,
   mState_JISX0201_1976Kana,
   mState_JISX0208_1978,
   mState_GB2312_1980,
   mState_JISX0208_1983,
   mState_KSC5601_1987,
   mState_JISX0212_1990,
   mState_JISX0208_1978_2ndbyte,
   mState_GB2312_1980_2ndbyte,
   mState_JISX0208_1983_2ndbyte,
   mState_KSC5601_1987_2ndbyte,
   mState_JISX0212_1990_2ndbyte,
   mState_ESC_2e,
   mState_ESC_4e,
   mState_ERROR
 } mState, mLastLegalState;
 PRInt32 mData;
 enum {
   G2_unknown,
   G2_ISO88591,
   G2_ISO88597
 } G2charset;
 nsIUnicodeDecoder *mGB2312Decoder;
 nsIUnicodeDecoder *mEUCKRDecoder;
 nsIUnicodeDecoder *mISO88597Decoder;
};
#endif // nsShiftJISToUnicode_h__
