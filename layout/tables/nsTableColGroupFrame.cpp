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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */
#include "nsTableColGroupFrame.h"
#include "nsTableColFrame.h"
#include "nsTableFrame.h"
#include "nsIHTMLTableColElement.h"
#include "nsIReflowCommand.h"
#include "nsIStyleContext.h"
#include "nsStyleConsts.h"
#include "nsIPresContext.h"
#include "nsIHTMLContent.h"
#include "nsHTMLParts.h"
#include "nsIPtr.h"
#include "nsHTMLAtoms.h"
#include "nsHTMLIIDs.h"
#include "nsCOMPtr.h"
#include "nsCSSRendering.h"
#include "nsIPresShell.h"
#include "nsLayoutAtoms.h"

NS_DEF_PTR(nsIContent);

static NS_DEFINE_IID(kIHTMLTableColElementIID, NS_IHTMLTABLECOLELEMENT_IID);

NS_IMETHODIMP
nsTableColGroupFrame::InitNewFrames(nsIPresContext* aPresContext, nsIFrame* aChildList)
{
  nsresult rv=NS_OK;
  nsTableFrame* tableFrame=nsnull;
  rv = nsTableFrame::GetTableFrame(this, tableFrame);
  if ((NS_SUCCEEDED(rv)) && (nsnull!=tableFrame)) {
    // Process the newly added column frames
    for (nsIFrame* kidFrame = aChildList; nsnull != kidFrame; kidFrame->GetNextSibling(&kidFrame)) {
      const nsStyleDisplay* display;
      kidFrame->GetStyleData(eStyleStruct_Display, (const nsStyleStruct *&)display);
      if (NS_STYLE_DISPLAY_TABLE_COLUMN == display->mDisplay) {
        // Set the preliminary values for the column frame
        PRInt32 colIndex = mStartColIndex + mColCount;
        ((nsTableColFrame *)(kidFrame))->InitColFrame (colIndex);
        PRInt32 repeat = ((nsTableColFrame *)(kidFrame))->GetSpan();
        mColCount += repeat;
        for (PRInt32 i=0; i<repeat; i++) {
          tableFrame->AddColumnFrame((nsTableColFrame *)kidFrame);
        }
      }
    }
    // colgroup's span attribute is how many columns the group represents
    // in the absence of any COL children
    // Note that this is the correct, though perhaps unexpected, behavior for the span attribute.  
    // The spec says that if there are any COL children, the colgroup's span is ignored.
    if (0==mColCount)
    {
      nsIFrame *firstImplicitColFrame=nsnull;
      nsIFrame *prevColFrame=nsnull;
      nsAutoString colTag;
      nsHTMLAtoms::col->ToString(colTag);
      mColCount = GetSpan();
      for (PRInt32 colIndex=0; colIndex<mColCount; colIndex++)
      {
        nsIHTMLContent *col=nsnull;
        // create an implicit col
        rv = NS_CreateHTMLElement(&col, colTag);  // ADDREF: col++
        //XXX mark the col implicit
        mContent->AppendChildTo((nsIContent*)col, PR_FALSE);

        // Create a new col frame
        nsIFrame* colFrame;
        NS_NewTableColFrame(&colFrame);

        // Set its style context
        nsCOMPtr<nsIStyleContext> colStyleContext;
        aPresContext->ResolveStyleContextFor(col, mStyleContext,
                                            PR_TRUE,
                                            getter_AddRefs(colStyleContext));
        colFrame->Init(aPresContext, col, this, colStyleContext, nsnull);
        colFrame->SetInitialChildList(aPresContext, nsnull, nsnull);

        // Set nsColFrame-specific information
        PRInt32 absColIndex = mStartColIndex + colIndex;
        ((nsTableColFrame *)(colFrame))->InitColFrame (absColIndex);
        ((nsTableColFrame *)colFrame)->SetColumnIndex(absColIndex);
        tableFrame->AddColumnFrame((nsTableColFrame *)colFrame);

        //hook into list of children
        if (nsnull==firstImplicitColFrame)
          firstImplicitColFrame = colFrame;
        else
          prevColFrame->SetNextSibling(colFrame);
        prevColFrame = colFrame;
      }

      // hook new columns into col group child list
      mFrames.AppendFrames(nsnull, firstImplicitColFrame);
    }
    SetStyleContextForFirstPass(aPresContext);
  }
  return rv;
}

