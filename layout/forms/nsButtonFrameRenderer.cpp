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
#include "nsButtonFrameRenderer.h"
#include "nsIRenderingContext.h"
#include "nsCSSRendering.h"
#include "nsIPresContext.h"
#include "nsIView.h"
#include "nsIViewManager.h"
#include "nsHTMLAtoms.h"
#include "nsINameSpaceManager.h"

#define ACTIVE   "active"
#define HOVER    "hover"
#define FOCUS    "focus"

MOZ_DECL_CTOR_COUNTER(nsButtonFrameRenderer)

nsButtonFrameRenderer::nsButtonFrameRenderer()
{
  MOZ_COUNT_CTOR(nsButtonFrameRenderer);
  mNameSpace = kNameSpaceID_HTML;
}

nsButtonFrameRenderer::~nsButtonFrameRenderer()
{
  MOZ_COUNT_DTOR(nsButtonFrameRenderer);
}

void
nsButtonFrameRenderer::SetNameSpace(PRInt32 aNameSpace)
{
  mNameSpace = aNameSpace;
}

void
nsButtonFrameRenderer::SetFrame(nsFrame* aFrame, nsIPresContext* aPresContext)
{
  mFrame = aFrame;
  ReResolveStyles(aPresContext);
}

nsIFrame*
nsButtonFrameRenderer::GetFrame()
{
  return mFrame;
}

PRInt32
nsButtonFrameRenderer::GetNameSpace()
{
  return mNameSpace;
}

void
nsButtonFrameRenderer::SetDisabled(PRBool aDisabled, PRBool notify)
{
  // get the content
  nsCOMPtr<nsIContent> content;
  mFrame->GetContent(getter_AddRefs(content));

  if (aDisabled)
    content->SetAttribute(mNameSpace, nsHTMLAtoms::disabled, nsAutoString(), notify);
  else
    content->UnsetAttribute(mNameSpace, nsHTMLAtoms::disabled, notify);

}

PRBool
nsButtonFrameRenderer::isDisabled() 
{
  // get the content
  nsCOMPtr<nsIContent> content;
  mFrame->GetContent(getter_AddRefs(content));
  nsAutoString value;
  if (NS_CONTENT_ATTR_HAS_VALUE == content->GetAttribute(mNameSpace, nsHTMLAtoms::disabled, value))
    return PR_TRUE;

  return PR_FALSE;
}

void
nsButtonFrameRenderer::Redraw(nsIPresContext* aPresContext)
{
  nsRect rect;
  mFrame->GetRect(rect);
  rect.x = 0;
  rect.y = 0;
  mFrame->Invalidate(aPresContext, rect, PR_FALSE);
}

void 
nsButtonFrameRenderer::PaintButton     (nsIPresContext* aPresContext,
          nsIRenderingContext& aRenderingContext,
          const nsRect& aDirtyRect,
          nsFramePaintLayer aWhichLayer,
          const nsRect& aRect)
{
  //printf("painted width='%d' height='%d'\n",aRect.width, aRect.height);

  // draw the border and background inside the focus and outline borders
  PaintBorderAndBackground(aPresContext, aRenderingContext, aDirtyRect, aWhichLayer, aRect);

  // draw the focus and outline borders
  PaintOutlineAndFocusBorders(aPresContext, aRenderingContext, aDirtyRect, aWhichLayer, aRect);
}

void
nsButtonFrameRenderer::PaintOutlineAndFocusBorders(nsIPresContext* aPresContext,
          nsIRenderingContext& aRenderingContext,
          const nsRect& aDirtyRect,
          nsFramePaintLayer aWhichLayer,
          const nsRect& aRect)
{
  // once we have all that let draw the focus if we have it. We will need to draw 2 focuses.
  // the inner and the outer. This is so we can do any kind of look and feel some buttons have
  // focus on the outside like mac and motif. While others like windows have it inside (dotted line).
  // Usually only one will be specifed. But I guess you could have both if you wanted to.

  nsRect rect;

  if (NS_FRAME_PAINT_LAYER_BACKGROUND == aWhichLayer) 
  {
    if (mOuterFocusStyle) {
      // ---------- paint the outer focus border -------------

      GetButtonOuterFocusRect(aRect, rect);

      const nsStyleBorder* border = (const nsStyleBorder*)mOuterFocusStyle ->GetStyleData(eStyleStruct_Border);

      nsCSSRendering::PaintBorder(aPresContext, aRenderingContext, mFrame,
                                  aDirtyRect, rect, *border, mOuterFocusStyle, 0);
    }

    // ---------- paint the inner focus border -------------
    if (mInnerFocusStyle) { 

      GetButtonInnerFocusRect(aRect, rect);

      const nsStyleBorder* border = (const nsStyleBorder*)mInnerFocusStyle ->GetStyleData(eStyleStruct_Border);

      nsCSSRendering::PaintBorder(aPresContext, aRenderingContext, mFrame,
                                  aDirtyRect, rect, *border, mInnerFocusStyle, 0);
    }
  }

}


