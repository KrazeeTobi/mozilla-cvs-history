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
 * Contributor(s):
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
#include "nsPageFrame.h"
#include "nsHTMLParts.h"
#include "nsIContent.h"
#include "nsIPresContext.h"
#include "nsIStyleContext.h"
#include "nsIRenderingContext.h"
#include "nsHTMLAtoms.h"
#include "nsHTMLIIDs.h"
#include "nsLayoutAtoms.h"
#include "nsIStyleSet.h"
#include "nsIPresShell.h"
#include "nsIDeviceContext.h"
#include "nsReadableUtils.h"
#include "nsIPrintPreviewContext.h"
#include "nsIPrintContext.h"
#include "nsPageContentFrame.h"

#include "nsIView.h" // view flags for clipping
#include "nsCSSRendering.h"

#include "nsHTMLContainerFrame.h" // view creation

#include "nsSimplePageSequence.h" // for nsSharedPageData
#include "nsIRegion.h"
#include "nsIViewManager.h"

// for page number localization formatting
#include "nsTextFormatter.h"

// Temporary
#include "nsIFontMetrics.h"

// Print Options
#include "nsIPrintSettings.h"
#include "nsGfxCIID.h"
#include "nsIServiceManager.h"
static NS_DEFINE_CID(kPrintOptionsCID, NS_PRINTOPTIONS_CID);

// Widget Creation
#include "nsIWidget.h"
#include "nsWidgetsCID.h"
static NS_DEFINE_CID(kCChildCID, NS_CHILD_CID);

#if defined(DEBUG_rods) || defined(DEBUG_dcone)
//#define DEBUG_PRINTING
#endif

#ifdef DEBUG_PRINTING
#define PRINT_DEBUG_MSG1(_msg1) fprintf(mDebugFD, (_msg1))
#define PRINT_DEBUG_MSG2(_msg1, _msg2) fprintf(mDebugFD, (_msg1), (_msg2))
#define PRINT_DEBUG_MSG3(_msg1, _msg2, _msg3) fprintf(mDebugFD, (_msg1), (_msg2), (_msg3))
#define PRINT_DEBUG_MSG4(_msg1, _msg2, _msg3, _msg4) fprintf(mDebugFD, (_msg1), (_msg2), (_msg3), (_msg4))
#define PRINT_DEBUG_MSG5(_msg1, _msg2, _msg3, _msg4, _msg5) fprintf(mDebugFD, (_msg1), (_msg2), (_msg3), (_msg4), (_msg5))
#else //--------------
#define PRINT_DEBUG_MSG1(_msg) 
#define PRINT_DEBUG_MSG2(_msg1, _msg2) 
#define PRINT_DEBUG_MSG3(_msg1, _msg2, _msg3) 
#define PRINT_DEBUG_MSG4(_msg1, _msg2, _msg3, _msg4) 
#define PRINT_DEBUG_MSG5(_msg1, _msg2, _msg3, _msg4, _msg5) 
#endif