NS_IMETHODIMP
nsTableColGroupFrame::AppendNewFrames(nsIPresContext* aPresContext, nsIFrame* aChildList)
{
  if (nsnull!=aChildList)
    mFrames.AppendFrames(nsnull, aChildList);
  return NS_OK;
}

NS_IMETHODIMP
nsTableColGroupFrame::SetInitialChildList(nsIPresContext* aPresContext,
                                          nsIAtom*        aListName,
                                          nsIFrame*       aChildList)
{
  nsresult result = AppendNewFrames(aPresContext, aChildList);
  if (NS_OK==result)
    result = InitNewFrames(aPresContext, aChildList);
  return result;
}

// Helper function. It marks the table frame as dirty and generates
// a reflow command
nsresult
nsTableColGroupFrame::AddTableDirtyReflowCommand(nsIPresContext* aPresContext,
                                                 nsIPresShell&   aPresShell,
                                                 nsIFrame*       aTableFrame)
{
  nsFrameState      frameState;
  nsIFrame*         tableParentFrame;
  nsIReflowCommand* reflowCmd;
  nsresult          rv;

  // Mark the table frame as dirty
  aTableFrame->GetFrameState(&frameState);
  frameState |= NS_FRAME_IS_DIRTY;
  aTableFrame->SetFrameState(frameState);

  // Target the reflow comamnd at its parent frame
  aTableFrame->GetParent(&tableParentFrame);
  rv = NS_NewHTMLReflowCommand(&reflowCmd, tableParentFrame,
                               nsIReflowCommand::ReflowDirty);
  if (NS_SUCCEEDED(rv)) {
    // Add the reflow command
    rv = aPresShell.AppendReflowCommand(reflowCmd);
    NS_RELEASE(reflowCmd);
  }

  return rv;
}

NS_IMETHODIMP
nsTableColGroupFrame::AppendFrames(nsIPresContext* aPresContext,
                                   nsIPresShell&   aPresShell,
                                   nsIAtom*        aListName,
                                   nsIFrame*       aFrameList)
{
  // Append the new frames to our child list
  mFrames.AppendFrames(nsnull, aFrameList);
  
  // Reset the starting column index of the col groups that follow
  PRInt32 startingColIndex = mStartColIndex;
  startingColIndex += GetColumnCount(); // has the side effect of resetting all column indexes

  nsIFrame *nextColGroupFrame=nsnull;
  GetNextSibling(&nextColGroupFrame);
  while (nextColGroupFrame)
  {
    const nsStyleDisplay *display;
    nextColGroupFrame->GetStyleData(eStyleStruct_Display, (const nsStyleStruct *&)display);
    if (NS_STYLE_DISPLAY_TABLE_COLUMN_GROUP == display->mDisplay)
    {
      startingColIndex += ((nsTableColGroupFrame *)nextColGroupFrame)->SetStartColumnIndex(startingColIndex);
    }
    nextColGroupFrame->GetNextSibling(&nextColGroupFrame);
  }

  // Today we need to rebuild the whole column cache.
  // If the table frame is ever recoded to build the column cache incrementally,
  // we could take advantage of that here
  nsTableFrame* tableFrame;
  nsTableFrame::GetTableFrame(this, tableFrame);
  tableFrame->InvalidateColumnCache();
  
  // Generate a reflow command so we reflow the table
  AddTableDirtyReflowCommand(aPresContext, aPresShell, tableFrame);
  return NS_OK;
}

