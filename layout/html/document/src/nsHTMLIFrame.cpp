/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "nsHTMLContainer.h"
#include "nsLeafFrame.h"
#include "nsIWebWidget.h"
#include "nsIPresContext.h"
#include "nsHTMLIIDs.h"
#include "nsRepository.h"
#include "nsIStreamListener.h"
#include "nsIURL.h"
#include "nsIDocument.h"
#include "nsIWebFrame.h"
#include "nsIView.h"

static NS_DEFINE_IID(kIStreamListenerIID, NS_ISTREAMNOTIFICATION_IID);
static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
static NS_DEFINE_IID(kIWebWidgetIID, NS_IWEBWIDGET_IID);
static NS_DEFINE_IID(kIWebFrameIID, NS_IWEBFRAME_IID);
static NS_DEFINE_IID(kCWebWidgetCID, NS_WEBWIDGET_CID);

// XXX temporary until doc manager/loader is in place
class TempObserver : public nsIStreamListener
{
public:
  TempObserver() {}

  ~TempObserver() {}
  // nsISupports
  NS_DECL_ISUPPORTS

  // nsIStreamListener
  NS_IMETHOD GetBindInfo(void);
  NS_IMETHOD OnProgress(PRInt32 aProgress, PRInt32 aProgressMax,
                        const nsString& aMsg);
  NS_IMETHOD OnStartBinding(const char *aContentType);
  NS_IMETHOD OnDataAvailable(nsIInputStream *pIStream, PRInt32 length);
  NS_IMETHOD OnStopBinding(PRInt32 status, const nsString& aMsg);

protected:

  nsString mURL;
  nsString mOverURL;
  nsString mOverTarget;
};

class nsHTMLIFrameFrame : public nsLeafFrame, public nsIWebFrame {

public:

  nsHTMLIFrameFrame(nsIContent* aContent, nsIFrame* aParentFrame);

  /**
    * @see nsIFrame::Paint
    */
  NS_IMETHOD Paint(nsIPresContext& aPresContext,
                   nsIRenderingContext& aRenderingContext,
                   const nsRect& aDirtyRect);

  /**
    * @see nsIFrame::Reflow
    */
  NS_IMETHOD Reflow(nsIPresContext*      aCX,
                    nsReflowMetrics&     aDesiredSize,
                    const nsReflowState& aReflowState,
                    nsReflowStatus&      aStatus);

  NS_IMETHOD MoveTo(nscoord aX, nscoord aY);
  NS_IMETHOD SizeTo(nscoord aWidth, nscoord aHeight);

  virtual nsIWebWidget* GetWebWidget();

  float GetTwipsToPixels();

  NS_DECL_ISUPPORTS

protected:

  virtual ~nsHTMLIFrameFrame();

  virtual void GetDesiredSize(nsIPresContext* aPresContext,
                              const nsReflowState& aReflowState,
                              nsReflowMetrics& aDesiredSize);

  void CreateWebWidget(nscoord aWidgth, nscoord aHeight, nsString& aURL); 

  nsIWebWidget* mWebWidget;

  // XXX fix these
  TempObserver* mTempObserver;
};

class nsHTMLIFrame : public nsHTMLContainer {
public:
 
  virtual nsresult  CreateFrame(nsIPresContext*  aPresContext,
                                nsIFrame*        aParentFrame,
                                nsIStyleContext* aStyleContext,
                                nsIFrame*&       aResult);

#if 0
  virtual nsContentAttr GetAttribute(nsIAtom* aAttribute,
                                     nsHTMLValue& aValue) const;

  virtual void SetAttribute(nsIAtom* aAttribute, const nsString& aValue);
#endif

protected:
  nsHTMLIFrame(nsIAtom* aTag);
  virtual  ~nsHTMLIFrame();
  
  friend nsresult NS_NewHTMLIFrame(nsIHTMLContent** aInstancePtrResult,
                                   nsIAtom* aTag);

};

// nsHTMLIFrameFrame

nsHTMLIFrameFrame::nsHTMLIFrameFrame(nsIContent* aContent, nsIFrame* aParentFrame)
  : nsLeafFrame(aContent, aParentFrame)
{
  mWebWidget = nsnull;
  mTempObserver = new TempObserver();
}

nsHTMLIFrameFrame::~nsHTMLIFrameFrame()
{
  NS_IF_RELEASE(mWebWidget);
  delete mTempObserver;
}

NS_IMPL_ADDREF(nsHTMLIFrameFrame);
NS_IMPL_RELEASE(nsHTMLIFrameFrame);

nsresult
nsHTMLIFrameFrame::QueryInterface(const nsIID& aIID,
                                 void** aInstancePtrResult)
{
  NS_PRECONDITION(nsnull != aInstancePtrResult, "null pointer");
  if (nsnull == aInstancePtrResult) {
    return NS_ERROR_NULL_POINTER;
  }
  if (aIID.Equals(kIWebFrameIID)) {
    *aInstancePtrResult = (void*) ((nsIWebFrame*)this);
    AddRef();
    return NS_OK;
  }
  if (aIID.Equals(kISupportsIID)) {
    *aInstancePtrResult = (void*) ((nsISupports*)((nsIWebFrame*)this));
    AddRef();
    return NS_OK;
  }
  return nsLeafFrame::QueryInterface(aIID, aInstancePtrResult);
}

