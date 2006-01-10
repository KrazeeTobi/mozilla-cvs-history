/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#include "nsFrameList.h"
#ifdef NS_DEBUG
#include "nsIFrameDebug.h"
#endif
#include "nsVoidArray.h"
#include "nsLayoutUtils.h"

#ifdef IBMBIDI
#include "nsCOMPtr.h"
#include "nsLayoutAtoms.h"
#include "nsILineIterator.h"

/**
 * Helper class for comparing the location of frames. Used by
 * GetPrevVisualFor() and GetNextVisualFor()
 */
#define LINE_MIN 0
#define XCOORD_MIN 0x80000000
#define MY_LINE_MAX 0x7fffffff
#define XCOORD_MAX 0x7fffffff
class nsFrameOrigin {
public:
  // default constructor
  nsFrameOrigin() {
    mLine = 0;
    mXCoord = 0;
    mDirection = NS_STYLE_DIRECTION_LTR;
  }

  nsFrameOrigin(PRInt32 line, nscoord xCoord, PRUint8 direction) {
    mLine = line;
    mXCoord = xCoord;
    mDirection = direction;
  }

  // copy constructor
  nsFrameOrigin(const nsFrameOrigin& aFrameOrigin) {
    mLine = aFrameOrigin.mLine;
    mXCoord = aFrameOrigin.mXCoord;
    mDirection = aFrameOrigin.mDirection;
  }

  ~nsFrameOrigin() {
  }

  nsFrameOrigin& operator=(const nsFrameOrigin& aFrameOrigin) { 
    mLine = aFrameOrigin.mLine;
    mXCoord = aFrameOrigin.mXCoord;
    mDirection = aFrameOrigin.mDirection;
    return *this;
  }

  PRBool operator<(const nsFrameOrigin& aFrameOrigin) {
    if (mLine < aFrameOrigin.mLine) {
      return PR_TRUE;
    }
    if (mLine > aFrameOrigin.mLine) {
      return PR_FALSE;
    }
    if (mDirection == NS_STYLE_DIRECTION_LTR && mXCoord < aFrameOrigin.mXCoord ||
        mDirection == NS_STYLE_DIRECTION_RTL && mXCoord > aFrameOrigin.mXCoord) {
      return PR_TRUE;
    }
    return PR_FALSE;
  }
  
  PRBool operator>(const nsFrameOrigin& aFrameOrigin) {
    if (mLine > aFrameOrigin.mLine) {
      return PR_TRUE;
    }
    if (mLine < aFrameOrigin.mLine) {
      return PR_FALSE;
    }
    if (mDirection == NS_STYLE_DIRECTION_LTR && mXCoord > aFrameOrigin.mXCoord ||
        mDirection == NS_STYLE_DIRECTION_RTL && mXCoord < aFrameOrigin.mXCoord) {
      return PR_TRUE;
    }
    return PR_FALSE;
  }

  PRBool operator==(const nsFrameOrigin& aFrameOrigin) {
    if (aFrameOrigin.mLine == mLine && aFrameOrigin.mXCoord == mXCoord) {
      return PR_TRUE;
    }
    return PR_FALSE;
  }

protected:
  PRInt32 mLine;
  nscoord mXCoord;
  PRUint8 mDirection;
};
#endif // IBMBIDI

void
nsFrameList::DestroyFrames(nsPresContext* aPresContext)
{
  nsIFrame* next;
  for (nsIFrame* frame = mFirstChild; frame; frame = next) {
    next = frame->GetNextSibling();
    frame->Destroy(aPresContext);
    mFirstChild = next;
  }
}

void
nsFrameList::AppendFrames(nsIFrame* aParent, nsIFrame* aFrameList)
{
  NS_PRECONDITION(nsnull != aFrameList, "null ptr");
  if (nsnull != aFrameList) {
    nsIFrame* lastChild = LastChild();
    if (nsnull == lastChild) {
      mFirstChild = aFrameList;
    }
    else {
      lastChild->SetNextSibling(aFrameList);
    }
    if (aParent) {
      for (nsIFrame* frame = aFrameList; frame;
           frame = frame->GetNextSibling()) {
        frame->SetParent(aParent);
      }
    }
  }
#ifdef DEBUG
  CheckForLoops();
#endif
}