NS_IMETHODIMP
nsTableColGroupFrame::InsertFrames(nsIPresContext* aPresContext,
                                   nsIPresShell&   aPresShell,
                                   nsIAtom*        aListName,
                                   nsIFrame*       aPrevFrame,
                                   nsIFrame*       aFrameList)
{
  // Insert the new frames into our child list
  mFrames.InsertFrames(nsnull, aPrevFrame, aFrameList);

  // Reset the starting column index of the col groups that follow
  PRInt32 startingColIndex=mStartColIndex;
  startingColIndex += GetColumnCount(); // has the side effect of resetting all column indexes

  nsIFrame *nextColGroupFrame=nsnull;
  GetNextSibling(&nextColGroupFrame);
  while (nextColGroupFrame)
  {
    const nsStyleDisplay *display;
    nextColGroupFrame->GetStyleData(eStyleStruct_Display, (const nsStyleStruct *&)display);
    if (NS_STYLE_DISPLAY_TABLE_COLUMN_GROUP == display->mDisplay)
    {
      startingColIndex += ((nsTableColGroupFrame *)nextColGroupFrame)->SetStartColumnIndex(startingColIndex);
    }
    nextColGroupFrame->GetNextSibling(&nextColGroupFrame);
  }

  // Today we need to rebuild the whole column cache.
  // If the table frame is ever recoded to build the column cache incrementally,
  // we could take advantage of that here.
  nsTableFrame* tableFrame;
  nsTableFrame::GetTableFrame(this, tableFrame);
  tableFrame->InvalidateColumnCache();

  // Generate a reflow command so we reflow the table
  AddTableDirtyReflowCommand(aPresContext, aPresShell, tableFrame);
  return NS_OK;
}

NS_IMETHODIMP
nsTableColGroupFrame::RemoveFrame(nsIPresContext* aPresContext,
                                  nsIPresShell&   aPresShell,
                                  nsIAtom*        aListName,
                                  nsIFrame*       aOldFrame)
{
  // Remove the frame from our child list
  mFrames.DestroyFrame(aPresContext, aOldFrame);

  // Reset the starting column index of the col groups that follow
  PRInt32 startingColIndex=mStartColIndex;
  startingColIndex += GetColumnCount(); // has the side effect of resetting all column indexes

  nsIFrame *nextColGroupFrame=nsnull;
  GetNextSibling(&nextColGroupFrame);
  while (nextColGroupFrame)
  {
    const nsStyleDisplay *display;
    nextColGroupFrame->GetStyleData(eStyleStruct_Display, (const nsStyleStruct *&)display);
    if (NS_STYLE_DISPLAY_TABLE_COLUMN_GROUP == display->mDisplay)
    {
      startingColIndex += ((nsTableColGroupFrame *)nextColGroupFrame)->SetStartColumnIndex(startingColIndex);
    }
    nextColGroupFrame->GetNextSibling(&nextColGroupFrame);
  }

  // Today we need to rebuild the whole column cache.
  // If the table frame is ever recoded to build the column cache incrementally,
  // we could take advantage of that here
  nsTableFrame* tableFrame;
  nsTableFrame::GetTableFrame(this, tableFrame);
  tableFrame->InvalidateColumnCache();

  // Generate a reflow command so we reflow the table
  AddTableDirtyReflowCommand(aPresContext, aPresShell, tableFrame);
  return NS_OK;
}

NS_METHOD nsTableColGroupFrame::Paint(nsIPresContext*      aPresContext,
                                      nsIRenderingContext& aRenderingContext,
                                      const nsRect&        aDirtyRect,
                                      nsFramePaintLayer    aWhichLayer)
{
  if (NS_FRAME_PAINT_LAYER_BACKGROUND == aWhichLayer) {
    nsCompatibility mode;
    aPresContext->GetCompatibilityMode(&mode);
    if (eCompatibility_Standard == mode) {
      const nsStyleDisplay* disp =
        (const nsStyleDisplay*)mStyleContext->GetStyleData(eStyleStruct_Display);
      if (disp->mVisible) {
        const nsStyleSpacing* spacing =
          (const nsStyleSpacing*)mStyleContext->GetStyleData(eStyleStruct_Spacing);
        const nsStyleColor* color =
          (const nsStyleColor*)mStyleContext->GetStyleData(eStyleStruct_Color);
        nsRect rect(0, 0, mRect.width, mRect.height);
        nsCSSRendering::PaintBackground(aPresContext, aRenderingContext, this,
                                        aDirtyRect, rect, *color, *spacing, 0, 0);
      }
    }
  }
  PaintChildren(aPresContext, aRenderingContext, aDirtyRect, aWhichLayer);
  return NS_OK;
}

