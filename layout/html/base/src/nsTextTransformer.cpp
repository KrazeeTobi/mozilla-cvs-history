/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation.  All Rights Reserved.
 */
#include "nsTextTransformer.h"
#include "nsIContent.h"
#include "nsIFrame.h"
#include "nsIStyleContext.h"
#include "nsITextContent.h"
#include "nsStyleConsts.h"
#include "nsILineBreaker.h"
#include "nsIWordBreaker.h"
#include "nsHTMLIIDs.h"
#include "nsIServiceManager.h"
#include "nsUnicharUtilCIID.h"
#include "nsICaseConversion.h"

static NS_DEFINE_IID(kUnicharUtilCID, NS_UNICHARUTIL_CID);
static NS_DEFINE_IID(kICaseConversionIID, NS_ICASECONVERSION_IID);
static nsICaseConversion* gCaseConv =  nsnull;
static nsrefcnt gCaseConvRefCnt = 0;

//#define DEBUG_GETPREVWORD
// XXX I'm sure there are other special characters
#define CH_NBSP 160

#define MAX_UNIBYTE 127

nsTextTransformer::nsTextTransformer(nsILineBreaker* aLineBreaker,
                                     nsIWordBreaker* aWordBreaker)
  : mBuffer(mAutoWordBuffer),
    mBufferLength(NS_TEXT_TRANSFORMER_AUTO_WORD_BUF_SIZE),
    mHasMultibyte(PR_FALSE),
    mLineBreaker(aLineBreaker),
    mWordBreaker(aWordBreaker)
{
  if (gCaseConvRefCnt++ == 0) {
    nsresult res;
    res = nsServiceManager::GetService(kUnicharUtilCID, kICaseConversionIID,
                                       (nsISupports**)&gCaseConv);
    NS_ASSERTION( NS_SUCCEEDED(res), "cannot get UnicharUtil");
    NS_ASSERTION( gCaseConv != NULL, "cannot get UnicharUtil");
  }
}

nsTextTransformer::~nsTextTransformer()
{
  if (mBuffer != mAutoWordBuffer) {
    delete [] mBuffer;
  }
  if (--gCaseConvRefCnt == 0) {
    nsServiceManager::ReleaseService(kUnicharUtilCID, gCaseConv);
    gCaseConv = nsnull;
  }
}

nsresult
nsTextTransformer::Init(nsIFrame* aFrame,
                        nsIContent* aContent,
                        PRInt32 aStartingOffset)
{
  // Get the frames text content
  nsITextContent* tc;
  if (NS_OK != aContent->QueryInterface(kITextContentIID, (void**) &tc)) {
    return NS_OK;
  }
  tc->GetText(mFrags, mNumFrags);
  NS_RELEASE(tc);
  mStartingOffset = aStartingOffset;
  mOffset = mStartingOffset;

  // Compute the total length of the text content.
  PRInt32 sum = 0;
  PRInt32 n = mNumFrags;
  const nsTextFragment* frag = mFrags;
  for (; --n >= 0; frag++) {
    sum += frag->GetLength();
  }
  mContentLength = sum;

  // Set current fragment and current fragment offset
  mCurrentFrag = mFrags;
  mCurrentFragOffset = 0;
  PRInt32 offset = 0;
  n = mNumFrags;
  for (frag = mFrags; --n >= 0; frag++) {
    if (aStartingOffset < offset + frag->GetLength()) {
      mCurrentFrag = frag;
      mCurrentFragOffset = aStartingOffset - offset;
      break;
    }
    offset += frag->GetLength();
  }
  if (mNumFrags && aStartingOffset == mContentLength){
    mCurrentFrag = mFrags + (mNumFrags -1);
    mCurrentFragOffset = mCurrentFrag->GetLength();
  }

  // Get the frames style and choose a transform proc
  const nsStyleText* styleText;
  aFrame->GetStyleData(eStyleStruct_Text, (const nsStyleStruct*&) styleText);
  mPreformatted = (NS_STYLE_WHITESPACE_PRE == styleText->mWhiteSpace) ||
    (NS_STYLE_WHITESPACE_MOZ_PRE_WRAP == styleText->mWhiteSpace);
  mTextTransform = styleText->mTextTransform;
  return NS_OK;
}