void
nsButtonFrameRenderer::PaintBorderAndBackground(nsIPresContext* aPresContext,
          nsIRenderingContext& aRenderingContext,
          const nsRect& aDirtyRect,
          nsFramePaintLayer aWhichLayer,
          const nsRect& aRect)

{

  if (NS_FRAME_PAINT_LAYER_BACKGROUND != aWhichLayer) 
    return;

  // get the button rect this is inside the focus and outline rects
  nsRect buttonRect;
  GetButtonRect(aRect, buttonRect);

  nsCOMPtr<nsIStyleContext> context;
  mFrame->GetStyleContext(getter_AddRefs(context));


  // get the styles
  const nsStyleBorder* border =
    (const nsStyleBorder*)context->GetStyleData(eStyleStruct_Border);
  const nsStyleBackground* color =
    (const nsStyleBackground*)context->GetStyleData(eStyleStruct_Background);
      

  // paint the border and background

  nsCSSRendering::PaintBackground(aPresContext, aRenderingContext, mFrame,
                                  aDirtyRect, buttonRect,  *color, *border, 0, 0);

  nsCSSRendering::PaintBorder(aPresContext, aRenderingContext, mFrame,
                              aDirtyRect, buttonRect, *border, context, 0);

}


void
nsButtonFrameRenderer::GetButtonOutlineRect(const nsRect& aRect, nsRect& outlineRect)
{
  outlineRect = aRect;
  outlineRect.Inflate(GetButtonOutlineBorderAndPadding());
}


void
nsButtonFrameRenderer::GetButtonOuterFocusRect(const nsRect& aRect, nsRect& focusRect)
{
  focusRect = aRect;
}

void
nsButtonFrameRenderer::GetButtonRect(const nsRect& aRect, nsRect& r)
{
  r = aRect;
  r.Deflate(GetButtonOuterFocusBorderAndPadding());
}


void
nsButtonFrameRenderer::GetButtonInnerFocusRect(const nsRect& aRect, nsRect& focusRect)
{
  GetButtonRect(aRect, focusRect);
  focusRect.Deflate(GetButtonBorderAndPadding());
  focusRect.Deflate(GetButtonInnerFocusMargin());
}

void
nsButtonFrameRenderer::GetButtonContentRect(const nsRect& aRect, nsRect& r)
{
  GetButtonInnerFocusRect(aRect, r);
  r.Deflate(GetButtonInnerFocusBorderAndPadding());
}


nsMargin
nsButtonFrameRenderer::GetButtonOuterFocusBorderAndPadding()
{
  nsMargin focusBorderAndPadding(0,0,0,0);

  if (mOuterFocusStyle) {
    nsStyleBorderPadding  bPad;
    mOuterFocusStyle->GetBorderPaddingFor(bPad);
    if (!bPad.GetBorderPadding(focusBorderAndPadding)) {
      NS_NOTYETIMPLEMENTED("percentage border");
    }
  }

  return focusBorderAndPadding;
}

nsMargin
nsButtonFrameRenderer::GetButtonBorderAndPadding()
{
  nsCOMPtr<nsIStyleContext> context;
  mFrame->GetStyleContext(getter_AddRefs(context));

  nsMargin innerFocusBorderAndPadding(0,0,0,0);
  nsStyleBorderPadding  bPad;
  context->GetBorderPaddingFor(bPad);
  if (!bPad.GetBorderPadding(innerFocusBorderAndPadding)) {
    NS_NOTYETIMPLEMENTED("percentage border");
  }
  return innerFocusBorderAndPadding;
}

/**
 * Gets the size of the buttons border this is the union of the normal and disabled borders.
 */
