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
#include "nsCOMPtr.h"
#include "nsImageFrame.h"
#include "nsFormControlHelper.h"
#include "nsIFormControlFrame.h"
#include "nsIFormControl.h"
#include "nsHTMLParts.h"
#include "nsIRenderingContext.h"
#include "nsIPresContext.h"
#include "nsIPresShell.h"
#include "nsIStyleContext.h"
#include "nsLeafFrame.h"
#include "nsCSSRendering.h"
#include "nsHTMLIIDs.h"
#include "nsIViewManager.h"
#include "nsISupports.h"
#include "nsHTMLAtoms.h"
#include "nsIView.h"
#include "nsViewsCID.h"
#include "nsIDeviceContext.h"
#include "nsIFontMetrics.h"
#include "nsIImage.h"
#include "nsStyleUtil.h"
#include "nsStyleConsts.h"
#include "nsFormFrame.h"
#include "nsFormControlFrame.h"

//Enumeration of possible mouse states used to detect mouse clicks
/*enum nsMouseState {
  eMouseNone,
  eMouseEnter,
  eMouseExit,
  eMouseDown,
  eMouseUp
};
*/
static NS_DEFINE_IID(kViewCID, NS_VIEW_CID);

#define nsImageControlFrameSuper nsImageFrame
class nsImageControlFrame : public nsImageControlFrameSuper,
                            public nsIFormControlFrame 
{
public:
  nsImageControlFrame();
  ~nsImageControlFrame();

  NS_IMETHOD Destroy(nsIPresContext *aPresContext);
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aInstancePtr);

  NS_IMETHOD Init(nsIPresContext*  aPresContext,
                  nsIContent*      aContent,
                  nsIFrame*        aParent,
                  nsIStyleContext* aContext,
                  nsIFrame*        aPrevInFlow);

  NS_IMETHOD Reflow(nsIPresContext*          aPresContext,
                    nsHTMLReflowMetrics&     aDesiredSize,
                    const nsHTMLReflowState& aReflowState,
                    nsReflowStatus&          aStatus);

  NS_IMETHOD HandleEvent(nsIPresContext* aPresContext, 
                         nsGUIEvent* aEvent,
                         nsEventStatus* aEventStatus);

#ifdef DEBUG
  NS_IMETHOD GetFrameName(nsString& aResult) const {
    return MakeFrameName("ImageControl", aResult);
  }
#endif

  NS_IMETHOD GetCursor(nsIPresContext* aPresContext,
                       nsPoint&        aPoint,
                       PRInt32&        aCursor);

  virtual void MouseClicked(nsIPresContext* aPresContext);

  virtual void SetFormFrame(nsFormFrame* aFormFrame) { mFormFrame = aFormFrame; }

  PRBool IsSuccessful(nsIFormControlFrame* aSubmitter);

  virtual PRInt32 GetMaxNumValues();

  virtual PRBool GetNamesValues(PRInt32 aMaxNumValues, PRInt32& aNumValues,
                                nsString* aValues, nsString* aNames);

  NS_IMETHOD GetType(PRInt32* aType) const;

  NS_IMETHOD GetName(nsString* aName);

  virtual void Reset(nsIPresContext*) {};

  void SetFocus(PRBool aOn, PRBool aRepaint);
  void ScrollIntoView(nsIPresContext* aPresContext);
  NS_IMETHOD GetFont(nsIPresContext* aPresContext, 
                     const nsFont*&  aFont);

  NS_IMETHOD GetFormContent(nsIContent*& aContent) const;
  virtual nscoord GetVerticalInsidePadding(nsIPresContext* aPresContext,
                                           float aPixToTwip,
                                           nscoord aInnerHeight) const;
  virtual nscoord GetHorizontalInsidePadding(nsIPresContext* aPresContext,
                                             float aPixToTwip, 
                                             nscoord aInnerWidth,
                                             nscoord aCharWidth) const;

  virtual nsresult RequiresWidget(PRBool &aRequiresWidget);


        // nsIFormControlFrame
  NS_IMETHOD SetProperty(nsIPresContext* aPresContext, nsIAtom* aName, const nsAReadableString& aValue);
  NS_IMETHOD GetProperty(nsIAtom* aName, nsAWritableString& aValue); 
  NS_IMETHOD SetSuggestedSize(nscoord aWidth, nscoord aHeight);

