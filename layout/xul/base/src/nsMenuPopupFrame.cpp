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
 *   Dean Tessman <dean_tessman@hotmail.com>
 */


#include "nsMenuPopupFrame.h"
#include "nsXULAtoms.h"
#include "nsHTMLAtoms.h"
#include "nsIContent.h"
#include "prtypes.h"
#include "nsIAtom.h"
#include "nsIPresContext.h"
#include "nsIStyleContext.h"
#include "nsCSSRendering.h"
#include "nsINameSpaceManager.h"
#include "nsIViewManager.h"
#include "nsWidgetsCID.h"
#include "nsMenuFrame.h"
#include "nsIPopupSetFrame.h"
#include "nsIDOMWindow.h"
#include "nsIDOMScreen.h"
#include "nsIScriptGlobalObject.h"
#include "nsIPresShell.h"
#include "nsIScriptObjectOwner.h"
#include "nsIDocument.h"
#include "nsIDeviceContext.h"
#include "nsRect.h"
#include "nsIDOMXULDocument.h"
#include "nsILookAndFeel.h"
#include "nsIComponentManager.h"


static NS_DEFINE_IID(kLookAndFeelCID, NS_LOOKANDFEEL_CID);
static NS_DEFINE_IID(kILookAndFeelIID, NS_ILOOKANDFEEL_IID);

const PRInt32 kMaxZ = 0x7fffffff; //XXX: Shouldn't there be a define somewhere for MaxInt for PRInt32

static NS_DEFINE_IID(kIFrameIID, NS_IFRAME_IID);

// NS_NewMenuPopupFrame
//
// Wrapper for creating a new menu popup container
//
nsresult
NS_NewMenuPopupFrame(nsIPresShell* aPresShell, nsIFrame** aNewFrame)
{
  NS_PRECONDITION(aNewFrame, "null OUT ptr");
  if (nsnull == aNewFrame) {
    return NS_ERROR_NULL_POINTER;
  }
  nsMenuPopupFrame* it = new (aPresShell) nsMenuPopupFrame (aPresShell);
  if ( !it )
    return NS_ERROR_OUT_OF_MEMORY;
  *aNewFrame = it;
  return NS_OK;
}

NS_IMETHODIMP_(nsrefcnt) 
nsMenuPopupFrame::AddRef(void)
{
  return NS_OK;
}

NS_IMETHODIMP_(nsrefcnt) 
nsMenuPopupFrame::Release(void)
{
    return NS_OK;
}


//
// QueryInterface
//
NS_INTERFACE_MAP_BEGIN(nsMenuPopupFrame)
  NS_INTERFACE_MAP_ENTRY(nsIMenuParent)
  NS_INTERFACE_MAP_ENTRY(nsITimerCallback)
NS_INTERFACE_MAP_END_INHERITING(nsBoxFrame)


//
// nsMenuPopupFrame ctor
//
nsMenuPopupFrame::nsMenuPopupFrame(nsIPresShell* aShell):nsBoxFrame(aShell),
mCurrentMenu(nsnull), mTimerMenu(nsnull), mCloseTimer(nsnull)
{
  SetIsContextMenu(PR_FALSE);   // we're not a context menu by default

} // ctor


NS_IMETHODIMP
nsMenuPopupFrame::Init(nsIPresContext*  aPresContext,
                       nsIContent*      aContent,
                       nsIFrame*        aParent,
                       nsIStyleContext* aContext,
                       nsIFrame*        aPrevInFlow)
{
  nsresult rv = nsBoxFrame::Init(aPresContext, aContent, aParent, aContext, aPrevInFlow);

  // XXX Hack
  mPresContext = aPresContext;

  CreateViewForFrame(aPresContext, this, aContext, PR_TRUE);

  // Now that we've made a view, remove it and insert it at the correct
  // position in the view hierarchy (as the root view).  We do this so that we
  // can draw the menus outside the confines of the window.
  nsIView* ourView;
  GetView(aPresContext, &ourView);

  nsIFrame* parent;
  aParent->GetParentWithView(aPresContext, &parent);
  nsIView* parentView;
  parent->GetView(aPresContext, &parentView);

  nsCOMPtr<nsIViewManager> viewManager;
  parentView->GetViewManager(*getter_AddRefs(viewManager));

  // Remove the view from its old position.
  viewManager->RemoveChild(parentView, ourView);

  // Reinsert ourselves as the root view with a maximum z-index.
  nsIView* rootView;
  viewManager->GetRootView(rootView);
  viewManager->InsertChild(rootView, ourView, kMaxZ);

  // XXX Hack. The menu's view should float above all other views,
  // so we use the nsIView::SetFloating() to tell the view manager
  // about that constraint.
  ourView->SetFloating(PR_TRUE);

  // XXX Hack. Change our transparency to be non-transparent
  // until the bug related to update of transparency on show/hide
  // is fixed.
  viewManager->SetViewContentTransparency(ourView, PR_FALSE);

  // Create a widget for ourselves.
  nsWidgetInitData widgetData;
  ourView->SetZIndex(kMaxZ);
  widgetData.mWindowType = eWindowType_popup;
  widgetData.mBorderStyle = eBorderStyle_default;

  // XXX make sure we are hidden (shouldn't this be done automatically?)
  ourView->SetVisibility(nsViewVisibility_kHide);
#ifdef XP_MAC
  printf("XP Popups: This is a nag to indicate that an inconsistent hack is being done on the Mac for popups.\n");
  static NS_DEFINE_IID(kCPopupCID,  NS_POPUP_CID);
  ourView->CreateWidget(kCPopupCID, &widgetData, nsnull);
#else
  static NS_DEFINE_IID(kCChildCID,  NS_CHILD_CID);
  ourView->CreateWidget(kCChildCID, &widgetData, nsnull);
#endif   

  return rv;
}

PRBool
nsMenuPopupFrame::GetInitialOrientation(PRBool& aIsHorizontal)
{
    // by default we are vertical
    aIsHorizontal = PR_FALSE;
    return nsBoxFrame::GetInitialOrientation(aIsHorizontal);
}

