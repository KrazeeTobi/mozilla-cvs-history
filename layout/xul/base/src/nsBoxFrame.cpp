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

//
// Eric Vaughan
// Netscape Communications
//
// See documentation in associated header file
//

#include "nsBoxFrame.h"
#include "nsIStyleContext.h"
#include "nsIPresContext.h"
#include "nsCOMPtr.h"
#include "nsHTMLIIDs.h"
#include "nsUnitConversion.h"
#include "nsINameSpaceManager.h"
#include "nsHTMLAtoms.h"
#include "nsXULAtoms.h"
#include "nsIReflowCommand.h"
#include "nsIContent.h"
#include "nsSpaceManager.h"
#include "nsHTMLParts.h"
#include "nsIViewManager.h"
#include "nsIPresShell.h"
#include "nsGenericHTMLElement.h"
#include "nsFrameNavigator.h"
#include "nsCSSRendering.h"
#include "nsIDOMMouseMotionListener.h"
#include "nsIDOMEventReceiver.h"
#include "nsIDOMUIEvent.h"
#include "nsIScrollableView.h"

#define CONSTANT float(0.0)
#define DEBUG_REFLOW 0
#define DEBUG_REDRAW 0
#define DEBUG_SPRING_SIZE 8
#define DEBUG_BORDER_SIZE 2
#define COIL_SIZE 8

/**
 * Only created when the box is in debug mode
 */
class nsBoxDebugInner: public nsIDOMMouseMotionListener
{
public:

  nsBoxDebugInner(nsBoxFrame* aThis)
  {
    mOuter = aThis;
    mRefCnt = 0;
  }
  virtual ~nsBoxDebugInner() {}

   NS_DECL_ISUPPORTS

public:
    nsCOMPtr<nsIStyleContext> mHorizontalDebugStyle;
    nsCOMPtr<nsIStyleContext> mVerticalDebugStyle;
    nsBoxFrame* mOuter;

    float mP2t;
    static nsIFrame* mDebugChild;
    void GetValue(const nsSize& a, const nsSize& b, char* value);
    void GetValue(float a, float b, char* value);
    void DrawSpring( nsIPresContext& aPresContext, nsIRenderingContext& aRenderingContext, float flex, nscoord x, nscoord y, nscoord size, nscoord springSize);
    void PaintSprings(nsIPresContext& aPresContext, nsIRenderingContext& aRenderingContext, const nsRect& aDirtyRect);
    void DrawLine(nsIRenderingContext& aRenderingContext, nscoord x1, nscoord y1, nscoord x2, nscoord y2);
    void FillRect(nsIRenderingContext& aRenderingContext, nscoord x, nscoord y, nscoord width, nscoord height);

    void AddListener();
    void RemoveListener();

    virtual nsresult MouseMove(nsIDOMEvent* aMouseEvent);
    virtual nsresult DragMove(nsIDOMEvent* aMouseEvent) { return NS_OK; }
    virtual nsresult HandleEvent(nsIDOMEvent* aEvent) { return NS_OK; }
};

/**
 * The boxes private implementation
 */
class nsBoxFrameInner
{
public:
  nsBoxFrameInner(nsBoxFrame* aThis)
  {
    mOuter = aThis;
    mDebugInner = nsnull;
  }

  ~nsBoxFrameInner()
  {
      if (mDebugInner) {
          mDebugInner->RemoveListener();
          NS_RELEASE(mDebugInner);
          mDebugInner = nsnull;
      }
  }

  void GetDebugInset(nsMargin& inset);
  void AdjustChildren(nsIPresContext& aPresContext, nsBoxFrame* aBox);
  void UpdatePseudoElements(nsIPresContext& aPresContext);

  // XXX for the moment we can only handle 100 children.
    // Should use a dynamic array.
    nsCOMPtr<nsISpaceManager> mSpaceManager; // We own this [OWNER].
    PRUint32 mFlags;
    nsBoxFrame* mOuter;
    nsBoxDebugInner* mDebugInner;

  //  PRBool mIsDebug;
};

nsIFrame* nsBoxDebugInner::mDebugChild = nsnull;

static NS_DEFINE_IID(kIDOMMouseMotionListenerIID, NS_IDOMMOUSEMOTIONLISTENER_IID);
static NS_DEFINE_IID(kScrollViewIID, NS_ISCROLLABLEVIEW_IID);

NS_IMPL_ISUPPORTS1(nsBoxDebugInner, nsIDOMMouseMotionListener)


nsresult
NS_NewBoxFrame ( nsIFrame** aNewFrame, PRUint32 aFlags )
{
  NS_PRECONDITION(aNewFrame, "null OUT ptr");
  if (nsnull == aNewFrame) {
    return NS_ERROR_NULL_POINTER;
  }
  nsBoxFrame* it = new nsBoxFrame(aFlags);
  if (nsnull == it)
    return NS_ERROR_OUT_OF_MEMORY;

  *aNewFrame = it;
  return NS_OK;
  
} // NS_NewBoxFrame

nsBoxFrame::nsBoxFrame(PRUint32 aFlags)
{
  mInner = new nsBoxFrameInner(this);

  // if not otherwise specified boxes by default are horizontal.
  mHorizontal = PR_TRUE;
  mInner->mFlags = aFlags;
}

nsBoxFrame::~nsBoxFrame()
{
  delete mInner;
}

/**
 * Initialize us. This is a good time to get the alignment of the box
 */
NS_IMETHODIMP
nsBoxFrame::Init(nsIPresContext&  aPresContext,
              nsIContent*      aContent,
              nsIFrame*        aParent,
              nsIStyleContext* aContext,
              nsIFrame*        aPrevInFlow)
{
  nsresult  rv = nsHTMLContainerFrame::Init(aPresContext, aContent, aParent, aContext, aPrevInFlow);

  // see if we are a vertical or horizontal box.
  nsString value;
  mContent->GetAttribute(kNameSpaceID_None, nsHTMLAtoms::align, value);
  if (value.EqualsIgnoreCase("vertical"))
    mHorizontal = PR_FALSE;
  else if (value.EqualsIgnoreCase("horizontal"))
    mHorizontal = PR_TRUE;

  nsSpaceManager* spaceManager = new nsSpaceManager(this);
  mInner->mSpaceManager = spaceManager;
  mInner->UpdatePseudoElements(aPresContext);

  return rv;
}

void
nsBoxFrame::GetInnerRect(nsRect& aInner)
{
    const nsStyleSpacing* spacing;

    nsresult rv = GetStyleData(eStyleStruct_Spacing,
                   (const nsStyleStruct*&) spacing);

    nsMargin border(0,0,0,0);
    spacing->GetBorderPadding(border);

    aInner = mRect;
    aInner.x = 0;
    aInner.y = 0;
    aInner.Deflate(border);

    border.SizeTo(0,0,0,0);
    mInner->GetDebugInset(border);
    aInner.Deflate(border);
}

/** 
 * Looks at the given frame and sees if its redefined preferred, min, or max sizes
 * if so it used those instead. Currently it gets its values from css
 */
void 
nsBoxFrame::GetRedefinedMinPrefMax(nsIPresContext&  aPresContext, nsIFrame* aFrame, nsCalculatedBoxInfo& aSize)
{
  // add in the css min, max, pref
    const nsStylePosition* position;
    nsresult rv = aFrame->GetStyleData(eStyleStruct_Position,
                  (const nsStyleStruct*&) position);

    // see if the width or height was specifically set
    if (position->mWidth.GetUnit() == eStyleUnit_Coord)  {
        aSize.prefSize.width = position->mWidth.GetCoordValue();
        aSize.prefWidthIntrinsic = PR_FALSE;
    }

    if (position->mHeight.GetUnit() == eStyleUnit_Coord) {
        aSize.prefSize.height = position->mHeight.GetCoordValue();     
        aSize.prefHeightIntrinsic = PR_FALSE;
    }
    
    // same for min size. Unfortunately min size is always set to 0. So for now
    // we will assume 0 means not set.
    if (position->mMinWidth.GetUnit() == eStyleUnit_Coord) {
        nscoord min = position->mMinWidth.GetCoordValue();
        if (min != 0)
           aSize.minSize.width = min;
    }

    if (position->mMinHeight.GetUnit() == eStyleUnit_Coord) {
        nscoord min = position->mMinHeight.GetCoordValue();
        if (min != 0)
           aSize.minSize.height = min;
    }

    // and max
    if (position->mMaxWidth.GetUnit() == eStyleUnit_Coord) {
        nscoord max = position->mMaxWidth.GetCoordValue();
        aSize.maxSize.width = max;
    }

    if (position->mMaxHeight.GetUnit() == eStyleUnit_Coord) {
        nscoord max = position->mMaxHeight.GetCoordValue();
        aSize.maxSize.height = max;
    }

    // get the flexibility
    nsCOMPtr<nsIContent> content;
    aFrame->GetContent(getter_AddRefs(content));

    PRInt32 error;
    nsAutoString value;

    if (NS_CONTENT_ATTR_HAS_VALUE == content->GetAttribute(kNameSpaceID_None, nsXULAtoms::flex, value))
    {
        value.Trim("%");
        // convert to a percent.
        aSize.flex = value.ToFloat(&error)/float(100.0);
    }

    if (NS_CONTENT_ATTR_HAS_VALUE == content->GetAttribute(kNameSpaceID_None, nsHTMLAtoms::width, value))
    {
        float p2t;
        aPresContext.GetScaledPixelsToTwips(&p2t);

        value.Trim("%");

        aSize.prefSize.width = NSIntPixelsToTwips(value.ToInteger(&error), p2t);
    }

    if (NS_CONTENT_ATTR_HAS_VALUE == content->GetAttribute(kNameSpaceID_None, nsHTMLAtoms::height, value))
    {
        float p2t;
        aPresContext.GetScaledPixelsToTwips(&p2t);

        value.Trim("%");

        aSize.prefSize.height = NSIntPixelsToTwips(value.ToInteger(&error), p2t);
    }
}

