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
#include "nsHTMLParts.h"
#include "nsLeafFrame.h"
#include "nsIPresContext.h"
#include "nsIPresShell.h"
#include "nsWidgetsCID.h"
#include "nsViewsCID.h"
#include "nsIView.h"
#include "nsIViewManager.h"
#include "nsIPluginHost.h"
#include "nsplugin.h"
#include "nsString.h"
#include "nsIContentViewerContainer.h"
#include "prmem.h"
#include "nsHTMLAtoms.h"
#include "nsIDocument.h"
#include "nsIURL.h"
#include "nsIPluginInstanceOwner.h"
#include "nsIHTMLContent.h"
#include "nsISupportsArray.h"
#include "plstr.h"
#include "nsILinkHandler.h"
#include "nsIJVMPluginTagInfo.h"
#include "nsIWebShell.h"
#include "nsIBrowserWindow.h"

// XXX For temporary paint code
#include "nsIStyleContext.h"

class nsObjectFrame;

class nsPluginInstanceOwner : public nsIPluginInstanceOwner,
                              public nsIPluginTagInfo2,
                              public nsIJVMPluginTagInfo
{
public:
  nsPluginInstanceOwner();
  ~nsPluginInstanceOwner();

  NS_DECL_ISUPPORTS

  //nsIPluginInstanceOwner interface

  NS_IMETHOD SetInstance(nsIPluginInstance *aInstance);

  NS_IMETHOD GetInstance(nsIPluginInstance *&aInstance);

  NS_IMETHOD GetWindow(nsPluginWindow *&aWindow);

  NS_IMETHOD GetMode(nsPluginMode *aMode);

  NS_IMETHOD CreateWidget(void);

  NS_IMETHOD GetURL(const char *aURL, const char *aTarget, void *aPostData);

  NS_IMETHOD ShowStatus(const char *aStatusMsg);

  //nsIPluginTagInfo interface

  NS_IMETHOD GetAttributes(PRUint16& n, const char*const*& names,
                           const char*const*& values);

  NS_IMETHOD GetAttribute(const char* name, const char* *result);

  //nsIPluginTagInfo2 interface

  NS_IMETHOD GetTagType(nsPluginTagType *result);

  NS_IMETHOD GetTagText(const char* *result);

  NS_IMETHOD GetParameters(PRUint16& n, const char*const*& names, const char*const*& values);

  NS_IMETHOD GetParameter(const char* name, const char* *result);
  
  NS_IMETHOD GetDocumentBase(const char* *result);
  
  NS_IMETHOD GetDocumentEncoding(const char* *result);
  
  NS_IMETHOD GetAlignment(const char* *result);
  
  NS_IMETHOD GetWidth(PRUint32 *result);
  
  NS_IMETHOD GetHeight(PRUint32 *result);
  
  NS_IMETHOD GetBorderVertSpace(PRUint32 *result);
  
  NS_IMETHOD GetBorderHorizSpace(PRUint32 *result);

  NS_IMETHOD GetUniqueID(PRUint32 *result);

  //nsIJVMPluginTagInfo interface

  NS_IMETHOD GetCode(const char* *result);

  NS_IMETHOD GetCodeBase(const char* *result);

  NS_IMETHOD GetArchive(const char* *result);

  NS_IMETHOD GetName(const char* *result);

  NS_IMETHOD GetMayScript(PRBool *result);

  //locals

  NS_IMETHOD Init(nsIPresContext *aPresContext, nsObjectFrame *aFrame);

private:
  nsPluginWindow    mPluginWindow;
  nsIPluginInstance *mInstance;
  nsObjectFrame     *mOwner;
  PRInt32           mNumAttrs;
  char              **mAttrNames;
  char              **mAttrVals;
  PRInt32           mNumParams;
  char              **mParamNames;
  char              **mParamVals;
  nsIWidget         *mWidget;
  nsIPresContext    *mContext;
};

#define nsObjectFrameSuper nsLeafFrame

class nsObjectFrame : public nsObjectFrameSuper {
public:
  nsObjectFrame(nsIContent* aContent, nsIFrame* aParentFrame);

//  NS_IMETHOD Init(nsIPresContext& aPresContext, nsIFrame *aChildList);

  NS_IMETHOD Reflow(nsIPresContext&          aPresContext,
                    nsHTMLReflowMetrics&     aDesiredSize,
                    const nsHTMLReflowState& aReflowState,
                    nsReflowStatus&          aStatus);
  NS_IMETHOD DidReflow(nsIPresContext& aPresContext,
                       nsDidReflowStatus aStatus);
  NS_IMETHOD Paint(nsIPresContext& aPresContext,
                   nsIRenderingContext& aRenderingContext,
                   const nsRect& aDirtyRect);
  NS_IMETHOD Scrolled(nsIView *aView);

  //local methods
  nsresult CreateWidget(nscoord aWidth, nscoord aHeight, PRBool aViewOnly);
  nsresult GetFullURL(nsString& aFullURL);

protected:
  virtual ~nsObjectFrame();

  virtual void GetDesiredSize(nsIPresContext* aPresContext,
                              const nsHTMLReflowState& aReflowState,
                              nsHTMLReflowMetrics& aDesiredSize);