PRBool
nsTextTransformer::GrowBuffer(PRBool aForNextWord)
{
  PRInt32 newLen = mBufferLength * 2;
  if (newLen <= 100) {
    newLen = 100;
  }
  PRUnichar* newBuffer = new PRUnichar[newLen];
  if (nsnull == newBuffer) {
    return PR_FALSE;
  }
  if (0 != mBufferLength) {
    if(aForNextWord)
      nsCRT::memcpy(newBuffer, mBuffer, sizeof(PRUnichar) * mBufferLength);
    else
      nsCRT::memcpy(&newBuffer[mBufferLength], mBuffer,
                    sizeof(PRUnichar) * mBufferLength);
    if (mBuffer != mAutoWordBuffer) {
      delete [] mBuffer;
    }
  }
  mBuffer = newBuffer;
  mBufferLength = newLen;
  return PR_TRUE;
}

PRUnichar*
nsTextTransformer::GetNextWord(PRBool aInWord,
                               PRInt32& aWordLenResult,
                               PRInt32& aContentLenResult,
                               PRBool& aIsWhitespaceResult,
                               PRBool aForLineBreak)
{
  NS_PRECONDITION(mOffset <= mContentLength, "bad offset");
  NS_PRECONDITION(((nsnull != mLineBreaker)||(!aForLineBreak)), "null in line breaker");
  NS_PRECONDITION(((nsnull != mWordBreaker)||( aForLineBreak)), "null in word breaker");

  // See if the content has been exhausted
  if (mOffset == mContentLength) {
    aWordLenResult = 0;
    aContentLenResult = 0;
    return nsnull;
  }

  PRUnichar* bp = mBuffer;
  PRUnichar* bufEnd = mBuffer + mBufferLength;
  const nsTextFragment* frag = mCurrentFrag;
  const nsTextFragment* lastFrag = mFrags + mNumFrags;
  PRInt32 wordLen = 1;
  PRInt32 contentLen = 1;

  // Set the isWhitespace flag by examining the next character in the
  // text fragment.
  PRInt32 offset = mCurrentFragOffset;
  PRUnichar firstChar;
  if (frag->Is2b()) {
    const PRUnichar* up = frag->Get2b();
    firstChar = up[offset];
  }
  else {
    const unsigned char* cp = (const unsigned char*) frag->Get1b();
    firstChar = PRUnichar(cp[offset]);
  }
  PRBool isWhitespace = XP_IS_SPACE(firstChar);
  offset++;
  if (isWhitespace) {
    if (mPreformatted) {
      if ('\t' == firstChar) {
        // Leave tab alone so that caller can expand it
      }
      else if ('\n' == firstChar) {
        // Advance content past newline but do not allow newline to
        // remain in the word.
        wordLen--;
      }
      else {
        firstChar = ' ';
      }
    }
    else {
      firstChar = ' ';
    }
  }
  else if (CH_NBSP == firstChar) {
    firstChar = ' ';
  }
  if (firstChar > MAX_UNIBYTE) mHasMultibyte = PR_TRUE;
  *bp++ = firstChar;
  if (offset == frag->GetLength()) {
    mCurrentFrag = ++frag;
    offset = 0;
  }
  mCurrentFragOffset = offset;
  if (isWhitespace && mPreformatted) {
    goto really_done;
  }

  PRInt32 numChars;
  while (frag < lastFrag) {
    PRInt32 fragLen = frag->GetLength();

    // Scan characters in this fragment that are the same kind as the
    // isWhitespace flag indicates.
    if (frag->Is2b()) {
      const PRUnichar* cp0 = frag->Get2b();
      const PRUnichar* end = cp0 + fragLen;
      const PRUnichar* cp = cp0 + offset;
      if (isWhitespace) {
        while (cp < end) {
          PRUnichar ch = *cp;
          if (XP_IS_SPACE(ch)) {
            cp++;
            continue;
          }
          numChars = (cp - offset) - cp0;
          contentLen += numChars;
          mCurrentFragOffset += numChars;
          goto done;
        }
        numChars = (cp - offset) - cp0;
        contentLen += numChars;
      }
      else {
		if(wordLen > 0) {
			nsresult res = NS_OK;
			PRBool breakBetween = PR_FALSE;
			if(aForLineBreak)
			   res = mLineBreaker->BreakInBetween(mBuffer, wordLen, 
				                         cp, (fragLen-offset), &breakBetween);
			else 
			   res = mWordBreaker->BreakInBetween(mBuffer, wordLen, 
				                         cp, (fragLen-offset), &breakBetween);
			if ( breakBetween )
				goto done;

			PRBool tryNextFrag = PR_FALSE;
			PRUint32 next;

			// Find next position
                        
			if(aForLineBreak)
			   res = mLineBreaker->Next(cp0, fragLen, offset, &next, &tryNextFrag);
      else
			   res = mWordBreaker->Next(cp0, fragLen, offset, &next, &tryNextFrag);
       
			
			numChars = (next - offset);
			// check buffer size before copy
			while((bp + numChars ) > bufEnd) {
				PRInt32 delta = bp - mBuffer;
				if(!GrowBuffer()) {
					goto done;
				}
				bp = mBuffer + delta;
				bufEnd = mBuffer + mBufferLength;
			}

			wordLen += numChars;
			mCurrentFragOffset += numChars;
			contentLen += numChars;
			end = cp + numChars;

			// 1. convert nbsp into space
			// 2. check mHasMultibyte flag
			// 3. copy buffer

			while(cp < end) {
		        PRUnichar ch = *cp++;
				if (CH_NBSP == ch) ch = ' ';
				if (ch > MAX_UNIBYTE) mHasMultibyte = PR_TRUE;
				*bp++ = ch;
			}
			if(! tryNextFrag) {
				// can decide break position inside this TextFrag
				goto done;
			}
		}
      }
    }
    else {
      const unsigned char* cp0 = (const unsigned char*) frag->Get1b();
      const unsigned char* end = cp0 + fragLen;
      const unsigned char* cp = cp0 + offset;
      if (isWhitespace) {
        while (cp < end) {
          PRUnichar ch = PRUnichar(*cp);
          if (XP_IS_SPACE(ch)) {
            cp++;
            continue;
          }
          numChars = (cp - offset) - cp0;
          contentLen += numChars;
          mCurrentFragOffset += numChars;
          goto done;
        }
        numChars = (cp - offset) - cp0;
        contentLen += numChars;
      }
      else {
        while (cp < end) {
          PRUnichar ch = PRUnichar(*cp);
          if (!XP_IS_SPACE(ch)) {
            if (CH_NBSP == ch) ch = ' ';
            if (ch > MAX_UNIBYTE) mHasMultibyte = PR_TRUE;
            cp++;

            // Store character in buffer; grow buffer if we have to
            NS_ASSERTION(bp < bufEnd, "whoops");
            *bp++ = ch;
            if (bp == bufEnd) {
              PRInt32 delta = bp - mBuffer;
              if (!GrowBuffer()) {
                goto done;
              }
              bp = mBuffer + delta;
              bufEnd = mBuffer + mBufferLength;
            }
            continue;
          }
          numChars = (cp - offset) - cp0;
          wordLen += numChars;
          contentLen += numChars;
          mCurrentFragOffset += numChars;
          goto done;
        }
        numChars = (cp - offset) - cp0;
        wordLen += numChars;
        contentLen += numChars;
      }
    }

    // Advance to next text fragment
    frag++;
    mCurrentFrag = frag;
    mCurrentFragOffset = 0;
    offset = 0;
  }

 done:;

  if (!isWhitespace)
  {
     switch(mTextTransform)
     {
       case NS_STYLE_TEXT_TRANSFORM_CAPITALIZE:
           gCaseConv->ToTitle(mBuffer, mBuffer, wordLen, !aInWord);
         break;
       case NS_STYLE_TEXT_TRANSFORM_LOWERCASE:
           gCaseConv->ToLower(mBuffer, mBuffer, wordLen );
         break;
       case NS_STYLE_TEXT_TRANSFORM_UPPERCASE:
           gCaseConv->ToUpper(mBuffer, mBuffer, wordLen );
         break;
       default:
         break;
     }
  }

 really_done:;
  mOffset += contentLen;
  NS_ASSERTION(mOffset <= mContentLength, "whoops");
  aWordLenResult = wordLen;
  aContentLenResult = contentLen;
  aIsWhitespaceResult = isWhitespace;

  return mBuffer;
}