/**
 * Given a frame gets its box info. If it does not have a box info then it will merely
 * get the normally defined min, pref, max stuff.
 *
 */
nsresult
nsBoxFrame::GetChildBoxInfo(nsIPresContext& aPresContext, const nsHTMLReflowState& aReflowState, nsIFrame* aFrame, nsCalculatedBoxInfo& aSize)
{
  aSize.clear();

  // see if the frame Innerements IBox interface
  
  // since frames are not refCounted, don't use nsCOMPtr with them
  //nsCOMPtr<nsIBox> ibox = do_QueryInterface(aFrame);

  // if it does ask it for its BoxSize and we are done
  nsIBox* ibox;
  if (NS_SUCCEEDED(aFrame->QueryInterface(nsIBox::GetIID(), (void**)&ibox)) && ibox) {
     ibox->GetBoxInfo(aPresContext, aReflowState, aSize); 
     // add in the border, padding, width, min, max
     GetRedefinedMinPrefMax(aPresContext, aFrame, aSize);
     return NS_OK;
  }   

 // start the preferred size as intrinsic
  aSize.prefSize.width = NS_INTRINSICSIZE;
  aSize.prefSize.height = NS_INTRINSICSIZE;
  aSize.prefWidthIntrinsic = PR_TRUE;
  aSize.prefHeightIntrinsic = PR_TRUE;

  // redefine anything depending on css
  GetRedefinedMinPrefMax(aPresContext, aFrame, aSize);

  // if we are still intrinsically sized the flow to get the size otherwise
  // we are done.
  if (aSize.prefSize.width == NS_INTRINSICSIZE || aSize.prefSize.height == NS_INTRINSICSIZE)
  {
    // subtract out the childs margin and border 
    const nsStyleSpacing* spacing;
    nsresult rv = aFrame->GetStyleData(eStyleStruct_Spacing,
                   (const nsStyleStruct*&) spacing);

    nsMargin margin(0,0,0,0);;
    spacing->GetMargin(margin);
    nsMargin border(0,0,0,0);
    spacing->GetBorderPadding(border);
    nsMargin total = margin + border;

    // add in childs margin and border
    if (aSize.prefSize.width != NS_INTRINSICSIZE)
        aSize.prefSize.width += (total.left + total.right);

    if (aSize.prefSize.height != NS_INTRINSICSIZE)
        aSize.prefSize.height += (total.top + total.bottom);

    // flow child at preferred size
    nsHTMLReflowMetrics desiredSize(nsnull);

    aSize.calculatedSize = aSize.prefSize;

    nsReflowStatus status;
    PRBool redraw;
    nsString reason("To get pref size");
    FlowChildAt(aFrame, aPresContext, desiredSize, aReflowState, status, aSize, redraw, reason);

    // remove margin and border
    desiredSize.height -= (total.top + total.bottom);
    desiredSize.width -= (total.left + total.right);

    // get the size returned and the it as the preferredsize.
    aSize.prefSize.width = desiredSize.width;
    aSize.prefSize.height = desiredSize.height;
  }

  return NS_OK;
}

/**
 * Ok what we want to do here is get all the children, figure out
 * their flexibility, preferred, min, max sizes and then stretch or
 * shrink them to fit in the given space.
 *
 * So we will have 3 passes. 
 * 1) get our min,max,preferred size.
 * 2) flow all our children to fit into the size we are given layout in
 * 3) move all the children to the right locations.
 */
NS_IMETHODIMP
nsBoxFrame::Reflow(nsIPresContext&   aPresContext,
                     nsHTMLReflowMetrics&     aDesiredSize,
                     const nsHTMLReflowState& aReflowState,
                     nsReflowStatus&          aStatus)
{

#if DEBUG_REFLOW
  if (NS_BLOCK_DOCUMENT_ROOT & mInner->mFlags) 
    printf("---------------- Begin Reflow ---------------\n");
#endif

  /*
  // see if we are debug
    mInner->mIsDebug = PR_FALSE;
    nsString value;

    if (NS_CONTENT_ATTR_HAS_VALUE == mContent->GetAttribute(kNameSpaceID_None, nsXULAtoms::debug, value))
    {
      mInner->mIsDebug = PR_TRUE;
    } else {
      // if our parent is debug we are too
      nsIFrame* parent = nsnull;
      GetParent(&parent);
      if (parent) {
        nsIBox* ibox;
        if (NS_SUCCEEDED(parent->QueryInterface(nsIBox::GetIID(), (void**)&ibox)) && ibox) {
           ibox->IsDebug(mInner->mIsDebug); 
        }
      }
    } 
    */

  // If we have a space manager, then set it in the reflow state
  if (mInner->mSpaceManager) {
    // Modify the reflow state and set the space manager
    nsHTMLReflowState&  reflowState = (nsHTMLReflowState&)aReflowState;
    reflowState.mSpaceManager = mInner->mSpaceManager;

    // Clear the spacemanager's regions.
    mInner->mSpaceManager->ClearRegions();
  }

  //--------------------------------------------------------------------
  //-------------- figure out the rect we need to fit into -------------
  //--------------------------------------------------------------------

  // this is the size of our box. Remember to subtract our our border. The size we are given
  // does not include it. So we have to adjust our rect accordingly.

  nscoord x = aReflowState.mComputedBorderPadding.left;
  nscoord y = aReflowState.mComputedBorderPadding.top;

  nsRect rect(x,y,aReflowState.mComputedWidth,aReflowState.mComputedHeight);
 
  //---------------------------------------------------------
  //------- handle incremental reflow --------------------
  //---------------------------------------------------------

  // if there is incremental we need to tell all the boxes below to blow away the
  // cached values for the children in the reflow list
  nsIFrame* incrementalChild = nsnull;
  if ( aReflowState.reason == eReflowReason_Incremental ) {
    nsIFrame* targetFrame;    
    // See if it's targeted at us
    aReflowState.reflowCommand->GetTarget(targetFrame);
    if (this == targetFrame) {
      // if it has redraw us
      Invalidate(nsRect(0,0,mRect.width,mRect.height), PR_FALSE);
    } else {
      // otherwise dirty our children
      Dirty(aReflowState,incrementalChild);
    }
  } 
#if 0
ListTag(stdout);
printf(": begin reflow reason=%s", 
       aReflowState.reason == eReflowReason_Incremental ? "incremental" : "other");
if (incrementalChild) { printf(" frame="); nsFrame::ListTag(stdout, incrementalChild); }
printf("\n");
#endif

  //------------------------------------------------------------------------------------------------
  //------- Figure out what our box size is. This will calculate our children's sizes as well ------
  //------------------------------------------------------------------------------------------------

  // get our size. This returns a boxSize that contains our min, max, pref sizes. It also
  // calculates all of our children sizes as well. It does not include our border we will have to include that 
  // later
  nsBoxInfo ourSize;
  GetBoxInfo(aPresContext, aReflowState, ourSize);

  //------------------------------------------------------------------------------------------------
  //------- Make sure the space we need to layout into adhears to our min, max, pref sizes    ------
  //------------------------------------------------------------------------------------------------

  BoundsCheck(ourSize, rect);
 
  // subtract out the insets. Insets are so subclasses like toolbars can wedge controls in and around the 
  // box. GetBoxInfo automatically adds them in. But we want to know the size we need to layout our children 
  // in so lets subtract them our for now.
  nsMargin inset(0,0,0,0);
  GetInset(inset);

  nsMargin debugInset(0,0,0,0);
  mInner->GetDebugInset(debugInset);
  inset += debugInset;

  rect.Deflate(inset);

  //-----------------------------------------------------------------------------------
  //------------------------- figure our our children's sizes  -------------------------
  //-----------------------------------------------------------------------------------

  // now that we know our child's min, max, pref sizes. Stretch our children out to fit into our size.
  // this will calculate each of our childs sizes.
  InvalidateChildren();
  LayoutChildrenInRect(rect);

  //-----------------------------------------------------------------------------------
  //------------------------- flow all the children -----------------------------------
  //-----------------------------------------------------------------------------------

  // flow each child at the new sizes we have calculated.
  FlowChildren(aPresContext, aDesiredSize, aReflowState, aStatus, rect);

  //-----------------------------------------------------------------------------------
  //------------------------- Adjust each childs x, y location-------------------------
  //-----------------------------------------------------------------------------------
   // set the x,y locations of each of our children. Taking into acount their margins, our border,
  // and insets.
  PlaceChildren(aPresContext,rect);

  //-----------------------------------------------------------------------------------
  //------------------------- Add our border and insets in ----------------------------
  //-----------------------------------------------------------------------------------

  rect.Inflate(inset);

  if (aReflowState.mComputedWidth != NS_INTRINSICSIZE && rect.width < aReflowState.mComputedWidth)
    rect.width = aReflowState.mComputedWidth;
 
  if (aReflowState.mComputedHeight != NS_INTRINSICSIZE && rect.height < aReflowState.mComputedHeight)
    rect.height = aReflowState.mComputedHeight;
 
  rect.Inflate(aReflowState.mComputedBorderPadding);

  // the rect might have gotten bigger so recalc ourSize
  aDesiredSize.width = rect.width;
  aDesiredSize.height = rect.height;

  aDesiredSize.ascent = aDesiredSize.height;
  aDesiredSize.descent = 0;
 
  aStatus = NS_FRAME_COMPLETE;
  
  nsRect damageArea(0,0,0,0);
  damageArea.y = 0;
  damageArea.height = aDesiredSize.height;
  damageArea.width = aDesiredSize.width;

  /*
   if ((NS_BLOCK_DOCUMENT_ROOT & mInner->mFlags)) {
     printf("----- Reflow --------\n");
     char* type;
     switch(aReflowState.reason) {
     case eReflowReason_Incremental:
        type = "incremental";
        break;
     case eReflowReason_Initial:
        type = "initial";
        break;
     case eReflowReason_Resize:
        type = "resize";
        break;
     case eReflowReason_StyleChange:
        type = "style change";
        break;
     default:
        type = "unknown";
        break;
     }

     printf("type=%s\n", type);
     printf("calculated: width=%d, height=%d\n", aReflowState.mComputedWidth, aReflowState.mComputedHeight);
     printf("desired: width=%d, height=%d\n", aDesiredSize.width, aDesiredSize.height);
   }
   */
#if 0
ListTag(stdout); printf(": reflow done\n");
#endif

  return NS_OK;
}