  nsresult SetURL(const nsString& aURLSpec);
  nsresult SetBaseHREF(const nsString& aBaseHREF);
  nsresult SetFullURL(const nsString& aURLSpec);

private:
  nsPluginInstanceOwner *mInstanceOwner;
  nsString              *mURLSpec;
  nsString              *mBaseHREF;
  nsString              *mFullURL;
  nsIFrame              *mFirstChild;
};

nsObjectFrame::nsObjectFrame(nsIContent* aContent, nsIFrame* aParentFrame)
  : nsObjectFrameSuper(aContent, aParentFrame)
{
}

nsObjectFrame::~nsObjectFrame()
{
  NS_IF_RELEASE(mInstanceOwner);

  if (nsnull != mURLSpec)
  {
    delete mURLSpec;
    mURLSpec = nsnull;
  }

  if (nsnull != mBaseHREF)
  {
    delete mBaseHREF;
    mBaseHREF = nsnull;
  }

  if (nsnull != mFullURL)
  {
    delete mFullURL;
    mFullURL = nsnull;
  }
}

static NS_DEFINE_IID(kViewCID, NS_VIEW_CID);
static NS_DEFINE_IID(kIViewIID, NS_IVIEW_IID);
static NS_DEFINE_IID(kWidgetCID, NS_CHILD_CID);

//NS_IMETHODIMP
//nsObjectFrame::Init(nsIPresContext& aPresContext, nsIFrame *aChildList)
//{
//  mFirstChild = aChildList;
//  return NS_OK;
//}

nsresult
nsObjectFrame::CreateWidget(nscoord aWidth, nscoord aHeight, PRBool aViewOnly)
{
  nsIView* view;

  // Create our view and widget

  nsresult result = 
    nsRepository::CreateInstance(kViewCID, nsnull, kIViewIID,
				 (void **)&view);
  if (NS_OK != result) {
    return result;
  }
  nsIViewManager *viewMan;    // need to release

  nsRect boundBox(0, 0, aWidth, aHeight); 

  nsIFrame* parWithView;
  nsIView *parView;

  GetParentWithView(parWithView);
  parWithView->GetView(parView);

  if (NS_OK == parView->GetViewManager(viewMan))
  {
  //  nsWidgetInitData* initData = GetWidgetInitData(*aPresContext); // needs to be deleted
    // initialize the view as hidden since we don't know the (x,y) until Paint
    result = view->Init(viewMan, boundBox, parView, &kWidgetCID, nsnull,
                        nsnull, 0, nsnull,
                        1.0f, nsViewVisibility_kHide);
  //  if (nsnull != initData) {
  //    delete(initData);
  //  }
    if (NS_OK != result) {
      result = NS_OK;       //XXX why OK? MMP
      goto exit;            //XXX sue me. MMP
    }

  #if 0
    // set the content's widget, so it can get content modified by the widget
    nsIWidget *widget;
    result = GetWidget(view, &widget);
    if (NS_OK == result) {
      nsInput* content = (nsInput *)mContent; // change this cast to QueryInterface 
      content->SetWidget(widget);
      NS_IF_RELEASE(widget);
    } else {
      NS_ASSERTION(0, "could not get widget");
    }
  #endif

    viewMan->InsertChild(parView, view, 0);
  }

  {
    //this is ugly. it was ripped off from didreflow(). MMP
    // Position and size view relative to its parent, not relative to our
    // parent frame (our parent frame may not have a view).

    nsIView* parentWithView;
    nsPoint origin;
    view->SetVisibility(nsViewVisibility_kShow);
    GetOffsetFromView(origin, parentWithView);
    viewMan->ResizeView(view, mRect.width, mRect.height);
    viewMan->MoveViewTo(view, origin.x, origin.y);
  }

  SetView(view);

exit:
  NS_IF_RELEASE(viewMan);  

  return result;
}

#define EMBED_DEF_DIM 50

void
nsObjectFrame::GetDesiredSize(nsIPresContext* aPresContext,
                              const nsHTMLReflowState& aReflowState,
                              nsHTMLReflowMetrics& aMetrics)
{
  // Determine our size stylistically
  PRBool haveWidth = PR_FALSE;
  PRBool haveHeight = PR_FALSE;
  if (aReflowState.HaveConstrainedWidth()) {
    aMetrics.width = aReflowState.minWidth;
    haveWidth = PR_TRUE;
  }
  if (aReflowState.HaveConstrainedHeight()) {
    aMetrics.height = aReflowState.minHeight;
    haveHeight = PR_TRUE;
  }

  // XXX Temporary auto-sizing logic
  if (!haveWidth) {
    if (haveHeight) {
      aMetrics.width = aMetrics.height;
    }
    else {
      float p2t = aPresContext->GetPixelsToTwips();
      aMetrics.width = NSIntPixelsToTwips(EMBED_DEF_DIM, p2t);
    }
  }
  if (!haveHeight) {
    if (haveWidth) {
      aMetrics.height = aMetrics.width;
    }
    else {
      float p2t = aPresContext->GetPixelsToTwips();
      aMetrics.height = NSIntPixelsToTwips(EMBED_DEF_DIM, p2t);
    }
  }
  aMetrics.ascent = aMetrics.height;
  aMetrics.descent = 0;
}

