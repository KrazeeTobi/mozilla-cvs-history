/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
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
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */


#include "nsPlaintextEditor.h"
#include "nsTextEditUtils.h"

#include "nsIDOMDocument.h"
#include "nsIDocument.h"
#include "nsIDOMEventReceiver.h" 
#include "nsIDOMNSEvent.h"
#include "nsIDOMMouseEvent.h"
#include "nsISelection.h"
#include "nsCRT.h"
#include "nsIServiceManagerUtils.h"

#include "nsIDOMRange.h"
#include "nsIDOMNSRange.h"
#include "nsISupportsArray.h"
#include "nsIDocumentEncoder.h"
#include "nsISupportsPrimitives.h"

// Drag & Drop, Clipboard
#include "nsIClipboard.h"
#include "nsITransferable.h"
#include "nsIDragService.h"
#include "nsIDOMNSUIEvent.h"

// Misc
#include "nsEditorUtils.h"

const PRUnichar nbsp = 160;

NS_IMETHODIMP nsPlaintextEditor::PrepareTransferable(nsITransferable **transferable)
{
  // Create generic Transferable for getting the data
  nsresult rv = nsComponentManager::CreateInstance("@mozilla.org/widget/transferable;1", nsnull, 
                                          NS_GET_IID(nsITransferable), 
                                          (void**)transferable);
  if (NS_FAILED(rv))
    return rv;

  // Get the nsITransferable interface for getting the data from the clipboard
  if (transferable) (*transferable)->AddDataFlavor(kUnicodeMime);
  return NS_OK;
}

nsresult nsPlaintextEditor::InsertTextAt(const nsAString &aStringToInsert,
                                         nsIDOMNode *aDestinationNode,
                                         PRInt32 aDestOffset,
                                         PRBool aDoDeleteSelection)
{
  if (aDestinationNode)
  {
    nsresult res;
    nsCOMPtr<nsISelection>selection;
    res = GetSelection(getter_AddRefs(selection));
    NS_ENSURE_SUCCESS(res, res);

    nsCOMPtr<nsIDOMNode> targetNode = aDestinationNode;
    PRInt32 targetOffset = aDestOffset;

    if (aDoDeleteSelection)
    {
      // Use an auto tracker so that our drop point is correctly
      // positioned after the delete.
      nsAutoTrackDOMPoint tracker(mRangeUpdater, &targetNode, &targetOffset);
      res = DeleteSelection(eNone);
      NS_ENSURE_SUCCESS(res, res);
    }

    res = selection->Collapse(targetNode, targetOffset);
    NS_ENSURE_SUCCESS(res, res);
  }

  return InsertText(aStringToInsert);
}

NS_IMETHODIMP nsPlaintextEditor::InsertTextFromTransferable(nsITransferable *aTransferable,
                                                            nsIDOMNode *aDestinationNode,
                                                            PRInt32 aDestOffset,
                                                            PRBool aDoDeleteSelection)
{
  nsresult rv = NS_OK;
  char* bestFlavor = nsnull;
  nsCOMPtr<nsISupports> genericDataObj;
  PRUint32 len = 0;
  if ( NS_SUCCEEDED(aTransferable->GetAnyTransferData(&bestFlavor, getter_AddRefs(genericDataObj), &len)) )
  {
    nsAutoTxnsConserveSelection dontSpazMySelection(this);
    nsAutoString flavor, stuffToPaste;
    flavor.AssignWithConversion( bestFlavor );   // just so we can use flavor.Equals()
    if (flavor.Equals(NS_LITERAL_STRING(kUnicodeMime)))
    {
      nsCOMPtr<nsISupportsString> textDataObj ( do_QueryInterface(genericDataObj) );
      if (textDataObj && len > 0)
      {
        nsAutoString text;
        textDataObj->GetData ( text );
        NS_ASSERTION(text.Length() <= (len/2), "Invalid length!");
        stuffToPaste.Assign ( text.get(), len / 2 );
        nsAutoEditBatch beginBatching(this);
        rv = InsertTextAt(stuffToPaste, aDestinationNode, aDestOffset, aDoDeleteSelection);
      }
    }
  }
  nsCRT::free(bestFlavor);
      
  // Try to scroll the selection into view if the paste/drop succeeded
  if (NS_SUCCEEDED(rv))
    ScrollSelectionIntoView(PR_FALSE);

  return rv;
}