/**
 * When all the childrens positions have been calculated and layed out. Flow each child
 * at its not size.
 */
nsresult
nsBoxFrame::FlowChildren(nsIPresContext&   aPresContext,
                     nsHTMLReflowMetrics&     aDesiredSize,
                     const nsHTMLReflowState& aReflowState,
                     nsReflowStatus&          aStatus,
                     nsRect& rect)
{
  PRBool redraw = PR_FALSE;

  //-----------------------------------
  // first pass flow all fixed children
  //-----------------------------------

  PRBool finished;
  nscoord passes = 0;
  nscoord changedIndex = -1;
  //nscoord count = 0;
  nsAutoString reason("initial");
  nsAutoString nextReason("initial");
  PRBool resized[100];

  for (int i=0; i < mSpringCount; i++)
      resized[i] = PR_FALSE;

  /*
  nsIFrame* childFrame = mFrames.FirstChild(); 
  while (nsnull != childFrame) 
  {    
   
      if (!mSprings[count].collapsed)
      {
        // reflow only fixed children
        if (mSprings[count].flex == 0.0) {
          FlowChildAt(childFrame, aPresContext, aDesiredSize, aReflowState, aStatus, mSprings[count], redraw, reason);

          // if its height greater than the max. Set the max to this height and set a flag
          // saying we will need to do another pass. But keep going there
          // may be another child that is bigger
          if (mHorizontal) {
            if (aDesiredSize.height > rect.height) {
                rect.height = aDesiredSize.height;
                InvalidateChildren();
                LayoutChildrenInRect(rect);
                nextReason = "child's height got bigger";
            } 
          } else {
            if (aDesiredSize.width > rect.width) {
                mSprings[count].minSize.width = aDesiredSize.width;
                rect.width = aDesiredSize.width;
                InvalidateChildren();
                LayoutChildrenInRect(rect);
                nextReason = "child's width got bigger";
            } 
          }
        }
      }      
    nsresult rv = childFrame->GetNextSibling(&childFrame);
    NS_ASSERTION(rv == NS_OK,"failed to get next child");
    count++;
  }
  */

  //reason = nextReason;


  // ----------------------
  // Flow all children 
  // ----------------------

  // ok what we want to do if flow each child at the location given in the spring.
  // unfortunately after flowing a child it might get bigger. We have not control over this
  // so it the child gets bigger or smaller than we expected we will have to do a 2nd, 3rd, 4th pass to 
  // adjust.

  changedIndex = -1;
  InvalidateChildren();
  LayoutChildrenInRect(rect);
 
  passes = 0;
  do 
  {
    finished = PR_TRUE;
    nscoord count = 0;
    nsIFrame* childFrame = mFrames.FirstChild(); 
    while (nsnull != childFrame) 
    {    
        // if we reached the index that changed we are done.
        if (count == changedIndex)
            break;

        if (!mSprings[count].collapsed)
        {
        // reflow if the child needs it or we are on a second pass
          FlowChildAt(childFrame, aPresContext, aDesiredSize, aReflowState, aStatus, mSprings[count], redraw, reason);
 
          // if the child got bigger then adjust our rect and all the children.
          ChildResized(aDesiredSize, rect, mSprings[count], resized, changedIndex, finished, count, nextReason);
        }
      
        
      nsresult rv = childFrame->GetNextSibling(&childFrame);
      NS_ASSERTION(rv == NS_OK,"failed to get next child");
      count++;
      reason = nextReason;
    }

    // if we get over 10 passes something probably when wrong.
    passes++;
    if (passes > 5) {
      NS_FRAME_TRACE(NS_FRAME_TRACE_CALLS,
                 ("bug"));
    }

    //NS_ASSERTION(passes <= 10,"Error infinte loop too many passes");
    if (passes > 10) {
       break;
    }


  } while (PR_FALSE == finished);

  // redraw things if needed.
  if (redraw) {
#if DEBUG_REDRAW
      ListTag(stdout);
      printf("is being redrawn\n");
#endif
    Invalidate(nsRect(0,0,mRect.width, mRect.height), PR_FALSE);
  }

  return NS_OK;
}

void
nsBoxFrame::ChildResized(nsHTMLReflowMetrics& aDesiredSize, nsRect& aRect, nsCalculatedBoxInfo& aInfo, PRBool* aResized, nscoord& aChangedIndex, PRBool& aFinished, nscoord aIndex, nsString& aReason)
{
  if (mHorizontal) {
      // if we are a horizontal box see if the child will fit inside us.
      if ( aDesiredSize.height > aRect.height) {
            // if we are a horizontal box and the the child it bigger than our height

            // ok if the height changed then we need to reflow everyone but us at the new height
            // so we will set the changed index to be us. And signal that we need a new pass.
            aRect.height = aDesiredSize.height;

            // remember we do not need to clear the resized list because changing the height of a horizontal box
            // will not affect the width of any of its children because block flow left to right, top to bottom. Just trust me
            // on this one.
            aFinished = PR_FALSE;
            aChangedIndex = aIndex;

            // relayout everything
            InvalidateChildren();
            LayoutChildrenInRect(aRect);
            aReason = "child's height got bigger";
      } else if (aDesiredSize.width > aInfo.calculatedSize.width) {
            // if the child is wider than we anticipated. This can happend for children that we were not able to get a
            // take on their min width. Like text, or tables.

            // because things flow from left to right top to bottom we know that
            // if we get wider that we can set the min size. This will only work
            // for width not height. Height must always be recalculated!
            aInfo.minSize.width = aDesiredSize.width;

            // our width now becomes the new size
            aInfo.calculatedSize.width = aDesiredSize.width;

            InvalidateChildren();

            // our index resized
            aResized[aIndex] = PR_TRUE;

            // if the width changed. mark our child as being resized
            for (int i=0; i < mSpringCount; i++)
              mSprings[i].sizeValid = aResized[i];

            LayoutChildrenInRect(aRect);
            aFinished = PR_FALSE;
            aChangedIndex = aIndex;
            aReason = "child's width got bigger";
      }
  } else {
     if ( aDesiredSize.width > aRect.width) {
            // ok if the height changed then we need to reflow everyone but us at the new height
            // so we will set the changed index to be us. And signal that we need a new pass.
            aRect.width = aDesiredSize.width;

            // because things flow from left to right top to bottom we know that
            // if we get wider that we can set the min size. This will only work
            // for width not height. Height must always be recalculated!
            aInfo.minSize.width = aDesiredSize.width;

            // if the width changed then clear out the resized list
            // but only do this if we are vertical box. On a horizontal box increasing the height will not change the
            // width of its children.
            for (int i=0; i < mSpringCount; i++)
               aResized[i] = PR_FALSE;

            aFinished = PR_FALSE;
            aChangedIndex = aIndex;

            // relayout everything
            InvalidateChildren();
            LayoutChildrenInRect(aRect);
            aReason = "child's height got bigger";
      } else if (aDesiredSize.height > aInfo.calculatedSize.height) {
            // our width now becomes the new size
            aInfo.calculatedSize.height = aDesiredSize.height;

            InvalidateChildren();

            // our index resized
            aResized[aIndex] = PR_TRUE;

            // if the width changed. mark our child as being resized
            for (int i=0; i < mSpringCount; i++)
              mSprings[i].sizeValid = aResized[i];

            LayoutChildrenInRect(aRect);
            aFinished = PR_FALSE;
            aChangedIndex = aIndex;
            aReason = "child's width got bigger";
      }
  }
}


void 
nsBoxFrame::CollapseChild(nsIFrame* frame)
{
    nsRect rect(0,0,0,0);
    frame->GetRect(rect);
    if (rect.width > 0 || rect.height > 0) {
      // shrink the frame
      frame->SizeTo(0,0);

      // shrink the view
      nsIView* view = nsnull;
      frame->GetView(&view);

      // if we find a view stop right here. All views under it
      // will be clipped.
      if (view) {
        view->SetDimensions(0,0,PR_FALSE);
        return;
      }
    
      // collapse the child
      nsIFrame* child = nsnull;
      frame->FirstChild(nsnull, &child);

      while (nsnull != child) 
      {
         CollapseChild(child);
         nsresult rv = child->GetNextSibling(&child);
         NS_ASSERTION(rv == NS_OK,"failed to get next child");
      }
    }
}