nsIWebWidget* nsHTMLIFrameFrame::GetWebWidget()
{
  NS_IF_ADDREF(mWebWidget);
  return mWebWidget;
}

float nsHTMLIFrameFrame::GetTwipsToPixels()
{
  nsISupports* parentSup;
  if (mWebWidget) {
    mWebWidget->GetContainer(&parentSup);
    if (parentSup) {
      nsIWebWidget* parentWidget;
      nsresult res = parentSup->QueryInterface(kIWebWidgetIID, (void**)&parentWidget);
      NS_RELEASE(parentSup);
      if (NS_OK == res) {
        nsIPresContext* presContext = parentWidget->GetPresContext();
        NS_RELEASE(parentWidget);
        if (presContext) {
          float ret = presContext->GetTwipsToPixels();
          NS_RELEASE(presContext);
          return ret;
        } 
      } else {
        NS_ASSERTION(0, "invalid web widget container");
      }
    } else {
      NS_ASSERTION(0, "invalid web widget container");
    }
  }
  return (float)0.05;  // this should not be reached
}


NS_METHOD
nsHTMLIFrameFrame::MoveTo(nscoord aX, nscoord aY)
{
printf("MoveTo %d %d \n", aX, aY);
  if ((aX != mRect.x) || (aY != mRect.y)) {
    mRect.x = aX;
    mRect.y = aY;
    if (mWebWidget) {
      float t2p = GetTwipsToPixels();
      mWebWidget->Move(NS_TO_INT_ROUND(t2p * aX),
                       NS_TO_INT_ROUND(t2p * aY));
    }
#if 0
    // Let the view know
    nsIView* view = nsnull;
    GetView(view);
    if (nsnull != view) {
      // Position view relative to it's parent, not relative to our
      // parent frame (our parent frame may not have a view). Also,
      // inset the view by the border+padding if present
      nsIView* parentWithView;
      nsPoint origin;
      GetOffsetFromView(origin, parentWithView);
      view->SetPosition(origin.x, origin.y);
      NS_IF_RELEASE(parentWithView);
      NS_RELEASE(view);
    }
#endif
  }
  return NS_OK;
}

NS_METHOD
nsHTMLIFrameFrame::SizeTo(nscoord aWidth, nscoord aHeight)
{
  mRect.width = aWidth;
  mRect.height = aHeight;
#if 0
  // Let the view know the correct size
  nsIView* view = nsnull;
  GetView(view);
  if (nsnull != view) {
    // XXX combo boxes need revision, they cannot have their height altered
    view->SetDimensions(aWidth, aHeight);
    NS_RELEASE(view);
  }
#endif
  return NS_OK;
}

NS_IMETHODIMP
nsHTMLIFrameFrame::Paint(nsIPresContext& aPresContext,
                         nsIRenderingContext& aRenderingContext,
                         const nsRect& aDirtyRect)
{
  //mWebWidget->Show();
  return NS_OK;
}


void TempMakeAbsURL(nsIContent* aContent, nsString& aRelURL, nsString& aAbsURL)
{
  nsIURL* docURL = nsnull;
  nsIDocument* doc = nsnull;
  aContent->GetDocument(doc);
  if (nsnull != doc) {
    docURL = doc->GetDocumentURL();
    NS_RELEASE(doc);
  }

  nsAutoString base;
  nsresult rv = NS_MakeAbsoluteURL(docURL, base, aRelURL, aAbsURL);
  NS_IF_RELEASE(docURL);
}

void nsHTMLIFrameFrame::CreateWebWidget(nscoord aWidth, nscoord aHeight, nsString& aURL) 
{
  NSRepository::CreateInstance(kCWebWidgetCID, nsnull, kIWebWidgetIID, (void**)&mWebWidget);

  if (nsnull == mWebWidget) {
    NS_ASSERTION(0, "could not create web widget");
    return;
  }
  nsresult result;

  // Get the parent web widget
  nsIFrame* parent;
  GetGeometricParent(parent);
  nsIWebWidget* parentWebWidget = nsnull;
  while (nsnull != parent) {
    nsIWebFrame* webFrame;
    result = parent->QueryInterface(kIWebFrameIID, (void**)&webFrame);
    if (NS_OK == result) {
      parentWebWidget = webFrame->GetWebWidget();
      break;
    } 
    parent->GetGeometricParent(parent);
  }
  if (!parentWebWidget) {
    parentWebWidget = mWebWidget->GetRootWebWidget();
  }
  mWebWidget->SetContainer(parentWebWidget);

  nsIPresContext* presContext = parentWebWidget->GetPresContext();

  float t2p = presContext->GetTwipsToPixels();

  nsIView* parentWithView;
  nsPoint origin;
  GetOffsetFromView(origin, parentWithView);
  //view->SetPosition(origin.x, origin.y);
  NS_IF_RELEASE(parentWithView);
  //NS_RELEASE(view);

  
  nsRect rect(NS_TO_INT_ROUND(origin.x * t2p), NS_TO_INT_ROUND(origin.y * t2p), 
              NS_TO_INT_ROUND(aWidth * t2p), NS_TO_INT_ROUND(aHeight * t2p));
  nsIWidget* parentWin;
  GetWindow(parentWin);
  mWebWidget->Init(parentWin->GetNativeData(NS_NATIVE_WINDOW), rect);
  NS_IF_RELEASE(parentWin);

  // load the document
  nsString absURL;
  TempMakeAbsURL(mContent, aURL, absURL);
  mWebWidget->LoadURL(absURL, mTempObserver);
  //mWebWidget->Show();
}

