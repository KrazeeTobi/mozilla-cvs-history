/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
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
 * Portions created by the Initial Developer are Copyright (C) 2003
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * Original Author: Aaron Leventhal (aaronl@netscape.com)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "nsDocAccessible.h"
#include "nsIDocument.h"
#include "nsIPresShell.h"
#include "nsIFrame.h"
#include "nsIDOMDocument.h"
#include "nsIDOMEventTarget.h"
#include "nsIDOMEvent.h"
#include "nsIDOMDocument.h"
#include "nsIDOMDocumentType.h"
#include "nsIDOMNSDocument.h"
#include "nsIDOMMutationEvent.h"
#ifdef MOZ_XUL
#include "nsIXULDocument.h"
#endif
#include "nsINameSpaceManager.h"
#include "nsIWebNavigation.h"
#include "nsIURI.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIServiceManager.h"
#include "nsHashtable.h"
#include "nsIDocShell.h"
#include "nsIDocShellTreeItem.h"
#include "nsIScrollableView.h"
#include "nsIAccessibilityService.h"
#ifdef MOZ_ACCESSIBILITY_ATK
#include "nsAccessibleEventData.h"
#endif

//=============================//
// nsDocAccessible  //
//=============================//

//-----------------------------------------------------
// construction 
//-----------------------------------------------------
nsDocAccessible::nsDocAccessible(nsIDOMNode *aDOMNode, nsIWeakReference* aShell):
  nsAccessibleWrap(aDOMNode, aShell), 
  mAccessNodeCache(nsnull), mWnd(nsnull), 
  mScrollWatchTimer(nsnull), mDocLoadTimer(nsnull), 
  mWebProgress(nsnull), mBusy(eBusyStateUninitialized), 
  mScrollPositionChangedTicks(0), mIsNewDocument(PR_FALSE)
{
  // Because of the way document loading happens, the new nsIWidget is created before
  // the old one is removed. Since it creates the nsDocAccessible, for a brief moment 
  // there can be 2 nsDocAccessible's for the content area, although for 2 different
  // pres shells.

  nsCOMPtr<nsIPresShell> shell(do_QueryReferent(mWeakShell));
  if (shell) {
    shell->GetDocument(getter_AddRefs(mDocument));
    nsCOMPtr<nsIViewManager> vm;
    shell->GetViewManager(getter_AddRefs(vm));
    nsCOMPtr<nsIWidget> widget;
    vm->GetWidget(getter_AddRefs(widget));
    mWnd = widget->GetNativeData(NS_NATIVE_WINDOW);
  }
  
  NS_ASSERTION(gGlobalDocAccessibleCache, "No global doc accessible cache");
  PutCacheEntry(gGlobalDocAccessibleCache, NS_STATIC_CAST(void*, mWeakShell), this);
#ifdef DEBUG
  printf("\nATTENTION: New doc accessible for weak shell %x\n", mWeakShell.get());
#endif

  // XXX aaronl should we use an algorithm for the initial cache size?
#ifdef OLD_HASH
  mAccessNodeCache = new nsSupportsHashtable(kDefaultCacheSize);
#else
  mAccessNodeCache = new nsInterfaceHashtable<nsVoidHashKey, nsIAccessNode>;
  mAccessNodeCache->Init(kDefaultCacheSize);
#endif
}

//-----------------------------------------------------
// destruction
//-----------------------------------------------------
nsDocAccessible::~nsDocAccessible()
{
}

NS_IMPL_ISUPPORTS_INHERITED6(nsDocAccessible, nsAccessible, nsIAccessibleDocument, 
                             nsIAccessibleEventReceiver, nsIWebProgressListener, 
                             nsIDOMMutationListener,
                             nsIScrollPositionListener, nsISupportsWeakReference)

NS_IMETHODIMP nsDocAccessible::AddEventListeners()
{
  AddContentDocListeners();
  return NS_OK;
}