/**
 * Given the boxes rect. Set the x,y locations of all its children. Taking into account
 * their margins
 */
nsresult
nsBoxFrame::PlaceChildren(nsIPresContext& aPresContext, nsRect& boxRect)
{
  // ------- set the childs positions ---------
  nscoord x = boxRect.x;
  nscoord y = boxRect.y;

  nsIFrame* childFrame = mFrames.FirstChild(); 
  nscoord count = 0;
  while (nsnull != childFrame) 
  {
    nsresult rv;

    // make collapsed children not show up
    if (mSprings[count].collapsed) {
      CollapseChild(childFrame);
    } else {
      const nsStyleSpacing* spacing;
      rv = childFrame->GetStyleData(eStyleStruct_Spacing,
                     (const nsStyleStruct*&) spacing);

      nsMargin margin(0,0,0,0);
      spacing->GetMargin(margin);

      if (mHorizontal) {
        x += margin.left;
        y = boxRect.y + margin.top;
      } else {
        y += margin.top;
        x = boxRect.x + margin.left;
      }

      nsRect rect;
      childFrame->GetRect(rect);
      rect.x = x;
      rect.y = y;
      childFrame->SetRect(rect);

      // add in the right margin
      if (mHorizontal)
        x += margin.right;
      else
        y += margin.bottom;
     
      if (mHorizontal) {
        x += rect.width;
        //width += rect.width + margin.left + margin.right;
      } else {
        y += rect.height;
        //height += rect.height + margin.top + margin.bottom;
      }
    }

    rv = childFrame->GetNextSibling(&childFrame);
    NS_ASSERTION(rv == NS_OK,"failed to get next child");
    count++;
  }

  return NS_OK;
}


/**
 * Flow an individual child. Special args:
 * count: the spring that will be used to lay out the child
 * incrementalChild: If incremental reflow this is the child that need to be reflowed.
 *                   when we finally do reflow the child we will set the child to null
 */

nsresult
nsBoxFrame::FlowChildAt(nsIFrame* childFrame, 
                     nsIPresContext& aPresContext,
                     nsHTMLReflowMetrics&     desiredSize,
                     const nsHTMLReflowState& aReflowState,
                     nsReflowStatus&          aStatus,
                     nsCalculatedBoxInfo&     aInfo,
                     PRBool& aRedraw,
                     nsString& aReason)
{
      aStatus = NS_FRAME_COMPLETE;
      nsReflowReason reason = aReflowState.reason;
      PRBool shouldReflow = PR_TRUE;

      // if the reason is incremental and the child is not marked as incremental. Then relow the child
      // as a resize instead.
      if (aInfo.isIncremental)
          reason = eReflowReason_Incremental;
      else if (reason == eReflowReason_Incremental)
          reason = eReflowReason_Resize;
      
      // subtract out the childs margin and border 
      const nsStyleSpacing* spacing;
      nsresult rv = childFrame->GetStyleData(eStyleStruct_Spacing,
                     (const nsStyleStruct*&) spacing);

      nsMargin margin(0,0,0,0);
      spacing->GetMargin(margin);

      // get the current size of the child
      nsRect currentRect(0,0,0,0);
      childFrame->GetRect(currentRect);

      // if we don't need a reflow then 
      // lets see if we are already that size. Yes? then don't even reflow. We are done.
      if (!aInfo.needsReflow && aInfo.calculatedSize.width != NS_INTRINSICSIZE && aInfo.calculatedSize.height != NS_INTRINSICSIZE) {

          // if the new calculated size has a 0 width or a 0 height
          if ((currentRect.width == 0 || currentRect.height == 0) && (aInfo.calculatedSize.width == 0 || aInfo.calculatedSize.height == 0)) {
               shouldReflow = PR_FALSE;
               desiredSize.width = aInfo.calculatedSize.width - (margin.left + margin.right);
               desiredSize.height = aInfo.calculatedSize.height - (margin.top + margin.bottom);
               childFrame->SizeTo(desiredSize.width, desiredSize.height);
          } else {
            desiredSize.width = currentRect.width;
            desiredSize.height = currentRect.height;

            // remove the margin. The rect of our child does not include it but our calculated size does.
            nscoord calcWidth = aInfo.calculatedSize.width - (margin.left + margin.right);
            nscoord calcHeight = aInfo.calculatedSize.height - (margin.top + margin.bottom);

            // don't reflow if we are already the right size
            if (currentRect.width == calcWidth && currentRect.height == calcHeight)
                  shouldReflow = PR_FALSE;
          }
      }      

      // ok now reflow the child into the springs calculated space
      if (shouldReflow) {

        nsMargin border(0,0,0,0);
        spacing->GetBorderPadding(border);

        const nsStylePosition* position;
        rv = childFrame->GetStyleData(eStyleStruct_Position,
                       (const nsStyleStruct*&) position);

        desiredSize.width = 0;
        desiredSize.height = 0;

        nsSize size(aInfo.calculatedSize.width, aInfo.calculatedSize.height);

        /*
        // lets also look at our intrinsic flag. This flag is to make things like HR work.
        // hr is funny if you flow it intrinsically you will get a size that is the height of
        // the current font size. But if you then flow the hr with a computed height of what was returned the
        // hr will be stretched out to fit. So basically the hr lays itself out differently depending 
        // on if you use intrinsic or or computed size. So to fix this we follow this policy. If any child
        // does not Innerement nsIBox then we set this flag. Then on a flow if we decide to flow at the preferred width
        // we flow it with a intrinsic width. This goes for height as well.
        if (aInfo.prefWidthIntrinsic && size.width == aInfo.prefSize.width) 
           size.width = NS_INTRINSICSIZE;

        if (aInfo.prefHeightIntrinsic && size.height == aInfo.prefSize.height) 
           size.height = NS_INTRINSICSIZE;
        */

        // only subrtact margin
        if (size.height != NS_INTRINSICSIZE)
            size.height -= (margin.top + margin.bottom);

        if (size.width != NS_INTRINSICSIZE)
            size.width -= (margin.left + margin.right);

        // create a reflow state to tell our child to flow at the given size.
        nsHTMLReflowState   reflowState(aPresContext, aReflowState, childFrame, nsSize(size.width, NS_INTRINSICSIZE));
        reflowState.reason = reason;

        if (size.height != NS_INTRINSICSIZE)
            size.height -= (border.top + border.bottom);

        if (size.width != NS_INTRINSICSIZE)
            size.width -= (border.left + border.right);

        reflowState.mComputedWidth = size.width;
        reflowState.mComputedHeight = size.height;

    //    nsSize maxElementSize(0, 0);
      //  desiredSize.maxElementSize = &maxElementSize;
        
#if DEBUG_REFLOW
  ListTag(stdout); 
  if (reason == eReflowReason_Incremental && aInfo.isIncremental) 
     printf(": INCREMENTALLY reflowing ");
  else
     printf(": reflowing ");

   nsFrame::ListTag(stdout, childFrame);
   char ch[100];
   aReason.ToCString(ch,100);
   printf("because (%s)\n", ch);
#endif
        // do the flow
        nsIHTMLReflow*      htmlReflow;

        rv = childFrame->QueryInterface(kIHTMLReflowIID, (void**)&htmlReflow);
        NS_ASSERTION(rv == NS_OK,"failed to get htmlReflow interface.");

        htmlReflow->WillReflow(aPresContext);
        htmlReflow->Reflow(aPresContext, desiredSize, reflowState, aStatus);

        NS_ASSERTION(NS_FRAME_IS_COMPLETE(aStatus), "bad status");

        nsFrameState  kidState;
        childFrame->GetFrameState(&kidState);

       // printf("width: %d, height: %d\n", desiredSize.mCombinedArea.width, desiredSize.mCombinedArea.height);

        if (kidState & NS_FRAME_OUTSIDE_CHILDREN) {
             desiredSize.width = desiredSize.mCombinedArea.width;
             desiredSize.height = desiredSize.mCombinedArea.height;
        }

        
//        if (maxElementSize.width > desiredSize.width)
  //          desiredSize.width = maxElementSize.width;

        PRBool changedSize = PR_FALSE;

        if (currentRect.width != desiredSize.width || currentRect.height != desiredSize.height)
           changedSize = PR_TRUE;
        
        // if the child got bigger then make sure the new size in our min max range
        if (changedSize) {
          
          // redraw if we changed size.
          aRedraw = PR_TRUE;

          if (aInfo.maxSize.width != NS_INTRINSICSIZE && desiredSize.width > aInfo.maxSize.width - (margin.left + margin.right))
              desiredSize.width = aInfo.maxSize.width - (margin.left + margin.right);

          // if the child was bigger than anticipated and there was a min size set thennn
          if (aInfo.calculatedSize.width != NS_INTRINSICSIZE && position->mMinWidth.GetUnit() == eStyleUnit_Coord) {
             nscoord min = position->mMinWidth.GetCoordValue();
             if (min != 0)
                 desiredSize.width = aInfo.calculatedSize.width - (margin.left + margin.right);
          }

          if (aInfo.maxSize.height != NS_INTRINSICSIZE && desiredSize.height > aInfo.maxSize.height - (margin.top + margin.bottom))
              desiredSize.height = aInfo.maxSize.height - (margin.top + margin.bottom);

          // if a min size was set we will always get the desired height
          if (aInfo.calculatedSize.height != NS_INTRINSICSIZE && position->mMinHeight.GetUnit() == eStyleUnit_Coord) {
             nscoord min = position->mMinHeight.GetCoordValue();
             if (min != 0)
                 desiredSize.height = aInfo.calculatedSize.height - (margin.top + margin.bottom);
          }

        }

          // set the rect
        childFrame->SizeTo(desiredSize.width, desiredSize.height);

        // Stub out desiredSize.maxElementSize so that when go out of
        // scope, nothing bad happens!
        desiredSize.maxElementSize = nsnull;

        // clear out the incremental child, so that we don't flow it incrementally again
        if (reason == eReflowReason_Incremental && aInfo.isIncremental) 
          aInfo.isIncremental = PR_FALSE;
      
      }
      // add the margin back in. The child should add its border automatically
      desiredSize.height += (margin.top + margin.bottom);
      desiredSize.width += (margin.left + margin.right);

      aInfo.needsReflow = PR_FALSE;

      return NS_OK;
}