PRUnichar*
nsTextTransformer::GetPrevWord(PRBool aInWord,
                               PRInt32& aWordLenResult,
                               PRInt32& aContentLenResult,
                               PRBool& aIsWhitespaceResult,
                               PRBool aForLineBreak)
{
  NS_PRECONDITION(mOffset <= mContentLength, "bad offset");
  NS_PRECONDITION(((nsnull != mLineBreaker)||(!aForLineBreak)), "null in line breaker");
  NS_PRECONDITION(((nsnull != mWordBreaker)||( aForLineBreak)), "null in word breaker");

  // See if the content has been exhausted
  if (mOffset == 0) {
    aWordLenResult = 0;
    aContentLenResult = 0;
    return nsnull;
  }

  PRUnichar* bp = mBuffer+mBufferLength-1;
  PRUnichar* bufEnd = mBuffer ;
  const nsTextFragment* frag = mCurrentFrag;
  const nsTextFragment* lastFrag = mFrags;//1st is the last
  PRInt32 wordLen = 1;
  PRInt32 contentLen = 1;

  // Set the isWhitespace flag by examining the next character in the
  // text fragment.
  PRInt32 offset = mCurrentFragOffset-1;
  PRUnichar firstChar;
  if (frag->Is2b()) {
    const PRUnichar* up = frag->Get2b();
    if (offset > 0)
       firstChar = up[offset];
    else 
       firstChar = up[0];
  }
  else {
    const unsigned char* cp = (const unsigned char*) frag->Get1b();
    if (offset > 0)
      firstChar = PRUnichar(cp[offset]);
    else
      firstChar = PRUnichar(cp[0]);
  }
  PRBool isWhitespace = XP_IS_SPACE(firstChar);
  offset--;
  if (isWhitespace) {
    if (mPreformatted) {
      if ('\t' == firstChar) {
        // Leave tab alone so that caller can expand it
      }
      else if ('\n' == firstChar) {
        // Advance content past newline but do not allow newline to
        // remain in the word.
        wordLen--;
      }
      else {
        firstChar = ' ';
      }
    }
    else {
      firstChar = ' ';
    }
  }
  else if (CH_NBSP == firstChar) {
    firstChar = ' ';
  }
  if(firstChar > MAX_UNIBYTE) mHasMultibyte = PR_TRUE;
  *bp-- = firstChar;
  mCurrentFragOffset = offset +1;
  if (offset < 0) {
    if (mCurrentFrag == mFrags){
      goto really_done;
    }
    mCurrentFrag = --frag;
    offset = mCurrentFrag->GetLength()-1;
  }
  if (isWhitespace && mPreformatted) {
    goto really_done;
  }

  PRInt32 numChars;
  do {
    // Scan characters in this fragment that are the same kind as the
    // isWhitespace flag indicates.
    if (frag->Is2b()) {
      const PRUnichar* cp0 = frag->Get2b();
      const PRUnichar* end = cp0;
      const PRUnichar* cp = cp0 + offset;
      if (isWhitespace) {
        while (cp > end) {
          PRUnichar ch = *cp;
          if (XP_IS_SPACE(ch)) {
            cp--;
            continue;
          }
          numChars = (cp0 + offset) - cp;
          contentLen += numChars;
          mCurrentFragOffset -= numChars;
          goto done;
        }
        numChars = (cp0 + offset) - cp;
        contentLen += numChars;
      }
      else {
         if(wordLen > 0) {
            nsresult res = NS_OK;
            PRBool breakBetween = PR_FALSE;
            if(aForLineBreak)
                res = mLineBreaker->BreakInBetween(
                              cp0, offset+1, 
                              &(mBuffer[mBufferLength-wordLen]), wordLen,
                              &breakBetween);
            else
                res = mWordBreaker->BreakInBetween(
                              cp0, offset+1, 
                              &(mBuffer[mBufferLength-wordLen]), wordLen,
                              &breakBetween);
            if ( breakBetween )
              goto done;

            PRBool tryPrevFrag = PR_FALSE;
            PRUint32 prev;

            // Find prev position

            if(aForLineBreak)
                res = mLineBreaker->Prev(cp0, offset, offset, &prev, &tryPrevFrag);
            else
                res = mWordBreaker->Prev(cp0, offset, offset, &prev, &tryPrevFrag);


            numChars = (offset - prev)+1;
            // check buffer size before copy
            while((bp - numChars ) < bufEnd) {
              PRInt32 delta = (&(mBuffer[mBufferLength])) - bp -1 ;
              if(!GrowBuffer()) {
                 goto done;
              }
              bp = (&(mBuffer[mBufferLength])) - delta - 1;
              bufEnd = mBuffer;
            }

            wordLen += numChars;
            mCurrentFragOffset -= numChars;
            contentLen += numChars;
            end = cp - numChars;

            // 1. convert nbsp into space
            // 2. check mHasMultibyte flag
            // 3. copy buffer

            while(cp > end) {
                    PRUnichar ch = *cp--;
              if (CH_NBSP == ch) ch = ' ';
              if (ch > MAX_UNIBYTE) mHasMultibyte = PR_TRUE;
              *bp-- = ch;
            }
            if(! tryPrevFrag) {
              // can decide break position inside this TextFrag
              goto done;
            }
        }
      }
    }
    else {
      const unsigned char* cp0 = (const unsigned char*) frag->Get1b();
      const unsigned char* end = cp0;
      const unsigned char* cp = cp0 + offset;
      if (isWhitespace) {
        while (cp > end) {
          PRUnichar ch = PRUnichar(*cp);
          if (XP_IS_SPACE(ch)) {
            cp--;
            continue;
          }
          numChars = (cp0 + offset) - cp;
          contentLen += numChars;
          mCurrentFragOffset -= numChars;
          goto done;
        }
        numChars = (cp0 + offset) - cp;
        contentLen += numChars;
      }
      else {
        while (cp >= end) {
          PRUnichar ch = PRUnichar(*cp);
          if (!XP_IS_SPACE(ch)) {
            if (CH_NBSP == ch) ch = ' ';
            if (ch > MAX_UNIBYTE) mHasMultibyte = PR_TRUE;
            cp--;

            // Store character in buffer; grow buffer if we have to
            NS_ASSERTION(bp > bufEnd, "whoops");
            *bp-- = ch;
            if (bp == bufEnd) {
              PRInt32 delta = (&(mBuffer[mBufferLength])) - bp - 1;
              if (!GrowBuffer(PR_FALSE)) {
                goto done;
              }
              bp = (&(mBuffer[mBufferLength])) - delta - 1;
              bufEnd = mBuffer;
            }
            continue;
          }
          numChars = (cp0 + offset) - cp;
          wordLen += numChars;
          contentLen += numChars;
          mCurrentFragOffset -= numChars;
          goto done;
        }
        numChars = (cp0 + offset) - cp;
        wordLen += numChars;
        contentLen += numChars;
      }
    }

    // Advance to next text fragment
    if (frag != lastFrag)
    {
      frag--;
      mCurrentFrag = frag;
      mCurrentFragOffset = mCurrentFrag->GetLength()-1;
      offset = mCurrentFragOffset;
    }
    else
      mCurrentFragOffset = 0;
  }
  while (frag > lastFrag);

 done:;

  if (!isWhitespace)
  {
     switch(mTextTransform)
     {
       case NS_STYLE_TEXT_TRANSFORM_CAPITALIZE:
           gCaseConv->ToTitle(&(mBuffer[mBufferLength-wordLen]), 
                              &(mBuffer[mBufferLength-wordLen]),
                              wordLen, !aInWord);
         break;
       case NS_STYLE_TEXT_TRANSFORM_LOWERCASE:
           gCaseConv->ToLower(&(mBuffer[mBufferLength-wordLen]), 
                              &(mBuffer[mBufferLength-wordLen]), 
                              wordLen );
         break;
       case NS_STYLE_TEXT_TRANSFORM_UPPERCASE:
           gCaseConv->ToUpper(&(mBuffer[mBufferLength-wordLen]), 
                              &(mBuffer[mBufferLength-wordLen]), 
                              wordLen );
         break;
       default:
         break;
     }
  }


 really_done:;
  mOffset -= contentLen;
  NS_ASSERTION(mOffset >= 0, "whoops");
  aWordLenResult = wordLen;
  aContentLenResult = contentLen;
  aIsWhitespaceResult = isWhitespace;


#ifdef DEBUG_GETPREVWORD
{
  printf(aIsWhitespaceResult ? "#1 WHITESPACE\n": "NOT WHITESPACE\n");
  if(! aIsWhitespaceResult)
  {
     PRUnichar* wordBufMem =  &(mBuffer[mBufferLength-wordLen]);
     PRInt32 ax;
     for(ax=0; ax<wordLen; ax++) {
        if(wordBufMem[ax] < 0x080)
           printf("%c", (char)wordBufMem[ax]);
        else
           printf("[U+%04X]", wordBufMem[ax]);
     }
     printf("(%d)\n",wordLen);
  }
}
#endif
  return &(mBuffer[mBufferLength-wordLen]);
}