void
nsMenuPopupFrame::GetViewOffset(nsIViewManager* aManager, nsIView* aView, 
  nsPoint& aPoint)
{
  aPoint.x = 0;
  aPoint.y = 0;
 
  nsIView *parent;
  nsRect bounds;

  parent = aView;
  while (nsnull != parent) {
    parent->GetBounds(bounds);
    aPoint.x += bounds.x;
    aPoint.y += bounds.y;
    parent->GetParent(parent);
  }
}

void
nsMenuPopupFrame::GetNearestEnclosingView(nsIPresContext* aPresContext, nsIFrame* aStartFrame, nsIView** aResult)
{
  *aResult = nsnull;
  aStartFrame->GetView(aPresContext, aResult);
  if (!*aResult) {
    nsIFrame* parent;
    aStartFrame->GetParentWithView(aPresContext, &parent);
    if (parent)
      parent->GetView(aPresContext, aResult);
  }
}


void GetWidgetForView(nsIView *aView, nsIWidget *&aWidget);
void GetWidgetForView(nsIView *aView, nsIWidget *&aWidget)
{
  aWidget = nsnull;
  nsIView *view = aView;
  while (!aWidget && view)
  {
    view->GetWidget(aWidget);
    if (!aWidget)
      view->GetParent(view);
  }
}


//
// AdjustClientXYForNestedDocuments
// 
// almost certainly, the document where the mouse was clicked is not
// the document that contains the popup, especially if we're viewing a page
// with frames. Thus we need to make adjustments to the client coordinates to
// take this into account and get them back into the relative coordinates of
// this document.
//
void
nsMenuPopupFrame :: AdjustClientXYForNestedDocuments ( nsIDOMXULDocument* inPopupDoc, nsIPresShell* inPopupShell, 
                                                         PRInt32 inClientX, PRInt32 inClientY, 
                                                         PRInt32* outAdjX, PRInt32* outAdjY )
{
  if ( !inPopupDoc || !outAdjX || !outAdjY )
    return;

  // Find the widget associated with the popup's document
  nsCOMPtr<nsIWidget> popupDocumentWidget;
  nsCOMPtr<nsIViewManager> viewManager;
  inPopupShell->GetViewManager(getter_AddRefs(viewManager));
  nsIView* rootView;
  viewManager->GetRootView(rootView);
  nscoord wOffsetX, wOffsetY;
  rootView->GetOffsetFromWidget(&wOffsetX, &wOffsetY, *getter_AddRefs(popupDocumentWidget));
  NS_WARN_IF_FALSE(popupDocumentWidget, "ACK, BAD WIDGET");
  
  // Find the widget associated with the target's document. Recall that we cached the
  // target popup node in the document of the popup in the nsXULPopupListener.
  nsCOMPtr<nsIDOMNode> targetNode;
  inPopupDoc->GetPopupNode(getter_AddRefs(targetNode));
  NS_WARN_IF_FALSE(targetNode, "ACK, BAD TARGET NODE");
  nsCOMPtr<nsIContent> targetAsContent ( do_QueryInterface(targetNode) );
  nsCOMPtr<nsIWidget> targetDocumentWidget;
  if ( targetAsContent ) {
    nsCOMPtr<nsIDocument> targetDocument;
    targetAsContent->GetDocument(*getter_AddRefs(targetDocument));
    nsCOMPtr<nsIPresShell> shell ( dont_AddRef(targetDocument->GetShellAt(0)) );
    nsCOMPtr<nsIViewManager> viewManagerTarget;
    shell->GetViewManager(getter_AddRefs(viewManagerTarget));
    nsIView* rootViewTarget;
    viewManagerTarget->GetRootView(rootViewTarget);
    nscoord unusedX, unusedY;
    rootViewTarget->GetOffsetFromWidget(&unusedX, &unusedY, *getter_AddRefs(targetDocumentWidget));
  }

  // the offset we need is the difference between the upper left corner of the two widgets. Use
  // screen coordinates to find the global offset between them.
  nsRect popupDocTopLeft;
  if ( popupDocumentWidget ) {
    nsRect topLeftClient ( 0, 0, 10, 10 );
    popupDocumentWidget->WidgetToScreen ( topLeftClient, popupDocTopLeft );
  }
  nsRect targetDocTopLeft;
  if ( targetDocumentWidget ) {
    nsRect topLeftClient ( 0, 0, 10, 10 );
    targetDocumentWidget->WidgetToScreen ( topLeftClient, targetDocTopLeft );
  }
  nsPoint pixelOffset ( targetDocTopLeft.x - popupDocTopLeft.x, targetDocTopLeft.y - popupDocTopLeft.y );

  *outAdjX = inClientX + pixelOffset.x;
  *outAdjY = inClientY + pixelOffset.y;
  
} // AdjustClientXYForNestedDocuments


//
// AdjustPositionForAnchorAlign
// 
// Uses the |popupanchor| and |popupalign| attributes on the popup to move the popup around and
// anchor it to its parent. |outFlushWithTopBottom| will be TRUE if the popup is flush with either
// the top or bottom edge of its parent, and FALSE if it is flush with the left or right edge of
// the parent.
// 
void
nsMenuPopupFrame :: AdjustPositionForAnchorAlign ( PRInt32* ioXPos, PRInt32* ioYPos, const nsRect & inParentRect,
                                                    const nsString& aPopupAnchor, const nsString& aPopupAlign,
                                                    PRBool* outFlushWithTopBottom )
{
  if (aPopupAnchor == "topright" && aPopupAlign == "topleft") {
    *ioXPos += inParentRect.width;
  }
  else if (aPopupAnchor == "topright" && aPopupAlign == "bottomright") {
    *ioXPos -= (mRect.width - inParentRect.width);
    *ioYPos -= mRect.height;
    *outFlushWithTopBottom = PR_TRUE;
  }
  else if (aPopupAnchor == "bottomright" && aPopupAlign == "bottomleft") {
    *ioXPos += inParentRect.width;
    *ioYPos -= (mRect.height - inParentRect.height);
  }
  else if (aPopupAnchor == "bottomright" && aPopupAlign == "topright") {
    *ioXPos -= (mRect.width - inParentRect.width);
    *ioYPos += inParentRect.height;
    *outFlushWithTopBottom = PR_TRUE;
  }
  else if (aPopupAnchor == "topleft" && aPopupAlign == "topright") {
    *ioXPos -= mRect.width;
  }
  else if (aPopupAnchor == "topleft" && aPopupAlign == "bottomleft") {
    *ioYPos -= mRect.height;
    *outFlushWithTopBottom = PR_TRUE;
  }
  else if (aPopupAnchor == "bottomleft" && aPopupAlign == "bottomright") {
    *ioXPos -= mRect.width;
    *ioYPos -= (mRect.height - inParentRect.height);
  }
  else if (aPopupAnchor == "bottomleft" && aPopupAlign == "topleft") {
    *ioYPos += inParentRect.height;
    *outFlushWithTopBottom = PR_TRUE;
  }
  else
    NS_WARNING ( "Hmmm, looks like you've hit a anchor/align case we weren't setup for." );

} // AdjustPositionForAnchorAlign