NS_IMETHODIMP nsPlaintextEditor::InsertFromDrop(nsIDOMEvent* aDropEvent)
{
  ForceCompositionEnd();
  
  nsresult rv;
  nsCOMPtr<nsIDragService> dragService = 
           do_GetService("@mozilla.org/widget/dragservice;1", &rv);
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIDragSession> dragSession;
  dragService->GetCurrentSession(getter_AddRefs(dragSession));
  if (!dragSession) return NS_OK;

  // transferable hooks
  nsCOMPtr<nsIDOMDocument> domdoc;
  GetDocument(getter_AddRefs(domdoc));
  if (!nsEditorHookUtils::DoAllowDropHook(domdoc, aDropEvent, dragSession))
    return NS_OK;

  // Get the nsITransferable interface for getting the data from the drop
  nsCOMPtr<nsITransferable> trans;
  rv = PrepareTransferable(getter_AddRefs(trans));
  if (NS_FAILED(rv)) return rv;
  if (!trans) return NS_OK;  // NS_ERROR_FAILURE; SHOULD WE FAIL?

  PRUint32 numItems = 0; 
  rv = dragSession->GetNumDropItems(&numItems);
  if (NS_FAILED(rv)) return rv;

  // Combine any deletion and drop insertion into one transaction
  nsAutoEditBatch beginBatching(this);

  PRBool deleteSelection = PR_FALSE;
  nsCOMPtr<nsIDOMNode> newSelectionParent;
  PRInt32 newSelectionOffset = 0;

  PRUint32 i; 
  PRBool doPlaceCaret = PR_TRUE;
  for (i = 0; i < numItems; ++i)
  {
    rv = dragSession->GetData(trans, i);
    if (NS_FAILED(rv)) return rv;
    if (!trans) return NS_OK; // NS_ERROR_FAILURE; Should we fail?

    if ( doPlaceCaret )
    {
      // check if the user pressed the key to force a copy rather than a move
      // if we run into problems here, we'll just assume the user doesn't want a copy
      PRBool userWantsCopy = PR_FALSE;

      nsCOMPtr<nsIDOMNSUIEvent> nsuiEvent (do_QueryInterface(aDropEvent));
      if (!nsuiEvent) return NS_ERROR_FAILURE;

      nsCOMPtr<nsIDOMMouseEvent> mouseEvent ( do_QueryInterface(aDropEvent) );
      if (mouseEvent)

#if defined(XP_MAC) || defined(XP_MACOSX)
        mouseEvent->GetAltKey(&userWantsCopy);
#else
        mouseEvent->GetCtrlKey(&userWantsCopy);
#endif
      // Source doc is null if source is *not* the current editor document
      nsCOMPtr<nsIDOMDocument> srcdomdoc;
      rv = dragSession->GetSourceDocument(getter_AddRefs(srcdomdoc));
      if (NS_FAILED(rv)) return rv;

      // Current doc is destination
      nsCOMPtr<nsIDOMDocument>destdomdoc; 
      rv = GetDocument(getter_AddRefs(destdomdoc)); 
      if (NS_FAILED(rv)) return rv;

      nsCOMPtr<nsISelection> selection;
      rv = GetSelection(getter_AddRefs(selection));
      if (NS_FAILED(rv)) return rv;
      if (!selection) return NS_ERROR_FAILURE;

      PRBool isCollapsed;
      rv = selection->GetIsCollapsed(&isCollapsed);
      if (NS_FAILED(rv)) return rv;
      
      // Parent and offset under the mouse cursor
      rv = nsuiEvent->GetRangeParent(getter_AddRefs(newSelectionParent));
      if (NS_FAILED(rv)) return rv;
      if (!newSelectionParent) return NS_ERROR_FAILURE;

      rv = nsuiEvent->GetRangeOffset(&newSelectionOffset);
      if (NS_FAILED(rv)) return rv;

      // We never have to delete if selection is already collapsed
      PRBool cursorIsInSelection = PR_FALSE;

      // Check if mouse is in the selection
      if (!isCollapsed)
      {
        PRInt32 rangeCount;
        rv = selection->GetRangeCount(&rangeCount);
        if (NS_FAILED(rv)) 
          return rv;

        for (PRInt32 j = 0; j < rangeCount; j++)
        {
          nsCOMPtr<nsIDOMRange> range;

          rv = selection->GetRangeAt(j, getter_AddRefs(range));
          if (NS_FAILED(rv) || !range) 
            continue;//dont bail yet, iterate through them all

          nsCOMPtr<nsIDOMNSRange> nsrange(do_QueryInterface(range));
          if (NS_FAILED(rv) || !nsrange) 
            continue;//dont bail yet, iterate through them all

          rv = nsrange->IsPointInRange(newSelectionParent, newSelectionOffset, &cursorIsInSelection);
          if(cursorIsInSelection)
            break;
        }
        if (cursorIsInSelection)
        {
          // Dragging within same doc can't drop on itself -- leave!
          // (We shouldn't get here - drag event shouldn't have started if over selection)
          if (srcdomdoc == destdomdoc)
            return NS_OK;
          
          // Dragging from another window onto a selection
          // XXX Decision made to NOT do this,
          //     note that 4.x does replace if dropped on
          //deleteSelection = PR_TRUE;
        }
        else 
        {
          // We are NOT over the selection
          if (srcdomdoc == destdomdoc)
          {
            // Within the same doc: delete if user doesn't want to copy
            deleteSelection = !userWantsCopy;
          }
          else
          {
            // Different source doc: Don't delete
            deleteSelection = PR_FALSE;
          }
        }
      }

      // We have to figure out whether to delete and relocate caret only once
      doPlaceCaret = PR_FALSE;
    }
    
    if (!nsEditorHookUtils::DoInsertionHook(domdoc, aDropEvent, trans))
      return NS_OK;

    rv = InsertTextFromTransferable(trans, newSelectionParent, newSelectionOffset, deleteSelection);
  }

  return rv;
}

