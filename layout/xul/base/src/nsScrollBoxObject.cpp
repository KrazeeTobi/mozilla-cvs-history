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
 * Original Author: David W. Hyatt (hyatt@netscape.com)
 *
 * Contributor(s): 
 */
#include "nsCOMPtr.h"
#include "nsIScrollBoxObject.h"
#include "nsBoxObject.h"
#include "nsIPresShell.h"
#include "nsIContent.h"
#include "nsIDocument.h"
#include "nsIDOMDocument.h"
#include "nsIDOMElement.h"
#include "nsIPresContext.h"
#include "nsIFrame.h"
#include "nsIScrollableView.h"
#include "nsIBox.h"

static NS_DEFINE_IID(kScrollViewIID, NS_ISCROLLABLEVIEW_IID);

class nsScrollBoxObject : public nsIScrollBoxObject, public nsBoxObject
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_NSISCROLLBOXOBJECT

  nsScrollBoxObject();
  virtual ~nsScrollBoxObject();

  virtual nsIScrollableView* GetScrollableView();

  /* additional members */
};

/* Implementation file */

NS_INTERFACE_MAP_BEGIN(nsScrollBoxObject)
  NS_INTERFACE_MAP_ENTRY(nsIScrollBoxObject)
NS_INTERFACE_MAP_END_INHERITING(nsBoxObject)

NS_IMPL_ADDREF_INHERITED(nsScrollBoxObject, nsBoxObject)
NS_IMPL_RELEASE_INHERITED(nsScrollBoxObject, nsBoxObject)

nsScrollBoxObject::nsScrollBoxObject()
{
  NS_INIT_ISUPPORTS();
  /* member initializers and constructor code */
}

nsScrollBoxObject::~nsScrollBoxObject()
{
  /* destructor code */
}

/* void scrollTo (in long x, in long y); */
NS_IMETHODIMP nsScrollBoxObject::ScrollTo(PRInt32 x, PRInt32 y)
{
  nsIScrollableView* scrollableView = GetScrollableView();
  if (!scrollableView)
    return NS_ERROR_FAILURE;
  
  return scrollableView->ScrollTo(x,y, NS_SCROLL_PROPERTY_ALWAYS_BLIT);
}

/* void scrollBy (in long dx, in long dy); */
NS_IMETHODIMP nsScrollBoxObject::ScrollBy(PRInt32 dx, PRInt32 dy)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void scrollByLine (in long dlines); */
NS_IMETHODIMP nsScrollBoxObject::ScrollByLine(PRInt32 dlines)
{
  nsIScrollableView* scrollableView = GetScrollableView();
  if (!scrollableView)
    return NS_ERROR_FAILURE;

  return scrollableView->ScrollByLines(0, dlines);
}

/* void scrollByIndex (in long dindexes); */
NS_IMETHODIMP nsScrollBoxObject::ScrollByIndex(PRInt32 dindexes)
{
    nsIScrollableView* scrollableView = GetScrollableView();
    if (!scrollableView)
       return NS_ERROR_FAILURE;

    // get our box
    nsIFrame* frame = GetFrame();
    nsCOMPtr<nsIBox> box (do_QueryInterface(frame));

    nsRect rect;
    nsIBox* scrolledBox;
    nsIBox* child;

    // get the scrolled box
    box->GetChildBox(&scrolledBox);

    // now get the scrolled boxes first child.
    scrolledBox->GetChildBox(&child);

    PRBool horiz = PR_FALSE;
    scrolledBox->GetOrientation(horiz);
    nsPoint cp;
    scrollableView->GetScrollPosition(cp.x,cp.y);
    nscoord diff = 0;
    PRInt32 curIndex = 0;

    // first find out what index we are currently at
    while(child) {
      child->GetBounds(rect);
      if (horiz) {
        diff = rect.x + rect.width;
        if (diff > cp.x) {
          break;
        }
      } else {
        diff = rect.y + rect.height;
        if (diff > cp.y) {
          break;
        }
      }
      child->GetNextBox(&child);
      curIndex++;
    }

    PRInt32 count = 0;

    if (dindexes == 0)
       return NS_OK;

    if (dindexes > 0) {
      while(child) {
        child->GetNextBox(&child);
        if (child)
          child->GetBounds(rect);
        count++;
        if (count >= dindexes)
          break;
      }

   } else if (dindexes < 0) {
      scrolledBox->GetChildBox(&child);
      while(child) {
        child->GetBounds(rect);
        if (count >= curIndex + dindexes)
          break;

        count++;
        child->GetNextBox(&child);

      }
   }

   if (horiz)
       return scrollableView->ScrollTo(rect.x, cp.y, NS_SCROLL_PROPERTY_ALWAYS_BLIT);
   else
       return scrollableView->ScrollTo(cp.x, rect.y, NS_SCROLL_PROPERTY_ALWAYS_BLIT);
}