/**
 * Given a box info object and a rect. Make sure the rect is not too small to layout the box and
 * not to big either.
 */
void
nsBoxFrame::BoundsCheck(const nsBoxInfo& aBoxInfo, nsRect& aRect)
{ 
  // if we are bieng flowed at our intrinsic width or height then set our width
  // to the biggest child.
  if (aRect.height == NS_INTRINSICSIZE ) 
      aRect.height = aBoxInfo.prefSize.height;

  if (aRect.width == NS_INTRINSICSIZE ) 
      aRect.width = aBoxInfo.prefSize.width;

  // make sure the available size is no bigger than the max size
  if (aRect.height > aBoxInfo.maxSize.height)
     aRect.height = aBoxInfo.maxSize.height;

  if (aRect.width > aBoxInfo.maxSize.width)
     aRect.width = aBoxInfo.maxSize.width;

  // make sure the available size is at least as big as the min size
  if (aRect.height < aBoxInfo.minSize.height)
     aRect.height = aBoxInfo.minSize.height;

  if (aRect.width < aBoxInfo.minSize.width)
     aRect.width = aBoxInfo.minSize.width;
     
}

/**
 * Ok when calculating a boxes size such as its min size we need to look at its children to figure it out.
 * But this isn't as easy as just adding up its childs min sizes. If the box is horizontal then we need to 
 * add up each child's min width but our min height should be the childs largest min height. This needs to 
 * be done for preferred size and max size as well. Of course for our max size we need to pick the smallest
 * max size. So this method facilitates the calculation. Just give it 2 sizes and a flag to ask whether is is
 * looking for the largest or smallest value (max needs smallest) and it will set the second value.
 */
void
nsBoxFrame::AddSize(const nsSize& a, nsSize& b, PRBool largest)
{

  // depending on the dimension switch either the width or the height component. 
  const nscoord& awidth  = mHorizontal ? a.width  : a.height;
  const nscoord& aheight = mHorizontal ? a.height : a.width;
  nscoord& bwidth  = mHorizontal ? b.width  : b.height;
  nscoord& bheight = mHorizontal ? b.height : b.width;

  // add up the widths make sure we check for intrinsic.
  if (bwidth != NS_INTRINSICSIZE) // if we are already intrinsic we are done
  {
    // otherwise if what we are adding is intrinsic then we just become instrinsic and we are done
    if (awidth == NS_INTRINSICSIZE)
       bwidth = NS_INTRINSICSIZE;
    else // add it on
       bwidth += awidth;
  }
  
  // store the largest or smallest height
  if ((largest && aheight > bheight) || (!largest && bheight < aheight)) 
      bheight = aheight;
}



void 
nsBoxFrame::GetInset(nsMargin& margin)
{
}

#define GET_WIDTH(size) (mHorizontal ? size.width : size.height)
#define GET_HEIGHT(size) (mHorizontal ? size.height : size.width)
#define GET_WIDTH_FOR(box, size) (box->mHorizontal ? box->size.width : box->size.height)
#define GET_HEIGHT_FOR(box, size) (box->mHorizontal ? box->size.height : box->size.width)


void
nsBoxFrame::InvalidateChildren()
{
    for (int i=0; i < mSpringCount; i++) {
        mSprings[i].sizeValid = PR_FALSE;
    }
}

void
nsBoxFrame::LayoutChildrenInRect(nsRect& size)
{
      if (mSpringCount == 0)
          return;

      PRInt32 sizeRemaining;
       
      if (mHorizontal)
          sizeRemaining = size.width;
      else
          sizeRemaining = size.height;

      float springConstantsRemaining = (float)0.0;
      int i;

      for (i=0; i<mSpringCount; i++) {
          nsCalculatedBoxInfo& spring = mSprings[i];
 
          // ignore collapsed children
          if (spring.collapsed)
              continue;

          // figure out the direction of the box and get the correct value either the width or height
          nscoord& pref = GET_WIDTH(spring.prefSize);
          // nscoord& max  = GET_WIDTH(spring.maxSize); // Not used.
          nscoord& min  = GET_WIDTH(spring.minSize);
         
          GET_HEIGHT(spring.calculatedSize) = GET_HEIGHT(size);
        
          if (pref < min)
              pref = min;

          if (spring.sizeValid) { 
             sizeRemaining -= GET_WIDTH(spring.calculatedSize);
          } else {
            if (spring.flex == 0.0)
            {
              spring.sizeValid = PR_TRUE;
              GET_WIDTH(spring.calculatedSize) = pref;
            }
            sizeRemaining -= pref;
            springConstantsRemaining += spring.flex;
          } 
      }

      nscoord& sz = GET_WIDTH(size);
      if (sz == NS_INTRINSICSIZE) {
          sz = 0;
          for (i=0; i<mSpringCount; i++) {
              nsCalculatedBoxInfo& spring=mSprings[i];

              // ignore collapsed springs
              if (spring.collapsed)
                 continue;

              nscoord& calculated = GET_WIDTH(spring.calculatedSize);
              nscoord& pref = GET_WIDTH(spring.prefSize);

              if (!spring.sizeValid) 
              {
                // set the calculated size to be the preferred size
                calculated = pref;
                spring.sizeValid = PR_TRUE;
              }

              // changed the size returned to reflect
              sz += calculated;
          }
          return;
      }

      PRBool limit = PR_TRUE;
      for (int pass=1; PR_TRUE == limit; pass++) {
          limit = PR_FALSE;
          for (i=0; i<mSpringCount; i++) {
              nsCalculatedBoxInfo& spring=mSprings[i];
              // ignore collapsed springs
              if (spring.collapsed)
                 continue;

              nscoord& pref = GET_WIDTH(spring.prefSize);
              nscoord& max  = GET_WIDTH(spring.maxSize);
              nscoord& min  = GET_WIDTH(spring.minSize);
              nscoord& calculated = GET_WIDTH(spring.calculatedSize);
     
              if (spring.sizeValid==PR_FALSE) {
                  PRInt32 newSize = pref + NSToIntRound(sizeRemaining*(spring.flex/springConstantsRemaining));
                  if (newSize<=min) {
                      calculated = min;
                      springConstantsRemaining -= spring.flex;
                      sizeRemaining += pref;
                      sizeRemaining -= calculated;
 
                      spring.sizeValid = PR_TRUE;
                      limit = PR_TRUE;
                  }
                  else if (newSize>=max) {
                      calculated = max;
                      springConstantsRemaining -= spring.flex;
                      sizeRemaining += pref;
                      sizeRemaining -= calculated;
                      spring.sizeValid = PR_TRUE;
                      limit = PR_TRUE;
                  }
              }
          }
      }

      float stretchFactor = (springConstantsRemaining == 0.0) ? float(0.0) : sizeRemaining/springConstantsRemaining;

        nscoord& s = GET_WIDTH(size);
        s = 0;
        for (i=0; i<mSpringCount; i++) {
             nsCalculatedBoxInfo& spring=mSprings[i];

             // ignore collapsed springs
             if (spring.collapsed)
                 continue;

             nscoord& pref = GET_WIDTH(spring.prefSize);
             nscoord& calculated = GET_WIDTH(spring.calculatedSize);
  
            if (spring.sizeValid==PR_FALSE) {
                calculated = pref + NSToIntFloor(spring.flex*stretchFactor);
                spring.sizeValid = PR_TRUE;
            }

            s += calculated;
        }
}


void 
nsBoxFrame::GetChildBoxInfo(PRInt32 aIndex, nsBoxInfo& aSize)
{
    NS_ASSERTION(aIndex >= 0 && aIndex < mSpringCount,"Index out of bounds!!");
    aSize = mSprings[aIndex];
}

void 
nsBoxFrame::SetChildNeedsRecalc(PRInt32 aIndex, PRBool aRecalc)
{
    NS_ASSERTION(aIndex >= 0 && aIndex < mSpringCount,"Index out of bounds!!");
    mSprings[aIndex].needsRecalc = PR_TRUE;
}