NS_IMETHODIMP
nsObjectFrame::Reflow(nsIPresContext&          aPresContext,
                      nsHTMLReflowMetrics&     aMetrics,
                      const nsHTMLReflowState& aReflowState,
                      nsReflowStatus&          aStatus)
{
  // Get our desired size
  GetDesiredSize(&aPresContext, aReflowState, aMetrics);
  if (nsnull != aMetrics.maxElementSize) {
    aMetrics.maxElementSize->width = aMetrics.width;
    aMetrics.maxElementSize->height = aMetrics.height;
  }

  // XXX deal with border and padding the usual way...wrap it up!

  nsIAtom* atom;
  mContent->GetTag(atom);
  if ((nsnull != atom) && (nsnull == mInstanceOwner)) {
    static NS_DEFINE_IID(kIPluginHostIID, NS_IPLUGINHOST_IID);
    static NS_DEFINE_IID(kIContentViewerContainerIID, NS_ICONTENT_VIEWER_CONTAINER_IID);

    nsISupports               *container;
    nsIPluginHost             *pm;
    nsIContentViewerContainer *cv;
    nsresult                  rv;

    mInstanceOwner = new nsPluginInstanceOwner();

    if (nsnull != mInstanceOwner) {
      NS_ADDREF(mInstanceOwner);
      mInstanceOwner->Init(&aPresContext, this);

      rv = aPresContext.GetContainer(&container);

      if (NS_OK == rv) {
        rv = container->QueryInterface(kIContentViewerContainerIID, (void **)&cv);

        if (NS_OK == rv) {
          rv = cv->QueryCapability(kIPluginHostIID, (void **)&pm);

          if (NS_OK == rv) {
            nsAutoString    type;
            char            *buf = nsnull;
            PRInt32         buflen;
            nsPluginWindow  *window;
            float           t2p = aPresContext.GetTwipsToPixels();
            nsAutoString    src, base, fullurl;


            mInstanceOwner->GetWindow(window);

            if (atom == nsHTMLAtoms::applet) {
              buf = (char *)PR_Malloc(PL_strlen("application/x-java-vm") + 1);
              PL_strcpy(buf, "application/x-java-vm");

              if (NS_CONTENT_ATTR_HAS_VALUE == mContent->GetAttribute("CODE", src)) {
                SetURL(src);

                if (NS_CONTENT_ATTR_HAS_VALUE == mContent->GetAttribute("CODEBASE", base))
                  SetBaseHREF(base);

                nsIPresShell  *shell = aPresContext.GetShell();
                nsIDocument   *doc = shell->GetDocument();
                nsIURL        *docURL = doc->GetDocumentURL();

                // Create an absolute URL
                nsresult rv = NS_MakeAbsoluteURL(docURL, base, *mURLSpec, fullurl);

                SetFullURL(fullurl);

                NS_RELEASE(shell);
                NS_RELEASE(docURL);
                NS_RELEASE(doc);
              }
            }
            else {
              mContent->GetAttribute(nsString("type"), type);

              buflen = type.Length();

              if (buflen > 0) {
                buf = (char *)PR_Malloc(buflen + 1);

                if (nsnull != buf)
                  type.ToCString(buf, buflen + 1);
              }

              //stream in the object source if there is one...

              if (NS_CONTENT_ATTR_HAS_VALUE == mContent->GetAttribute("SRC", src)) {
                SetURL(src);

                if (NS_CONTENT_ATTR_HAS_VALUE == mContent->GetAttribute(NS_HTML_BASE_HREF, base))
                  SetBaseHREF(base);

                nsIPresShell  *shell = aPresContext.GetShell();
                nsIDocument   *doc = shell->GetDocument();
                nsIURL        *docURL = doc->GetDocumentURL();

                // Create an absolute URL
                nsresult rv = NS_MakeAbsoluteURL(docURL, base, *mURLSpec, fullurl);

                SetFullURL(fullurl);

                NS_RELEASE(shell);
                NS_RELEASE(docURL);
                NS_RELEASE(doc);
              }
            }

            nsIView *parentWithView;
            nsPoint origin;

            GetOffsetFromView(origin, parentWithView);

            window->x = NSTwipsToIntPixels(origin.x, t2p);
            window->y = NSTwipsToIntPixels(origin.y, t2p);
            window->width = NSTwipsToIntPixels(aMetrics.width, t2p);
            window->height = NSTwipsToIntPixels(aMetrics.height, t2p);
            window->clipRect.top = 0;
            window->clipRect.left = 0;
//              window->clipRect.top = NSTwipsToIntPixels(origin.y, t2p);
//              window->clipRect.left = NSTwipsToIntPixels(origin.x, t2p);
            window->clipRect.bottom = NSTwipsToIntPixels(aMetrics.height, t2p);
            window->clipRect.right = NSTwipsToIntPixels(aMetrics.width, t2p);
#ifdef XP_UNIX
            window->ws_info = nsnull;   //XXX need to figure out what this is. MMP
#endif
            rv = pm->InstantiatePlugin(buf, fullurl, mInstanceOwner);

            PR_Free((void *)buf);

            NS_RELEASE(pm);
          }
          NS_RELEASE(cv);
        }
        NS_RELEASE(container);
      }
    }
    NS_RELEASE(atom);
  }

  aStatus = NS_FRAME_COMPLETE;
  return NS_OK;
}