NS_IMETHODIMP nsPlaintextEditor::CanDrag(nsIDOMEvent *aDragEvent, PRBool *aCanDrag)
{
  if (!aCanDrag)
    return NS_ERROR_NULL_POINTER;
  /* we really should be checking the XY coordinates of the mouseevent and ensure that
   * that particular point is actually within the selection (not just that there is a selection)
   */
  *aCanDrag = PR_FALSE;
 
  // KLUDGE to work around bug 50703
  // After double click and object property editing, 
  //  we get a spurious drag event
  if (mIgnoreSpuriousDragEvent)
  {
    mIgnoreSpuriousDragEvent = PR_FALSE;
    return NS_OK;
  }
   
  nsCOMPtr<nsISelection> selection;
  nsresult res = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res)) return res;
    
  PRBool isCollapsed;
  res = selection->GetIsCollapsed(&isCollapsed);
  if (NS_FAILED(res)) return res;
  
  // if we are collapsed, we have no selection so nothing to drag
  if ( isCollapsed )
    return NS_OK;

  nsCOMPtr<nsIDOMEventTarget> eventTarget;

  nsCOMPtr<nsIDOMNSEvent> nsevent(do_QueryInterface(aDragEvent));

  if (nsevent) {
    res = nsevent->GetTmpRealOriginalTarget(getter_AddRefs(eventTarget));
    if (NS_FAILED(res)) return res;
  }

  if (eventTarget)
  {
    nsCOMPtr<nsIDOMNode> eventTargetDomNode = do_QueryInterface(eventTarget);
    if ( eventTargetDomNode )
    {
      PRBool isTargetedCorrectly = PR_FALSE;
      res = selection->ContainsNode(eventTargetDomNode, PR_FALSE, &isTargetedCorrectly);
      if (NS_FAILED(res)) return res;

      *aCanDrag = isTargetedCorrectly;
    }
  }

  if (NS_FAILED(res)) return res;
  if (!*aCanDrag) return NS_OK;

  nsCOMPtr<nsIDOMDocument> domdoc;
  GetDocument(getter_AddRefs(domdoc));
  *aCanDrag = nsEditorHookUtils::DoAllowDragHook(domdoc, aDragEvent);
  return NS_OK;
}

