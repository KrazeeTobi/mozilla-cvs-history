/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
#include "nsEditorMode.h"
#include "nsEditorInterfaces.h"
#include "nsIDOMEventReceiver.h"
#include "nsIDOMEventCapturer.h"
#include "nsString.h"
#include "nsIDOMText.h"
#include "nsIDOMElement.h"

static nsIDOMDocument* kDomDoc;
static nsIDOMNode* kCurrentNode;

static NS_DEFINE_IID(kIDOMTextIID, NS_IDOMTEXT_IID);
static NS_DEFINE_IID(kIDOMElementIID, NS_IDOMELEMENT_IID);

nsresult NS_InitEditorMode(nsIDOMDocument *aDOMDocument)
{
  NS_IF_RELEASE(kCurrentNode);
  NS_IF_RELEASE(kDomDoc);
  
  kCurrentNode = nsnull;

  kDomDoc = aDOMDocument;
  NS_IF_ADDREF(kDomDoc);

  RegisterEventListeners();
  
  return NS_OK;
}

nsresult RegisterEventListeners()
{
  nsIDOMEventReceiver *er;

  static NS_DEFINE_IID(kIDOMEventReceiverIID, NS_IDOMEVENTRECEIVER_IID);
  static NS_DEFINE_IID(kIDOMMouseListenerIID, NS_IDOMMOUSELISTENER_IID);
  static NS_DEFINE_IID(kIDOMKeyListenerIID, NS_IDOMKEYLISTENER_IID);
  
  if (NS_OK == kDomDoc->QueryInterface(kIDOMEventReceiverIID, (void**) &er)) {
  
      nsIDOMEventListener * mouseListener;
      nsIDOMEventListener * keyListener;

      if (NS_OK == NS_NewEditorKeyListener(&keyListener)) {
        er->AddEventListener(keyListener, kIDOMKeyListenerIID);
      }
      if (NS_OK == NS_NewEditorMouseListener(&mouseListener)) {
        er->AddEventListener(mouseListener, kIDOMMouseListenerIID);
      }
  }
  
  return NS_OK;
}

nsresult nsDeleteLast()
{
  nsIDOMNode *mNode;
  nsIDOMText *mText;
  nsString mStr;
  PRInt32 mLength;

  if (NS_OK == nsGetCurrentNode(&mNode) && 
      NS_OK == mNode->QueryInterface(kIDOMTextIID, (void**)&mText)) {
    mText->GetData(mStr);
    mLength = mStr.Length();
    if (mLength > 0) {
      mText->Remove(mLength-1, 1);
    }
    NS_RELEASE(mText);
  }

  NS_IF_RELEASE(mNode);

  return NS_OK;
}

nsresult GetFirstTextNode(nsIDOMNode *aNode, nsIDOMNode **aRetNode)
{
  PRInt32 mType;
  PRBool mCNodes;
  
  *aRetNode = nsnull;

  aNode->GetNodeType(&mType);

  if (aNode->ELEMENT == mType) {
    if (NS_OK == aNode->GetHasChildNodes(&mCNodes) && PR_TRUE == mCNodes) {
      nsIDOMNode *mNode, *mSibNode;

      aNode->GetFirstChild(&mNode);
      while(nsnull == *aRetNode) {
        GetFirstTextNode(mNode, aRetNode);
        mNode->GetNextSibling(&mSibNode);
        NS_RELEASE(mNode);
        mNode = mSibNode;
      }
      NS_IF_RELEASE(mNode);
    }
  }
  else if (aNode->TEXT == mType) {
    *aRetNode = aNode;
    NS_ADDREF(aNode);
  }

  return NS_OK;
}

nsresult nsGetCurrentNode(nsIDOMNode ** aNode)
{
  if (kCurrentNode != nsnull) {
    *aNode = kCurrentNode;
    NS_ADDREF(kCurrentNode);
    return NS_OK;
  }
  
  /* If no node set, get first text node */
  nsIDOMNode *mDocNode = nsnull, *mFirstTextNode;

  if (NS_OK == kDomDoc->GetFirstChild(&mDocNode) && 
      NS_OK == GetFirstTextNode(mDocNode, &mFirstTextNode)) {
    nsSetCurrentNode(mFirstTextNode);
    NS_RELEASE(mFirstTextNode);
    NS_RELEASE(mDocNode);
    *aNode = kCurrentNode;
    NS_ADDREF(kCurrentNode);
    return NS_OK;
  }

  NS_IF_RELEASE(mDocNode);
  return NS_ERROR_FAILURE;
}

nsresult nsSetCurrentNode(nsIDOMNode * aNode)
{
  NS_IF_RELEASE(kCurrentNode);
  kCurrentNode = aNode;
  NS_ADDREF(kCurrentNode);
  return NS_OK;
}

nsresult nsAppendText(nsString *aStr)
{
  nsIDOMNode *mNode = nsnull;
  nsIDOMText *mText;

  if (NS_OK == nsGetCurrentNode(&mNode) && 
      NS_OK == mNode->QueryInterface(kIDOMTextIID, (void**)&mText)) {
    mText->Append(*aStr);
    NS_RELEASE(mText);
  }

  NS_IF_RELEASE(mNode);

  return NS_OK;
}