void
nsFrameList::AppendFrame(nsIFrame* aParent, nsIFrame* aFrame)
{
  NS_PRECONDITION(nsnull != aFrame, "null ptr");
  if (nsnull != aFrame) {
    NS_PRECONDITION(!aFrame->GetNextSibling(), "Can only append one frame here");
    nsIFrame* lastChild = LastChild();
    if (nsnull == lastChild) {
      mFirstChild = aFrame;
    }
    else {
      lastChild->SetNextSibling(aFrame);
    }
    if (nsnull != aParent) {
      aFrame->SetParent(aParent);
    }
  }
#ifdef DEBUG
  CheckForLoops();
#endif
}

PRBool
nsFrameList::RemoveFrame(nsIFrame* aFrame, nsIFrame* aPrevSiblingHint)
{
  NS_PRECONDITION(nsnull != aFrame, "null ptr");
  if (aFrame) {
    nsIFrame* nextFrame = aFrame->GetNextSibling();
    if (aFrame == mFirstChild) {
      mFirstChild = nextFrame;
      aFrame->SetNextSibling(nsnull);
      return PR_TRUE;
    }
    else {
      nsIFrame* prevSibling = aPrevSiblingHint;
      if (!prevSibling || prevSibling->GetNextSibling() != aFrame) {
        prevSibling = GetPrevSiblingFor(aFrame);
      }
      if (prevSibling) {
        prevSibling->SetNextSibling(nextFrame);
        aFrame->SetNextSibling(nsnull);
        return PR_TRUE;
      }
    }
  }
  // aFrame was not in the list. 
  return PR_FALSE;
}

PRBool
nsFrameList::RemoveFirstChild()
{
  if (mFirstChild) {
    nsIFrame* nextFrame = mFirstChild->GetNextSibling();
    mFirstChild->SetNextSibling(nsnull);
    mFirstChild = nextFrame;
    return PR_TRUE;
  }
  return PR_FALSE;
}

PRBool
nsFrameList::DestroyFrame(nsPresContext* aPresContext, nsIFrame* aFrame)
{
  NS_PRECONDITION(nsnull != aFrame, "null ptr");
  if (RemoveFrame(aFrame)) {
    aFrame->Destroy(aPresContext);
    return PR_TRUE;
  }
  return PR_FALSE;
}

void
nsFrameList::InsertFrame(nsIFrame* aParent,
                         nsIFrame* aPrevSibling,
                         nsIFrame* aNewFrame)
{
  NS_PRECONDITION(nsnull != aNewFrame, "null ptr");
  if (nsnull != aNewFrame) {
    if (nsnull == aPrevSibling) {
      aNewFrame->SetNextSibling(mFirstChild);
      mFirstChild = aNewFrame;
    }
    else {
      nsIFrame* nextFrame = aPrevSibling->GetNextSibling();
      aPrevSibling->SetNextSibling(aNewFrame);
      aNewFrame->SetNextSibling(nextFrame);
    }
    if (aParent) {
      aNewFrame->SetParent(aParent);
    }
  }
#ifdef DEBUG
  CheckForLoops();
#endif
}

void
nsFrameList::InsertFrames(nsIFrame* aParent,
                          nsIFrame* aPrevSibling,
                          nsIFrame* aFrameList)
{
  NS_PRECONDITION(nsnull != aFrameList, "null ptr");
  if (nsnull != aFrameList) {
    nsIFrame* lastNewFrame = nsnull;
    if (aParent) {
      for (nsIFrame* frame = aFrameList; frame;
           frame = frame->GetNextSibling()) {
        frame->SetParent(aParent);
        lastNewFrame = frame;
      }
    }

    // Get the last new frame if necessary
    if (!lastNewFrame) {
      nsFrameList tmp(aFrameList);
      lastNewFrame = tmp.LastChild();
    }

    // Link the new frames into the child list
    if (nsnull == aPrevSibling) {
      lastNewFrame->SetNextSibling(mFirstChild);
      mFirstChild = aFrameList;
    }
    else {
      nsIFrame* nextFrame = aPrevSibling->GetNextSibling();
      aPrevSibling->SetNextSibling(aFrameList);
      lastNewFrame->SetNextSibling(nextFrame);
    }
  }
#ifdef DEBUG
  CheckForLoops();
#endif
}