nsMargin
nsButtonFrameRenderer::GetButtonInnerFocusMargin()
{
  nsMargin innerFocusMargin(0,0,0,0);

  if (mInnerFocusStyle) {
    // get the outer focus border and padding
    const nsStyleMargin* margin = (const nsStyleMargin*)mInnerFocusStyle ->GetStyleData(eStyleStruct_Margin);
    margin->GetMargin(innerFocusMargin);
  }

  return innerFocusMargin;
}

nsMargin
nsButtonFrameRenderer::GetButtonInnerFocusBorderAndPadding()
{
  nsMargin innerFocusBorderAndPadding(0,0,0,0);

  if (mInnerFocusStyle) {
    // get the outer focus border and padding
    nsStyleBorderPadding  bPad;
    mInnerFocusStyle->GetBorderPaddingFor(bPad);
    if (!bPad.GetBorderPadding(innerFocusBorderAndPadding)) {
      NS_NOTYETIMPLEMENTED("percentage border");
    }
  }

  return innerFocusBorderAndPadding;
}

nsMargin
nsButtonFrameRenderer::GetButtonOutlineBorderAndPadding()
{
  nsMargin borderAndPadding(0,0,0,0);
  return borderAndPadding;
}

// gets the full size of our border with all the focus borders
nsMargin
nsButtonFrameRenderer::GetFullButtonBorderAndPadding()
{
  return GetAddedButtonBorderAndPadding() + GetButtonBorderAndPadding();
}

// gets all the focus borders and padding that will be added to the regular border
nsMargin
nsButtonFrameRenderer::GetAddedButtonBorderAndPadding()
{
  return GetButtonOuterFocusBorderAndPadding() + GetButtonInnerFocusMargin() + GetButtonInnerFocusBorderAndPadding();
}

/**
 * Call this when styles change
 */
void 
nsButtonFrameRenderer::ReResolveStyles(nsIPresContext* aPresContext)
{

  // get all the styles
  nsCOMPtr<nsIContent> content;
  mFrame->GetContent(getter_AddRefs(content));
  nsCOMPtr<nsIStyleContext> context;
  mFrame->GetStyleContext(getter_AddRefs(context));


  // style for the inner such as a dotted line (Windows)
  aPresContext->ProbePseudoStyleContextFor(content, nsHTMLAtoms::mozFocusInnerPseudo, context,
                                          PR_FALSE,
                                          getter_AddRefs(mInnerFocusStyle));

  // style for outer focus like a ridged border (MAC).
  aPresContext->ProbePseudoStyleContextFor(content, nsHTMLAtoms::mozFocusOuterPseudo, context,
                                          PR_FALSE,
                                          getter_AddRefs(mOuterFocusStyle));

}

nsresult 
nsButtonFrameRenderer::GetStyleContext(PRInt32 aIndex, nsIStyleContext** aStyleContext) const
{
  NS_PRECONDITION(nsnull != aStyleContext, "null OUT parameter pointer");
  if (aIndex < 0) {
    return NS_ERROR_INVALID_ARG;
  }
  *aStyleContext = nsnull;
  switch (aIndex) {

  case NS_BUTTON_RENDERER_FOCUS_INNER_CONTEXT_INDEX:
    *aStyleContext = mInnerFocusStyle;
    NS_IF_ADDREF(*aStyleContext);
    break;
  case NS_BUTTON_RENDERER_FOCUS_OUTER_CONTEXT_INDEX:
    *aStyleContext = mOuterFocusStyle;
    NS_IF_ADDREF(*aStyleContext);
    break;
  default:
    return NS_ERROR_INVALID_ARG;
  }
  return NS_OK;
}

nsresult 
nsButtonFrameRenderer::SetStyleContext(PRInt32 aIndex, nsIStyleContext* aStyleContext)
{
  if (aIndex < 0) {
    return NS_ERROR_INVALID_ARG;
  }
  switch (aIndex) {
 
  case NS_BUTTON_RENDERER_FOCUS_INNER_CONTEXT_INDEX:
    mInnerFocusStyle = aStyleContext;
    break;
  case NS_BUTTON_RENDERER_FOCUS_OUTER_CONTEXT_INDEX:
    mOuterFocusStyle = aStyleContext;
    break;
  }
  return NS_OK;
}