/* void removeAccessibleEventListener (); */
NS_IMETHODIMP nsDocAccessible::RemoveEventListeners()
{
  RemoveContentDocListeners();
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::GetAccName(nsAString& aAccName) 
{ 
  return GetTitle(aAccName);
}

NS_IMETHODIMP nsDocAccessible::GetAccRole(PRUint32 *_retval)
{
  *_retval = ROLE_PANE;
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::GetAccValue(nsAString& aAccValue)
{
  return GetURL(aAccValue);
}

NS_IMETHODIMP nsDocAccessible::GetAccState(PRUint32 *aAccState)
{
  *aAccState = STATE_FOCUSABLE;
  if (mBusy == eBusyStateLoading)
    *aAccState |= STATE_BUSY;
  return NS_OK;
}

// ------- nsIAccessibleDocument Methods (5) ---------------

NS_IMETHODIMP nsDocAccessible::GetURL(nsAString& aURL)
{ 
  if (!mDocument) {
    return NS_ERROR_FAILURE; // Document has been shut down
  }
  nsCOMPtr<nsISupports> container;
  mDocument->GetContainer(getter_AddRefs(container));
  nsCOMPtr<nsIWebNavigation> webNav(do_GetInterface(container));
  nsCAutoString theURL;
  if (webNav) {
    nsCOMPtr<nsIURI> pURI;
    webNav->GetCurrentURI(getter_AddRefs(pURI));
    if (pURI) 
      pURI->GetSpec(theURL);
  }
  //XXXaaronl Need to use CopyUTF8toUCS2(nsDependentCString(theURL), aURL); 
  //          when it's written
  aURL.Assign(NS_ConvertUTF8toUCS2(theURL)); 
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::GetTitle(nsAString& aTitle)
{
  return mDocument? mDocument->GetDocumentTitle(aTitle): NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsDocAccessible::GetMimeType(nsAString& aMimeType)
{
  nsCOMPtr<nsIDOMNSDocument> domnsDocument(do_QueryInterface(mDocument));
  if (domnsDocument) {
    return domnsDocument->GetContentType(aMimeType);
  }
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsDocAccessible::GetDocType(nsAString& aDocType)
{
  nsCOMPtr<nsIDOMDocument> domDoc(do_QueryInterface(mDocument));
  nsCOMPtr<nsIDOMDocumentType> docType;

#ifdef MOZ_XUL
  nsCOMPtr<nsIXULDocument> xulDoc(do_QueryInterface(mDocument));
  if (xulDoc) {
    aDocType = NS_LITERAL_STRING("window"); // doctype not implemented for XUL at time of writing - causes assertion
    return NS_OK;
  } else
#endif
  if (domDoc && NS_SUCCEEDED(domDoc->GetDoctype(getter_AddRefs(docType))) && docType) {
    return docType->GetName(aDocType);
  }

  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsDocAccessible::GetNameSpaceURIForID(PRInt16 aNameSpaceID, nsAString& aNameSpaceURI)
{
  if (mDocument) {
    nsCOMPtr<nsINameSpaceManager> nameSpaceManager =
        do_GetService(NS_NAMESPACEMANAGER_CONTRACTID);
    if (nameSpaceManager) 
      return nameSpaceManager->GetNameSpaceURI(aNameSpaceID, aNameSpaceURI);
  }
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsDocAccessible::GetCaretAccessible(nsIAccessibleCaret **aCaretAccessible)
{
  // We only have a caret accessible on the root document
  *aCaretAccessible = nsnull;
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::GetWindow(void **aWindow)
{
  *aWindow = mWnd;
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::GetCachedAccessNode(void *aUniqueID, nsIAccessNode **aAccessNode)
{
  NS_ASSERTION(mAccessNodeCache, "No accessibility cache for document");
  GetCacheEntry(mAccessNodeCache, aUniqueID, aAccessNode); // Addrefs for us
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::CacheAccessNode(void *aUniqueID, nsIAccessNode *aAccessNode)
{
  NS_ASSERTION(mAccessNodeCache, "No accessibility cache for document");
  PutCacheEntry(mAccessNodeCache, aUniqueID, aAccessNode);
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::Destroy()
{
  NS_ASSERTION(gGlobalDocAccessibleCache, "No global doc accessible cache");
  if (gGlobalDocAccessibleCache) {
#ifdef OLD_HASH
    nsVoidKey key(NS_STATIC_CAST(void*, mWeakShell));
    gGlobalDocAccessibleCache->Remove(&key);
#else
    gGlobalDocAccessibleCache->Remove(NS_STATIC_CAST(void*, mWeakShell));
#endif
  }
  return Shutdown();
}

NS_IMETHODIMP nsDocAccessible::Shutdown()
{
  if (!mWeakShell) {
    return NS_OK;  // Already shutdown
  }

  mWeakShell = nsnull;  // Avoid reentrancy

  RemoveEventListeners();
  if (mScrollWatchTimer) {
    mScrollWatchTimer->Cancel();
    mScrollWatchTimer = nsnull;
  }
  if (mDocLoadTimer) {
    mDocLoadTimer->Cancel();
    mDocLoadTimer = nsnull;
  }
  mWebProgress = nsnull;

  if (mAccessNodeCache) {
#ifdef OLD_HASH
    nsSupportsHashtable *hashToClear = mAccessNodeCache; // Avoid reentrency
#else
    nsInterfaceHashtable<nsVoidHashKey, nsIAccessNode> *hashToClear = mAccessNodeCache; // Avoid reentrency
#endif
    mAccessNodeCache = nsnull;
    ClearCache(hashToClear);
    delete hashToClear;
  }

  NS_ASSERTION(gGlobalDocAccessibleCache, "Global doc cache does not exist");
  mDocument = nsnull;
  return nsAccessibleWrap::Shutdown();
}

nsIFrame* nsDocAccessible::GetFrame()
{
  nsCOMPtr<nsIPresShell> shell(do_QueryReferent(mWeakShell));

  nsIFrame* root = nsnull;
  if (shell) 
    shell->GetRootFrame(&root);

  return root;
}

void nsDocAccessible::GetBounds(nsRect& aBounds, nsIFrame** aRelativeFrame)
{
  *aRelativeFrame = GetFrame();
  if (*aRelativeFrame)
    (*aRelativeFrame)->GetRect(aBounds);
}

void nsDocAccessible::AddContentDocListeners()
{
  // 1) Set up scroll position listener
  // 2) Set up web progress listener - we need to know 
  //    when page loading is finished
  //    That way we can send the STATE_CHANGE events for 
  //    the MSAA root "pane" object (ROLE_PANE),
  //    and change the STATE_BUSY bit flag
  //    Do this only for top level content documents

  nsCOMPtr<nsIPresShell> presShell(GetPresShell());
  if (!presShell)
    return;

  AddScrollListener(presShell);

  nsCOMPtr<nsISupports> container;
  mDocument->GetContainer(getter_AddRefs(container));

  nsCOMPtr<nsIDocShellTreeItem> docShellTreeItem(do_QueryInterface(container));
  if (!docShellTreeItem)
    return;

  // Make sure we're a content docshell
  // We don't want to listen to chrome progress
  PRInt32 itemType;
  docShellTreeItem->GetItemType(&itemType);

  if (itemType != nsIDocShellTreeItem::typeContent)
    return;

  // Make sure we're the top content doc shell 
  // We don't want to listen to iframe progress
  nsCOMPtr<nsIDocShellTreeItem> topOfContentTree;
  docShellTreeItem->GetSameTypeRootTreeItem(getter_AddRefs(topOfContentTree));
  if (topOfContentTree != docShellTreeItem)
    return;
  
  nsCOMPtr<nsIPresContext> context; 
  presShell->GetPresContext(getter_AddRefs(context));
  if (!context)
    return;

  mWebProgress = do_GetInterface(docShellTreeItem);
  if (!mWebProgress)
    return;

  mWebProgress->AddProgressListener(this, nsIWebProgress::NOTIFY_LOCATION | 
                                    nsIWebProgress::NOTIFY_STATE_DOCUMENT);


  mIsNewDocument = PR_TRUE;
  mBusy = eBusyStateLoading;
  PRBool isLoading;

  mWebProgress->GetIsLoadingDocument(&isLoading);
  if (!isLoading) {
    // If already loaded, fire "done loading" event after short timeout
    // If we fired the event here, we'd get reentrancy problems
    // Otherwise it will be fired from OnStateChange when the load is done
    mDocLoadTimer = do_CreateInstance("@mozilla.org/timer;1");
    if (mDocLoadTimer) {
      mDocLoadTimer->InitWithFuncCallback(DocLoadCallback, this, 1,
                                          nsITimer::TYPE_ONE_SHOT);
    }
  }

  // add ourself as a mutation event listener 
  // (this slows down mozilla about 3%, but only used when accessibility APIs active)
  nsCOMPtr<nsIDOMEventTarget> target(do_QueryInterface(mDocument));
  NS_ASSERTION(target, "No dom event target for document");
  nsresult rv = target->AddEventListener(NS_LITERAL_STRING("DOMAttrModified"), 
    this, PR_TRUE);
  NS_ASSERTION(NS_SUCCEEDED(rv), "failed to register listener");
  rv = target->AddEventListener(NS_LITERAL_STRING("DOMSubtreeModified"), 
    this, PR_TRUE);
  NS_ASSERTION(NS_SUCCEEDED(rv), "failed to register listener");
  rv = target->AddEventListener(NS_LITERAL_STRING("DOMNodeInserted"), 
    this, PR_TRUE);
  NS_ASSERTION(NS_SUCCEEDED(rv), "failed to register listener");
  rv = target->AddEventListener(NS_LITERAL_STRING("DOMNodeRemoved"), 
    this, PR_TRUE);
  NS_ASSERTION(NS_SUCCEEDED(rv), "failed to register listener");
  rv = target->AddEventListener(NS_LITERAL_STRING("DOMNodeInsertedIntoDocument"), 
    this, PR_TRUE);
  NS_ASSERTION(NS_SUCCEEDED(rv), "failed to register listener");
  rv = target->AddEventListener(NS_LITERAL_STRING("DOMNodeRemovedFromDocument"), 
    this, PR_TRUE);
  NS_ASSERTION(NS_SUCCEEDED(rv), "failed to register listener");
}

void nsDocAccessible::RemoveContentDocListeners()
{
  // Remove listeners associated with content documents

  // Remove web progress listener
  if (mWebProgress) {
    mWebProgress->RemoveProgressListener(this);
    mWebProgress = nsnull;
  }

  // Remove scroll position listener
  nsCOMPtr<nsIPresShell> presShell(do_QueryReferent(mWeakShell));
  RemoveScrollListener(presShell);

  nsCOMPtr<nsIDOMEventTarget> target(do_QueryInterface(mDocument));
  NS_ASSERTION(target, "No dom event target for document");
  target->RemoveEventListener(NS_LITERAL_STRING("DOMAttrModified"), this, PR_TRUE);
  target->RemoveEventListener(NS_LITERAL_STRING("DOMSubtreeModified"), this, PR_TRUE);
  target->RemoveEventListener(NS_LITERAL_STRING("DOMNodeInserted"), this, PR_TRUE);
  target->RemoveEventListener(NS_LITERAL_STRING("DOMNodeRemoved"), this, PR_TRUE);
  target->RemoveEventListener(NS_LITERAL_STRING("DOMNodeInsertedIntoDocument"), this, PR_TRUE);
  target->RemoveEventListener(NS_LITERAL_STRING("DOMNodeRemovedFromDocument"), this, PR_TRUE);
}

void nsDocAccessible::FireDocLoadFinished()
{
  if (!mDocument || !mWeakShell)
    return;  // Document has been shut down

  // Hook up our new accessible with our parent
  if (!mParent) {
    nsCOMPtr<nsIDocument> parentDoc;
    mDocument->GetParentDocument(getter_AddRefs(parentDoc));
    if (parentDoc) {
      nsCOMPtr<nsIContent> ownerContent;
      parentDoc->FindContentForSubDocument(mDocument, 
                                           getter_AddRefs(ownerContent));
      nsCOMPtr<nsIDOMNode> ownerNode(do_QueryInterface(ownerContent));
      if (ownerNode) {
        nsCOMPtr<nsIAccessibilityService> accService = 
          do_GetService("@mozilla.org/accessibilityService;1");
        if (accService) {
          // XXX aaronl: ideally we would traverse the presshell chain
          // Since there's no easy way to do that, we cheat and use
          // the document hierarchy. GetAccessibleFor() is bad because
          // it doesn't support our concept of multiple presshells per doc.
          // It should be changed to use GetAccessibleInWeakShell()
          nsCOMPtr<nsIAccessible> accParent;
          accService->GetAccessibleFor(ownerNode, getter_AddRefs(accParent));
          SetAccParent(accParent);
          if (accParent) {
            accParent->SetAccFirstChild(this);
          }
        }
      }
    }
  }
  
  if (mIsNewDocument) {
    mIsNewDocument = PR_FALSE;

    if (mBusy != eBusyStateDone) {
      mBusy = eBusyStateDone;
#ifndef MOZ_ACCESSIBILITY_ATK
      FireToolkitEvent(nsIAccessibleEventReceiver::EVENT_STATE_CHANGE, this, nsnull);
#endif
    }
  }
}

void nsDocAccessible::DocLoadCallback(nsITimer *aTimer, void *aClosure)
{
  // Doc has finished loading, fire "load finished" event
  // This path is only used if the doc was already finished loading
  // when the DocAccessible was created. 
  // Otherwise, ::OnStateChange() fires the event when doc is loaded.

  nsDocAccessible *docAcc = NS_REINTERPRET_CAST(nsDocAccessible*, aClosure);
  if (docAcc)
    docAcc->FireDocLoadFinished();
}

void nsDocAccessible::ScrollTimerCallback(nsITimer *aTimer, void *aClosure)
{
  nsDocAccessible *docAcc = NS_REINTERPRET_CAST(nsDocAccessible*, aClosure);

  if (docAcc && docAcc->mScrollPositionChangedTicks && 
      ++docAcc->mScrollPositionChangedTicks > 2) {
    // Whenever scroll position changes, mScrollPositionChangeTicks gets reset to 1
    // We only want to fire accessibilty scroll event when scrolling stops or pauses
    // Therefore, we wait for no scroll events to occur between 2 ticks of this timer
    // That indicates a pause in scrolling, so we fire the accessibilty scroll event
    docAcc->FireToolkitEvent(nsIAccessibleEventReceiver::EVENT_SCROLLINGEND, docAcc, nsnull);
    docAcc->mScrollPositionChangedTicks = 0;
    if (docAcc->mScrollWatchTimer) {
      docAcc->mScrollWatchTimer->Cancel();
      docAcc->mScrollWatchTimer = nsnull;
    }
  }
}

void nsDocAccessible::AddScrollListener(nsIPresShell *aPresShell)
{
  nsCOMPtr<nsIViewManager> vm;

  if (aPresShell)
    aPresShell->GetViewManager(getter_AddRefs(vm));

  nsIScrollableView* scrollableView = nsnull;
  if (vm)
    vm->GetRootScrollableView(&scrollableView);

  if (scrollableView)
    scrollableView->AddScrollPositionListener(NS_STATIC_CAST(nsIScrollPositionListener *, this));
}

void nsDocAccessible::RemoveScrollListener(nsIPresShell *aPresShell)
{
  nsCOMPtr<nsIViewManager> vm;

  if (aPresShell)
    aPresShell->GetViewManager(getter_AddRefs(vm));

  nsIScrollableView* scrollableView = nsnull;
  if (vm)
    vm->GetRootScrollableView(&scrollableView);

  if (scrollableView)
    scrollableView->RemoveScrollPositionListener(NS_STATIC_CAST(nsIScrollPositionListener *, this));
}

NS_IMETHODIMP nsDocAccessible::ScrollPositionWillChange(nsIScrollableView *aView, nscoord aX, nscoord aY)
{
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::ScrollPositionDidChange(nsIScrollableView *aScrollableView, nscoord aX, nscoord aY)
{
  // Start new timer, if the timer cycles at least 1 full cycle without more scroll position changes, 
  // then the ::Notify() method will fire the accessibility event for scroll position changes
  const PRUint32 kScrollPosCheckWait = 50;
  if (mScrollWatchTimer) {
    mScrollWatchTimer->SetDelay(kScrollPosCheckWait);  // Create new timer, to avoid leaks
  }
  else {
    mScrollWatchTimer = do_CreateInstance("@mozilla.org/timer;1");
    if (mScrollWatchTimer) {
      mScrollWatchTimer->InitWithFuncCallback(ScrollTimerCallback, this,
                                              kScrollPosCheckWait, 
                                              nsITimer::TYPE_REPEATING_SLACK);
    }
  }
  mScrollPositionChangedTicks = 1;
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::OnStateChange(nsIWebProgress *aWebProgress,
  nsIRequest *aRequest, PRUint32 aStateFlags, nsresult aStatus)
{
  if ((aStateFlags & STATE_IS_DOCUMENT) && (aStateFlags & STATE_STOP))
    FireDocLoadFinished();   // Doc is ready!

  return NS_OK;
}

/* void onProgressChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in long aCurSelfProgress, in long aMaxSelfProgress, in long aCurTotalProgress, in long aMaxTotalProgress); */
NS_IMETHODIMP nsDocAccessible::OnProgressChange(nsIWebProgress *aWebProgress,
  nsIRequest *aRequest, PRInt32 aCurSelfProgress, PRInt32 aMaxSelfProgress,
  PRInt32 aCurTotalProgress, PRInt32 aMaxTotalProgress)
{
  NS_NOTREACHED("notification excluded in AddProgressListener(...)");
  return NS_OK;
}

/* void onLocationChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in nsIURI location); */
NS_IMETHODIMP nsDocAccessible::OnLocationChange(nsIWebProgress *aWebProgress,
  nsIRequest *aRequest, nsIURI *location)
{
  // Load has been verified, it will occur, about to commence

  // We won't fire a "doc finished loading" event on this nsDocAccessible 
  // Instead we fire that on the new nsDocAccessible that is created for the new doc
  mIsNewDocument = PR_FALSE;   // We're a doc that's going away

  if (mBusy != eBusyStateLoading) {
    mBusy = eBusyStateLoading; 
    // Fire a "new doc has started to load" event
#ifndef MOZ_ACCESSIBILITY_ATK
    FireToolkitEvent(nsIAccessibleEventReceiver::EVENT_STATE_CHANGE, this, nsnull);
#else
    AtkChildrenChange childrenData;
    childrenData.index = -1;
    childrenData.child = 0;
    childrenData.add = PR_FALSE;
    FireToolkitEvent(nsIAccessibleEventReceiver::EVENT_REORDER , this, &childrenData);
#endif
  }

  return NS_OK;
}

/* void onStatusChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in nsresult aStatus, in wstring aMessage); */
NS_IMETHODIMP nsDocAccessible::OnStatusChange(nsIWebProgress *aWebProgress,
  nsIRequest *aRequest, nsresult aStatus, const PRUnichar *aMessage)
{
  NS_NOTREACHED("notification excluded in AddProgressListener(...)");
  return NS_OK;
}

/* void onSecurityChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in unsigned long state); */
NS_IMETHODIMP nsDocAccessible::OnSecurityChange(nsIWebProgress *aWebProgress,
  nsIRequest *aRequest, PRUint32 state)
{
  NS_NOTREACHED("notification excluded in AddProgressListener(...)");
  return NS_OK;
}

// ---------- Mutation event listeners ------------
NS_IMETHODIMP nsDocAccessible::NodeInserted(nsIDOMEvent* aEvent)
{
  HandleMutationEvent(aEvent, nsIAccessibleEventReceiver::EVENT_CREATE);

  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::NodeRemoved(nsIDOMEvent* aEvent)
{
  // The related node for the event will be the parent of the removed node or subtree

  HandleMutationEvent(aEvent, nsIAccessibleEventReceiver::EVENT_DESTROY);

  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::SubtreeModified(nsIDOMEvent* aEvent)
{
  HandleMutationEvent(aEvent, nsIAccessibleEventReceiver::EVENT_REORDER);
 
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::AttrModified(nsIDOMEvent* aMutationEvent)
{
  // XXX todo
  // We will probably need to handle special cases here
  // For example, if an <img>'s usemap attribute is modified
  // Otherwise it may just be a state change, for example an object changing
  // its visibility.
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::NodeRemovedFromDocument(nsIDOMEvent* aMutationEvent)
{
  // Not implemented yet, see bug 74220
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::NodeInsertedIntoDocument(nsIDOMEvent* aMutationEvent)
{

  // Not implemented yet, see bug 74219
  // This is different from NodeInserted() in that it's fired when
  // a node is inserted into a document, but isn't necessarily mean that
  // it's becoming part of the DOM tree.
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::HandleEvent(nsIDOMEvent* aEvent)
{
  NS_NOTREACHED("Should be handled by specific methods like NodeInserted, etc.");
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::CharacterDataModified(nsIDOMEvent* aMutationEvent)
{
  return NS_OK;
}

void nsDocAccessible::InvalidateCacheSubtree(nsIDOMNode *aStartNode)
{
  // Invalidate cache subtree
  // We have to check for accessibles for each dom node by traversing DOM tree
  // instead of just the accessible tree, although that would be faster
  // Otherwise we might miss the nsAccessNode's that are not nsAccessible's.

  if (!aStartNode)
    return;
  nsCOMPtr<nsIDOMNode> iterNode(aStartNode), nextNode;
  nsCOMPtr<nsIAccessNode> accessNode;

  do {
    GetCachedAccessNode(NS_STATIC_CAST(void*, iterNode), 
                        getter_AddRefs(accessNode));
    if (accessNode) {
      // XXX aaronl todo: accessibles that implement their own subtrees,
      // like html combo boxes and xul trees, need to shutdown all of their own
      // children when they override Shutdown()
      accessNode->Shutdown();
    }

    iterNode->GetFirstChild(getter_AddRefs(nextNode));
    if (nextNode) {
      iterNode = nextNode;
      continue;
    }

    if (iterNode == aStartNode)
      break;
    iterNode->GetNextSibling(getter_AddRefs(nextNode));
    if (nextNode) {
      iterNode = nextNode;
      continue;
    }

    do {
      iterNode->GetParentNode(getter_AddRefs(nextNode));
      if (!nextNode || nextNode == aStartNode) {
        return;
      }
      nextNode->GetNextSibling(getter_AddRefs(iterNode));
      if (iterNode)
        break;
      iterNode = nextNode;
    } while (PR_TRUE);
  }
  while (iterNode && iterNode != aStartNode);
}

void nsDocAccessible::HandleMutationEvent(nsIDOMEvent *aEvent, PRUint32 aAccessibleEventType) 
{
  if (mBusy != eBusyStateDone)
    return;  // We don't want mutation events until doc is fully loaded

  nsCOMPtr<nsIDOMEvent> domEvent(do_QueryInterface(aEvent));
  nsCOMPtr<nsIDOMEventTarget> domEventTarget;
  if (domEvent) {
    domEvent->GetTarget(getter_AddRefs(domEventTarget));
  }
  nsCOMPtr<nsIDOMNode> targetNode(do_QueryInterface(domEventTarget));
  if (!targetNode)
    return;

  nsCOMPtr<nsIDOMNode> subTreeNode;
  if (aAccessibleEventType == nsIAccessibleEventReceiver::EVENT_CREATE) {
    // DOMNodeInserted
    // We'll need to invalidate the child count and cached children of
    // the direct parent, otherwise no new accessibles will appear at this
    // place in the tree.
    // However, don't invalidate the entire subtree - most of it is still good.
    targetNode->GetParentNode(getter_AddRefs(subTreeNode));
    NS_ASSERTION(subTreeNode, "No valid parent of insertion");
    targetNode = subTreeNode;
  }
  else {
    if (aAccessibleEventType == nsIAccessibleEventReceiver::EVENT_REORDER) {
      // DOMSubtreeModified
      // The event target is the parent of the reordered tree
      // We don't want to invalidate that entire subtree.
      nsCOMPtr<nsIDOMMutationEvent> mutationEvent(do_QueryInterface(aEvent));
      NS_ASSERTION(mutationEvent, "Not a mutation event!");
      mutationEvent->GetRelatedNode(getter_AddRefs(subTreeNode));
      NS_ASSERTION(subTreeNode, "No old sub tree being replaced in DOMSubtreeModified");
    }
    else {
      // nsIAccessibleEventReceiver::EVENT_DESTROY / DOMNodeRemoved
      subTreeNode = targetNode;
    }
    InvalidateCacheSubtree(subTreeNode);
  }

  // We need to get an accessible for the mutation event's target node
  // If there is no accessible for that node, we need to keep moving up the parent
  // chain so there is some accessible.
  // We will use this accessible to fire the accessible mutation event.
  // We're guaranteed success, because we will eventually end up at the doc accessible,
  // and there is always one of those.
  // XXX aaronl todo: are MSAA mutation events always fired on the container?
  //                  if so, we always need to get the dom parent at least once

  nsCOMPtr<nsIAccessibilityService> accService = 
    do_GetService("@mozilla.org/accessibilityService;1");
  if (!accService)
    return;

  nsCOMPtr<nsIAccessible> accessible;
  while (NS_FAILED(accService->GetAccessibleInWeakShell(targetNode, mWeakShell, 
                                                        getter_AddRefs(accessible)))) {
    targetNode->GetParentNode(getter_AddRefs(subTreeNode));
    NS_ASSERTION(subTreeNode, "Crawled up parent chain without finding accessible");
    targetNode = subTreeNode;
  }

  accessible->InvalidateChildren();

#ifdef XP_WIN
  // Windows MSAA clients crash if the listen to create or destroy events
  aAccessibleEventType = nsIAccessibleEventReceiver::EVENT_REORDER;
#endif
  accessible->FireToolkitEvent(aAccessibleEventType, accessible, nsnull);
}