protected:
  void GetTranslatedRect(nsIPresContext* aPresContext, nsRect& aRect); // XXX this implementation is a copy of nsHTMLButtonControlFrame
  NS_IMETHOD_(nsrefcnt) AddRef(void);
  NS_IMETHOD_(nsrefcnt) Release(void);

  nsFormFrame* mFormFrame;
  nsMouseState mLastMouseState;
  nsPoint mLastClickPoint; 
  nsCursor mPreviousCursor;
  nsRect mTranslatedRect;
  PRBool mGotFocus;

};


nsImageControlFrame::nsImageControlFrame()
{
  mLastMouseState = eMouseNone;
  mLastClickPoint = nsPoint(0,0);
  mPreviousCursor = eCursor_standard;
  mTranslatedRect = nsRect(0,0,0,0);
  mGotFocus       = PR_FALSE;
  mFormFrame      = nsnull;
}

nsImageControlFrame::~nsImageControlFrame()
{
}

NS_IMETHODIMP
nsImageControlFrame::Destroy(nsIPresContext *aPresContext)
{
  nsFormControlFrame::RegUnRegAccessKey(aPresContext, NS_STATIC_CAST(nsIFrame*, this), PR_FALSE);

  if (mFormFrame) {
    mFormFrame->RemoveFormControlFrame(*this);
    mFormFrame = nsnull;
  }
  return nsImageControlFrameSuper::Destroy(aPresContext);
}

nsresult
NS_NewImageControlFrame(nsIPresShell* aPresShell, nsIFrame** aNewFrame)
{
  NS_PRECONDITION(aNewFrame, "null OUT ptr");
  if (nsnull == aNewFrame) {
    return NS_ERROR_NULL_POINTER;
  }
  nsImageControlFrame* it = new (aPresShell) nsImageControlFrame;
  if (!it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  *aNewFrame = it;
  return NS_OK;
}

// Frames are not refcounted, no need to AddRef
NS_IMETHODIMP
nsImageControlFrame::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
  NS_PRECONDITION(0 != aInstancePtr, "null ptr");
  if (NULL == aInstancePtr) {
    return NS_ERROR_NULL_POINTER;
  }
  if (aIID.Equals(NS_GET_IID(nsIFormControlFrame))) {
    *aInstancePtr = (void*) ((nsIFormControlFrame*) this);
    return NS_OK;
  }
  return nsImageControlFrameSuper::QueryInterface(aIID, aInstancePtr);
}

nsrefcnt nsImageControlFrame::AddRef(void)
{
  NS_WARNING("not supported");
  return 1;
}

nsrefcnt nsImageControlFrame::Release(void)
{
  NS_WARNING("not supported");
  return 1;
}

NS_IMETHODIMP
nsImageControlFrame::Init(nsIPresContext*  aPresContext,
                          nsIContent*      aContent,
                          nsIFrame*        aParent,
                          nsIStyleContext* aContext,
                          nsIFrame*        aPrevInFlow)
{
  // call our base class
  nsresult  rv = nsImageControlFrameSuper::Init(aPresContext, aContent, aParent,
                                                aContext, aPrevInFlow);
  
  // create our view, we need a view to grab the mouse 
  nsIView* view;
  GetView(aPresContext, &view);
  if (!view) {
    nsresult result = nsComponentManager::CreateInstance(kViewCID, nsnull, NS_GET_IID(nsIView), (void **)&view);
    nsCOMPtr<nsIPresShell> presShell;
    aPresContext->GetShell(getter_AddRefs(presShell));
    nsCOMPtr<nsIViewManager> viewMan;
    presShell->GetViewManager(getter_AddRefs(viewMan));

    nsIFrame* parWithView;
    nsIView *parView;
    GetParentWithView(aPresContext, &parWithView);
    parWithView->GetView(aPresContext, &parView);
    // the view's size is not know yet, but its size will be kept in synch with our frame.
    nsRect boundBox(0, 0, 0, 0); 
    result = view->Init(viewMan, boundBox, parView);
    view->SetContentTransparency(PR_TRUE);
    viewMan->InsertChild(parView, view, 0);
    SetView(aPresContext, view);

    const nsStyleColor* color = (const nsStyleColor*) mStyleContext->GetStyleData(eStyleStruct_Color);
    // set the opacity
    viewMan->SetViewOpacity(view, color->mOpacity);
  }

  return rv;
}