PRIntn
nsTableColGroupFrame::GetSkipSides() const
{
  PRIntn skip = 0;
  if (nsnull != mPrevInFlow) {
    skip |= 1 << NS_SIDE_TOP;
  }
  if (nsnull != mNextInFlow) {
    skip |= 1 << NS_SIDE_BOTTOM;
  }
  return skip;
}

NS_METHOD nsTableColGroupFrame::Reflow(nsIPresContext*          aPresContext,
                                       nsHTMLReflowMetrics&     aDesiredSize,
                                       const nsHTMLReflowState& aReflowState,
                                       nsReflowStatus&          aStatus)
{
  NS_ASSERTION(nsnull!=mContent, "bad state -- null content for frame");
  nsresult rv=NS_OK;
  // for every content child that (is a column thingy and does not already have a frame)
  // create a frame and adjust it's style
  nsIFrame* kidFrame = nsnull;
 
  if (eReflowReason_Incremental == aReflowState.reason) {
    rv = IncrementalReflow(aPresContext, aDesiredSize, aReflowState, aStatus);
  }

  for (kidFrame = mFrames.FirstChild(); nsnull != kidFrame;
       kidFrame->GetNextSibling(&kidFrame)) {
    // Give the child frame a chance to reflow, even though we know it'll have 0 size
    nsHTMLReflowMetrics kidSize(nsnull);
    // XXX Use a valid reason...
    nsHTMLReflowState kidReflowState(aPresContext, aReflowState, kidFrame,
                                     nsSize(0,0), eReflowReason_Initial);

    nsReflowStatus status;
    ReflowChild(kidFrame, aPresContext, kidSize, kidReflowState, 0, 0, 0, status);
    FinishReflowChild(kidFrame, aPresContext, kidSize, 0, 0, 0);
  }

  aDesiredSize.width=0;
  aDesiredSize.height=0;
  aDesiredSize.ascent=aDesiredSize.height;
  aDesiredSize.descent=0;
  if (nsnull!=aDesiredSize.maxElementSize)
  {
    aDesiredSize.maxElementSize->width=0;
    aDesiredSize.maxElementSize->height=0;
  }
  aStatus = NS_FRAME_COMPLETE;
  return rv;
}

NS_METHOD nsTableColGroupFrame::IncrementalReflow(nsIPresContext*          aPresContext,
                                                  nsHTMLReflowMetrics&     aDesiredSize,
                                                  const nsHTMLReflowState& aReflowState,
                                                  nsReflowStatus&          aStatus)
{
  nsresult  rv = NS_OK;

  // determine if this frame is the target or not
  nsIFrame *target=nsnull;
  rv = aReflowState.reflowCommand->GetTarget(target);
  if ((PR_TRUE==NS_SUCCEEDED(rv)) && (nsnull!=target))
  {
    if (this==target)
      rv = IR_TargetIsMe(aPresContext, aDesiredSize, aReflowState, aStatus);
    else
    {
      // Get the next frame in the reflow chain
      nsIFrame* nextFrame;
      aReflowState.reflowCommand->GetNext(nextFrame);
      rv = IR_TargetIsChild(aPresContext, aDesiredSize, aReflowState, aStatus, nextFrame);
    }
  }
  return rv;
}

NS_METHOD nsTableColGroupFrame::IR_TargetIsMe(nsIPresContext*          aPresContext,
                                              nsHTMLReflowMetrics&     aDesiredSize,
                                              const nsHTMLReflowState& aReflowState,
                                              nsReflowStatus&          aStatus)
{
  nsresult rv = NS_OK;
  aStatus = NS_FRAME_COMPLETE;
  nsIReflowCommand::ReflowType type;
  aReflowState.reflowCommand->GetType(type);
  nsIFrame *objectFrame;
  aReflowState.reflowCommand->GetChildFrame(objectFrame); 
  const nsStyleDisplay *childDisplay=nsnull;
  if (nsnull!=objectFrame)
    objectFrame->GetStyleData(eStyleStruct_Display, ((const nsStyleStruct *&)childDisplay));
  switch (type)
  {
  case nsIReflowCommand::StyleChanged :
    rv = IR_StyleChanged(aPresContext, aDesiredSize, aReflowState, aStatus);
    break;

  case nsIReflowCommand::ContentChanged :
    NS_ASSERTION(PR_FALSE, "illegal reflow type: ContentChanged");
    rv = NS_ERROR_ILLEGAL_VALUE;
    break;
  
  default:
    NS_NOTYETIMPLEMENTED("unexpected reflow command type");
    rv = NS_ERROR_NOT_IMPLEMENTED;
    break;
  }

  return rv;
}