//
// IsMoreRoomOnOtherSideOfParent
//
// Determine if there is more room on the screen for the popup to live if it was positioned
// on the flip side of the parent from the side it is flush against (ie, if it's top edge was
// flush against the bottom, is there more room if its bottom edge were flush against the top)
//
PRBool
nsMenuPopupFrame :: IsMoreRoomOnOtherSideOfParent ( PRBool inFlushAboveBelow, PRInt32 inScreenViewLocX, PRInt32 inScreenViewLocY,
                                                     const nsRect & inScreenParentFrameRect, PRInt32 inScreenTopTwips, PRInt32 inScreenLeftTwips,
                                                     PRInt32 inScreenBottomTwips, PRInt32 inScreenRightTwips )
{
  PRBool switchSides = PR_FALSE;
  if ( inFlushAboveBelow ) {
    PRInt32 availAbove = inScreenParentFrameRect.y - inScreenTopTwips;
    PRInt32 availBelow = inScreenBottomTwips - (inScreenParentFrameRect.y + inScreenParentFrameRect.height) ;
    if ( inScreenViewLocY > inScreenParentFrameRect.y )       // view is now below parent
      switchSides = availAbove > availBelow;
    else
      switchSides = availBelow > availAbove;
  }
  else {
    PRInt32 availLeft = inScreenParentFrameRect.x - inScreenLeftTwips;
    PRInt32 availRight = inScreenRightTwips - (inScreenParentFrameRect.x + inScreenParentFrameRect.width) ;
    if ( inScreenViewLocX > inScreenParentFrameRect.x )       // view is now to the right of parent
      switchSides = availLeft > availRight;
    else
      switchSides = availRight > availLeft;           
  }

  return switchSides;
  
} // IsMoreRoomOnOtherSideOfParent


//
// MovePopupToOtherSideOfParent
//
// Move the popup to the other side of the parent (ie, if it the popup's top edge is flush against the
// bottom of its parent, move the popup so that its bottom edge is now flush against the top of its
// parent...same idea for left/right).
//
// NOTE: In moving the popup, it may need to change size in order to stay on the screen. This will
//       have the side effect of touching |mRect|.
//
void
nsMenuPopupFrame :: MovePopupToOtherSideOfParent ( PRBool inFlushAboveBelow, PRInt32* ioXPos, PRInt32* ioYPos, 
                                                     PRInt32* ioScreenViewLocX, PRInt32* ioScreenViewLocY,
                                                     const nsRect & inScreenParentFrameRect, PRInt32 inScreenTopTwips, PRInt32 inScreenLeftTwips,
                                                     PRInt32 inScreenBottomTwips, PRInt32 inScreenRightTwips )
{
  if ( inFlushAboveBelow ) {
    if ( *ioScreenViewLocY > inScreenParentFrameRect.y ) {     // view is currently below parent
      // move it above.
      PRInt32 shiftDistY = inScreenParentFrameRect.height + mRect.height;
      *ioYPos -= shiftDistY;
      *ioScreenViewLocY -= shiftDistY;
      // trim it to fit.
      if ( *ioScreenViewLocY < inScreenTopTwips ) {
        PRInt32 trimY = inScreenTopTwips - *ioScreenViewLocY;
        *ioYPos += trimY;
        *ioScreenViewLocY += trimY;
        mRect.height -= trimY;
      }
    }
    else {                                               // view is currently above parent
      // move it below
      PRInt32 shiftDistY = inScreenParentFrameRect.height + mRect.height;
      *ioYPos += shiftDistY;
      *ioScreenViewLocY += shiftDistY;
    }
  }
  else {
    if ( *ioScreenViewLocX > inScreenParentFrameRect.x ) {     // view is currently to the right of the parent
      // move it to the left.
      PRInt32 shiftDistX = inScreenParentFrameRect.width + mRect.width;
      *ioXPos -= shiftDistX;
      *ioScreenViewLocX -= shiftDistX;
      // trim it to fit.
      if ( *ioScreenViewLocX < inScreenLeftTwips ) {
        PRInt32 trimX = inScreenLeftTwips - *ioScreenViewLocX;
        *ioXPos += trimX;
        *ioScreenViewLocX += trimX;
        mRect.width -= trimX;
      }
    }
    else {                                               // view is currently to the right of the parent
      // move it to the right
      PRInt32 shiftDistX = inScreenParentFrameRect.width + mRect.width;
      *ioXPos += shiftDistX;
      *ioScreenViewLocX += shiftDistX;
    }               
  }

} // MovePopupToOtherSideOfParent