PRBool
nsFrameList::Split(nsIFrame* aAfterFrame, nsIFrame** aNextFrameResult)
{
  NS_PRECONDITION(nsnull != aAfterFrame, "null ptr");
  NS_PRECONDITION(nsnull != aNextFrameResult, "null ptr");
  NS_ASSERTION(ContainsFrame(aAfterFrame), "split after unknown frame");

  if (aNextFrameResult && aAfterFrame) {
    nsIFrame* nextFrame = aAfterFrame->GetNextSibling();
    aAfterFrame->SetNextSibling(nsnull);
    *aNextFrameResult = nextFrame;
    return PR_TRUE;
  }
  return PR_FALSE;
}

nsIFrame*
nsFrameList::PullFrame(nsIFrame* aParent,
                       nsIFrame* aLastChild,
                       nsFrameList& aFromList)
{
  NS_PRECONDITION(nsnull != aParent, "null ptr");

  nsIFrame* pulledFrame = nsnull;
  if (nsnull != aParent) {
    pulledFrame = aFromList.FirstChild();
    if (nsnull != pulledFrame) {
      // Take frame off old list
      aFromList.RemoveFirstChild();

      // Put it on the end of this list
      if (nsnull == aLastChild) {
        NS_ASSERTION(nsnull == mFirstChild, "bad aLastChild");
        mFirstChild = pulledFrame;
      }
      else {
        aLastChild->SetNextSibling(pulledFrame);
      }
      pulledFrame->SetParent(aParent);
    }
  }
#ifdef DEBUG
  CheckForLoops();
#endif
  return pulledFrame;
}

nsIFrame*
nsFrameList::LastChild() const
{
  nsIFrame* frame = mFirstChild;
  if (!frame) {
    return nsnull;
  }

  nsIFrame* next = frame->GetNextSibling();
  while (next) {
    frame = next;
    next = frame->GetNextSibling();
  }
  return frame;
}

nsIFrame*
nsFrameList::FrameAt(PRInt32 aIndex) const
{
  NS_PRECONDITION(aIndex >= 0, "invalid arg");
  if (aIndex < 0) return nsnull;
  nsIFrame* frame = mFirstChild;
  while ((aIndex-- > 0) && frame) {
    frame = frame->GetNextSibling();
  }
  return frame;
}

PRBool
nsFrameList::ContainsFrame(const nsIFrame* aFrame) const
{
  NS_PRECONDITION(nsnull != aFrame, "null ptr");
  nsIFrame* frame = mFirstChild;
  while (frame) {
    if (frame == aFrame) {
      return PR_TRUE;
    }
    frame = frame->GetNextSibling();
  }
  return PR_FALSE;
}

PRInt32
nsFrameList::GetLength() const
{
  PRInt32 count = 0;
  nsIFrame* frame = mFirstChild;
  while (frame) {
    count++;
    frame = frame->GetNextSibling();
  }
  return count;
}

static int PR_CALLBACK CompareByContentOrder(const void* aF1, const void* aF2,
                                             void* aDummy)
{
  const nsIFrame* f1 = NS_STATIC_CAST(const nsIFrame*, aF1);
  const nsIFrame* f2 = NS_STATIC_CAST(const nsIFrame*, aF2);
  if (f1->GetContent() != f2->GetContent()) {
    return nsLayoutUtils::CompareTreePosition(f1->GetContent(), f2->GetContent());
  }

  if (f1 == f2) {
    return 0;
  }

  const nsIFrame* f;
  for (f = f2; f; f = f->GetPrevInFlow()) {
    if (f == f1) {
      // f1 comes before f2 in the flow
      return -1;
    }
  }
  for (f = f1; f; f = f->GetPrevInFlow()) {
    if (f == f2) {
      // f1 comes after f2 in the flow
      return 1;
    }
  }

  NS_ASSERTION(PR_FALSE, "Frames for same content but not in relative flow order");
  return 0;
}