NS_METHOD nsTableColGroupFrame::IR_StyleChanged(nsIPresContext*          aPresContext,
                                                nsHTMLReflowMetrics&     aDesiredSize,
                                                const nsHTMLReflowState& aReflowState,
                                                nsReflowStatus&          aStatus)
{
  nsresult rv = NS_OK;
  // we presume that all the easy optimizations were done in the nsHTMLStyleSheet before we were called here
  // XXX: we can optimize this when we know which style attribute changed
  nsTableFrame* tableFrame=nsnull;
  rv = nsTableFrame::GetTableFrame(this, tableFrame);
  if ((NS_SUCCEEDED(rv)) && (nsnull!=tableFrame))
  {
    tableFrame->InvalidateColumnCache();
    tableFrame->InvalidateFirstPassCache();
  }
  return rv;
}

NS_METHOD nsTableColGroupFrame::IR_TargetIsChild(nsIPresContext*          aPresContext,
                                                 nsHTMLReflowMetrics&     aDesiredSize,
                                                 const nsHTMLReflowState& aReflowState,
                                                 nsReflowStatus&          aStatus,
                                                 nsIFrame *               aNextFrame)
{
  nsresult rv;
 
  // Remember the old col count
  const PRInt32 oldColCount = GetColumnCount();

  // Pass along the reflow command
  nsHTMLReflowMetrics desiredSize(nsnull);
  nsHTMLReflowState kidReflowState(aPresContext, aReflowState, aNextFrame,
                                   nsSize(aReflowState.availableWidth,
                                          aReflowState.availableHeight));
  rv = ReflowChild(aNextFrame, aPresContext, desiredSize, kidReflowState, 0, 0, 0, aStatus);
  aNextFrame->DidReflow(aPresContext, NS_FRAME_REFLOW_FINISHED);
  if (NS_FAILED(rv))
    return rv;

  nsTableFrame *tableFrame=nsnull;
  rv = nsTableFrame::GetTableFrame(this, tableFrame);
  if ((NS_SUCCEEDED(rv)) && (nsnull!=tableFrame))
  {
    // compare the new col count to the old col count.  
    // If they are the same, we just need to rebalance column widths
    // If they differ, we need to fix up other column groups and the column cache
    const PRInt32 newColCount = GetColumnCount(); // this will set the new column indexes if necessary
    if (oldColCount==newColCount)
      tableFrame->InvalidateColumnWidths();
    else
      tableFrame->InvalidateColumnCache();
  }
  return rv;
}