nsresult 
nsMenuPopupFrame::SyncViewWithFrame(nsIPresContext* aPresContext,
                                    const nsString& aPopupAnchor,
                                    const nsString& aPopupAlign,
                                    nsIFrame* aFrame, 
                                    PRInt32 aXPos, PRInt32 aYPos)
{
  NS_ENSURE_ARG(aPresContext);
  nsPoint parentPos;
  nsCOMPtr<nsIViewManager> viewManager;

  //
  // Collect info about our parent view and the frame we're sync'ing to
  //

  nsIView* parentView = nsnull;
  GetNearestEnclosingView(aPresContext, aFrame, &parentView);
  if (!parentView)
    return NS_OK;

  parentView->GetViewManager(*getter_AddRefs(viewManager));
  GetViewOffset(viewManager, parentView, parentPos);
  nsIView* view = nsnull;
  GetView(aPresContext, &view);

  nsIView* containingView = nsnull;
  nsPoint offset;
  aFrame->GetOffsetFromView(aPresContext, offset, &containingView);
  
  nsRect parentRect;
  aFrame->GetRect(parentRect);

  float p2t, t2p;
  aPresContext->GetScaledPixelsToTwips(&p2t);

  nsCOMPtr<nsIDeviceContext> dx;
  viewManager->GetDeviceContext(*getter_AddRefs(dx));
  dx->GetAppUnitsToDevUnits(t2p);

  // get the document and the global script object
  nsCOMPtr<nsIPresShell> presShell;
  aPresContext->GetShell(getter_AddRefs(presShell));
  nsCOMPtr<nsIDocument> document;
  presShell->GetDocument(getter_AddRefs(document));
  nsCOMPtr<nsIScriptGlobalObject> scriptGlobalObject;
  document->GetScriptGlobalObject(getter_AddRefs(scriptGlobalObject));
  
  // |xpos| and |ypos| hold the x and y positions of where the popup will be moved to,
  // in _twips_, in the coordinate system of the _parent view_.
  PRInt32 xpos = 0, ypos = 0;

  // if we are anchored to our parent, there are certain things we don't want to do
  // when repositioning the view to fit on the screen, such as end up positioned over
  // the parent. When doing this reposition, we want to move the popup to the side with
  // the most room. The combination of anchor and alignment dictate if we readjst 
  // above/below or to the left/right.
  PRBool anchoredToParent = PR_FALSE;
  PRBool readjustAboveBelow = PR_FALSE;
  
  if ( aXPos != -1 || aYPos != -1 ) {
  
    // for this case, we've been handed a specific x/y location (in client coordinates) for
    // the popup. However, we may be deeply nested in a frameset, etc and so the client coordinates
    // need some adjusting. 
    nsCOMPtr<nsIDOMXULDocument> xulDoc ( do_QueryInterface(document) );
    PRInt32 newXPos = 0, newYPos = 0;
    AdjustClientXYForNestedDocuments ( xulDoc, presShell, aXPos, aYPos, &newXPos, &newYPos );

    xpos = NSIntPixelsToTwips(newXPos, p2t);
    ypos = NSIntPixelsToTwips(newYPos, p2t);
  } 
  else {
    anchoredToParent = PR_TRUE;

    xpos = parentPos.x + offset.x;
    ypos = parentPos.y + offset.y;
    
    // move the popup according to the anchor/alignment attributes. This will also tell us
    // which axis the popup is flush against in case we have to move it around later.
    AdjustPositionForAnchorAlign ( &xpos, &ypos, parentRect, aPopupAnchor, aPopupAlign, &readjustAboveBelow );    
  }
  
  nsCOMPtr<nsIDOMWindow> window(do_QueryInterface(scriptGlobalObject));
  nsCOMPtr<nsIDOMScreen> screen;
  window->GetScreen(getter_AddRefs(screen));
  PRInt32 screenWidth;
  PRInt32 screenHeight;
  screen->GetAvailWidth(&screenWidth);
  screen->GetAvailHeight(&screenHeight);
  
  // Compute info about the screen dimensions. Because of multiple monitor systems,
  // the left or top sides of the screen may be in negative space (main monitor is on the
  // right, etc). We need to be sure to do the right thing.
  PRInt32 screenLeft;
  PRInt32 screenTop;
  screen->GetAvailLeft(&screenLeft);
  screen->GetAvailTop(&screenTop); 
  PRInt32 screenRight;
  if(screenLeft<0)
    screenRight = screenWidth + screenLeft;
  else
    screenRight = screenWidth - screenLeft;  
  PRInt32 screenBottom;
  if(screenTop<0)
    screenBottom = screenHeight + screenTop;
  else
    screenBottom = screenHeight - screenTop; 
  PRInt32 screenTopTwips    = NSIntPixelsToTwips(screenTop, p2t);
  PRInt32 screenLeftTwips   = NSIntPixelsToTwips(screenLeft, p2t);
  PRInt32 screenWidthTwips  = NSIntPixelsToTwips(screenWidth, p2t);
  PRInt32 screenHeightTwips = NSIntPixelsToTwips(screenHeight, p2t);
  PRInt32 screenRightTwips  = NSIntPixelsToTwips(screenRight, p2t);
  PRInt32 screenBottomTwips = NSIntPixelsToTwips(screenBottom, p2t);
  
  // Recall that |xpos| and |ypos| are in the coordinate system of the parent view. In
  // order to determine the screen coordinates of where our view will end up, we
  // need to find the x/y position of the parent view in screen coords. That is done
  // by getting the widget associated with the parent view and determining the offset 
  // based on converting (0,0) in its coordinate space to screen coords. We then
  // offset that point by (|xpos|,|ypos|) to get the true screen coordinates of
  // the view. *whew*
  nsCOMPtr<nsIWidget> parentViewWidget;
  GetWidgetForView ( parentView, *getter_AddRefs(parentViewWidget) );
  nsRect localParentWidgetRect(0,0,0,0), screenParentWidgetRect;
  parentViewWidget->WidgetToScreen ( localParentWidgetRect, screenParentWidgetRect );
  PRInt32 screenViewLocX = NSIntPixelsToTwips(screenParentWidgetRect.x,p2t) + (xpos - parentPos.x);
  PRInt32 screenViewLocY = NSIntPixelsToTwips(screenParentWidgetRect.y,p2t) + (ypos - parentPos.y);

  if ( anchoredToParent ) {
    
    //
    // Popup is anchored to the parent, guarantee that it does not cover the parent. We
    // shouldn't do anything funky if it will already fit on the screen as is.
    //

    // compute screen coordinates of parent frame so we can play with it. Make sure we put it
    // into twips as everything else is as well.
    nsRect screenParentFrameRect ( NSTwipsToIntPixels(offset.x,t2p), NSTwipsToIntPixels(offset.y,t2p),
                                    parentRect.width, parentRect.height );
    parentViewWidget->WidgetToScreen ( screenParentFrameRect, screenParentFrameRect );
    screenParentFrameRect.x = NSIntPixelsToTwips(screenParentFrameRect.x, p2t);
    screenParentFrameRect.y = NSIntPixelsToTwips(screenParentFrameRect.y, p2t);
    
    // if it doesn't fit on the screen, do our magic.
    if ( (screenViewLocX + mRect.width) > screenRightTwips ||
          (screenViewLocY + mRect.height) > screenBottomTwips ) {
      
      // figure out which side of the parent has the most free space so we can move/resize
      // the popup there. This should still work if the parent frame is partially screen.
      PRBool switchSides = IsMoreRoomOnOtherSideOfParent ( readjustAboveBelow, screenViewLocX, screenViewLocY,
                                                            screenParentFrameRect, screenTopTwips, screenLeftTwips,
                                                            screenBottomTwips, screenRightTwips );
      
      // move the popup to the correct side, if necessary. Note that MovePopupToOtherSideOfParent() 
      // can change width/height of |mRect|.
      if ( switchSides )
        MovePopupToOtherSideOfParent ( readjustAboveBelow, &xpos, &ypos, &screenViewLocX, &screenViewLocY, 
                                        screenParentFrameRect, screenTopTwips, screenLeftTwips,
                                        screenBottomTwips, screenRightTwips );
                                        
      // We are allowed to move the popup along the axis to which we're not anchored to the parent
      // in order to get it to not spill off the screen.
      if ( readjustAboveBelow ) {
        // move left to be on screen.
        if ( (screenViewLocX + mRect.width) > screenRightTwips ) {
          PRInt32 moveDistX = (screenViewLocX + mRect.width) - screenRightTwips;
          screenViewLocX -= moveDistX;
          xpos -= moveDistX;
        }
      }
      else {
        // move it up to be on screen.
        if ( (screenViewLocY + mRect.height) > screenBottomTwips ) {
          PRInt32 moveDistY = (screenViewLocY + mRect.height) - screenBottomTwips;
          screenViewLocY -= moveDistY;
          ypos -= moveDistY; 
        } 
      }
      
      // Resize it to fit on the screen. By this point, we've given the popup as much
      // room as we can w/out covering the parent. If it still can't be as big
      // as it wants to be, well, it just has to suck up and deal. 
      PRInt32 xSpillage = (screenViewLocX + mRect.width) - screenRightTwips;
      if ( xSpillage > 0 )
        mRect.width -= xSpillage;
      PRInt32 ySpillage = (screenViewLocY + mRect.height) - screenBottomTwips;
      if ( ySpillage > 0 )
        mRect.height -= ySpillage;

    } // if it doesn't fit on screen
  } // if anchored to parent
  else {
  
    //
    // Popup not anchored to anything, just make sure it's on the screen by any
    // means necessary
    //

    // add back in the parentPos offset. Not sure why, but we need this for mail/news
    // context menus and we can't do this in the case where there popup is anchored.
    screenViewLocX += parentPos.x;
    screenViewLocY += parentPos.y;    
    
    // shrink to fit onto the screen, vertically and horizontally
    if(mRect.width > screenWidthTwips) 
        mRect.width = screenWidthTwips;    
    if(mRect.height > screenHeightTwips)
        mRect.height = screenHeightTwips;   
    
    // we now know where the view is...make sure that it's still onscreen at all!
    if ( screenViewLocX < screenLeftTwips ) {
      PRInt32 moveDistX = screenLeftTwips - screenViewLocX;
      xpos += moveDistX;
      screenViewLocX += moveDistX;
    }
    if ( screenViewLocY < screenTopTwips ) {
      PRInt32 moveDistY = screenTopTwips - screenViewLocY;
      ypos += moveDistY;
      screenViewLocY += moveDistY;
    }

    // ensure it is not even partially offscreen.
    if ( (screenViewLocX + mRect.width) > screenRightTwips ) {
      // as a result of moving the popup, it might end up under the mouse. This
      // would be bad as the subsequent mouse_up would trigger whatever
      // unsuspecting item happens to be at that position. To get around this, make
      // move it so the right edge is where the mouse is, as we're guaranteed
      // that the mouse is on the screen!
      xpos -= mRect.width;      
    }
    if ( (screenViewLocY + mRect.height) > screenBottomTwips )
      ypos -= (screenViewLocY + mRect.height) - screenBottomTwips;
      
  }  

  // finally move and resize it
  viewManager->MoveViewTo(view, xpos, ypos); 
  viewManager->ResizeView(view, mRect.width, mRect.height);
  
  nsAutoString shouldDisplay;
  mContent->GetAttribute(kNameSpaceID_None, nsXULAtoms::menutobedisplayed, shouldDisplay);
  if ( shouldDisplay.Equals("true") )
    mContent->SetAttribute(kNameSpaceID_None, nsXULAtoms::menuactive, "true", PR_TRUE);

  return NS_OK;
}