// Marks the frame as dirty and generates an incremental reflow
// command targeted at this frame
nsresult
nsBoxFrame::GenerateDirtyReflowCommand(nsIPresContext& aPresContext,
                                       nsIPresShell&   aPresShell)
{
  nsCOMPtr<nsIReflowCommand>  reflowCmd;
  nsresult                    rv;
  
  mState |= NS_FRAME_IS_DIRTY;
  rv = NS_NewHTMLReflowCommand(getter_AddRefs(reflowCmd), this,
                               nsIReflowCommand::ReflowDirty);
  if (NS_SUCCEEDED(rv)) {
    // Add the reflow command
    rv = aPresShell.AppendReflowCommand(reflowCmd);
  }

  return rv;
}

NS_IMETHODIMP
nsBoxFrame::RemoveFrame(nsIPresContext& aPresContext,
                           nsIPresShell& aPresShell,
                           nsIAtom* aListName,
                           nsIFrame* aOldFrame)
{
      // need to rebuild all the springs.
      for (int i=0; i < mSpringCount; i++) 
             mSprings[i].clear();
      
      // remove the child frame
      mFrames.DestroyFrame(aPresContext, aOldFrame);

      // mark us dirty and generate a reflow command
      return GenerateDirtyReflowCommand(aPresContext, aPresShell);
}

NS_IMETHODIMP
nsBoxFrame::InsertFrames(nsIPresContext& aPresContext,
                            nsIPresShell& aPresShell,
                            nsIAtom* aListName,
                            nsIFrame* aPrevFrame,
                            nsIFrame* aFrameList)
{
  // need to rebuild all the springs.
  for (int i=0; i < mSpringCount; i++) 
         mSprings[i].clear();

  mFrames.InsertFrames(nsnull, aPrevFrame, aFrameList);
  
  // mark us dirty and generate a reflow command
  return GenerateDirtyReflowCommand(aPresContext, aPresShell);
}

NS_IMETHODIMP
nsBoxFrame::AppendFrames(nsIPresContext& aPresContext,
                           nsIPresShell&   aPresShell,
                           nsIAtom*        aListName,
                           nsIFrame*       aFrameList)
{

  // need to rebuild all the springs.
  for (int i=0; i < mSpringCount; i++) 
      mSprings[i].clear();

   mFrames.AppendFrames(nsnull, aFrameList); 
   
   // mark us dirty and generate a reflow command
   return GenerateDirtyReflowCommand(aPresContext, aPresShell);
}



NS_IMETHODIMP
nsBoxFrame::AttributeChanged(nsIPresContext* aPresContext,
                               nsIContent* aChild,
                               nsIAtom* aAttribute,
                               PRInt32 aHint)
{
  nsresult rv = nsHTMLContainerFrame::AttributeChanged(aPresContext, aChild,
                                              aAttribute, aHint);

  if (NS_OK != rv) {
    return rv;
  }

  return NS_OK;
}

/**
 * Goes though each child asking for its size to determine our size. Returns our box size minus our border.
 * This method is defined in nsIBox interface.
 */
NS_IMETHODIMP
nsBoxFrame::GetBoxInfo(nsIPresContext& aPresContext, const nsHTMLReflowState& aReflowState, nsBoxInfo& aSize)
{
   nsresult rv;

   aSize.clear();
 
   // run through all the children and get there min, max, and preferred sizes
   // return us the size of the box
   nscoord count = 0;
   nsIFrame* childFrame = mFrames.FirstChild(); 

   while (nsnull != childFrame) 
   {  
    // if a child needs recalculation then ask it for its size. Otherwise
    // just use the size we already have.
    if (mSprings[count].needsRecalc)
    {
      // see if the child is collapsed
      const nsStyleDisplay* disp;
      childFrame->GetStyleData(eStyleStruct_Display, ((const nsStyleStruct *&)disp));

      // if collapsed then the child will have no size
      if (disp->mVisible == NS_STYLE_VISIBILITY_COLLAPSE) 
         mSprings[count].collapsed = PR_TRUE;
      else {
        // get the size of the child. This is the min, max, preferred, and spring constant
        // it does not include its border.
        rv = GetChildBoxInfo(aPresContext, aReflowState, childFrame, mSprings[count]);
        NS_ASSERTION(rv == NS_OK,"failed to child box info");
        if (NS_FAILED(rv))
         return rv;

        // add in the child's margin and border/padding if there is one.
        const nsStyleSpacing* spacing;
        rv = childFrame->GetStyleData(eStyleStruct_Spacing,
                                      (const nsStyleStruct*&) spacing);

        NS_ASSERTION(rv == NS_OK,"failed to get spacing info");
        if (NS_FAILED(rv))
           return rv;

        nsMargin margin(0,0,0,0);
        spacing->GetMargin(margin);
        nsSize m(margin.left+margin.right,margin.top+margin.bottom);
        mSprings[count].minSize += m;
        mSprings[count].prefSize += m;
        if (mSprings[count].maxSize.width != NS_INTRINSICSIZE)
           mSprings[count].maxSize.width += m.width;

        if (mSprings[count].maxSize.height != NS_INTRINSICSIZE)
           mSprings[count].maxSize.height += m.height;

        spacing->GetBorderPadding(margin);
        nsSize b(margin.left+margin.right,margin.top+margin.bottom);
        mSprings[count].minSize += b;
        mSprings[count].prefSize += b;
        if (mSprings[count].maxSize.width != NS_INTRINSICSIZE)
           mSprings[count].maxSize.width += b.width;

        if (mSprings[count].maxSize.height != NS_INTRINSICSIZE)
           mSprings[count].maxSize.height += b.height;
      }

      // ok we don't need to calc this guy again
      mSprings[count].needsRecalc = PR_FALSE;
    } 

    AddChildSize(aSize, mSprings[count]);

    rv = childFrame->GetNextSibling(&childFrame);
    NS_ASSERTION(rv == NS_OK,"failed to get next child");
    if (NS_FAILED(rv))
       return rv;

    count++;
  }

  mSpringCount = count;

  // add the insets into our size. This is merely some extra space subclasses like toolbars
  // can place around us. Toolbars use it to place extra control like grippies around things.
  nsMargin inset(0,0,0,0);
  GetInset(inset);

  nsMargin debugInset(0,0,0,0);
  mInner->GetDebugInset(debugInset);
  inset += debugInset;

  nsSize in(inset.left+inset.right,inset.top+inset.bottom);
  aSize.minSize += in;
  aSize.prefSize += in;

  return rv;
}

void
nsBoxFrame::AddChildSize(nsBoxInfo& aInfo, nsBoxInfo& aChildInfo)
{
    // now that we know our child's min, max, pref sizes figure OUR size from them.
    AddSize(aChildInfo.minSize,  aInfo.minSize,  PR_FALSE);
    AddSize(aChildInfo.maxSize,  aInfo.maxSize,  PR_TRUE);
    AddSize(aChildInfo.prefSize, aInfo.prefSize, PR_FALSE);
}

/**
 * Boxes work differently that regular HTML elements. Each box knows if it needs to be reflowed or not
 * So when a box gets an incremental reflow. It runs down all the children and marks them for reflow. If it
 * Reaches a child that is not a box then it marks that child as incremental so when it is flowed next it 
 * will be flowed incrementally.
 */
NS_IMETHODIMP
nsBoxFrame::Dirty(const nsHTMLReflowState& aReflowState, nsIFrame*& incrementalChild)
{
  incrementalChild = nsnull;
  nsresult rv = NS_OK;

  // Dirty any children that need it.
  nsIFrame* frame;
  aReflowState.reflowCommand->GetNext(frame);

  nscoord count = 0;
  nsIFrame* childFrame = mFrames.FirstChild(); 
  while (nsnull != childFrame) 
  {
    if (childFrame == frame) {
        // clear the spring so it is recalculated on the flow
        mSprings[count].clear();
        //nsCOMPtr<nsIBox> ibox = do_QueryInterface(childFrame);
        // can't use nsCOMPtr on non-refcounted things like frames
        nsIBox* ibox;
        if (NS_SUCCEEDED(childFrame->QueryInterface(nsIBox::GetIID(), (void**)&ibox)) && ibox)
            ibox->Dirty(aReflowState, incrementalChild);
        else
            incrementalChild = frame;

        // if we found a leaf. Then mark it as being incremental. So when we
        // flow it we will flow it incrementally
        if (incrementalChild == childFrame)
          mSprings[count].isIncremental = PR_TRUE;

        break;

    }

    rv = childFrame->GetNextSibling(&childFrame);
    NS_ASSERTION(rv == NS_OK,"failed to get next child");
    if (NS_FAILED(rv))
      return rv;

    count++;
  }

  return rv;
}

NS_IMETHODIMP
nsBoxFrame :: Paint ( nsIPresContext& aPresContext,
                      nsIRenderingContext& aRenderingContext,
                      const nsRect& aDirtyRect,
                      nsFramePaintLayer aWhichLayer)
{
  const nsStyleDisplay* disp = (const nsStyleDisplay*)
  mStyleContext->GetStyleData(eStyleStruct_Display);

  // if we aren't visible then we are done.
  if (!disp->mVisible) 
	   return NS_OK;  

  // if we are visible then tell our superclass to paint
  nsresult r = nsHTMLContainerFrame::Paint(aPresContext, aRenderingContext, aDirtyRect,
                       aWhichLayer);

  if (NS_FRAME_PAINT_LAYER_FOREGROUND == aWhichLayer) {
        if (mInner->mDebugInner) {
            mInner->mDebugInner->PaintSprings(aPresContext, aRenderingContext, aDirtyRect);
        }
  }
 

  return r;
}