// Subclass hook for style post processing
NS_METHOD nsTableColGroupFrame::SetStyleContextForFirstPass(nsIPresContext* aPresContext)
{
  // get the table frame
  nsTableFrame* tableFrame=nsnull;
  nsresult rv = nsTableFrame::GetTableFrame(this, tableFrame);
  if ((NS_SUCCEEDED(rv)) && (nsnull!=tableFrame))
  {  
    // get the style for the table frame
    const nsStyleTable *tableStyle;
    tableFrame->GetStyleData(eStyleStruct_Table, (const nsStyleStruct *&)tableStyle);

    // if COLS is set, then map it into the COL frames
    if (NS_STYLE_TABLE_COLS_NONE != tableStyle->mCols)
    {
      // set numCols to the number of columns effected by the COLS attribute
      PRInt32 numCols=0;
      if (NS_STYLE_TABLE_COLS_ALL == tableStyle->mCols)
        numCols = mFrames.GetLength();
      else 
        numCols = tableStyle->mCols;

      // for every column effected, set its width style
      PRInt32 colIndex=0;
      nsIFrame *colFrame=mFrames.FirstChild();
      while (nsnull!=colFrame)
      {
        const nsStyleDisplay * colDisplay=nsnull;
        colFrame->GetStyleData(eStyleStruct_Display, ((const nsStyleStruct *&)colDisplay));
        if (NS_STYLE_DISPLAY_TABLE_COLUMN == colDisplay->mDisplay)
        {
          nsCOMPtr<nsIStyleContext> colStyleContext;
          nsStylePosition * colPosition=nsnull;
          colFrame->GetStyleContext(getter_AddRefs(colStyleContext));
          colPosition = (nsStylePosition*)colStyleContext->GetMutableStyleData(eStyleStruct_Position);
          if (colIndex<numCols)
          {
            nsStyleCoord width (1, eStyleUnit_Proportional);
            colPosition->mWidth = width;
          }
          else
          {
            colPosition->mWidth.SetCoordValue(0);
          }
          colStyleContext->RecalcAutomaticData(aPresContext);
          colIndex++;
        }
        colFrame->GetNextSibling(&colFrame);
      }
    }
    else
    {
      // propagate the colgroup width attribute down to the columns if they have no width of their own
      nsStylePosition* position = (nsStylePosition*)mStyleContext->GetStyleData(eStyleStruct_Position);
      if (eStyleUnit_Null!=position->mWidth.GetUnit())
      {
        // now for every column that doesn't have it's own width, set the width style
        nsIFrame *colFrame=mFrames.FirstChild();
        while (nsnull!=colFrame)
        {
          const nsStyleDisplay * colDisplay=nsnull;
          colFrame->GetStyleData(eStyleStruct_Display, ((const nsStyleStruct *&)colDisplay));
          if (NS_STYLE_DISPLAY_TABLE_COLUMN == colDisplay->mDisplay)
          {
            nsCOMPtr<nsIStyleContext> colStyleContext;
            const nsStylePosition * colPosition=nsnull;
            colFrame->GetStyleData(eStyleStruct_Position, (const nsStyleStruct *&)colPosition); // get a read-only version of the style context
            //XXX: how do I know this is auto because it's defaulted, vs. set explicitly to "auto"?
            if (eStyleUnit_Auto==colPosition->mWidth.GetUnit())
            {
              // notice how we defer getting a mutable style context until we're sure we really need one
              colFrame->GetStyleContext(getter_AddRefs(colStyleContext));
              nsStylePosition * mutableColPosition = (nsStylePosition*)colStyleContext->GetMutableStyleData(eStyleStruct_Position);
              mutableColPosition->mWidth = position->mWidth;
              colStyleContext->RecalcAutomaticData(aPresContext);
            }
          }
          colFrame->GetNextSibling(&colFrame);
        }
      }
    }
    //mStyleContext->RecalcAutomaticData(aPresContext);
  }
  return rv;
}


/** returns the number of columns represented by this group.
  * if there are col children, count them (taking into account the span of each)
  * else, check my own span attribute.
  */
int nsTableColGroupFrame::GetColumnCount ()
{
  mColCount=0;
  nsIFrame *childFrame = mFrames.FirstChild();
  while (nsnull!=childFrame)
  {
    const nsStyleDisplay *childDisplay;
    childFrame->GetStyleData(eStyleStruct_Display, ((const nsStyleStruct *&)childDisplay));
    if (NS_STYLE_DISPLAY_TABLE_COLUMN == childDisplay->mDisplay)
    {
      nsTableColFrame *col = (nsTableColFrame *)childFrame;
      col->SetColumnIndex (mStartColIndex + mColCount);
      mColCount += col->GetSpan();
    }
    childFrame->GetNextSibling(&childFrame);
  }
  if (0==mColCount)
  { // there were no children of this colgroup that were columns.  So use my span attribute
    const nsStyleTable *tableStyle;
    GetStyleData(eStyleStruct_Table, (const nsStyleStruct *&)tableStyle);
    mColCount = tableStyle->mSpan;
  }
  return mColCount;
}

nsTableColFrame * nsTableColGroupFrame::GetFirstColumn()
{
  return GetNextColumn(nsnull);
}