NS_IMETHODIMP
nsMenuPopupFrame::GetNextMenuItem(nsIMenuFrame* aStart, nsIMenuFrame** aResult)
{
  nsIFrame* currFrame = nsnull;
  nsIFrame* startFrame = nsnull;
  if (aStart) {
    aStart->QueryInterface(kIFrameIID, (void**)&currFrame); 
    if (currFrame) {
      startFrame = currFrame;
      currFrame->GetNextSibling(&currFrame);
    }
  }
  else currFrame = mFrames.FirstChild();
  
  while (currFrame) {
    nsCOMPtr<nsIContent> current;
    currFrame->GetContent(getter_AddRefs(current));

    // See if it's a menu item.
    if (IsValidItem(current)) {
      nsCOMPtr<nsIMenuFrame> menuFrame = do_QueryInterface(currFrame);
      *aResult = menuFrame.get();
      NS_IF_ADDREF(*aResult);
      return NS_OK;
    }
    currFrame->GetNextSibling(&currFrame);
  }

  currFrame = mFrames.FirstChild();

  // Still don't have anything. Try cycling from the beginning.
  while (currFrame && currFrame != startFrame) {
    nsCOMPtr<nsIContent> current;
    currFrame->GetContent(getter_AddRefs(current));
    
    // See if it's a menu item.
    if (IsValidItem(current)) {
      nsCOMPtr<nsIMenuFrame> menuFrame = do_QueryInterface(currFrame);
      *aResult = menuFrame.get();
      NS_IF_ADDREF(*aResult);
      return NS_OK;
    }

    currFrame->GetNextSibling(&currFrame);
  }

  // No luck. Just return our start value.
  *aResult = aStart;
  return NS_OK;
}