NS_IMETHODIMP
nsObjectFrame::DidReflow(nsIPresContext& aPresContext,
                         nsDidReflowStatus aStatus)
{
  nsresult rv = nsObjectFrameSuper::DidReflow(aPresContext, aStatus);

  // The view is created hidden; once we have reflowed it and it has been
  // positioned then we show it.
  if (NS_FRAME_REFLOW_FINISHED == aStatus) {
    nsIView* view = nsnull;
    GetView(view);
    if (nsnull != view) {
      view->SetVisibility(nsViewVisibility_kShow);
    }

    if (nsnull != mInstanceOwner) {
      nsPluginWindow    *window;

      if (NS_OK == mInstanceOwner->GetWindow(window)) {
        nsIView           *parentWithView;
        nsPoint           origin;
        nsIPluginInstance *inst;
        float             t2p = aPresContext.GetTwipsToPixels();
        nscoord           offx, offy;

        GetOffsetFromView(origin, parentWithView);

#if 0
        parentWithView->GetScrollOffset(&offx, &offy);
#else
        offx = offy = 0;
#endif

//        window->x = NSTwipsToIntPixels(origin.x, t2p);
//        window->y = NSTwipsToIntPixels(origin.y, t2p);
        window->x = 0;
        window->y = 0;

        window->clipRect.top = 0;
        window->clipRect.left = 0;
//        window->clipRect.top = NSTwipsToIntPixels(origin.y, t2p);
//        window->clipRect.left = NSTwipsToIntPixels(origin.x, t2p);
//        window->clipRect.top = NSTwipsToIntPixels(origin.y + offy, t2p);
//        window->clipRect.left = NSTwipsToIntPixels(origin.x + offx, t2p);
        window->clipRect.bottom = window->clipRect.top + window->height;
        window->clipRect.right = window->clipRect.left + window->width;

        if (NS_OK == mInstanceOwner->GetInstance(inst)) {
          inst->SetWindow(window);
          NS_RELEASE(inst);
        }
      }
    }
  }
  return rv;
}

NS_IMETHODIMP
nsObjectFrame::Paint(nsIPresContext& aPresContext,
                     nsIRenderingContext& aRenderingContext,
                     const nsRect& aDirtyRect)
{
  const nsStyleFont* font =
    (const nsStyleFont*)mStyleContext->GetStyleData(eStyleStruct_Font);

  aRenderingContext.SetFont(font->mFont);
  aRenderingContext.SetColor(NS_RGB(192, 192, 192));
  aRenderingContext.FillRect(0, 0, mRect.width, mRect.height);
  aRenderingContext.SetColor(NS_RGB(0, 0, 0));
  aRenderingContext.DrawRect(0, 0, mRect.width, mRect.height);
  float p2t = aPresContext.GetPixelsToTwips();
  nscoord px3 = NSIntPixelsToTwips(3, p2t);
  nsAutoString tmp;
  nsIAtom* atom;
  mContent->GetTag(atom);
  if (nsnull != atom) {
    atom->ToString(tmp);
    NS_RELEASE(atom);
    aRenderingContext.DrawString(tmp, px3, px3, 0);
  }
  return NS_OK;
}

nsresult
nsObjectFrame::SetURL(const nsString& aURLSpec)
{
  if (nsnull != mURLSpec) {
    delete mURLSpec;
  }
  mURLSpec = new nsString(aURLSpec);
  if (nsnull == mURLSpec) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return NS_OK;
}

nsresult
nsObjectFrame::SetFullURL(const nsString& aURLSpec)
{
  if (nsnull != mFullURL) {
    delete mFullURL;
  }
  mFullURL = new nsString(aURLSpec);
  if (nsnull == mFullURL) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return NS_OK;
}

nsresult
nsObjectFrame::SetBaseHREF(const nsString& aBaseHREF)
{
  if (nsnull != mBaseHREF) {
    delete mBaseHREF;
  }
  mBaseHREF = new nsString(aBaseHREF);
  if (nsnull == mBaseHREF) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return NS_OK;
}

nsresult
nsObjectFrame::Scrolled(nsIView *aView)
{
  return NS_OK;
}

nsresult nsObjectFrame :: GetFullURL(nsString& aFullURL)
{
  if (nsnull != mFullURL)
    aFullURL = *mFullURL;
  else
    aFullURL = nsString("");

  return NS_OK;
}


nsresult
NS_NewObjectFrame(nsIContent* aContent, nsIFrame* aParentFrame,
                  nsIFrame*& aFrameResult)
{
  aFrameResult = new nsObjectFrame(aContent, aParentFrame);
  if (nsnull == aFrameResult) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return NS_OK;
}

//plugin instance owner

nsPluginInstanceOwner :: nsPluginInstanceOwner()
{
  NS_INIT_REFCNT();

  memset(&mPluginWindow, 0, sizeof(mPluginWindow));
  mInstance = nsnull;
  mOwner = nsnull;
  mNumAttrs = 0;
  mAttrNames = nsnull;
  mAttrVals = nsnull;
  mWidget = nsnull;
  mContext = nsnull;
  mNumParams = 0;
  mParamNames = nsnull;
  mParamVals = nsnull;
}