NS_METHOD
nsImageControlFrame::Reflow(nsIPresContext*         aPresContext,
                           nsHTMLReflowMetrics&     aDesiredSize,
                           const nsHTMLReflowState& aReflowState,
                           nsReflowStatus&          aStatus)
{
  DO_GLOBAL_REFLOW_COUNT("nsImageControlFrame", aReflowState.reason);
  if (!mFormFrame && (eReflowReason_Initial == aReflowState.reason)) {
    nsFormControlFrame::RegUnRegAccessKey(aPresContext, NS_STATIC_CAST(nsIFrame*, this), PR_TRUE);
    // add ourself as an nsIFormControlFrame
    nsFormFrame::AddFormControlFrame(aPresContext, *NS_STATIC_CAST(nsIFrame*, this));
  }
  return nsImageControlFrameSuper::Reflow(aPresContext, aDesiredSize, aReflowState, aStatus);
}

NS_METHOD 
nsImageControlFrame::HandleEvent(nsIPresContext* aPresContext, 
                                 nsGUIEvent* aEvent,
                                 nsEventStatus* aEventStatus)
{
  NS_ENSURE_ARG_POINTER(aEventStatus);

  // do we have user-input style?
  const nsStyleUserInterface* uiStyle;
  GetStyleData(eStyleStruct_UserInterface,  (const nsStyleStruct *&)uiStyle);
  if (uiStyle->mUserInput == NS_STYLE_USER_INPUT_NONE || uiStyle->mUserInput == NS_STYLE_USER_INPUT_DISABLED)
    return nsFrame::HandleEvent(aPresContext, aEvent, aEventStatus);

  if (nsFormFrame::GetDisabled(this)) { // XXX cache disabled
    return NS_OK;
  }

  *aEventStatus = nsEventStatus_eIgnore;

  switch (aEvent->message) {
    case NS_MOUSE_LEFT_BUTTON_UP:
    {
      // Store click point for GetNamesValues
      // Do this on MouseUp because the specs don't say and that's what IE does.
      float t2p;
      aPresContext->GetTwipsToPixels(&t2p);
      mLastClickPoint.x = NSTwipsToIntPixels(aEvent->point.x, t2p);
      mLastClickPoint.y = NSTwipsToIntPixels(aEvent->point.y, t2p);

      mGotFocus = PR_TRUE;
      break;
    }
  }
  return nsImageControlFrameSuper::HandleEvent(aPresContext, aEvent,
                                               aEventStatus);
}

void 
nsImageControlFrame::SetFocus(PRBool aOn, PRBool aRepaint)
{
  mGotFocus = aOn;
  /*if (aRepaint) {
    nsRect rect(0, 0, mRect.width, mRect.height);
    Invalidate(rect, PR_TRUE);
  }*/
}

void
nsImageControlFrame::ScrollIntoView(nsIPresContext* aPresContext)
{
  if (aPresContext) {
    nsCOMPtr<nsIPresShell> presShell;
    aPresContext->GetShell(getter_AddRefs(presShell));
    if (presShell) {
      presShell->ScrollFrameIntoView(this,
                   NS_PRESSHELL_SCROLL_IF_NOT_VISIBLE,NS_PRESSHELL_SCROLL_IF_NOT_VISIBLE);
    }
  }
}

void
nsImageControlFrame::GetTranslatedRect(nsIPresContext* aPresContext, nsRect& aRect)
{
  nsIView* view;
  nsPoint viewOffset(0,0);
  GetOffsetFromView(aPresContext, viewOffset, &view);
  while (nsnull != view) {
    nsPoint tempOffset;
    view->GetPosition(&tempOffset.x, &tempOffset.y);
    viewOffset += tempOffset;
    view->GetParent(view);
  }
  aRect = nsRect(viewOffset.x, viewOffset.y, mRect.width, mRect.height);
}

NS_IMETHODIMP
nsImageControlFrame::GetType(PRInt32* aType) const
{
  *aType = NS_FORM_INPUT_IMAGE;
  return NS_OK;
}

NS_IMETHODIMP
nsImageControlFrame::GetName(nsString* aResult)
{
  if (nsnull == aResult) {
    return NS_OK;
  } else {
    return nsFormFrame::GetName(this, *aResult);
  }
}

PRBool
nsImageControlFrame::IsSuccessful(nsIFormControlFrame* aSubmitter)
{
  // Image control will only add it's value if it is clicked on.
  // XXX Is this right?
  return (this == (aSubmitter));
}

PRInt32
nsImageControlFrame::GetMaxNumValues() 
{
  return 2;
}