NS_IMETHODIMP
nsMenuPopupFrame::GetPreviousMenuItem(nsIMenuFrame* aStart, nsIMenuFrame** aResult)
{
  nsIFrame* currFrame = nsnull;
  nsIFrame* startFrame = nsnull;
  if (aStart) {
    aStart->QueryInterface(kIFrameIID, (void**)&currFrame);
    if (currFrame) {
      startFrame = currFrame;
      currFrame = mFrames.GetPrevSiblingFor(currFrame);
    }
  }
  else currFrame = mFrames.LastChild();

  while (currFrame) {
    nsCOMPtr<nsIContent> current;
    currFrame->GetContent(getter_AddRefs(current));

    // See if it's a menu item.
    if (IsValidItem(current)) {
      nsCOMPtr<nsIMenuFrame> menuFrame = do_QueryInterface(currFrame);
      *aResult = menuFrame.get();
      NS_IF_ADDREF(*aResult);
      return NS_OK;
    }
    currFrame = mFrames.GetPrevSiblingFor(currFrame);
  }

  currFrame = mFrames.LastChild();

  // Still don't have anything. Try cycling from the end.
  while (currFrame && currFrame != startFrame) {
    nsCOMPtr<nsIContent> current;
    currFrame->GetContent(getter_AddRefs(current));
    
    // See if it's a menu item.
    if (IsValidItem(current)) {
      nsCOMPtr<nsIMenuFrame> menuFrame = do_QueryInterface(currFrame);
      *aResult = menuFrame.get();
      NS_IF_ADDREF(*aResult);
      return NS_OK;
    }

    currFrame = mFrames.GetPrevSiblingFor(currFrame);
  }

  // No luck. Just return our start value.
  *aResult = aStart;
  return NS_OK;
}

NS_IMETHODIMP nsMenuPopupFrame::GetCurrentMenuItem(nsIMenuFrame** aResult)
{
  *aResult = mCurrentMenu;
  NS_IF_ADDREF(*aResult);
  return NS_OK;
}

NS_IMETHODIMP nsMenuPopupFrame::SetCurrentMenuItem(nsIMenuFrame* aMenuItem)
{
  if (mCurrentMenu == aMenuItem)
    return NS_OK;
  
  // Unset the current child.
  if (mCurrentMenu) {
    PRBool isOpen = PR_FALSE;
    mCurrentMenu->MenuIsOpen(isOpen);
    mCurrentMenu->SelectMenu(PR_FALSE);
    if (isOpen) {
      // Don't close up immediately.
      // Kick off a close timer.
      KillCloseTimer(); // Ensure we don't have another stray waiting closure.
      PRInt32 menuDelay = 300;   // ms

      nsILookAndFeel * lookAndFeel;
      if (NS_OK == nsComponentManager::CreateInstance(kLookAndFeelCID, nsnull, 
                      kILookAndFeelIID, (void**)&lookAndFeel)) {
        lookAndFeel->GetMetric(nsILookAndFeel::eMetric_SubmenuDelay, menuDelay);
       NS_RELEASE(lookAndFeel);
      }

      // Kick off the timer.
      NS_NewTimer(getter_AddRefs(mCloseTimer));
      mCloseTimer->Init(this, menuDelay, NS_PRIORITY_HIGHEST); 
      mTimerMenu = mCurrentMenu;
    }
  }

  // Set the new child.
  if (aMenuItem) {
    aMenuItem->SelectMenu(PR_TRUE);
  }

  mCurrentMenu = aMenuItem;

  return NS_OK;
}