nsresult
NS_NewPageFrame(nsIPresShell* aPresShell, nsIFrame** aNewFrame)
{
  NS_PRECONDITION(aNewFrame, "null OUT ptr");

  nsPageFrame* it = new (aPresShell) nsPageFrame;
  if (nsnull == it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  *aNewFrame = it;
  return NS_OK;
}

nsPageFrame::nsPageFrame() :
  mSupressHF(PR_FALSE),
  mClipRect(-1, -1, -1, -1)
{
#ifdef NS_DEBUG
  mDebugFD = stdout;
#endif
}

nsPageFrame::~nsPageFrame()
{
}


NS_IMETHODIMP
nsPageFrame::SetInitialChildList(nsIPresContext* aPresContext,
                                      nsIAtom*        aListName,
                                      nsIFrame*       aChildList)
{
  nsIView * view;
  aChildList->GetView(aPresContext, &view);
  if (view != nsnull) {
    nscoord dx, dy;
    nsIWidget* widget;
    view->GetOffsetFromWidget(&dx, &dy, widget);
    nsCOMPtr<nsIPrintPreviewContext> ppContext = do_QueryInterface(aPresContext);
    if (ppContext && widget != nsnull) {
      view->CreateWidget(kCChildCID);  
    }
  }

  return nsContainerFrame::SetInitialChildList(aPresContext, aListName, aChildList);
}

NS_IMETHODIMP nsPageFrame::Reflow(nsIPresContext*          aPresContext,
                                  nsHTMLReflowMetrics&     aDesiredSize,
                                  const nsHTMLReflowState& aReflowState,
                                  nsReflowStatus&          aStatus)
{
  DO_GLOBAL_REFLOW_COUNT("nsPageFrame", aReflowState.reason);
  DISPLAY_REFLOW(aPresContext, this, aReflowState, aDesiredSize, aStatus);
  aStatus = NS_FRAME_COMPLETE;  // initialize out parameter

  if (eReflowReason_Incremental != aReflowState.reason) {
    // Do we have any children?
    // XXX We should use the overflow list instead...
    nsIFrame*           firstFrame  = mFrames.FirstChild();
    nsPageContentFrame* contentPage = NS_STATIC_CAST(nsPageContentFrame*, firstFrame);
    NS_ASSERTION(contentPage, "There should always be a content page");

#ifdef NS_DEBUG
    nsCOMPtr<nsIAtom> type;
    firstFrame->GetFrameType(getter_AddRefs(type));
    NS_ASSERTION(type.get() == nsLayoutAtoms::pageContentFrame, "This frame isn't a pageContentFrame");
#endif

    if (contentPage && contentPage->mFrames.IsEmpty() && nsnull != mPrevInFlow) {
      nsPageFrame*        prevPage        = NS_STATIC_CAST(nsPageFrame*, mPrevInFlow);
      nsPageContentFrame* prevContentPage = NS_STATIC_CAST(nsPageContentFrame*, prevPage->mFrames.FirstChild());
      nsIFrame*           prevLastChild   = prevContentPage->mFrames.LastChild();

      // Create a continuing child of the previous page's last child
      nsIPresShell* presShell;
      nsIStyleSet*  styleSet;
      nsIFrame*     newFrame;

      aPresContext->GetShell(&presShell);
      presShell->GetStyleSet(&styleSet);
      NS_RELEASE(presShell);
      styleSet->CreateContinuingFrame(aPresContext, prevLastChild, this, &newFrame);
      NS_RELEASE(styleSet);
      contentPage->mFrames.SetFrames(newFrame);
    }

    // Resize our frame allowing it only to be as big as we are
    // XXX Pay attention to the page's border and padding...
    if (mFrames.NotEmpty()) {
      nsIFrame* frame = mFrames.FirstChild();
      // When availableHeight is NS_UNCONSTRAINEDSIZE it means we are reflowing a single page
      // to print selection. So this means we want to use NS_UNCONSTRAINEDSIZE without altering it
      nscoord avHeight;
      if (aReflowState.availableHeight == NS_UNCONSTRAINEDSIZE) {
        avHeight = NS_UNCONSTRAINEDSIZE;
      } else {
        avHeight = mPD->mReflowRect.height - mPD->mReflowMargin.top - mPD->mReflowMargin.bottom;
      }
      nsSize  maxSize(mPD->mReflowRect.width - mPD->mReflowMargin.right - mPD->mReflowMargin.left, 
                      avHeight);
      nsHTMLReflowState kidReflowState(aPresContext, aReflowState, frame, maxSize);
      kidReflowState.mFlags.mIsTopOfPage = PR_TRUE;

      // calc location of frame
      nscoord xc = mPD->mReflowMargin.left + mPD->mDeadSpaceMargin.left + mPD->mExtraMargin.left;
      nscoord yc = mPD->mReflowMargin.top + mPD->mDeadSpaceMargin.top + mPD->mExtraMargin.top;

      // Get the child's desired size
      ReflowChild(frame, aPresContext, aDesiredSize, kidReflowState, xc, yc, 0, aStatus);


      // Place and size the child
      FinishReflowChild(frame, aPresContext, &kidReflowState, aDesiredSize, xc, yc, 0);

      // Make sure the child is at least as tall as our max size (the containing window)
      if (aDesiredSize.height < aReflowState.availableHeight &&
          aReflowState.availableHeight != NS_UNCONSTRAINEDSIZE) {
        aDesiredSize.height = aReflowState.availableHeight;
      }

      nsIView * view;
      frame->GetView(aPresContext, &view);
      if (view) {
        nsCOMPtr<nsIViewManager> vm;
        view->GetViewManager(*getter_AddRefs(vm));
        nsCOMPtr<nsIRegion> region = dont_AddRef(nsSimplePageSequenceFrame::CreateRegion());
        region->SetTo(0,0, aDesiredSize.width, aDesiredSize.height);
        vm->SetViewChildClipRegion(view, region);
      }

#ifdef NS_DEBUG
      // Is the frame complete?
      if (NS_FRAME_IS_COMPLETE(aStatus)) {
        nsIFrame* childNextInFlow;

        frame->GetNextInFlow(&childNextInFlow);
        NS_ASSERTION(nsnull == childNextInFlow, "bad child flow list");
      }
#endif
    }
    PRINT_DEBUG_MSG2("PageFrame::Reflow %p ", this);
    PRINT_DEBUG_MSG5("[%d,%d][%d,%d]\n", aDesiredSize.width, aDesiredSize.height, aReflowState.availableWidth, aReflowState.availableHeight);

    // Return our desired size
    aDesiredSize.width = aReflowState.availableWidth;
    if (aReflowState.availableHeight != NS_UNCONSTRAINEDSIZE) {
      aDesiredSize.height = aReflowState.availableHeight;
    }
  }
  PRINT_DEBUG_MSG2("PageFrame::Reflow %p ", this);
  PRINT_DEBUG_MSG3("[%d,%d]\n", aReflowState.availableWidth, aReflowState.availableHeight);

  return NS_OK;
}

void nsPageFrame::SetClipRect(nsRect* aClipRect) 
{
  mClipRect = *aClipRect; 
  nsIFrame*           firstFrame  = mFrames.FirstChild();
  nsPageContentFrame* contentPage = NS_STATIC_CAST(nsPageContentFrame*, firstFrame);
  NS_ASSERTION(contentPage, "There should always be a content page");
  contentPage->SetClipRect(aClipRect);
}


NS_IMETHODIMP
nsPageFrame::GetFrameType(nsIAtom** aType) const
{
  NS_PRECONDITION(nsnull != aType, "null OUT parameter pointer");
  *aType = nsLayoutAtoms::pageFrame; 
  NS_ADDREF(*aType);
  return NS_OK;
}

#ifdef DEBUG
NS_IMETHODIMP
nsPageFrame::GetFrameName(nsAString& aResult) const
{
  return MakeFrameName(NS_LITERAL_STRING("Page"), aResult);
}
#endif

NS_IMETHODIMP
nsPageFrame::IsPercentageBase(PRBool& aBase) const
{
  aBase = PR_TRUE;
  return NS_OK;
}

//------------------------------------------------------------------------------
// helper function for converting from char * to unichar
static PRUnichar *
GetUStr(const char * aCStr)
{
  return ToNewUnicode(nsDependentCString(aCStr));
}

// Remove fix below when string gets fixed
#define WORKAROUND_FOR_BUG_110335
// replace the &<code> with the value, but if the value is empty
// set the string to zero length
static void
SubstValueForCode(nsString& aStr, PRUnichar * aUKey, PRUnichar * aUStr)
{
#ifdef WORKAROUND_FOR_BUG_110335
  PRUnichar* uKeyStr = aUKey;

  // Check to make sure our subst code &<code> isn't in the data string
  // for example &T for title is in QB&T
  nsAutoString dataStr(aUStr);
  nsAutoString newKey(aUKey);
  PRBool fixingSubstr = dataStr.Find(newKey) > -1;
  if (fixingSubstr) {
    // well, the code is in the data str so make up a new code
    // but make sure it it isn't in either substs string or the data string
    char* code = "~!@#$%^*()_+=][}{`';:|?/.,:\"<>";
    PRInt32 codeInx = 0;
    PRInt32 codeLen = PRUint32(strlen(code));
    while ((dataStr.Find(newKey) > -1 || aStr.Find(newKey) > -1) && codeInx < codeLen) {
      newKey.SetCharAt((PRUnichar)code[codeInx++], 0);
    }

    // Check to see if we can do the substitution
    if (codeInx == codeLen) {
      aStr.SetLength(0);
      return; // bail we just can't do it
    }

    // Ok, we have the new code, so repplace the old code 
    // in the dest str with the new code
    nsAutoString oldKey(aUKey);
    aStr.ReplaceSubstring(oldKey, newKey);
    uKeyStr = ToNewUnicode(newKey);
  }

  nsAutoString str;
  str = aUStr;
  if (str.Length() == 0) {
    aStr.SetLength(0);
  } else {
    aStr.ReplaceSubstring(uKeyStr, aUStr);
  }

  // Free uKeyStr only if we fixed the string.
  if (fixingSubstr) {
    nsMemory::Free(uKeyStr);
  }
#else
  nsAutoString str;
  str = aUStr;
  if (str.Length() == 0) {
    aStr.SetLength(0);
  } else {
    aStr.ReplaceSubstring(aUKey, aUStr);
  }
#endif  // WORKAROUND_FOR_BUG_110335
}
// done with static helper functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void 
nsPageFrame::ProcessSpecialCodes(const nsString& aStr, nsString& aNewStr)
{

  aNewStr = aStr;

  // Search to see if the &D code is in the string 
  // then subst in the current date/time
  PRUnichar * kDate = GetUStr("&D");
  if (kDate != nsnull) {
    if (aStr.Find(kDate) > -1) {
      if (mPD->mDateTimeStr != nsnull) {
        aNewStr.ReplaceSubstring(kDate, mPD->mDateTimeStr);
      } else {
        aNewStr.ReplaceSubstring(kDate, NS_LITERAL_STRING("").get());
      }
      nsMemory::Free(kDate);
      return;
    }
    nsMemory::Free(kDate);
  }

  // NOTE: Must search for &PT before searching for &P
  //
  // Search to see if the "page number and page" total code are in the string
  // and replace the page number and page total code with the actual values
  PRUnichar * kPage = GetUStr("&PT");
  if (kPage != nsnull) {
    if (aStr.Find(kPage) > -1) {
      PRUnichar * uStr = nsTextFormatter::smprintf(mPD->mPageNumAndTotalsFormat, mPageNum, mTotNumPages);
      aNewStr.ReplaceSubstring(kPage, uStr);
      nsMemory::Free(uStr);
      nsMemory::Free(kPage);
      return;
    }
    nsMemory::Free(kPage);
  }

  // Search to see if the page number code is in the string
  // and replace the page number code with the actual values
  kPage = GetUStr("&P");
  if (kPage != nsnull) {
    if (aStr.Find(kPage) > -1) {
      PRUnichar * uStr = nsTextFormatter::smprintf(mPD->mPageNumFormat, mPageNum);
      aNewStr.ReplaceSubstring(kPage, uStr);
      nsMemory::Free(uStr);
      nsMemory::Free(kPage);
      return;
    }
    nsMemory::Free(kPage);
  }

  PRUnichar * kTitle = GetUStr("&T");
  if (kTitle != nsnull) {
    if (aStr.Find(kTitle) > -1) {
      PRUnichar * uTitle;
      mPD->mPrintOptions->GetTitle(&uTitle);   // creates memory
      SubstValueForCode(aNewStr, kTitle, uTitle);
      nsMemory::Free(uTitle);
      nsMemory::Free(kTitle);
      return;
    }
    nsMemory::Free(kTitle);
  }

  PRUnichar * kDocURL = GetUStr("&U");
  if (kDocURL != nsnull) {
    if (aStr.Find(kDocURL) > -1) {
      PRUnichar * uDocURL;
      mPD->mPrintOptions->GetDocURL(&uDocURL);   // creates memory
      SubstValueForCode(aNewStr, kDocURL, uDocURL);
      nsMemory::Free(uDocURL);
      nsMemory::Free(kDocURL);
      return;
    }
    nsMemory::Free(kDocURL);
  }
}


//------------------------------------------------------------------------------
nscoord nsPageFrame::GetXPosition(nsIRenderingContext& aRenderingContext, 
                                  const nsRect&        aRect, 
                                  PRInt32              aJust,
                                  const nsString&      aStr)
{
  PRInt32 width;
  aRenderingContext.GetWidth(aStr, width);

  nscoord x = aRect.x;
  switch (aJust) {
    case nsIPrintSettings::kJustLeft:
      x += mPD->mExtraMargin.left + mPD->mEdgePaperMargin.left;
      break;

    case nsIPrintSettings::kJustCenter:
      x += (aRect.width - width) / 2;
      break;

    case nsIPrintSettings::kJustRight:
      x += aRect.width - width - mPD->mExtraMargin.right - mPD->mEdgePaperMargin.right;
      break;
  } // switch

  NS_ASSERTION(x >= 0, "x can't be less than zero");
  x = PR_MAX(x, 0);
  return x;
}

//------------------------------------------------------------------------------
// Draw a Header or footer text lrft,right or center justified
// @parm aRenderingContext - rendering content ot draw into
// @parm aHeaderFooter - indicates whether it is a header or footer
// @parm aJust - indicates the justification of the text
// @parm aStr - The string to be drawn
// @parm aRect - the rect of the page
// @parm aHeight - the height of the text
// @parm aUseHalfThePage - indicates whether the text should limited to  the width
//                         of the entire page or just half the page
void
nsPageFrame::DrawHeaderFooter(nsIRenderingContext& aRenderingContext,
                              nsIFrame *           aFrame,
                              nsHeaderFooterEnum   aHeaderFooter,
                              PRInt32              aJust,
                              const nsString&      aStr1,
                              const nsString&      aStr2,
                              const nsString&      aStr3,
                              const nsRect&        aRect,
                              nscoord              aAscent,
                              nscoord              aHeight)
{
  PRInt32 numStrs = 0;
  if (!aStr1.IsEmpty()) numStrs++;
  if (!aStr2.IsEmpty()) numStrs++;
  if (!aStr3.IsEmpty()) numStrs++;

  if (numStrs == 0) return;
  nscoord strSpace = aRect.width / numStrs;

  if (!aStr1.IsEmpty()) {
    DrawHeaderFooter(aRenderingContext, aFrame, aHeaderFooter, nsIPrintSettings::kJustLeft, aStr1, aRect, aAscent, aHeight, strSpace);
  }
  if (!aStr2.IsEmpty()) {
    DrawHeaderFooter(aRenderingContext, aFrame, aHeaderFooter, nsIPrintSettings::kJustCenter, aStr2, aRect, aAscent, aHeight, strSpace);
  }
  if (!aStr3.IsEmpty()) {
    DrawHeaderFooter(aRenderingContext, aFrame, aHeaderFooter, nsIPrintSettings::kJustRight, aStr3, aRect, aAscent, aHeight, strSpace);
  }
}

//------------------------------------------------------------------------------
// Draw a Header or footer text lrft,right or center justified
// @parm aRenderingContext - rendering content ot draw into
// @parm aHeaderFooter - indicates whether it is a header or footer
// @parm aJust - indicates the justification of the text
// @parm aStr - The string to be drawn
// @parm aRect - the rect of the page
// @parm aHeight - the height of the text
// @parm aWidth - available width for any one of the strings
void
nsPageFrame::DrawHeaderFooter(nsIRenderingContext& aRenderingContext,
                              nsIFrame *           aFrame,
                              nsHeaderFooterEnum   aHeaderFooter,
                              PRInt32              aJust,
                              const nsString&      aStr,
                              const nsRect&        aRect,
                              nscoord              aAscent,
                              nscoord              aHeight,
                              nscoord              aWidth)
{

  nscoord contentWidth = aWidth - (mPD->mEdgePaperMargin.left + mPD->mEdgePaperMargin.right);

  // first make sure we have a vaild string and that the height of the
  // text will fit in the margin
  if (aStr.Length() > 0 && 
      ((aHeaderFooter == eHeader && aHeight < mMargin.top) ||
       (aHeaderFooter == eFooter && aHeight < mMargin.bottom))) {
    // measure the width of the text
    nsAutoString str;
    ProcessSpecialCodes(aStr, str);

    PRInt32 width;
    aRenderingContext.GetWidth(str, width);
    PRBool addEllipse = PR_FALSE;

    // trim the text and add the elipses if it won't fit
    while (width >= contentWidth && str.Length() > 1) {
      str.SetLength(str.Length()-1);
      aRenderingContext.GetWidth(str, width);
      addEllipse = PR_TRUE;
    }
    if (addEllipse && str.Length() > 3) {
      str.SetLength(str.Length()-3);
      str.Append(NS_LITERAL_STRING("..."));
      aRenderingContext.GetWidth(str, width);
    }

    // cacl the x and y positions of the text
    nsRect rect(aRect);
    nscoord x = GetXPosition(aRenderingContext, rect, aJust, str);
    nscoord y;
    if (aHeaderFooter == eHeader) {
      y = rect.y + mPD->mExtraMargin.top + mPD->mEdgePaperMargin.top;
    } else {
      y = rect.y + rect.height - aHeight - mPD->mExtraMargin.bottom - mPD->mEdgePaperMargin.bottom;
    }

    // set up new clip and draw the text
    PRBool clipEmpty;
    aRenderingContext.PushState();
    aRenderingContext.SetColor(NS_RGB(0,0,0));
    aRenderingContext.SetClipRect(rect, nsClipCombine_kReplace, clipEmpty);
    aRenderingContext.DrawString(str, x, y + aAscent);
    aRenderingContext.PopState(clipEmpty);
#ifdef DEBUG_PRINTING
    PRINT_DEBUG_MSG2("Page: %p", this);
    const char * s = NS_ConvertUCS2toUTF8(str).get();
    if (s) {
      PRINT_DEBUG_MSG2(" [%s]", s);
    }
    char justStr[64];
    switch (aJust) {
      case nsIPrintSettings::kJustLeft:strcpy(justStr, "Left");break;
      case nsIPrintSettings::kJustCenter:strcpy(justStr, "Center");break;
      case nsIPrintSettings::kJustRight:strcpy(justStr, "Right");break;
    } // switch
    PRINT_DEBUG_MSG2(" HF: %s ", aHeaderFooter==eHeader?"Header":"Footer");
    PRINT_DEBUG_MSG2(" JST: %s ", justStr);
    PRINT_DEBUG_MSG3(" x,y: %d,%d", x, y);
    PRINT_DEBUG_MSG2(" Hgt: %d \n", aHeight);
#endif
  }
}

//------------------------------------------------------------------------------
NS_IMETHODIMP
nsPageFrame::Paint(nsIPresContext*      aPresContext,
                   nsIRenderingContext& aRenderingContext,
                   const nsRect&        aDirtyRect,
                   nsFramePaintLayer    aWhichLayer,
                   PRUint32             aFlags)
{
  aRenderingContext.PushState();
  aRenderingContext.SetColor(NS_RGB(255,255,255));

  nsRect rect;
  PRBool clipEmpty;
  PRBool specialClipIsSet = mClipRect.width != -1 || mClipRect.height != -1;

  if (specialClipIsSet) {
#ifdef DEBUG_PRINTING
    if (NS_FRAME_PAINT_LAYER_FOREGROUND == aWhichLayer) {
      printf("*** ClipRect: %5d,%5d,%5d,%5d\n", mClipRect.x, mClipRect.y, mClipRect.width, mClipRect.height);
    }
#endif
    aRenderingContext.SetClipRect(mClipRect, nsClipCombine_kReplace, clipEmpty);
    rect = mClipRect;
  } else {
    rect = mRect;
  }

  if (NS_FRAME_PAINT_LAYER_BACKGROUND == aWhichLayer) {

    nsCOMPtr<nsIPrintPreviewContext> ppContext = do_QueryInterface(aPresContext);
    if (ppContext) {
      // fill page with White
      aRenderingContext.SetColor(NS_RGB(255,255,255));
      rect.x = 0;
      rect.y = 0;
      rect.width  -= mPD->mShadowSize.width;
      rect.height -= mPD->mShadowSize.height;
      aRenderingContext.FillRect(rect);
      // draw line around outside of page
      aRenderingContext.SetColor(NS_RGB(0,0,0));
      aRenderingContext.DrawRect(rect);

      if (mPD->mShadowSize.width > 0 && mPD->mShadowSize.height > 0) {
        aRenderingContext.SetColor(NS_RGB(51,51,51));
        nsRect r(0,0, mRect.width, mRect.height);
        nsRect shadowRect;
        shadowRect.x = r.x + r.width - mPD->mShadowSize.width;
        shadowRect.y = r.y + mPD->mShadowSize.height;
        shadowRect.width  = mPD->mShadowSize.width;
        shadowRect.height = r.height - mPD->mShadowSize.height;
        aRenderingContext.FillRect(shadowRect);

        shadowRect.x = r.x + mPD->mShadowSize.width;
        shadowRect.y = r.y + r.height - mPD->mShadowSize.height;
        shadowRect.width  = r.width - mPD->mShadowSize.width;
        shadowRect.height = mPD->mShadowSize.height;
        aRenderingContext.FillRect(shadowRect);
      }
    }
  }

  DrawBackground(aPresContext,aRenderingContext,aDirtyRect) ;

  nsresult rv = nsContainerFrame::Paint(aPresContext, aRenderingContext, aDirtyRect, aWhichLayer);

#if defined(DEBUG_rods) || defined(DEBUG_dcone)
  if (NS_FRAME_PAINT_LAYER_FOREGROUND == aWhichLayer) {
    fprintf(mDebugFD, "PF::Paint    -> %p  SupHF: %s  Rect: [%5d,%5d,%5d,%5d] SC:%s\n", this, 
            mSupressHF?"Yes":"No", mRect.x, mRect.y, mRect.width, mRect.height, specialClipIsSet?"Yes":"No");
    fprintf(stdout, "PF::Paint    -> %p  SupHF: %s  Rect: [%5d,%5d,%5d,%5d] SC:%s\n", this, 
            mSupressHF?"Yes":"No", mRect.x, mRect.y, mRect.width, mRect.height, specialClipIsSet?"Yes":"No");
  }
#endif

  if (NS_FRAME_PAINT_LAYER_FOREGROUND == aWhichLayer && !mSupressHF) {
    // For PrintPreview the 
    if (!mPD->mPrintSettings) {
      nsCOMPtr<nsIPrintPreviewContext> ppContext = do_QueryInterface(aPresContext);
      if (ppContext) {
        ppContext->GetPrintSettings(getter_AddRefs(mPD->mPrintSettings));
      }
    }
    NS_ASSERTION(mPD->mPrintSettings, "Must have a good PrintSettings here!");

    // get the current margin
    mPD->mPrintSettings->GetMarginInTwips(mMargin);

    rect.SetRect(0, 0, mRect.width - mPD->mShadowSize.width, mRect.height - mPD->mShadowSize.height);

    aRenderingContext.SetFont(*mPD->mHeadFootFont);
    aRenderingContext.SetColor(NS_RGB(0,0,0));

    // Get the FontMetrics to determine width.height of strings
    nsCOMPtr<nsIDeviceContext> deviceContext;
    aPresContext->GetDeviceContext(getter_AddRefs(deviceContext));
    NS_ASSERTION(deviceContext, "Couldn't get the device context"); 
    nsCOMPtr<nsIFontMetrics> fontMet;
    deviceContext->GetMetricsFor(*mPD->mHeadFootFont, *getter_AddRefs(fontMet));
    nscoord ascent = 0;
    nscoord visibleHeight = 0;
    if (fontMet) {
      fontMet->GetHeight(visibleHeight);
      fontMet->GetMaxAscent(ascent);
    }

    // print document headers and footers
    PRUnichar * headers[3];
    mPD->mPrintSettings->GetHeaderStrLeft(&headers[0]);   // creates memory
    mPD->mPrintSettings->GetHeaderStrCenter(&headers[1]); // creates memory
    mPD->mPrintSettings->GetHeaderStrRight(&headers[2]);  // creates memory
    DrawHeaderFooter(aRenderingContext, this, eHeader, nsIPrintSettings::kJustLeft, 
                     nsAutoString(headers[0]), nsAutoString(headers[1]), nsAutoString(headers[2]), 
                     rect, ascent, visibleHeight);
    PRInt32 i;
    for (i=0;i<3;i++) nsMemory::Free(headers[i]);

    PRUnichar * footers[3];
    mPD->mPrintSettings->GetFooterStrLeft(&footers[0]);   // creates memory
    mPD->mPrintSettings->GetFooterStrCenter(&footers[1]); // creates memory
    mPD->mPrintSettings->GetFooterStrRight(&footers[2]);  // creates memory
    DrawHeaderFooter(aRenderingContext, this, eFooter, nsIPrintSettings::kJustRight, 
                     nsAutoString(footers[0]), nsAutoString(footers[1]), nsAutoString(footers[2]), 
                     rect, ascent, visibleHeight);
    for (i=0;i<3;i++) nsMemory::Free(footers[i]);

  }

  aRenderingContext.PopState(clipEmpty);

  return rv;
}

//------------------------------------------------------------------------------
void
nsPageFrame::SetPageNumInfo(PRInt32 aPageNumber, PRInt32 aTotalPages) 
{ 
  mPageNum     = aPageNumber; 
  mTotNumPages = aTotalPages;
}


//------------------------------------------------------------------------------
void
nsPageFrame::DrawBackground(nsIPresContext*      aPresContext,
                            nsIRenderingContext& aRenderingContext,
                            const nsRect&        aDirtyRect) 
{
  nsSimplePageSequenceFrame* seqFrame = NS_STATIC_CAST(nsSimplePageSequenceFrame*, mParent);
  if (seqFrame != nsnull) {
    nsRect rect = mPD->mReflowRect;
    rect.Deflate(mPD->mReflowMargin);
    rect.Deflate(mPD->mExtraMargin);

    const nsStyleBorder* border = NS_STATIC_CAST(const nsStyleBorder*,
                             mStyleContext->GetStyleData(eStyleStruct_Border));

    nsCSSRendering::PaintBackground(aPresContext, aRenderingContext, this,
                                    aDirtyRect, rect, *border, 0, 0);      
  }
}