// Paint one child frame
void
nsBoxFrame::PaintChild(nsIPresContext&      aPresContext,
                             nsIRenderingContext& aRenderingContext,
                             const nsRect&        aDirtyRect,
                             nsIFrame*            aFrame,
                             nsFramePaintLayer    aWhichLayer)
{
      const nsStyleDisplay* disp;
      aFrame->GetStyleData(eStyleStruct_Display, ((const nsStyleStruct *&)disp));

      // if collapsed don't paint the child.
      if (disp->mVisible == NS_STYLE_VISIBILITY_COLLAPSE) 
         return;

      nsHTMLContainerFrame::PaintChild(aPresContext, aRenderingContext, aDirtyRect, aFrame, aWhichLayer);
}

void 
nsBoxDebugInner::PaintSprings(nsIPresContext& aPresContext, nsIRenderingContext& aRenderingContext, const nsRect& aDirtyRect)
{
    
        // remove our border
        const nsStyleSpacing* spacing;
        nsresult rv = mOuter->GetStyleData(eStyleStruct_Spacing,
                       (const nsStyleStruct*&) spacing);

        nsMargin border(0,0,0,0);
        spacing->GetBorderPadding(border);

        float p2t;
        aPresContext.GetScaledPixelsToTwips(&p2t);
        nscoord onePixel = NSIntPixelsToTwips(1, p2t);

        nsIStyleContext* debugStyle;
        if (mOuter->mHorizontal)
            debugStyle = mHorizontalDebugStyle;
        else
            debugStyle = mVerticalDebugStyle;

        const nsStyleSpacing* debugSpacing =
        (const nsStyleSpacing*)debugStyle->GetStyleData(eStyleStruct_Spacing);
        const nsStyleColor* debugColor =
        (const nsStyleColor*)debugStyle->GetStyleData(eStyleStruct_Color);

        nsMargin margin(0,0,0,0);
        debugSpacing->GetMargin(margin);

        border += margin;

        nsRect inner(0,0,mOuter->mRect.width, mOuter->mRect.height);
        inner.Deflate(border);

        // paint our debug border
        nsCSSRendering::PaintBorder(aPresContext, aRenderingContext, mOuter,
                                    aDirtyRect, inner, *debugSpacing, debugStyle, 0);

        // get the debug border dimensions
        nsMargin debugBorder(0,0,0,0);
        debugSpacing->GetBorderPadding(debugBorder);


        // paint the springs.
        nscoord x, y, borderSize, springSize;
        

        aRenderingContext.SetColor(debugColor->mColor);
        

        if (mOuter->mHorizontal) 
        {
            x = inner.x;
            y = inner.y + onePixel;
            x += debugBorder.left;
            springSize = debugBorder.top - onePixel*4;
        } else {
            x = inner.y;
            y = inner.x + onePixel;
            x += debugBorder.top;
            springSize = debugBorder.left - onePixel*4;
        }

        for (int i=0; i < mOuter->mSpringCount; i++) {
            nsSize& size = mOuter->mSprings[i].calculatedSize;
            if (!mOuter->mSprings[i].collapsed) {
                if (mOuter->mHorizontal) 
                    borderSize = size.width;
                else 
                    borderSize = size.height;

                DrawSpring(aPresContext, aRenderingContext, mOuter->mSprings[i].flex, x, y, borderSize, springSize);
                x += borderSize;
            }
        }
}

void
nsBoxDebugInner::DrawLine(nsIRenderingContext& aRenderingContext, nscoord x1, nscoord y1, nscoord x2, nscoord y2)
{
    if (mOuter->mHorizontal)
       aRenderingContext.DrawLine(x1,y1,x2,y2);
    else
       aRenderingContext.DrawLine(y1,x1,y2,x2);
}

void
nsBoxDebugInner::FillRect(nsIRenderingContext& aRenderingContext, nscoord x, nscoord y, nscoord width, nscoord height)
{
    if (mOuter->mHorizontal)
       aRenderingContext.FillRect(x,y,width,height);
    else
       aRenderingContext.FillRect(y,x,height,width);
}

void 
nsBoxDebugInner::DrawSpring(nsIPresContext& aPresContext, nsIRenderingContext& aRenderingContext, float flex, nscoord x, nscoord y, nscoord size, nscoord springSize)
{
        // PRBool h = mOuter->mHorizontal;
    
        float p2t;
        aPresContext.GetScaledPixelsToTwips(&p2t);
        nscoord onePixel = NSIntPixelsToTwips(1, p2t);

     // if we do draw the coils
        int distance = 0;
        int center = 0;
        int offset = 0;
        int coilSize = COIL_SIZE*onePixel;
        int halfSpring = springSize/2;

        distance = size;
        center = y + halfSpring;
        offset = x;

        int coils = distance/coilSize;

        int halfCoilSize = coilSize/2;

        if (flex == 0.0) {
            DrawLine(aRenderingContext, x,y + springSize/2, x + size, y + springSize/2);
        } else {
            for (int i=0; i < coils; i++)
            {
                   DrawLine(aRenderingContext, offset, center+halfSpring, offset+halfCoilSize, center-halfSpring);
                   DrawLine(aRenderingContext, offset+halfCoilSize, center-halfSpring, offset+coilSize, center+halfSpring);

                   offset += coilSize;
            }
        }

        FillRect(aRenderingContext, x + size - springSize/2, y, springSize/2, springSize);
        FillRect(aRenderingContext, x, y, springSize/2, springSize);

        //DrawKnob(aPresContext, aRenderingContext, x + size - springSize, y, springSize);
}

void
nsBoxFrameInner::UpdatePseudoElements(nsIPresContext& aPresContext) 
{
    nsCOMPtr<nsIStyleContext> hs;
    nsCOMPtr<nsIStyleContext> vs;

	nsCOMPtr<nsIAtom> atom ( getter_AddRefs(NS_NewAtom(":-moz-horizontal-box-debug")) );
	aPresContext.ProbePseudoStyleContextFor(mOuter->mContent, atom, mOuter->mStyleContext,
										  PR_FALSE,
										  getter_AddRefs(hs));

  	atom = getter_AddRefs(NS_NewAtom(":-moz-vertical-box-debug"));
	aPresContext.ProbePseudoStyleContextFor(mOuter->mContent, atom, mOuter->mStyleContext,
										  PR_FALSE,
										  getter_AddRefs(vs));

    if (hs && vs) {
        if (!mDebugInner) {
            mDebugInner = new nsBoxDebugInner(mOuter);
            NS_ADDREF(mDebugInner);
            mDebugInner->AddListener();
        }
        mDebugInner->mHorizontalDebugStyle = hs;
        mDebugInner->mVerticalDebugStyle = vs;
        aPresContext.GetScaledPixelsToTwips(&mDebugInner->mP2t);
    } else {
        if (mDebugInner) 
        {
            mDebugInner->RemoveListener();
            NS_RELEASE(mDebugInner);
            mDebugInner = nsnull;
        }
    }
}

void
nsBoxFrameInner::GetDebugInset(nsMargin& inset)
{
    inset.SizeTo(0,0,0,0);

    if (mDebugInner) 
    {
        nsIStyleContext* style;
        if (mOuter->mHorizontal)
            style = mDebugInner->mHorizontalDebugStyle;
        else
            style = mDebugInner->mVerticalDebugStyle;

        const nsStyleSpacing* debugSpacing =
        (const nsStyleSpacing*)style->GetStyleData(eStyleStruct_Spacing);

        debugSpacing->GetBorderPadding(inset);
        nsMargin margin(0,0,0,0);
        debugSpacing->GetMargin(margin);
        inset += margin;
    }
}

NS_IMETHODIMP nsBoxFrame::QueryInterface(REFNSIID aIID, void** aInstancePtr)      
{           
  if (NULL == aInstancePtr) {                                            
    return NS_ERROR_NULL_POINTER;                                        
  }                                                                      
                                                                         
  *aInstancePtr = NULL;                                                  
                                                                                        
  if (aIID.Equals(kIBoxIID)) {                                         
    *aInstancePtr = (void*)(nsIBox*) this;                                        
    NS_ADDREF_THIS();                                                    
    return NS_OK;                                                        
  }   

  return nsHTMLContainerFrame::QueryInterface(aIID, aInstancePtr);                                     
}

NS_IMETHODIMP_(nsrefcnt) 
nsBoxFrame::AddRef(void)
{
  return NS_OK;
}

NS_IMETHODIMP_(nsrefcnt) 
nsBoxFrame::Release(void)
{
    return NS_OK;
}

NS_IMETHODIMP
nsBoxFrame::GetFrameName(nsString& aResult) const
{
  aResult = "Box";
  return NS_OK;
}

nsCalculatedBoxInfo::nsCalculatedBoxInfo()
{
   clear();
}

nsCalculatedBoxInfo::nsCalculatedBoxInfo(const nsBoxInfo& aInfo):nsBoxInfo(aInfo)
{
    needsReflow = PR_TRUE;
    needsRecalc = PR_TRUE;
    collapsed = PR_FALSE;

    calculatedSize.width = 0;
    calculatedSize.height = 0;

    sizeValid = PR_FALSE;
    isIncremental = PR_FALSE;
}

void 
nsCalculatedBoxInfo::clear()
{       
    nsBoxInfo::clear();
    needsReflow = PR_TRUE;
    needsRecalc = PR_TRUE;
    collapsed = PR_FALSE;

    calculatedSize.width = 0;
    calculatedSize.height = 0;

    prefWidthIntrinsic = PR_FALSE;
    prefHeightIntrinsic = PR_FALSE;

    sizeValid = PR_FALSE;
}