NS_IMETHODIMP nsPlaintextEditor::DoDrag(nsIDOMEvent *aDragEvent)
{
  nsresult rv;

  nsCOMPtr<nsITransferable> trans;
  rv = PutDragDataInTransferable(getter_AddRefs(trans));
  if (NS_FAILED(rv)) return rv;
  if (!trans) return NS_OK; // maybe there was nothing to copy?

 /* get the drag service */
  nsCOMPtr<nsIDragService> dragService = 
           do_GetService("@mozilla.org/widget/dragservice;1", &rv);
  if (NS_FAILED(rv)) return rv;

  /* create an array of transferables */
  nsCOMPtr<nsISupportsArray> transferableArray;
  NS_NewISupportsArray(getter_AddRefs(transferableArray));
  if (transferableArray == nsnull)
    return NS_ERROR_OUT_OF_MEMORY;

  /* add the transferable to the array */
  rv = transferableArray->AppendElement(trans);
  if (NS_FAILED(rv)) return rv;

  // check our transferable hooks (if any)
  nsCOMPtr<nsIDOMDocument> domdoc;
  GetDocument(getter_AddRefs(domdoc));
  if (!nsEditorHookUtils::DoDragHook(domdoc, aDragEvent, trans))
    return NS_OK;

  /* invoke drag */
  nsCOMPtr<nsIDOMEventTarget> eventTarget;
  rv = aDragEvent->GetTarget(getter_AddRefs(eventTarget));
  if (NS_FAILED(rv)) return rv;
  nsCOMPtr<nsIDOMNode> domnode = do_QueryInterface(eventTarget);

  unsigned int flags;
  // in some cases we'll want to cut rather than copy... hmmmmm...
  flags = nsIDragService::DRAGDROP_ACTION_COPY + nsIDragService::DRAGDROP_ACTION_MOVE;

  rv = dragService->InvokeDragSession(domnode, transferableArray, nsnull, flags);
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIDOMNSEvent> nsevent(do_QueryInterface(aDragEvent));
  if (nsevent)
    nsevent->PreventBubble();

  return rv;
}

NS_IMETHODIMP nsPlaintextEditor::Paste(PRInt32 aSelectionType)
{
  ForceCompositionEnd();

  // Get Clipboard Service
  nsresult rv;
  nsCOMPtr<nsIClipboard> clipboard(do_GetService("@mozilla.org/widget/clipboard;1", &rv));
  if ( NS_FAILED(rv) )
    return rv;
    
  // Get the nsITransferable interface for getting the data from the clipboard
  nsCOMPtr<nsITransferable> trans;
  rv = PrepareTransferable(getter_AddRefs(trans));
  if (NS_SUCCEEDED(rv) && trans)
  {
    // Get the Data from the clipboard  
    if (NS_SUCCEEDED(clipboard->GetData(trans, aSelectionType)) && IsModifiable())
    {
      // handle transferable hooks
      nsCOMPtr<nsIDOMDocument> domdoc;
      GetDocument(getter_AddRefs(domdoc));
      if (!nsEditorHookUtils::DoInsertionHook(domdoc, nsnull, trans))
        return NS_OK;

      rv = InsertTextFromTransferable(trans, nsnull, nsnull, PR_TRUE);
    }
  }

  return rv;
}


NS_IMETHODIMP nsPlaintextEditor::CanPaste(PRInt32 aSelectionType, PRBool *aCanPaste)
{
  if (!aCanPaste)
    return NS_ERROR_NULL_POINTER;
  *aCanPaste = PR_FALSE;
  
  // can't paste if readonly
  if (!IsModifiable())
    return NS_OK;

  nsresult rv;
  nsCOMPtr<nsIClipboard> clipboard(do_GetService("@mozilla.org/widget/clipboard;1", &rv));
  if (NS_FAILED(rv)) return rv;
  
  // the flavors that we can deal with
  const char* const textEditorFlavors[] = { kUnicodeMime, nsnull };

  nsCOMPtr<nsISupportsArray> flavorsList;
  rv = nsComponentManager::CreateInstance(NS_SUPPORTSARRAY_CONTRACTID, nsnull, 
         NS_GET_IID(nsISupportsArray), getter_AddRefs(flavorsList));
  if (NS_FAILED(rv)) return rv;
  
  PRUint32 editorFlags;
  GetFlags(&editorFlags);
  
  // add the flavors for text editors
  for (const char* const* flavor = textEditorFlavors; *flavor; flavor++)
  {
    nsCOMPtr<nsISupportsCString> flavorString;            
    nsComponentManager::CreateInstance(NS_SUPPORTS_CSTRING_CONTRACTID, nsnull, 
         NS_GET_IID(nsISupportsCString), getter_AddRefs(flavorString));
    if (flavorString)
    {
      flavorString->SetData(nsDependentCString(*flavor));
      flavorsList->AppendElement(flavorString);
    }
  }
  
  PRBool haveFlavors;
  rv = clipboard->HasDataMatchingFlavors(flavorsList, aSelectionType, &haveFlavors);
  if (NS_FAILED(rv)) return rv;
  
  *aCanPaste = haveFlavors;
  return NS_OK;
}