nsPluginInstanceOwner :: ~nsPluginInstanceOwner()
{
  PRInt32 cnt;

  if (nsnull != mInstance)
  {
    mInstance->Stop();
    mInstance->Destroy();
    NS_RELEASE(mInstance);
  }

  mOwner = nsnull;

  for (cnt = 0; cnt < mNumAttrs; cnt++)
  {
    if ((nsnull != mAttrNames) && (nsnull != mAttrNames[cnt]))
    {
      PR_Free(mAttrNames[cnt]);
      mAttrNames[cnt] = nsnull;
    }

    if ((nsnull != mAttrVals) && (nsnull != mAttrVals[cnt]))
    {
      PR_Free(mAttrVals[cnt]);
      mAttrVals[cnt] = nsnull;
    }
  }

  if (nsnull != mAttrNames)
  {
    PR_Free(mAttrNames);
    mAttrNames = nsnull;
  }

  if (nsnull != mAttrVals)
  {
    PR_Free(mAttrVals);
    mAttrVals = nsnull;
  }

  for (cnt = 0; cnt < mNumParams; cnt++)
  {
    if ((nsnull != mParamNames) && (nsnull != mParamNames[cnt]))
    {
      PR_Free(mParamNames[cnt]);
      mParamNames[cnt] = nsnull;
    }

    if ((nsnull != mParamVals) && (nsnull != mParamVals[cnt]))
    {
      PR_Free(mParamVals[cnt]);
      mParamVals[cnt] = nsnull;
    }
  }

  if (nsnull != mParamNames)
  {
    PR_Free(mParamNames);
    mParamNames = nsnull;
  }

  if (nsnull != mParamVals)
  {
    PR_Free(mParamVals);
    mParamVals = nsnull;
  }

  NS_IF_RELEASE(mWidget);
  mContext = nsnull;
}

static NS_DEFINE_IID(kIPluginInstanceOwnerIID, NS_IPLUGININSTANCEOWNER_IID); 
static NS_DEFINE_IID(kIPluginTagInfoIID, NS_IPLUGINTAGINFO_IID); 
static NS_DEFINE_IID(kIPluginTagInfo2IID, NS_IPLUGINTAGINFO2_IID); 
static NS_DEFINE_IID(kIJVMPluginTagInfoIID, NS_IPLUGINTAGINFO2_IID); 
static NS_DEFINE_IID(kIWebShellIID, NS_IWEB_SHELL_IID); 
static NS_DEFINE_IID(kIBrowserWindowIID, NS_IBROWSER_WINDOW_IID); 
static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);

NS_IMPL_ADDREF(nsPluginInstanceOwner);
NS_IMPL_RELEASE(nsPluginInstanceOwner);

nsresult nsPluginInstanceOwner :: QueryInterface(const nsIID& aIID,
                                                 void** aInstancePtrResult)
{
  NS_PRECONDITION(nsnull != aInstancePtrResult, "null pointer");

  if (nsnull == aInstancePtrResult)
    return NS_ERROR_NULL_POINTER;

  if (aIID.Equals(kIPluginInstanceOwnerIID))
  {
    *aInstancePtrResult = (void *)((nsIPluginInstanceOwner *)this);
    AddRef();
    return NS_OK;
  }

  if (aIID.Equals(kIPluginTagInfoIID))
  {
    *aInstancePtrResult = (void *)((nsIPluginTagInfo *)this);
    AddRef();
    return NS_OK;
  }

  if (aIID.Equals(kIPluginTagInfo2IID))
  {
    *aInstancePtrResult = (void *)((nsIPluginTagInfo2 *)this);
    AddRef();
    return NS_OK;
  }

  if (aIID.Equals(kIJVMPluginTagInfoIID))
  {
    *aInstancePtrResult = (void *)((nsIJVMPluginTagInfo *)this);
    AddRef();
    return NS_OK;
  }

  if (aIID.Equals(kISupportsIID))
  {
    *aInstancePtrResult = (void *)((nsISupports *)((nsIPluginTagInfo *)this));
    AddRef();
    return NS_OK;
  }

  return NS_NOINTERFACE;
}