NS_IMETHODIMP  
nsBoxFrame::GetFrameForPoint(const nsPoint& aPoint, 
                             nsIFrame**     aFrame)
{   
    nsresult rv = nsHTMLContainerFrame::GetFrameForPoint(aPoint, aFrame);
    return rv;
}

void
nsBoxDebugInner::GetValue(const nsSize& a, const nsSize& b, char* ch) 
{
    char width[100];
    char height[100];
    
    if (a.width == NS_INTRINSICSIZE)
        sprintf(width,"%s","INF");
    else 
        sprintf(width,"%d", nscoord(a.width/mP2t));
    
    if (a.height == NS_INTRINSICSIZE)
        sprintf(height,"%s","INF");
    else 
        sprintf(height,"%d", nscoord(a.height/mP2t));
    

    sprintf(ch, "(%s%s, %s%s)", width, (b.width != NS_INTRINSICSIZE ? "[CSS]" : ""),
                    height, (b.height != NS_INTRINSICSIZE ? "[CSS]" : ""));

}

void
nsBoxDebugInner::GetValue(float a, float b, char* ch) 
{
    sprintf(ch, "(%f)", a);             
}

void
nsBoxDebugInner::AddListener()
{
  nsCOMPtr<nsIContent> content;
  mOuter->GetContent(getter_AddRefs(content));

  nsCOMPtr<nsIDOMEventReceiver> reciever(do_QueryInterface(content));

  reciever->AddEventListenerByIID(NS_STATIC_CAST(nsIDOMMouseMotionListener*,this), kIDOMMouseMotionListenerIID);
}

void
nsBoxDebugInner::RemoveListener()
{
  nsCOMPtr<nsIContent> content;
  mOuter->GetContent(getter_AddRefs(content));

  nsCOMPtr<nsIDOMEventReceiver> reciever(do_QueryInterface(content));

  reciever->RemoveEventListenerByIID(NS_STATIC_CAST(nsIDOMMouseMotionListener*,this),kIDOMMouseMotionListenerIID);
}

nsresult
nsBoxDebugInner::MouseMove(nsIDOMEvent* aMouseEvent)
{

          nsCOMPtr<nsIDOMUIEvent> uiEvent(do_QueryInterface(aMouseEvent));

          PRInt32 xPx = 0;
          PRInt32 yPx = 0;
          uiEvent->GetClientX(&xPx);
          uiEvent->GetClientY(&yPx);
              
           nscoord onePixel = NSIntPixelsToTwips(1, mP2t);
           nscoord x = xPx*onePixel;
           nscoord y = yPx*onePixel;

           // get it into our coordintate system by subtracting our parents offsets.
           nsIFrame* parent = mOuter;
           while(parent != nsnull)
           {
              // if we hit a scrollable view make sure we take into account
              // how much we are scrolled.
              nsIScrollableView* scrollingView;
              nsIView*           view;
              parent->GetView(&view);
              if (view) {
                nsresult result = view->QueryInterface(kScrollViewIID, (void**)&scrollingView);
                if (NS_SUCCEEDED(result)) {
                    nscoord xoff = 0;
                    nscoord yoff = 0;
                    scrollingView->GetScrollPosition(xoff, yoff);
                    x += xoff;
                    y += yoff;         
                }
              }
       
             nsRect r;
             parent->GetRect(r);
             x -= r.x;
             y -= r.y;
             parent->GetParent(&parent);
           }

        int count = 0;
        nsIFrame* childFrame = mOuter->mFrames.FirstChild(); 
        while (nsnull != childFrame) 
        {    
            nsRect r(0,0,0,0);
            childFrame->GetRect(r);

            // if we are not in the child. But in the spring above the child.
            if (!r.Contains(x,y) && ((mOuter->mHorizontal && x >= r.x && x < r.x + r.width) ||
                (!mOuter->mHorizontal && y >= r.y && y < r.y + r.height))) {

                    // found it but we already showed it.
                    if (mDebugChild == childFrame)
                        return NS_OK;

                    nsCOMPtr<nsIContent> content;
                    childFrame->GetContent(getter_AddRefs(content));

                    nsString id;
                    content->GetAttribute(kNameSpaceID_None, nsHTMLAtoms::id, id);
                    char idValue[100];
                    id.ToCString(idValue,100);
                       
                    nsString kClass;
                    content->GetAttribute(kNameSpaceID_None, nsHTMLAtoms::kClass, kClass);
                    char kClassValue[100];
                    kClass.ToCString(kClassValue,100);

                    nsCOMPtr<nsIAtom> tag;
                    content->GetTag(*getter_AddRefs(tag));
                    nsString tagString;
                    tag->ToString(tagString);
                    char tagValue[100];
                    tagString.ToCString(tagValue,100);

                    printf("--------------------\n");
                    printf("Tag='%s', id='%s' class='%s'\n", tagValue, idValue, kClassValue);

                    mDebugChild = childFrame;
                    nsCalculatedBoxInfo aSize;
                    aSize.prefSize.width = NS_INTRINSICSIZE;
                    aSize.prefSize.height = NS_INTRINSICSIZE;

                    aSize.minSize.width = NS_INTRINSICSIZE;
                    aSize.minSize.height = NS_INTRINSICSIZE;

                    aSize.maxSize.width = NS_INTRINSICSIZE;
                    aSize.maxSize.height = NS_INTRINSICSIZE;

                    aSize.calculatedSize.width = NS_INTRINSICSIZE;
                    aSize.calculatedSize.height = NS_INTRINSICSIZE;

                    aSize.flex = -1.0;

                  // add in the css min, max, pref
                    const nsStylePosition* position;
                    nsresult rv = childFrame->GetStyleData(eStyleStruct_Position,
                                  (const nsStyleStruct*&) position);

                    // see if the width or height was specifically set
                    if (position->mWidth.GetUnit() == eStyleUnit_Coord)  {
                        aSize.prefSize.width = position->mWidth.GetCoordValue();
                    }

                    if (position->mHeight.GetUnit() == eStyleUnit_Coord) {
                        aSize.prefSize.height = position->mHeight.GetCoordValue();     
                    }
    
                    // same for min size. Unfortunately min size is always set to 0. So for now
                    // we will assume 0 means not set.
                    if (position->mMinWidth.GetUnit() == eStyleUnit_Coord) {
                        nscoord min = position->mMinWidth.GetCoordValue();
                        if (min != 0)
                           aSize.minSize.width = min;
                    }

                    if (position->mMinHeight.GetUnit() == eStyleUnit_Coord) {
                        nscoord min = position->mMinHeight.GetCoordValue();
                        if (min != 0)
                           aSize.minSize.height = min;
                    }

                    // and max
                    if (position->mMaxWidth.GetUnit() == eStyleUnit_Coord) {
                        nscoord max = position->mMaxWidth.GetCoordValue();
                        aSize.maxSize.width = max;
                    }

                    if (position->mMaxHeight.GetUnit() == eStyleUnit_Coord) {
                        nscoord max = position->mMaxHeight.GetCoordValue();
                        aSize.maxSize.height = max;
                    }

                    PRInt32 error;
                    nsAutoString value;

                    if (NS_CONTENT_ATTR_HAS_VALUE == content->GetAttribute(kNameSpaceID_None, nsXULAtoms::flex, value))
                    {
                        value.Trim("%");
                        // convert to a percent.
                        aSize.flex = value.ToFloat(&error)/float(100.0);
                    }

                    if (NS_CONTENT_ATTR_HAS_VALUE == content->GetAttribute(kNameSpaceID_None, nsHTMLAtoms::width, value))
                    {
                        nsHTMLValue aResult;
                        nsGenericHTMLElement::ParseValueOrPercent(value, aResult, eHTMLUnit_Pixel);
                        aSize.prefSize.width = NSIntPixelsToTwips(aResult.GetPixelValue(), mP2t);
                    }

                    if (NS_CONTENT_ATTR_HAS_VALUE == content->GetAttribute(kNameSpaceID_None, nsHTMLAtoms::height, value))
                    {
                        nsHTMLValue aResult;
                        nsGenericHTMLElement::ParseValueOrPercent(value, aResult, eHTMLUnit_Pixel);
                        aSize.prefSize.height = NSIntPixelsToTwips(aResult.GetPixelValue(), mP2t);
                    }

                    char min[100];
                    char pref[100];
                    char max[100];
                    char calc[100];
                    char flex[100];

                    GetValue(mOuter->mSprings[count].minSize,  aSize.minSize, min);
                    GetValue(mOuter->mSprings[count].prefSize, aSize.prefSize, pref);
                    GetValue(mOuter->mSprings[count].maxSize, aSize.maxSize, max);
                    GetValue(mOuter->mSprings[count].calculatedSize,  aSize.calculatedSize, calc);
                    GetValue(mOuter->mSprings[count].flex,  aSize.flex, flex);


                    printf("min%s, pref%s, max%s, actual%s, flex=%s\n", 
                        min,
                        pref,
                        max,
                        calc,
                        flex
                    );
                    return NS_OK;
                }
          
          nsresult rv = childFrame->GetNextSibling(&childFrame);
          NS_ASSERTION(rv == NS_OK,"failed to get next child");
          count++;
        }

       // mDebugChild = nsnull;

        return NS_OK;
}