/* void scrollToLine (in long line); */
NS_IMETHODIMP nsScrollBoxObject::ScrollToLine(PRInt32 line)
{
  nsIScrollableView* scrollableView = GetScrollableView();
  if (!scrollableView)
    return NS_ERROR_FAILURE;
  
  nscoord height = 0;
  scrollableView->GetLineHeight(&height);
  scrollableView->ScrollTo(0,height*line, NS_SCROLL_PROPERTY_ALWAYS_BLIT);

  return NS_OK;
}

/* void scrollToElement (in nsIDOMElement child); */
NS_IMETHODIMP nsScrollBoxObject::ScrollToElement(nsIDOMElement *child)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void scrollToIndex (in long index); */
NS_IMETHODIMP nsScrollBoxObject::ScrollToIndex(PRInt32 index)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getPosition (out long x, out long y); */
NS_IMETHODIMP nsScrollBoxObject::GetPosition(PRInt32 *x, PRInt32 *y)
{
  nsIScrollableView* scrollableView = GetScrollableView();
  if (!scrollableView)
    return NS_ERROR_FAILURE;
  
  return scrollableView->GetScrollPosition(*x,*y);
}

/* void getScrolledSize (out long width, out long height); */
NS_IMETHODIMP nsScrollBoxObject::GetScrolledSize(PRInt32 *width, PRInt32 *height)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void ensureElementIsVisible (in nsIDOMElement child); */
NS_IMETHODIMP nsScrollBoxObject::EnsureElementIsVisible(nsIDOMElement *child)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void ensureIndexIsVisible (in long index); */
NS_IMETHODIMP nsScrollBoxObject::EnsureIndexIsVisible(PRInt32 index)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void ensureLineIsVisible (in long line); */
NS_IMETHODIMP nsScrollBoxObject::EnsureLineIsVisible(PRInt32 line)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

nsIScrollableView* 
nsScrollBoxObject::GetScrollableView()
{
  // get the frame.
  nsIFrame* frame = GetFrame();
  if (!frame) 
    return nsnull;
  
  nsCOMPtr<nsIPresContext> context;
  mPresShell->GetPresContext(getter_AddRefs(context));
  nsIView* view;
  frame->GetView(context, &view);
  nsIScrollableView* scrollingView = nsnull;
  if (NS_SUCCEEDED(view->QueryInterface(kScrollViewIID, (void**)&scrollingView))) {  
    return scrollingView;
  }

  return nsnull;
}

nsresult
NS_NewScrollBoxObject(nsIBoxObject** aResult)
{
  *aResult = new nsScrollBoxObject;
  if (!*aResult)
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(*aResult);
  return NS_OK;
}




#if 0

----------------------------------------------

//#define XULTREE

// XXX Hack
#include "nsTreeOuterFrame.h"

class nsTreeBoxObject : public nsITreeBoxObject, public nsBoxObject
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSITREEBOXOBJECT

  nsTreeBoxObject();
  virtual ~nsTreeBoxObject();
  
  // XXX Will go away as soon as I get off tables.
  nsIFrame* GetFrame();

protected:
};

/* Implementation file */
NS_IMPL_ADDREF(nsTreeBoxObject)
NS_IMPL_RELEASE(nsTreeBoxObject)

NS_IMETHODIMP 
nsTreeBoxObject::QueryInterface(REFNSIID iid, void** aResult)
{
  if (!aResult)
    return NS_ERROR_NULL_POINTER;
  
  if (iid.Equals(NS_GET_IID(nsITreeBoxObject))) {
    *aResult = (nsITreeBoxObject*)this;
    NS_ADDREF(this);
    return NS_OK;
  }

  return nsBoxObject::QueryInterface(iid, aResult);
}
  
nsTreeBoxObject::nsTreeBoxObject()
{
  NS_INIT_ISUPPORTS();
}

nsTreeBoxObject::~nsTreeBoxObject()
{
  /* destructor code */
}

// XXX Whole function is a hack that will go away.
nsIFrame*
nsTreeBoxObject::GetFrame()
{
#ifdef XULTREE
  return nsBoxObject::GetFrame();
#else
  nsIFrame* frame = nsBoxObject::GetFrame();
  if (!frame)
    return nsnull;

  nsTreeOuterFrame* outerFrame = (nsTreeOuterFrame*)frame;
  nsCOMPtr<nsIPresContext> presContext;
  mPresShell->GetPresContext(getter_AddRefs(presContext));
  
  nsITreeFrame* treeFrame = outerFrame->FindTreeFrame(presContext);
  if (!treeFrame)
    return nsnull;

  return (nsIFrame*)treeFrame;
#endif
}