nsTableColFrame * nsTableColGroupFrame::GetNextColumn(nsIFrame *aChildFrame)
{
  nsTableColFrame *result = nsnull;
  nsIFrame *childFrame = aChildFrame;
  if (nsnull==childFrame)
    childFrame = mFrames.FirstChild();
  while (nsnull!=childFrame)
  {
    const nsStyleDisplay *childDisplay;
    childFrame->GetStyleData(eStyleStruct_Display, ((const nsStyleStruct *&)childDisplay));
    if (NS_STYLE_DISPLAY_TABLE_COLUMN == childDisplay->mDisplay)
    {
      result = (nsTableColFrame *)childFrame;
      break;
    }
    childFrame->GetNextSibling(&childFrame);
  }
  return result;
}


nsTableColFrame * nsTableColGroupFrame::GetColumnAt (PRInt32 aColIndex)
{
  nsTableColFrame *result = nsnull;
  PRInt32 count = 0;
  nsIFrame *childFrame = mFrames.FirstChild();

  while (nsnull!=childFrame) {
    const nsStyleDisplay *childDisplay;
    childFrame->GetStyleData(eStyleStruct_Display, ((const nsStyleStruct *&)childDisplay));
    if (NS_STYLE_DISPLAY_TABLE_COLUMN == childDisplay->mDisplay) {
      nsTableColFrame *col = (nsTableColFrame *)childFrame;
      count += col->GetSpan();
      if (aColIndex<=count) {
        result = col;
      }
    }
    childFrame->GetNextSibling(&childFrame);
  }

  return result;
}

PRInt32 nsTableColGroupFrame::GetSpan()
{
  PRInt32 span=1;
  const nsStyleTable* tableStyle=nsnull;
  GetStyleData(eStyleStruct_Table, (const nsStyleStruct *&)tableStyle);    
  if (nsnull!=tableStyle)
  {
    span = tableStyle->mSpan;
  }
  return span;
}

/* this may be needed when IsSynthetic is properly implemented 
PRBool nsTableColGroupFrame::IsManufactured()
{
  PRBool result = PR_FALSE;
  nsIFrame *firstCol =  GetFirstColumn();
  if (nsTableFrame::IsSynthetic(this) && 
      ((nsnull==firstCol) || nsTableFrame::IsSynthetic(firstCol)))
    result = PR_TRUE;
  return result;
}
*/

/** returns colcount because it is frequently used in the context of 
  * shuffling relative colgroup order, and it's convenient to not have to
  * call GetColumnCount redundantly.
  */
PRInt32 nsTableColGroupFrame::SetStartColumnIndex (int aIndex)
{
  PRInt32 result = mColCount;
  if (aIndex != mStartColIndex)
  {  
    mStartColIndex = aIndex;
    mColCount=0;
    result = GetColumnCount(); // has the side effect of setting each column index based on new start index
  }
  return result;
}

void nsTableColGroupFrame::DeleteColFrame(nsIPresContext* aPresContext, nsTableColFrame* aColFrame)
{
  mFrames.DestroyFrame(aPresContext, aColFrame);
}

/* ----- global methods ----- */

nsresult 
NS_NewTableColGroupFrame(nsIFrame** aNewFrame)
{
  NS_PRECONDITION(aNewFrame, "null OUT ptr");
  if (nsnull == aNewFrame) {
    return NS_ERROR_NULL_POINTER;
  }
  nsTableColGroupFrame* it = new nsTableColGroupFrame;
  if (nsnull == it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  *aNewFrame = it;
  return NS_OK;
}

NS_IMETHODIMP
nsTableColGroupFrame::GetFrameType(nsIAtom** aType) const
{
  NS_PRECONDITION(nsnull != aType, "null OUT parameter pointer");
  *aType = nsLayoutAtoms::tableColGroupFrame; 
  NS_ADDREF(*aType);
  return NS_OK;
}

#ifdef DEBUG
NS_IMETHODIMP
nsTableColGroupFrame::GetFrameName(nsString& aResult) const
{
  return MakeFrameName("TableColGroup", aResult);
}

NS_IMETHODIMP
nsTableColGroupFrame::SizeOf(nsISizeOfHandler* aHandler, PRUint32* aResult) const
{
  if (!aResult) {
    return NS_ERROR_NULL_POINTER;
  }
  PRUint32 sum = sizeof(*this);
  *aResult = sum;
  return NS_OK;
}
#endif
