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

#include "nsGfxRadioControlFrame.h"
#include "nsHTMLAtoms.h"
#include "nsHTMLParts.h"
#include "nsFormFrame.h"
#include "nsIFormControl.h"
#include "nsIContent.h"
#include "nsWidgetsCID.h"
#include "nsIComponentManager.h"
#include "nsCOMPtr.h"
#include "nsCSSRendering.h"
#include "nsIPresState.h"
#include "nsINameSpaceManager.h"


nsresult
NS_NewGfxRadioControlFrame(nsIPresShell* aPresShell, nsIFrame** aNewFrame)
{
  NS_PRECONDITION(aNewFrame, "null OUT ptr");
  if (nsnull == aNewFrame) {
    return NS_ERROR_NULL_POINTER;
  }
  nsGfxRadioControlFrame* it = new (aPresShell) nsGfxRadioControlFrame;
  if (!it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  *aNewFrame = it;
  return NS_OK;
}

nsGfxRadioControlFrame::nsGfxRadioControlFrame()
{
   // Initialize GFX-rendered state
  mChecked = PR_FALSE;
  mRadioButtonFaceStyle = nsnull;
}

nsGfxRadioControlFrame::~nsGfxRadioControlFrame()
{
  NS_IF_RELEASE(mRadioButtonFaceStyle);
}

// Frames are not refcounted, no need to AddRef
NS_IMETHODIMP
nsGfxRadioControlFrame::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
  NS_PRECONDITION(0 != aInstancePtr, "null ptr");
  if (NULL == aInstancePtr) {
    return NS_ERROR_NULL_POINTER;
  }
  if (aIID.Equals(NS_GET_IID(nsIRadioControlFrame))) {
    *aInstancePtr = (void*) ((nsIRadioControlFrame*) this);
    return NS_OK;
  }
  if (aIID.Equals(NS_GET_IID(nsIStatefulFrame))) {
    *aInstancePtr = (void*) ((nsIStatefulFrame*) this);
    return NS_OK;
  }
 
  return nsFormControlFrame::QueryInterface(aIID, aInstancePtr);
}


//--------------------------------------------------------------
NS_IMETHODIMP
nsGfxRadioControlFrame::GetAdditionalStyleContext(PRInt32 aIndex, 
                                                  nsIStyleContext** aStyleContext) const
{
  NS_PRECONDITION(nsnull != aStyleContext, "null OUT parameter pointer");
  if (aIndex < 0) {
    return NS_ERROR_INVALID_ARG;
  }
  *aStyleContext = nsnull;
  switch (aIndex) {
  case NS_GFX_RADIO_CONTROL_FRAME_FACE_CONTEXT_INDEX:
    *aStyleContext = mRadioButtonFaceStyle;
    NS_IF_ADDREF(*aStyleContext);
    break;
  default:
    return NS_ERROR_INVALID_ARG;
  }
  return NS_OK;
}

//--------------------------------------------------------------
NS_IMETHODIMP
nsGfxRadioControlFrame::SetAdditionalStyleContext(PRInt32 aIndex, 
                                                  nsIStyleContext* aStyleContext)
{
  if (aIndex < 0) {
    return NS_ERROR_INVALID_ARG;
  }
  switch (aIndex) {
  case NS_GFX_RADIO_CONTROL_FRAME_FACE_CONTEXT_INDEX:
    NS_IF_RELEASE(mRadioButtonFaceStyle);
    mRadioButtonFaceStyle = aStyleContext;
    NS_IF_ADDREF(aStyleContext);
    break;
  }
  return NS_OK;
}

//--------------------------------------------------------------
NS_IMETHODIMP nsGfxRadioControlFrame::SetProperty(nsIPresContext* aPresContext, nsIAtom* aName, const nsString& aValue)
{
  if (nsHTMLAtoms::checked == aName) {
    PRBool state = (aValue == NS_STRING_TRUE) ? PR_TRUE : PR_FALSE;


    // if there is no form than the radiobtn is an orphan
    if (mFormFrame) {
      // if this failed then it didn't have a named radio group
      if (NS_FAILED(mFormFrame->OnRadioChecked(aPresContext, *this, state))) {
        // The line below is commented out to duplicate NavQuirks behavior
        //SetRadioState(aPresContext, state);
      }
    } else {
      SetRadioState(aPresContext, state);
    }
  }
  else {
    return nsFormControlFrame::SetProperty(aPresContext, aName, aValue);
  }

  return NS_OK;    
}