void
nsFrameList::SortByContentOrder()
{
  if (!mFirstChild)
    return;

  nsAutoVoidArray array;
  nsIFrame* f;
  for (f = mFirstChild; f; f = f->GetNextSibling()) {
    array.AppendElement(f);
  }
  array.Sort(CompareByContentOrder, nsnull);
  f = mFirstChild = NS_STATIC_CAST(nsIFrame*, array.FastElementAt(0));
  for (PRInt32 i = 1; i < array.Count(); ++i) {
    nsIFrame* ff = NS_STATIC_CAST(nsIFrame*, array.FastElementAt(i));
    f->SetNextSibling(ff);
    f = ff;
  }
  f->SetNextSibling(nsnull);
}

nsIFrame*
nsFrameList::GetPrevSiblingFor(nsIFrame* aFrame) const
{
  NS_PRECONDITION(nsnull != aFrame, "null ptr");
  if (aFrame == mFirstChild) {
    return nsnull;
  }
  nsIFrame* frame = mFirstChild;
  while (frame) {
    nsIFrame* next = frame->GetNextSibling();
    if (next == aFrame) {
      break;
    }
    frame = next;
  }
  return frame;
}

void
nsFrameList::VerifyParent(nsIFrame* aParent) const
{
#ifdef NS_DEBUG
  for (nsIFrame* frame = mFirstChild; frame;
       frame = frame->GetNextSibling()) {
    NS_ASSERTION(frame->GetParent() == aParent, "bad parent");
  }
#endif
}

#ifdef NS_DEBUG
void
nsFrameList::List(FILE* out) const
{
  fputs("<\n", out);
  for (nsIFrame* frame = mFirstChild; frame;
       frame = frame->GetNextSibling()) {
    nsIFrameDebug*  frameDebug;
    if (NS_SUCCEEDED(frame->QueryInterface(NS_GET_IID(nsIFrameDebug), (void**)&frameDebug))) {
      frameDebug->List(out, 1);
    }
  }
  fputs(">\n", out);
}
#endif

#ifdef IBMBIDI
nsIFrame*
nsFrameList::GetPrevVisualFor(nsIFrame* aFrame) const
{
  NS_PRECONDITION(nsnull != aFrame, "null ptr");
  nsILineIterator* iter;

  if (aFrame->GetType() == nsLayoutAtoms::blockFrame)
    return GetPrevSiblingFor(aFrame);

  nsIFrame* frame;
  nsIFrame* furthestFrame = nsnull;

  frame = mFirstChild;

  nsIFrame* blockFrame = aFrame->GetParent();
  if (!blockFrame)
    return GetPrevSiblingFor(aFrame);

  PRUint8 direction = blockFrame->GetStyleVisibility()->mDirection;

  nsresult result = blockFrame->QueryInterface(NS_GET_IID(nsILineIterator), (void**)&iter);
  if (NS_FAILED(result) || !iter) { // If the parent is not a block frame, just check all the siblings

    nsFrameOrigin maxOrig(0, direction == NS_STYLE_DIRECTION_LTR ? XCOORD_MIN : XCOORD_MAX, direction);
    nsFrameOrigin limOrig(0, aFrame->GetRect().x, direction);
    
    while (frame) {
      nsFrameOrigin testOrig(0, frame->GetRect().x, direction);
      if (testOrig > maxOrig && testOrig < limOrig) { // we are looking for the highest value less than the current one
        maxOrig = testOrig;
        furthestFrame = frame;
      }
      frame = frame->GetNextSibling();
    }
    return furthestFrame;

  }

  // Otherwise use the LineIterator to check the siblings on this line and the previous line
  if (!blockFrame || !iter)
    return nsnull;

  nsFrameOrigin maxOrig(LINE_MIN, direction == NS_STYLE_DIRECTION_LTR ? XCOORD_MIN : XCOORD_MAX, direction);
  PRInt32 testLine, thisLine;

  result = iter->FindLineContaining(aFrame, &thisLine);
  if (NS_FAILED(result) || thisLine < 0)
    return nsnull;

  nsFrameOrigin limOrig(thisLine, aFrame->GetRect().x, direction);

  while (frame) {
    if (NS_SUCCEEDED(iter->FindLineContaining(frame, &testLine))
        && testLine >= 0
        && (testLine == thisLine || testLine == thisLine - 1)) {
      nsFrameOrigin testOrig(testLine, frame->GetRect().x, direction);
      if (testOrig > maxOrig && testOrig < limOrig) { // we are looking for the highest value less than the current one
        maxOrig = testOrig;
        furthestFrame = frame;
      }
    }
    frame = frame->GetNextSibling();
  }
  return furthestFrame;
}