NS_IMETHODIMP
nsHTMLIFrameFrame::Reflow(nsIPresContext*      aPresContext,
                          nsReflowMetrics&     aDesiredSize,
                          const nsReflowState& aReflowState,
                          nsReflowStatus&      aStatus)
{
  GetDesiredSize(aPresContext, aReflowState, aDesiredSize);

  if (nsnull == mWebWidget) {
    nsAutoString url("test9a.html");
    CreateWebWidget(aDesiredSize.width, aDesiredSize.height, url);
    mWebWidget->Show();
  }
  aStatus = NS_FRAME_COMPLETE;
  return NS_OK;
}

void 
nsHTMLIFrameFrame::GetDesiredSize(nsIPresContext* aPresContext,
                                  const nsReflowState& aReflowState,
                                  nsReflowMetrics& aDesiredSize)
{
  float p2t = aPresContext->GetPixelsToTwips();
  aDesiredSize.width  = (int) (p2t * 200);
  aDesiredSize.height = (int) (p2t * 200);
  aDesiredSize.ascent = aDesiredSize.height;
  aDesiredSize.descent = 0;
}

// nsHTMLIFrame

nsHTMLIFrame::nsHTMLIFrame(nsIAtom* aTag) : nsHTMLContainer(aTag)
{
}

nsHTMLIFrame::~nsHTMLIFrame()
{
}

nsresult
nsHTMLIFrame::CreateFrame(nsIPresContext*  aPresContext,
                          nsIFrame*        aParentFrame,
                          nsIStyleContext* aStyleContext,
                          nsIFrame*&       aResult)
{
  nsIFrame* frame = new nsHTMLIFrameFrame(this, aParentFrame);
  if (nsnull == frame) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  aResult = frame;
  return NS_OK;
}

nsresult
NS_NewHTMLIFrame(nsIHTMLContent** aInstancePtrResult,
                 nsIAtom* aTag)
{
  NS_PRECONDITION(nsnull != aInstancePtrResult, "null ptr");
  if (nsnull == aInstancePtrResult) {
    return NS_ERROR_NULL_POINTER;
  }

  nsIHTMLContent* it = new nsHTMLIFrame(aTag);

  if (nsnull == it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return it->QueryInterface(kIHTMLContentIID, (void**) aInstancePtrResult);
}




// XXX temp implementation

NS_IMPL_ADDREF(TempObserver);
NS_IMPL_RELEASE(TempObserver);

nsresult
TempObserver::QueryInterface(const nsIID& aIID,
                            void** aInstancePtrResult)
{
  NS_PRECONDITION(nsnull != aInstancePtrResult, "null pointer");
  if (nsnull == aInstancePtrResult) {
    return NS_ERROR_NULL_POINTER;
  }
  if (aIID.Equals(kIStreamListenerIID)) {
    *aInstancePtrResult = (void*) ((nsIStreamListener*)this);
    AddRef();
    return NS_OK;
  }
  if (aIID.Equals(kISupportsIID)) {
    *aInstancePtrResult = (void*) ((nsISupports*)((nsIDocumentObserver*)this));
    AddRef();
    return NS_OK;
  }
  return NS_NOINTERFACE;
}


NS_IMETHODIMP
TempObserver::GetBindInfo(void)
{
  return NS_OK;
}

NS_IMETHODIMP
TempObserver::OnProgress(PRInt32 aProgress, PRInt32 aProgressMax,
                        const nsString& aMsg)
{
  fputs("[progress ", stdout);
  fputs(mURL, stdout);
  printf(" %d %d ", aProgress, aProgressMax);
  fputs(aMsg, stdout);
  fputs("]\n", stdout);
  return NS_OK;
}

NS_IMETHODIMP
TempObserver::OnStartBinding(const char *aContentType)
{
  fputs("Loading ", stdout);
  fputs(mURL, stdout);
  fputs("\n", stdout);
  return NS_OK;
}

NS_IMETHODIMP
TempObserver::OnDataAvailable(nsIInputStream *pIStream, PRInt32 length)
{
  return NS_OK;
}

NS_IMETHODIMP
TempObserver::OnStopBinding(PRInt32 status, const nsString& aMsg)
{
  fputs("Done loading ", stdout);
  fputs(mURL, stdout);
  fputs("\n", stdout);
  return NS_OK;
}