//--------------------------------------------------------------
NS_IMETHODIMP nsGfxRadioControlFrame::GetProperty(nsIAtom* aName, nsString& aValue)
{
  // Return the value of the property from the widget it is not null.
  // If is null, assume the widget is GFX-rendered and return a member variable instead.

  if (nsHTMLAtoms::checked == aName) {
	  nsFormControlHelper::GetBoolString(GetRadioState(), aValue);
  } else {
    return nsFormControlFrame::GetProperty(aName, aValue);
  }

  return NS_OK;    
}

//--------------------------------------------------------------
PRBool
nsGfxRadioControlFrame::GetChecked() 
{
  PRBool checked = PR_FALSE;
  GetCurrentCheckState(&checked);
  return(checked);
}

//--------------------------------------------------------------
PRBool
nsGfxRadioControlFrame::GetDefaultChecked() 
{
  PRBool checked = PR_FALSE;
  GetDefaultCheckState(&checked);
  return(checked);
}

//--------------------------------------------------------------
void
nsGfxRadioControlFrame::SetChecked(nsIPresContext* aPresContext, PRBool aValue, PRBool aSetInitialValue)
{
  if (aSetInitialValue) {
    if (aValue) {
      mContent->SetAttribute(kNameSpaceID_HTML, nsHTMLAtoms::checked, nsAutoString("1"), PR_FALSE); // XXX should be "empty" value
    } else {
      mContent->SetAttribute(kNameSpaceID_HTML, nsHTMLAtoms::checked, nsAutoString("0"), PR_FALSE);
    }
  }

  SetRadioState(aPresContext, aValue);
}

//--------------------------------------------------------------
NS_IMETHODIMP
nsGfxRadioControlFrame::SetRadioButtonFaceStyleContext(nsIStyleContext *aRadioButtonFaceStyleContext)
{
  mRadioButtonFaceStyle = aRadioButtonFaceStyleContext;
  NS_ADDREF(mRadioButtonFaceStyle);
  return NS_OK;
}

//--------------------------------------------------------------
PRBool
nsGfxRadioControlFrame::GetNamesValues(PRInt32 aMaxNumValues, PRInt32& aNumValues,
                                    nsString* aValues, nsString* aNames)
{
  nsAutoString name;
  nsresult result = GetName(&name);
  if ((aMaxNumValues <= 0) || (NS_CONTENT_ATTR_HAS_VALUE != result)) {
    return PR_FALSE;
  }

  PRBool state = GetRadioState();

  if(PR_TRUE != state) {
    return PR_FALSE;
  }

  nsAutoString value;
  result = GetValue(&value);
  
  if (NS_CONTENT_ATTR_HAS_VALUE == result) {
    aValues[0] = value;
  } else {
    aValues[0] = "on";
  }
  aNames[0]  = name;
  aNumValues = 1;

  return PR_TRUE;
}

//--------------------------------------------------------------
void 
nsGfxRadioControlFrame::Reset(nsIPresContext* aPresContext) 
{
  PRBool checked = PR_TRUE;
  GetDefaultCheckState(&checked);
  SetCurrentCheckState(checked);
}  

//--------------------------------------------------------------
void
nsGfxRadioControlFrame::PaintRadioButton(nsIPresContext* aPresContext,
                                      nsIRenderingContext& aRenderingContext,
                                      const nsRect& aDirtyRect)
{
   
  PRBool checked = PR_TRUE;
  GetCurrentCheckState(&checked); // Get check state from the content model
  if (PR_TRUE == checked) {
    // Paint the button for the radio button using CSS background rendering code
   if (nsnull != mRadioButtonFaceStyle) {
     const nsStyleColor* myColor = (const nsStyleColor*)
          mRadioButtonFaceStyle->GetStyleData(eStyleStruct_Color);
     const nsStyleSpacing* mySpacing = (const nsStyleSpacing*)
          mRadioButtonFaceStyle->GetStyleData(eStyleStruct_Spacing);
     const nsStylePosition* myPosition = (const nsStylePosition*)
          mRadioButtonFaceStyle->GetStyleData(eStyleStruct_Position);

     nscoord width = myPosition->mWidth.GetCoordValue();
     nscoord height = myPosition->mHeight.GetCoordValue();
       // Position the button centered within the radio control's rectangle.
     nscoord x = (mRect.width - width) / 2;
     nscoord y = (mRect.height - height) / 2;
     nsRect rect(x, y, width, height); 

     nsCSSRendering::PaintBackground(aPresContext, aRenderingContext, this,
                                        aDirtyRect, rect, *myColor, *mySpacing, 0, 0);
     nsCSSRendering::PaintBorder(aPresContext, aRenderingContext, this,
                                  aDirtyRect, rect, *mySpacing, mRadioButtonFaceStyle, 0);
   }
  }
}