nsIFrame*
nsFrameList::GetNextVisualFor(nsIFrame* aFrame) const
{
  NS_PRECONDITION(nsnull != aFrame, "null ptr");
  nsILineIterator* iter;

  if (aFrame->GetType() == nsLayoutAtoms::blockFrame) {
    return aFrame->GetNextSibling();
  }

  nsIFrame* frame;
  nsIFrame* nearestFrame = nsnull;

  frame = mFirstChild;

  nsIFrame* blockFrame = aFrame->GetParent();
  if (!blockFrame)
    return GetPrevSiblingFor(aFrame);

  PRUint8 direction = blockFrame->GetStyleVisibility()->mDirection;
  
  nsresult result = blockFrame->QueryInterface(NS_GET_IID(nsILineIterator), (void**)&iter);
  if (NS_FAILED(result) || !iter) { // If the parent is not a block frame, just check all the siblings

    nsFrameOrigin minOrig(0, direction == NS_STYLE_DIRECTION_LTR ? XCOORD_MAX : XCOORD_MIN, direction);
    nsFrameOrigin limOrig(0, aFrame->GetRect().x, direction);
    while (frame) {
      nsFrameOrigin testOrig(0, frame->GetRect().x, direction);
      if (testOrig < minOrig && testOrig > limOrig) { // we are looking for the lowest value greater than the current one
        minOrig = testOrig;
        nearestFrame = frame;
      }
      frame = frame->GetNextSibling();
    }
    return nearestFrame;
  }

  // Otherwise use the LineIterator to check the siblings on this line and the previous line
  if (!blockFrame || !iter)
    return nsnull;

  nsFrameOrigin minOrig(MY_LINE_MAX, direction == NS_STYLE_DIRECTION_LTR ? XCOORD_MAX : XCOORD_MIN, direction);
  PRInt32 testLine, thisLine;

  result = iter->FindLineContaining(aFrame, &thisLine);
  if (NS_FAILED(result) || thisLine < 0)
    return nsnull;

  nsFrameOrigin limOrig(thisLine, aFrame->GetRect().x, direction);

  while (frame) {
    if (NS_SUCCEEDED(iter->FindLineContaining(frame, &testLine))
        && testLine >= 0
        && (testLine == thisLine || testLine == thisLine + 1)) {
      nsFrameOrigin testOrig(testLine, frame->GetRect().x, direction);
      if (testOrig < minOrig && testOrig > limOrig) { // we are looking for the lowest value greater than the current one
        minOrig = testOrig;
        nearestFrame = frame;
      }
    }
    frame = frame->GetNextSibling();
  }
  return nearestFrame;
}
#endif

#ifdef DEBUG
void
nsFrameList::CheckForLoops()
{
  if (!mFirstChild) {
    return;
  }
  
  // Simple algorithm to find a loop in a linked list -- advance pointers
  // through it at speeds of 1 and 2, and if they ever get to be equal bail
  nsIFrame *first = mFirstChild, *second = mFirstChild;
  do {
    first = first->GetNextSibling();
    second = second->GetNextSibling();
    if (!second) {
      break;
    }
    second = second->GetNextSibling();
    if (first == second) {
      // Loop detected!  Since second advances faster, they can't both be null;
      // we would have broken out of the loop long ago.
      NS_ERROR("loop in frame list.  This will probably hang soon.");
      break;
    }                           
  } while (first && second);
}
#endif