/* void ensureIndexIsVisible (in long rowIndex); */
NS_IMETHODIMP nsTreeBoxObject::EnsureIndexIsVisible(PRInt32 aRowIndex)
{
  nsIFrame* frame = GetFrame();
  if (!frame)
    return NS_OK;
  
  nsCOMPtr<nsITreeFrame> treeFrame(do_QueryInterface(frame));
  return treeFrame->EnsureRowIsVisible(aRowIndex);
}

/* void scrollToIndex (in long rowIndex); */
NS_IMETHODIMP nsTreeBoxObject::ScrollToIndex(PRInt32 aRowIndex)
{
  nsIFrame* frame = GetFrame();
  if (!frame)
    return NS_OK;
  
  nsCOMPtr<nsITreeFrame> treeFrame(do_QueryInterface(frame));
  return treeFrame->ScrollToIndex(aRowIndex);
}

/* nsIDOMElement getNextItem (in nsIDOMElement startItem, in long delta); */
NS_IMETHODIMP nsTreeBoxObject::GetNextItem(nsIDOMElement *aStartItem, PRInt32 aDelta, nsIDOMElement **aResult)
{
  nsIFrame* frame = GetFrame();
  if (!frame)
    return NS_OK;

  nsCOMPtr<nsITreeFrame> treeFrame(do_QueryInterface(frame));
  
  return treeFrame->GetNextItem(aStartItem, aDelta, aResult);
}

/* nsIDOMElement getPreviousItem (in nsIDOMElement startItem, in long delta); */
NS_IMETHODIMP nsTreeBoxObject::GetPreviousItem(nsIDOMElement *aStartItem, PRInt32 aDelta, nsIDOMElement **aResult)
{
  nsIFrame* frame = GetFrame();
  if (!frame)
    return NS_OK;

  nsCOMPtr<nsITreeFrame> treeFrame(do_QueryInterface(frame));
  return treeFrame->GetPreviousItem(aStartItem, aDelta, aResult);
}

/* nsIDOMElement getItemAtIndex (in long index); */
NS_IMETHODIMP nsTreeBoxObject::GetItemAtIndex(PRInt32 index, nsIDOMElement **_retval)
{
  nsIFrame* frame = GetFrame();
  if (!frame)
    return NS_OK;

  nsCOMPtr<nsITreeFrame> treeFrame(do_QueryInterface(frame));
  return treeFrame->GetItemAtIndex(index, _retval);
}

/* long getIndexOfItem (in nsIDOMElement item); */
NS_IMETHODIMP nsTreeBoxObject::GetIndexOfItem(nsIDOMElement* aElement, PRInt32 *aResult)
{
  *aResult = -1;

  nsIFrame* frame = GetFrame();
  if (!frame)
    return NS_OK;

  nsCOMPtr<nsITreeFrame> treeFrame(do_QueryInterface(frame));
  nsCOMPtr<nsIPresContext> presContext;
  mPresShell->GetPresContext(getter_AddRefs(presContext));
  return treeFrame->GetIndexOfItem(presContext, aElement, aResult);
}

/* void getNumberOfVisibleRows (); */
NS_IMETHODIMP nsTreeBoxObject::GetNumberOfVisibleRows(PRInt32 *aResult)
{
  nsIFrame* frame = GetFrame();
  if (!frame)
    return NS_OK;
  
  nsCOMPtr<nsITreeFrame> treeFrame(do_QueryInterface(frame));
  return treeFrame->GetNumberOfVisibleRows(aResult);
}

/* void getIndexOfFirstVisibleRow (); */
NS_IMETHODIMP nsTreeBoxObject::GetIndexOfFirstVisibleRow(PRInt32 *aResult)
{
  nsIFrame* frame = GetFrame();
  if (!frame)
    return NS_OK;
  
  nsCOMPtr<nsITreeFrame> treeFrame(do_QueryInterface(frame));
  return treeFrame->GetIndexOfFirstVisibleRow(aResult);
}

// Creation Routine ///////////////////////////////////////////////////////////////////////

nsresult
NS_NewTreeBoxObject(nsIBoxObject** aResult)
{
  *aResult = new nsTreeBoxObject;
  if (!*aResult)
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(*aResult);
  return NS_OK;
}

#endif