//--------------------------------------------------------------
NS_METHOD 
nsGfxRadioControlFrame::Paint(nsIPresContext* aPresContext,
                           nsIRenderingContext& aRenderingContext,
                           const nsRect& aDirtyRect,
                           nsFramePaintLayer aWhichLayer)
{
 	const nsStyleDisplay* disp = (const nsStyleDisplay*)
	mStyleContext->GetStyleData(eStyleStruct_Display);
	if (!disp->IsVisible())
		return NS_OK;

     // Paint the background
  nsFormControlFrame::Paint(aPresContext, aRenderingContext, aDirtyRect, aWhichLayer);

  if (NS_FRAME_PAINT_LAYER_FOREGROUND == aWhichLayer) {
    PaintRadioButton(aPresContext, aRenderingContext, aDirtyRect);
  }
  return NS_OK;
}


//--------------------------------------------------------------
PRBool nsGfxRadioControlFrame::GetRadioState()
{
  return mChecked;
}

//--------------------------------------------------------------
void nsGfxRadioControlFrame::SetRadioState(nsIPresContext* aPresContext, PRBool aValue)
{
  mChecked = aValue;
	nsFormControlHelper::ForceDrawFrame(aPresContext, this);
}

void 
nsGfxRadioControlFrame::InitializeControl(nsIPresContext* aPresContext)
{
  nsFormControlFrame::InitializeControl(aPresContext);

   // set the widget to the initial state
  PRBool checked = PR_FALSE;
  nsresult result = GetDefaultCheckState(&checked);
  if (NS_CONTENT_ATTR_HAS_VALUE == result) {
    SetRadioState(aPresContext, checked);
  }
}

//----------------------------------------------------------------------
// nsIStatefulFrame
//----------------------------------------------------------------------
NS_IMETHODIMP
nsGfxRadioControlFrame::GetStateType(nsIPresContext* aPresContext, nsIStatefulFrame::StateType* aStateType)
{
  *aStateType = nsIStatefulFrame::eRadioType;
  return NS_OK;
}

//----------------------------------------------------------------------
NS_IMETHODIMP
nsGfxRadioControlFrame::SaveState(nsIPresContext* aPresContext, nsIPresState** aState)
{
  // Construct a pres state.
  NS_NewPresState(aState); // The addref happens here.
  
  // This string will hold a single item, whether or not we're checked.
  nsAutoString stateString;
	nsFormControlHelper::GetBoolString(GetRadioState(), stateString);
  (*aState)->SetStateProperty("checked", stateString);

  return NS_OK;
}
        


//----------------------------------------------------------------------
NS_IMETHODIMP
nsGfxRadioControlFrame::RestoreState(nsIPresContext* aPresContext, nsIPresState* aState)
{
  if (!mDidInit) {
    mPresContext = aPresContext;
    InitializeControl(aPresContext);
    mDidInit = PR_TRUE;
  }

  mIsRestored = PR_TRUE;
  nsAutoString string;
  aState->GetStateProperty("checked", string);
  PRBool state = (string == NS_STRING_TRUE) ? PR_TRUE : PR_FALSE;

  SetRadioState(aPresContext, state); // sets mChecked
  mRestoredChecked = mChecked;


  return NS_OK;
}


//----------------------------------------------------------------------
// Extra Debug Helper Methods
//----------------------------------------------------------------------
#ifdef DEBUG_rodsXXX
NS_IMETHODIMP 
nsGfxRadioControlFrame::Reflow(nsIPresContext*          aPresContext, 
                               nsHTMLReflowMetrics&     aDesiredSize,
                               const nsHTMLReflowState& aReflowState, 
                               nsReflowStatus&          aStatus)
{
  nsresult rv = nsNativeFormControlFrame::Reflow(aPresContext, aDesiredSize, aReflowState, aStatus);

  COMPARE_QUIRK_SIZE("nsGfxRadioControlFrame", 12, 11) 
  return rv;
}
#endif