nsresult
nsPlaintextEditor::SetupDocEncoder(nsIDocumentEncoder **aDocEncoder)
{
  nsCOMPtr<nsIDOMDocument> domdoc;
  nsresult rv = GetDocument(getter_AddRefs(domdoc));
  if (NS_FAILED(rv)) return rv;
	
  nsCOMPtr<nsIDocument> doc = do_QueryInterface(domdoc);
  if (!doc) return NS_ERROR_FAILURE;

  // find out if we're a plaintext control or not
  PRUint32 editorFlags = 0;
  rv = GetFlags(&editorFlags);
  if (NS_FAILED(rv)) return rv;

  PRBool bIsPlainTextControl = ((editorFlags & eEditorPlaintextMask) != 0);

  // get correct mimeType and document encoder flags set
  nsAutoString mimeType;
  PRUint32 docEncoderFlags = 0;
  if (bIsPlainTextControl)
  {
    docEncoderFlags |= nsIDocumentEncoder::OutputBodyOnly | nsIDocumentEncoder::OutputPreformatted;
    mimeType = NS_LITERAL_STRING(kUnicodeMime);
  }
  else
    mimeType = NS_LITERAL_STRING(kHTMLMime);

  // set up docEncoder
  nsCOMPtr<nsIDocumentEncoder> encoder = do_CreateInstance(NS_HTMLCOPY_ENCODER_CONTRACTID);
  if (!encoder)
    return NS_ERROR_OUT_OF_MEMORY;

  rv = encoder->Init(doc, mimeType, docEncoderFlags);
  if (NS_FAILED(rv)) return rv;
    
  /* get the selection to be dragged */
  nsCOMPtr<nsISelection> selection;
  rv = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(rv)) return rv;

  rv = encoder->SetSelection(selection);
  if (NS_FAILED(rv)) return rv;

  *aDocEncoder = encoder;
  NS_ADDREF(*aDocEncoder);
  return NS_OK;
}

nsresult
nsPlaintextEditor::PutDragDataInTransferable(nsITransferable **aTransferable)
{
  *aTransferable = nsnull;
  nsCOMPtr<nsIDocumentEncoder> docEncoder;
  nsresult rv = SetupDocEncoder(getter_AddRefs(docEncoder));
  if (NS_FAILED(rv)) return rv;

  // grab a string
  nsAutoString buffer;
  rv = docEncoder->EncodeToString(buffer);
  if (NS_FAILED(rv)) return rv;

  // if we have an empty string, we're done; otherwise continue
  if (buffer.IsEmpty())
    return NS_OK;

  nsCOMPtr<nsISupportsString> dataWrapper =
                        do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = dataWrapper->SetData(buffer);
  if (NS_FAILED(rv)) return rv;

  /* create html flavor transferable */
  nsCOMPtr<nsITransferable> trans = do_CreateInstance("@mozilla.org/widget/transferable;1", &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  // find out if we're a plaintext control or not
  PRUint32 editorFlags = 0;
  rv = GetFlags(&editorFlags);
  if (NS_FAILED(rv)) return rv;

  PRBool bIsPlainTextControl = ((editorFlags & eEditorPlaintextMask) != 0);
  if (bIsPlainTextControl)
  {
    // Add the unicode flavor to the transferable
    rv = trans->AddDataFlavor(kUnicodeMime);
    if (NS_FAILED(rv)) return rv;
  }
  else
  {
    rv = trans->AddDataFlavor(kHTMLMime);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIFormatConverter> htmlConverter = do_CreateInstance("@mozilla.org/widget/htmlformatconverter;1");
    NS_ENSURE_TRUE(htmlConverter, NS_ERROR_FAILURE);

    rv = trans->SetConverter(htmlConverter);
    if (NS_FAILED(rv)) return rv;
  }

  // QI the data object an |nsISupports| so that when the transferable holds
  // onto it, it will addref the correct interface.
  nsCOMPtr<nsISupports> nsisupportsDataWrapper = do_QueryInterface(dataWrapper);
  rv = trans->SetTransferData(bIsPlainTextControl ? kUnicodeMime : kHTMLMime,
                   nsisupportsDataWrapper, buffer.Length() * sizeof(PRUnichar));
  if (NS_FAILED(rv)) return rv;

  *aTransferable = trans;
  NS_ADDREF(*aTransferable);
  return NS_OK;
}