PRBool
nsImageControlFrame::GetNamesValues(PRInt32 aMaxNumValues, PRInt32& aNumValues,
                                     nsString* aValues, nsString* aNames)
{
  if (aMaxNumValues <= 0) {
    return PR_FALSE;
  }

  char buf[20];
  aNumValues = 2;

  sprintf(&buf[0], "%d", mLastClickPoint.x);
  aValues[0].AssignWithConversion(&buf[0]);

  sprintf(&buf[0], "%d", mLastClickPoint.y);
  aValues[1].AssignWithConversion(&buf[0]);

  nsAutoString name;
  nsresult result = GetName(&name);
  if (NS_CONTENT_ATTR_HAS_VALUE == result && (name.Length() > 0)) {
    aNames[0] = name;
    aNames[0].AppendWithConversion(".x");
    aNames[1] = name;
    aNames[1].AppendWithConversion(".y");
  } else {
    // If the Image Element has no name, simply return x and y
    // to Nav and IE compatability.
    aNames[0].AssignWithConversion("x");
    aNames[1].AssignWithConversion("y");
  }

  return PR_TRUE;
}

NS_IMETHODIMP
nsImageControlFrame::GetCursor(nsIPresContext* aPresContext,
                               nsPoint&        aPoint,
                               PRInt32&        aCursor)
{
  // Use style defined cursor if one is provided, otherwise when
  // the cursor style is "auto" we use the pointer cursor.
  const nsStyleColor* styleColor;
  GetStyleData(eStyleStruct_Color, (const nsStyleStruct*&)styleColor);
  NS_ASSERTION(styleColor,"null color style struct");

  if (styleColor) {
    aCursor = styleColor->mCursor;
    if (NS_STYLE_CURSOR_AUTO == aCursor) {
      aCursor = NS_STYLE_CURSOR_POINTER;
    }
  } else {
    aCursor = NS_STYLE_CURSOR_POINTER;
  }
  return NS_OK;
}

void
nsImageControlFrame::MouseClicked(nsIPresContext* aPresContext) 
{
  PRInt32 type;
  GetType(&type);

  if ((nsnull != mFormFrame) && !nsFormFrame::GetDisabled(this)) {
    nsIContent *formContent = nsnull;
    mFormFrame->GetContent(&formContent);

    nsEventStatus status;
    nsEvent event;
    event.eventStructType = NS_EVENT;
    event.message = NS_FORM_SUBMIT;
    if (nsnull != formContent) {
      nsCOMPtr<nsIPresShell> shell;
      nsresult result = aPresContext->GetShell(getter_AddRefs(shell));
      if (NS_SUCCEEDED(result) && shell) {
        shell->HandleDOMEventWithTarget(formContent, &event, &status);
      }
      NS_RELEASE(formContent);
    }
    if (nsEventStatus_eConsumeNoDefault != status) {
      mFormFrame->OnSubmit(aPresContext, this);
    }
  } 
}

NS_IMETHODIMP
nsImageControlFrame::GetFont(nsIPresContext* aPresContext, 
                             const nsFont*&  aFont)
{
  return nsFormControlHelper::GetFont(this, aPresContext, mStyleContext, aFont);
}

NS_IMETHODIMP
nsImageControlFrame::GetFormContent(nsIContent*& aContent) const
{
  nsIContent* content;
  nsresult    rv;

  rv = GetContent(&content);
  aContent = content;
  return rv;
}

nscoord 
nsImageControlFrame::GetVerticalInsidePadding(nsIPresContext* aPresContext,
                                              float aPixToTwip, 
                                              nscoord aInnerHeight) const
{
   return 0;
}

nscoord 
nsImageControlFrame::GetHorizontalInsidePadding(nsIPresContext* aPresContext,
                                               float aPixToTwip, 
                                               nscoord aInnerWidth,
                                               nscoord aCharWidth) const
{
  return 0;
}

nsresult nsImageControlFrame::RequiresWidget(PRBool& aRequiresWidget)
{
  aRequiresWidget = PR_FALSE;
  return NS_OK;
}


NS_IMETHODIMP nsImageControlFrame::SetProperty(nsIPresContext* aPresContext,
                                               nsIAtom* aName,
                                               const nsAReadableString& aValue)
{
  return NS_OK;
}

NS_IMETHODIMP nsImageControlFrame::GetProperty(nsIAtom* aName,
                                               nsAWritableString& aValue)
{
  aValue.Truncate();
  return NS_OK;
}

NS_IMETHODIMP nsImageControlFrame::SetSuggestedSize(nscoord aWidth, nscoord aHeight)
{
//  mSuggestedWidth = aWidth;
//  mSuggestedHeight = aHeight;
  return NS_OK;
}