NS_IMETHODIMP
nsMenuPopupFrame::CaptureMouseEvents(nsIPresContext* aPresContext, PRBool aGrabMouseEvents)
{
  // get its view
  nsIView* view = nsnull;
  GetView(aPresContext, &view);
  nsCOMPtr<nsIViewManager> viewMan;
  PRBool result;

  nsCOMPtr<nsIWidget> widget;

  if (view) {
    view->GetViewManager(*getter_AddRefs(viewMan));
    if (viewMan) {
      view->GetWidget(*getter_AddRefs(widget));
      if (aGrabMouseEvents) {
        viewMan->GrabMouseEvents(view,result);
        mIsCapturingMouseEvents = PR_TRUE;
        //widget->CaptureMouse(PR_TRUE);
      } else {
        viewMan->GrabMouseEvents(nsnull,result);
        mIsCapturingMouseEvents = PR_FALSE;
        //widget->CaptureMouse(PR_FALSE);
      }
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsMenuPopupFrame::Escape(PRBool& aHandledFlag)
{
  if (!mCurrentMenu)
    return NS_OK;

  // See if our menu is open.
  PRBool isOpen = PR_FALSE;
  mCurrentMenu->MenuIsOpen(isOpen);
  if (isOpen) {
    // Let the child menu handle this.
    mCurrentMenu->Escape(aHandledFlag);
    if (!aHandledFlag) {
      // We should close up.
      mCurrentMenu->OpenMenu(PR_FALSE);
      aHandledFlag = PR_TRUE;
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsMenuPopupFrame::Enter()
{
  // Give it to the child.
  if (mCurrentMenu)
    mCurrentMenu->Enter();

  return NS_OK;
}

nsIMenuFrame*
nsMenuPopupFrame::FindMenuWithShortcut(PRUint32 aLetter)
{
  // Enumerate over our list of frames.
  nsIFrame* currFrame = mFrames.FirstChild();
  while (currFrame) {
    nsCOMPtr<nsIContent> current;
    currFrame->GetContent(getter_AddRefs(current));
    
    // See if it's a menu item.
    if (IsValidItem(current)) {
      // Get the shortcut attribute.
      nsAutoString shortcutKey = "";
      current->GetAttribute(kNameSpaceID_None, nsXULAtoms::accesskey, shortcutKey);
      if (shortcutKey.Length() > 0) {
        // We've got something.
        char tempChar[2];
        tempChar[0] = aLetter;
        tempChar[1] = 0;
        nsAutoString tempChar2 = tempChar;
  
        if (shortcutKey.EqualsIgnoreCase(tempChar2)) {
          // We match!
          nsCOMPtr<nsIMenuFrame> menuFrame = do_QueryInterface(currFrame);
          if (menuFrame)
            return menuFrame.get();
          return nsnull;
        }
      }
    }
    currFrame->GetNextSibling(&currFrame);
  }
  return nsnull;
}

NS_IMETHODIMP 
nsMenuPopupFrame::ShortcutNavigation(PRUint32 aLetter, PRBool& aHandledFlag)
{
  if (mCurrentMenu) {
    PRBool isOpen = PR_FALSE;
    mCurrentMenu->MenuIsOpen(isOpen);
    if (isOpen) {
      // No way this applies to us. Give it to our child.
      mCurrentMenu->ShortcutNavigation(aLetter, aHandledFlag);
      return NS_OK;
    }
  }

  // This applies to us. Let's see if one of the shortcuts applies
  nsIMenuFrame* result = FindMenuWithShortcut(aLetter);
  if (result) {
    // We got one!
    aHandledFlag = PR_TRUE;
    SetCurrentMenuItem(result);
    result->Enter();
  }

  return NS_OK;
}

NS_IMETHODIMP
nsMenuPopupFrame::KeyboardNavigation(PRUint32 aDirection, PRBool& aHandledFlag)
{
  // This method only gets called if we're open.
  if (!mCurrentMenu && (aDirection == NS_VK_RIGHT || aDirection == NS_VK_LEFT)) {
    // We've been opened, but we haven't had anything selected.
    // We can handle RIGHT, but our parent handles LEFT.
    if (aDirection == NS_VK_RIGHT) {
      nsIMenuFrame* nextItem;
      GetNextMenuItem(nsnull, &nextItem);
      if (nextItem) {
        aHandledFlag = PR_TRUE;
        SetCurrentMenuItem(nextItem);
      }
    }
    return NS_OK;
  }

  PRBool isContainer = PR_FALSE;
  PRBool isOpen = PR_FALSE;
  if (mCurrentMenu) {
    mCurrentMenu->MenuIsContainer(isContainer);
    mCurrentMenu->MenuIsOpen(isOpen);

    if (isOpen) {
      // Give our child a shot.
      mCurrentMenu->KeyboardNavigation(aDirection, aHandledFlag);
    }
    else if (aDirection == NS_VK_RIGHT && isContainer) {
      // The menu is not yet open. Open it and select the first item.
      aHandledFlag = PR_TRUE;
      mCurrentMenu->OpenMenu(PR_TRUE);
      mCurrentMenu->SelectFirstItem();
    }
  }

  if (aHandledFlag)
    return NS_OK; // The child menu took it for us.

  // For the vertical direction, we can move up or down.
  if (aDirection == NS_VK_UP || aDirection == NS_VK_DOWN) {
    
    nsIMenuFrame* nextItem;
    
    if (aDirection == NS_VK_DOWN)
      GetNextMenuItem(mCurrentMenu, &nextItem);
    else GetPreviousMenuItem(mCurrentMenu, &nextItem);

    SetCurrentMenuItem(nextItem);

    aHandledFlag = PR_TRUE;
  }
  else if (mCurrentMenu && isContainer && isOpen) {
    if (aDirection == NS_VK_LEFT) {
      // Close it up.
      mCurrentMenu->OpenMenu(PR_FALSE);
      aHandledFlag = PR_TRUE;
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsMenuPopupFrame::GetParentPopup(nsIMenuParent** aMenuParent)
{
  *aMenuParent = nsnull;
  nsIFrame* frame;
  GetParent(&frame);
  if (frame) {
    nsIFrame* grandparent;
    frame->GetParent(&grandparent);
    if (grandparent) {
      nsCOMPtr<nsIMenuParent> menuParent = do_QueryInterface(grandparent);
      if (menuParent) {
        *aMenuParent = menuParent.get();
        NS_ADDREF(*aMenuParent);
      }
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
nsMenuPopupFrame::HideChain()
{
  // Stop capturing rollups
  // (must do this during Hide, which happens before the menu item is executed,
  // since this reinstates normal event handling.)
  if (nsMenuFrame::mDismissalListener)
    nsMenuFrame::mDismissalListener->Unregister();
  
  nsIFrame* frame;
  GetParent(&frame);
  if (frame) {
    nsCOMPtr<nsIPopupSetFrame> popupSetFrame = do_QueryInterface(frame);
    if (popupSetFrame) {
      // Destroy the popup.
      popupSetFrame->HidePopup();
      return NS_OK;
    }
    
    nsCOMPtr<nsIMenuFrame> menuFrame = do_QueryInterface(frame);
    if (!menuFrame)
      return NS_OK;

    menuFrame->ActivateMenu(PR_FALSE);
    menuFrame->SelectMenu(PR_FALSE);

    // Get the parent.
    nsCOMPtr<nsIMenuParent> menuParent;
    menuFrame->GetMenuParent(getter_AddRefs(menuParent));
    if (menuParent)
      menuParent->HideChain();
  }

  return NS_OK;
}

NS_IMETHODIMP
nsMenuPopupFrame::DismissChain()
{
  // Stop capturing rollups
  if (nsMenuFrame::mDismissalListener)
    nsMenuFrame::mDismissalListener->Unregister();
  
  // Get our menu parent.
  nsIFrame* frame;
  GetParent(&frame);
  if (frame) {
    nsCOMPtr<nsIPopupSetFrame> popupSetFrame = do_QueryInterface(frame);
    if (popupSetFrame) {
      // Destroy the popup.
      popupSetFrame->DestroyPopup();
      return NS_OK;
    }

    nsCOMPtr<nsIMenuFrame> menuFrame = do_QueryInterface(frame);
    if (!menuFrame)
      return NS_OK;
    
    menuFrame->OpenMenu(PR_FALSE);

    // Get the parent.
    nsCOMPtr<nsIMenuParent> menuParent;
    menuFrame->GetMenuParent(getter_AddRefs(menuParent));
    if (menuParent)
      menuParent->DismissChain();
  }

  return NS_OK;
}

NS_IMETHODIMP
nsMenuPopupFrame::GetWidget(nsIWidget **aWidget)
{
  // Get parent view
  nsIView * view = nsnull;
  nsMenuPopupFrame::GetNearestEnclosingView(mPresContext, this, &view);
  if (!view)
    return NS_OK;

  view->GetWidget(*aWidget);
  return NS_OK;
}

NS_IMETHODIMP
nsMenuPopupFrame::CreateDismissalListener()
{
  nsMenuDismissalListener *listener = new nsMenuDismissalListener();
  if (!listener) return NS_ERROR_OUT_OF_MEMORY;
  nsMenuFrame::mDismissalListener = listener;
  NS_ADDREF(listener);
  return NS_OK;
}

NS_IMETHODIMP
nsMenuPopupFrame::InstallKeyboardNavigator()
{
  if (mKeyboardNavigator)
    return NS_OK;

  nsCOMPtr<nsIDocument> doc;
  mContent->GetDocument(*getter_AddRefs(doc));
  nsCOMPtr<nsIDOMEventReceiver> target = do_QueryInterface(doc);
  
  mTarget = target;
  mKeyboardNavigator = new nsMenuListener(this);
  NS_IF_ADDREF(mKeyboardNavigator);

  target->AddEventListener("keypress", (nsIDOMKeyListener*)mKeyboardNavigator, PR_TRUE); 
  target->AddEventListener("keydown", (nsIDOMKeyListener*)mKeyboardNavigator, PR_TRUE);  
  target->AddEventListener("keyup", (nsIDOMKeyListener*)mKeyboardNavigator, PR_TRUE);   
  
  return NS_OK;
}

NS_IMETHODIMP
nsMenuPopupFrame::RemoveKeyboardNavigator()
{
  if (!mKeyboardNavigator)
    return NS_OK;

  mTarget->RemoveEventListener("keypress", (nsIDOMKeyListener*)mKeyboardNavigator, PR_TRUE);
  mTarget->RemoveEventListener("keydown", (nsIDOMKeyListener*)mKeyboardNavigator, PR_TRUE);
  mTarget->RemoveEventListener("keyup", (nsIDOMKeyListener*)mKeyboardNavigator, PR_TRUE);
  
  NS_IF_RELEASE(mKeyboardNavigator);

  return NS_OK;
}

// helpers /////////////////////////////////////////////////////////////

PRBool 
nsMenuPopupFrame::IsValidItem(nsIContent* aContent)
{
  nsCOMPtr<nsIAtom> tag;
  aContent->GetTag(*getter_AddRefs(tag));
  if (tag && (tag.get() == nsXULAtoms::menu ||
              tag.get() == nsXULAtoms::menuitem) &&
      !IsDisabled(aContent))
      return PR_TRUE;

  return PR_FALSE;
}

PRBool 
nsMenuPopupFrame::IsDisabled(nsIContent* aContent)
{
  nsString disabled = "";
  aContent->GetAttribute(kNameSpaceID_None, nsHTMLAtoms::disabled, disabled);
  if (disabled == "true")
    return PR_TRUE;
  return PR_FALSE;
}

NS_IMETHODIMP 
nsMenuPopupFrame::HandleEvent(nsIPresContext* aPresContext, 
                              nsGUIEvent*     aEvent,
                              nsEventStatus*  aEventStatus)
{
  return nsBoxFrame::HandleEvent(aPresContext, aEvent, aEventStatus);
}

NS_IMETHODIMP
nsMenuPopupFrame::Destroy(nsIPresContext* aPresContext)
{
  //nsCOMPtr<nsIDOMEventReceiver> target = do_QueryInterface(mContent);
  //target->RemoveEventListener("mousemove", mMenuPopupEntryListener, PR_TRUE);
  return nsBoxFrame::Destroy(aPresContext);
}

NS_IMETHODIMP
nsMenuPopupFrame::GetFrameForPoint(nsIPresContext* aPresContext,
                                   const nsPoint& aPoint,
                                   nsIFrame** aFrame)
{
  nsRect rect;
  GetRect(rect);
  if (rect.Contains(aPoint)) {
    return nsBoxFrame::GetFrameForPoint(aPresContext, aPoint, aFrame);
  }
  
  *aFrame = this;
  return NS_OK;
}

NS_IMETHODIMP_(void)
nsMenuPopupFrame::Notify(nsITimer* aTimer)
{
  // Our timer has fired.
  if (aTimer == mCloseTimer.get()) {
    PRBool menuOpen = PR_FALSE;
    mTimerMenu->MenuIsOpen(menuOpen);
    if (menuOpen) {
      if (mCurrentMenu != mTimerMenu) {
        // See if our child has a current menu.
        // If so, then we need to be selected.
        nsIFrame* child;
        mTimerMenu->GetMenuChild(&child);
        if (child) {
          nsCOMPtr<nsIMenuParent> menuPopup = do_QueryInterface(child);
          nsCOMPtr<nsIMenuFrame> frame;
          menuPopup->GetCurrentMenuItem(getter_AddRefs(frame));
          if (frame) {
            SetCurrentMenuItem(mTimerMenu);
          }
          else {
            // Close up.
            mTimerMenu->OpenMenu(PR_FALSE);
          }
        }
        else {
          // Close up.
          mTimerMenu->OpenMenu(PR_FALSE);
        }
      }
    }
    mCloseTimer->Cancel();
    mCloseTimer = nsnull;
  }
  
  mCloseTimer = nsnull;
  mTimerMenu = nsnull;
}

NS_IMETHODIMP
nsMenuPopupFrame::KillCloseTimer()
{
  if (mCloseTimer && mTimerMenu) {
    PRBool menuOpen = PR_FALSE;
    mTimerMenu->MenuIsOpen(menuOpen);
    if (menuOpen) {
      mTimerMenu->OpenMenu(PR_FALSE);
    }
    mCloseTimer->Cancel();
    mCloseTimer = nsnull;
    mTimerMenu = nsnull;
  }
  return NS_OK;
}