NS_IMETHODIMP nsPluginInstanceOwner :: SetInstance(nsIPluginInstance *aInstance)
{
  NS_IF_RELEASE(mInstance);
  mInstance = aInstance;
  NS_IF_ADDREF(mInstance);

  return NS_OK;
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetWindow(nsPluginWindow *&aWindow)
{
  aWindow = &mPluginWindow;
  return NS_OK;
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetMode(nsPluginMode *aMode)
{
  *aMode = nsPluginMode_Embedded;
  return NS_OK;
}

static NS_DEFINE_IID(kIHTMLContentIID, NS_IHTMLCONTENT_IID);
static NS_DEFINE_IID(kILinkHandlerIID, NS_ILINKHANDLER_IID);

NS_IMETHODIMP nsPluginInstanceOwner :: GetAttributes(PRUint16& n,
                                                     const char*const*& names,
                                                     const char*const*& values)
{
  nsresult          rv;
  nsIHTMLContent    *content;
  nsIContent        *icontent;

  if ((nsnull == mAttrNames) && (nsnull != mOwner))
  {
    rv = mOwner->GetContent(icontent);

    if (rv == NS_OK)
    {
      rv = icontent->QueryInterface(kIHTMLContentIID, (void **)&content);

      if (NS_OK == rv)
      {
        nsISupportsArray  *array;

        rv = NS_NewISupportsArray(&array);

        if (NS_OK == rv)
        {
          PRInt32 cnt, numattrs;

          if ((NS_OK == content->GetAllAttributeNames(array, numattrs)) && (numattrs > 0))
          {
            mAttrNames = (char **)PR_Calloc(sizeof(char *) * numattrs, 1);
            mAttrVals = (char **)PR_Calloc(sizeof(char *) * numattrs, 1);

            if ((nsnull != mAttrNames) && (nsnull != mAttrVals))
            {
              for (cnt = 0; cnt < numattrs; cnt++)
              {
                nsIAtom       *atom = (nsIAtom *)array->ElementAt(cnt);
                nsAutoString  name, val;

                if (nsnull != atom)
                {
                  atom->ToString(name);

                  if (NS_CONTENT_ATTR_HAS_VALUE == icontent->GetAttribute(name, val))
                  {
                    mAttrNames[mNumAttrs] = (char *)PR_Malloc(name.Length() + 1);
                    mAttrVals[mNumAttrs] = (char *)PR_Malloc(val.Length() + 1);

                    if ((nsnull != mAttrNames[mNumAttrs]) &&
                        (nsnull != mAttrVals[mNumAttrs]))
                    {
                      name.ToCString(mAttrNames[mNumAttrs], name.Length() + 1);
                      val.ToCString(mAttrVals[mNumAttrs], val.Length() + 1);

                      mNumAttrs++;
                    }
                    else
                    {
                      if (nsnull != mAttrNames[mNumAttrs])
                      {
                        PR_Free(mAttrNames[mNumAttrs]);
                        mAttrNames[mNumAttrs] = nsnull;
                      }

                      if (nsnull != mAttrVals[mNumAttrs])
                      {
                        PR_Free(mAttrVals[mNumAttrs]);
                        mAttrVals[mNumAttrs] = nsnull;
                      }
                    }
                  }

                  NS_RELEASE(atom);
                }
              }
            }
          }

          NS_RELEASE(array);
        }

        NS_RELEASE(content);
      }

      NS_RELEASE(icontent);
    }
  }
  else
    rv = NS_OK;

  n = mNumAttrs;
  names = (const char **)mAttrNames;
  values = (const char **)mAttrVals;

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetAttribute(const char* name, const char* *result)
{
  PRInt32 count;

  if (nsnull == mAttrNames)
  {
    PRUint16  numattrs;
    const char * const *names, * const *vals;

    GetAttributes(numattrs, names, vals);
  }

  for (count = 0; count < mNumAttrs; count++)
  {
    if (0 == PL_strcasecmp(mAttrNames[count], name))
    {
      *result = mAttrVals[count];
      break;
    }
  }

  if (count >= mNumAttrs)
    *result = "";

  return NS_OK;
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetInstance(nsIPluginInstance *&aInstance)
{
  if (nsnull != mInstance)
  {
    aInstance = mInstance;
    NS_ADDREF(mInstance);

    return NS_OK;
  }
  else
    return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetURL(const char *aURL, const char *aTarget, void *aPostData)
{
  nsISupports     *container;
  nsILinkHandler  *lh;
  nsresult        rv;
  nsIView         *view;
  nsPoint         origin;

  if ((nsnull != mOwner) && (nsnull != mContext))
  {
    rv = mOwner->GetOffsetFromView(origin, view);

    if (NS_OK == rv)
    {
      rv = mContext->GetContainer(&container);

      if (NS_OK == rv)
      {
        rv = container->QueryInterface(kILinkHandlerIID, (void **)&lh);

        if (NS_OK == rv)
        {
          nsAutoString  uniurl = nsAutoString(aURL);
          nsAutoString  unitarget = nsAutoString(aTarget);
          nsAutoString  base, fullurl;

          mOwner->GetFullURL(base);

          nsIPresShell  *shell = mContext->GetShell();
          nsIDocument   *doc = shell->GetDocument();
          nsIURL        *docURL = doc->GetDocumentURL();

          // Create an absolute URL
          rv = NS_MakeAbsoluteURL(docURL, base, uniurl, fullurl);

          NS_RELEASE(shell);
          NS_RELEASE(docURL);
          NS_RELEASE(doc);

          if (NS_OK == rv)
            rv = lh->OnLinkClick(mOwner, fullurl.GetUnicode(), unitarget.GetUnicode(), nsnull);

          NS_RELEASE(lh);
        }

        NS_RELEASE(container);
      }
    }
  }
  else
    rv = NS_ERROR_FAILURE;

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner :: ShowStatus(const char *aStatusMsg)
{
  nsresult  rv = NS_ERROR_FAILURE;

  if (nsnull != mContext)
  {
    nsISupports *cont;

    rv = mContext->GetContainer(&cont);

    if ((NS_OK == rv) && (nsnull != cont))
    {
      nsIWebShell *ws;

      rv = cont->QueryInterface(kIWebShellIID, (void **)&ws);

      if (NS_OK == rv)
      {
        nsIWebShell *rootWebShell;

        ws->GetRootWebShell(rootWebShell);

        if (nsnull != rootWebShell)
        {
          nsIWebShellContainer *rootContainer;

          rv = rootWebShell->GetContainer(rootContainer);

          if (nsnull != rootContainer)
          {
            nsIBrowserWindow *browserWindow;

            if (NS_OK == rootContainer->QueryInterface(kIBrowserWindowIID, (void**)&browserWindow))
            {
              nsAutoString  msg = nsAutoString(aStatusMsg);

              rv = browserWindow->SetStatus(msg.GetUnicode());
              NS_RELEASE(browserWindow);
            }

            NS_RELEASE(rootContainer);
          }

          NS_RELEASE(rootWebShell);
        }

        NS_RELEASE(ws);
      }

      NS_RELEASE(cont);
    }
  }

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetTagType(nsPluginTagType *result)
{
  nsresult rv = NS_ERROR_FAILURE;

  *result = nsPluginTagType_Unknown;

  if (nsnull != mOwner)
  {
    nsIContent  *cont;

    mOwner->GetContent(cont);

    if (nsnull != cont)
    {
      nsIAtom     *atom;

      cont->GetTag(atom);

      if (nsnull != atom)
      {
        if (atom == nsHTMLAtoms::applet)
          *result = nsPluginTagType_Applet;
        else if (atom == nsHTMLAtoms::embed)
          *result = nsPluginTagType_Embed;
        else if (atom == nsHTMLAtoms::object)
          *result = nsPluginTagType_Object;

        rv = NS_OK;
        NS_RELEASE(atom);
      }

      NS_RELEASE(cont);
    }
  }

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetTagText(const char* *result)
{
printf("instance owner gettagtext called\n");
  *result = "";
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetParameters(PRUint16& n, const char*const*& names, const char*const*& values)
{
  nsresult rv = NS_ERROR_FAILURE;

  if ((nsnull == mParamNames) && (nsnull != mOwner))
  {
    nsIContent  *cont;

    mOwner->GetContent(cont);

    if (nsnull != cont)
    {
      PRBool  haskids = PR_FALSE;

      cont->CanContainChildren(haskids);

      if (PR_TRUE == haskids)
      {
        PRInt32 numkids, idx, numparams = 0;

        cont->ChildCount(numkids);

        //first pass, count number of param tags...

        for (idx = 0; idx < numkids; idx++)
        {
          nsIContent  *kid;

          cont->ChildAt(idx, kid);

          if (nsnull != kid)
          {
            nsIAtom     *atom;

            kid->GetTag(atom);

            if (nsnull != atom)
            {
              if (atom == nsHTMLAtoms::param)
                numparams++;

              NS_RELEASE(atom);
            }

            NS_RELEASE(kid);
          }
        }

        if (numparams > 0)
        {
          //now we need to create arrays
          //representing the parameter name/value pairs...

          mParamNames = (char **)PR_Calloc(sizeof(char *) * numparams, 1);
          mParamVals = (char **)PR_Calloc(sizeof(char *) * numparams, 1);

          if ((nsnull != mParamNames) && (nsnull != mParamVals))
          {
            for (idx = 0; idx < numkids; idx++)
            {
              nsIContent  *kid;

              cont->ChildAt(idx, kid);

              if (nsnull != kid)
              {
                nsIAtom     *atom;

                kid->GetTag(atom);

                if (nsnull != atom)
                {
                  if (atom == nsHTMLAtoms::param)
                  {
                    nsAutoString  val, name;

                    //add param to list...

                    if ((NS_CONTENT_ATTR_HAS_VALUE == kid->GetAttribute("NAME", name)) &&
                        (NS_CONTENT_ATTR_HAS_VALUE == kid->GetAttribute("VALUE", val)))
                    {
                      mParamNames[mNumParams] = (char *)PR_Malloc(name.Length() + 1);
                      mParamVals[mNumParams] = (char *)PR_Malloc(val.Length() + 1);

                      if ((nsnull != mParamNames[mNumParams]) &&
                          (nsnull != mParamVals[mNumParams]))
                      {
                        name.ToCString(mParamNames[mNumParams], name.Length() + 1);
                        val.ToCString(mParamVals[mNumParams], val.Length() + 1);

                        mNumParams++;
                      }
                      else
                      {
                        if (nsnull != mParamNames[mNumParams])
                        {
                          PR_Free(mParamNames[mNumParams]);
                          mParamNames[mNumParams] = nsnull;
                        }

                        if (nsnull != mParamVals[mNumParams])
                        {
                          PR_Free(mParamVals[mNumParams]);
                          mParamVals[mNumParams] = nsnull;
                        }
                      }
                    }
                  }

                  NS_RELEASE(atom);
                }
              }

              NS_RELEASE(kid);
            }
          }
        }
      }

      rv = NS_OK;
      NS_RELEASE(cont);
    }
  }

  n = mNumParams;
  names = (const char **)mParamNames;
  values = (const char **)mParamVals;

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetParameter(const char* name, const char* *result)
{
  PRInt32 count;

  if (nsnull == mParamNames)
  {
    PRUint16  numattrs;
    const char * const *names, * const *vals;

    GetParameters(numattrs, names, vals);
  }

  for (count = 0; count < mNumParams; count++)
  {
    if (0 == PL_strcasecmp(mParamNames[count], name))
    {
      *result = mParamVals[count];
      break;
    }
  }

  if (count >= mNumParams)
    *result = "";

  return NS_OK;
}
  
NS_IMETHODIMP nsPluginInstanceOwner :: GetDocumentBase(const char* *result)
{
  if (nsnull != mContext)
  {
    nsIPresShell  *shell = mContext->GetShell();
    nsIDocument   *doc = shell->GetDocument();
    nsIURL        *docURL = doc->GetDocumentURL();

    *result = docURL->GetSpec();

    NS_RELEASE(shell);
    NS_RELEASE(docURL);
    NS_RELEASE(doc);

    return NS_OK;
  }
  else
  {
    *result = "";
    return NS_ERROR_FAILURE;
  }
}
  
NS_IMETHODIMP nsPluginInstanceOwner :: GetDocumentEncoding(const char* *result)
{
printf("instance owner getdocumentencoding called\n");
  return NS_ERROR_FAILURE;
}
  
NS_IMETHODIMP nsPluginInstanceOwner :: GetAlignment(const char* *result)
{
  return GetAttribute("ALIGN", result);
}
  
NS_IMETHODIMP nsPluginInstanceOwner :: GetWidth(PRUint32 *result)
{
  nsresult    rv;
  const char  *width;

  rv = GetAttribute("WIDTH", &width);

  if (NS_OK == rv)
  {
    if (*result != 0)
      *result = (PRUint32)atol(width);
    else
      *result = 0;
  }
  else
    *result = 0;

  return rv;
}
  
NS_IMETHODIMP nsPluginInstanceOwner :: GetHeight(PRUint32 *result)
{
  nsresult    rv;
  const char  *height;

  rv = GetAttribute("HEIGHT", &height);

  if (NS_OK == rv)
  {
    if (*result != 0)
      *result = (PRUint32)atol(height);
    else
      *result = 0;
  }
  else
    *result = 0;

  return rv;
}
  
NS_IMETHODIMP nsPluginInstanceOwner :: GetBorderVertSpace(PRUint32 *result)
{
  nsresult    rv;
  const char  *vspace;

  rv = GetAttribute("VSPACE", &vspace);

  if (NS_OK == rv)
  {
    if (*result != 0)
      *result = (PRUint32)atol(vspace);
    else
      *result = 0;
  }
  else
    *result = 0;

  return rv;
}
  
NS_IMETHODIMP nsPluginInstanceOwner :: GetBorderHorizSpace(PRUint32 *result)
{
  nsresult    rv;
  const char  *hspace;

  rv = GetAttribute("HSPACE", &hspace);

  if (NS_OK == rv)
  {
    if (*result != 0)
      *result = (PRUint32)atol(hspace);
    else
      *result = 0;
  }
  else
    *result = 0;

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetUniqueID(PRUint32 *result)
{
  *result = (PRUint32)mContext;
  return NS_OK;
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetCode(const char* *result)
{
  return GetAttribute("CODE", result);
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetCodeBase(const char* *result)
{
  return GetAttribute("CODEBASE", result);
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetArchive(const char* *result)
{
printf("instance owner getarchive called\n");
  *result = "";
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetName(const char* *result)
{
  return GetAttribute("NAME", result);
}

NS_IMETHODIMP nsPluginInstanceOwner :: GetMayScript(PRBool *result)
{
printf("instance owner getmayscript called\n");
  *result = PR_FALSE;
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsPluginInstanceOwner :: Init(nsIPresContext* aPresContext, nsObjectFrame *aFrame)
{
  //do not addref to avoid circular refs. MMP
  mContext = aPresContext;
  mOwner = aFrame;
  return NS_OK;
}

NS_IMETHODIMP nsPluginInstanceOwner :: CreateWidget(void)
{
  nsIView   *view;
  nsresult  rv = NS_ERROR_FAILURE;

  if (nsnull != mOwner)
  {
    // Create view if necessary

    mOwner->GetView(view);

    if (nsnull == view)
    {
      PRBool    windowless;

      mInstance->GetValue(nsPluginInstanceVariable_WindowlessBool, (void *)&windowless);

      rv = mOwner->CreateWidget(mPluginWindow.width,
                                mPluginWindow.height,
                                windowless);
      if (NS_OK == rv)
      {
        nsIView   *view;

        mOwner->GetView(view);
        view->GetWidget(mWidget);

        if (PR_TRUE == windowless)
        {
          mPluginWindow.window = nsnull;    //XXX this needs to be a HDC
          mPluginWindow.type = nsPluginWindowType_Drawable;
        }
        else
        {
          mPluginWindow.window = (nsPluginPort *)mWidget->GetNativeData(NS_NATIVE_WINDOW);
          mPluginWindow.type = nsPluginWindowType_Window;
        }
      }
    }
  }

  return rv;
}
