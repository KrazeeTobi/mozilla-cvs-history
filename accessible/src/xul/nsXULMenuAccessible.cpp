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
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Author: Aaron Leventhal (aaronl@netscape.com)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#include "nsXULMenuAccessible.h"
#include "nsIDOMElement.h"
#include "nsIDOMXULElement.h"
#include "nsIMutableArray.h"
#include "nsIDOMXULSelectCntrlItemEl.h"
#include "nsIDOMXULMultSelectCntrlEl.h"
#include "nsIDOMKeyEvent.h"
#include "nsIPrefService.h"
#include "nsIPrefBranch.h"
#include "nsIServiceManager.h"
#include "nsIPresShell.h"
#include "nsIContent.h"
#include "nsGUIEvent.h"
#include "nsXULFormControlAccessible.h"


/** ------------------------------------------------------ */
/**  Impl. of nsXULSelectableAccessible                    */
/** ------------------------------------------------------ */

// Helper methos
nsXULSelectableAccessible::nsXULSelectableAccessible(nsIDOMNode* aDOMNode,
                                                     nsIWeakReference* aShell):
nsAccessibleWrap(aDOMNode, aShell)
{
  mSelectControl = do_QueryInterface(aDOMNode);
}

NS_IMPL_ISUPPORTS_INHERITED1(nsXULSelectableAccessible, nsAccessible, nsIAccessibleSelectable)

NS_IMETHODIMP nsXULSelectableAccessible::Shutdown()
{
  mSelectControl = nsnull;
  return nsAccessibleWrap::Shutdown();
}

nsresult nsXULSelectableAccessible::ChangeSelection(PRInt32 aIndex, PRUint8 aMethod, PRBool *aSelState)
{
  *aSelState = PR_FALSE;

  if (!mSelectControl) {
    return NS_ERROR_FAILURE;
  }
  nsCOMPtr<nsIAccessible> childAcc;
  GetChildAt(aIndex, getter_AddRefs(childAcc));
  nsCOMPtr<nsIAccessNode> accNode = do_QueryInterface(childAcc);
  NS_ENSURE_TRUE(accNode, NS_ERROR_FAILURE);

  nsCOMPtr<nsIDOMNode> childNode;
  accNode->GetDOMNode(getter_AddRefs(childNode));
  nsCOMPtr<nsIDOMXULSelectControlItemElement> item(do_QueryInterface(childNode));
  NS_ENSURE_TRUE(item, NS_ERROR_FAILURE);

  item->GetSelected(aSelState);
  if (eSelection_GetState == aMethod) {
    return NS_OK;
  }

  nsCOMPtr<nsIDOMXULMultiSelectControlElement> xulMultiSelect =
    do_QueryInterface(mSelectControl);

  if (eSelection_Add == aMethod && !(*aSelState)) {
    return xulMultiSelect ? xulMultiSelect->AddItemToSelection(item) :
                            mSelectControl->SetSelectedItem(item);
  }
  if (eSelection_Remove == aMethod && (*aSelState)) {
    return xulMultiSelect ? xulMultiSelect->RemoveItemFromSelection(item) :
                            mSelectControl->SetSelectedItem(nsnull);
  }
  return NS_ERROR_FAILURE;
}

// Interface methods
NS_IMETHODIMP nsXULSelectableAccessible::GetSelectedChildren(nsIArray **aChildren)
{
  *aChildren = nsnull;
  if (!mSelectControl) {
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsIAccessibilityService> accService = GetAccService();
  NS_ENSURE_TRUE(accService, NS_ERROR_FAILURE);

  nsCOMPtr<nsIMutableArray> selectedAccessibles =
    do_CreateInstance(NS_ARRAY_CONTRACTID);
  NS_ENSURE_STATE(selectedAccessibles);

  // For XUL multi-select control
  nsCOMPtr<nsIDOMXULMultiSelectControlElement> xulMultiSelect =
    do_QueryInterface(mSelectControl);
  nsCOMPtr<nsIAccessible> selectedAccessible;
  if (xulMultiSelect) {
    PRInt32 length = 0;
    xulMultiSelect->GetSelectedCount(&length);
    for (PRInt32 index = 0; index < length; index++) {
      nsCOMPtr<nsIDOMXULSelectControlItemElement> selectedItem;
      xulMultiSelect->GetSelectedItem(index, getter_AddRefs(selectedItem));
      nsCOMPtr<nsIDOMNode> selectedNode(do_QueryInterface(selectedItem));
      accService->GetAccessibleInWeakShell(selectedNode, mWeakShell,
                                           getter_AddRefs(selectedAccessible));
      if (selectedAccessible)
        selectedAccessibles->AppendElement(selectedAccessible, PR_FALSE);
    }
  }
  else {  // Single select?
    nsCOMPtr<nsIDOMXULSelectControlItemElement> selectedItem;
    mSelectControl->GetSelectedItem(getter_AddRefs(selectedItem));
    nsCOMPtr<nsIDOMNode> selectedNode(do_QueryInterface(selectedItem));
    if(selectedNode) {
      accService->GetAccessibleInWeakShell(selectedNode, mWeakShell,
                                           getter_AddRefs(selectedAccessible));
      if (selectedAccessible)
        selectedAccessibles->AppendElement(selectedAccessible, PR_FALSE);
    }
  }

  PRUint32 uLength = 0;
  selectedAccessibles->GetLength(&uLength);
  if (uLength != 0) { // length of nsIArray containing selected options
    NS_ADDREF(*aChildren = selectedAccessibles);
  }

  return NS_OK;
}

// return the nth selected child's nsIAccessible object
NS_IMETHODIMP nsXULSelectableAccessible::RefSelection(PRInt32 aIndex, nsIAccessible **aAccessible)
{
  *aAccessible = nsnull;
  if (!mSelectControl) {
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsIDOMXULSelectControlItemElement> selectedItem;
  nsCOMPtr<nsIDOMXULMultiSelectControlElement> xulMultiSelect =
    do_QueryInterface(mSelectControl);
  if (xulMultiSelect)
    xulMultiSelect->GetSelectedItem(aIndex, getter_AddRefs(selectedItem));

  if (aIndex == 0)
    mSelectControl->GetSelectedItem(getter_AddRefs(selectedItem));

  if (selectedItem) {
    nsCOMPtr<nsIAccessibilityService> accService = GetAccService();
    if (accService) {
      accService->GetAccessibleInWeakShell(selectedItem, mWeakShell, aAccessible);
      if (*aAccessible) {
        NS_ADDREF(*aAccessible);
        return NS_OK;
      }
    }
  }

  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsXULSelectableAccessible::GetSelectionCount(PRInt32 *aSelectionCount)
{
  *aSelectionCount = 0;
  if (!mSelectControl) {
    return NS_ERROR_FAILURE;
  }

  // For XUL multi-select control
  nsCOMPtr<nsIDOMXULMultiSelectControlElement> xulMultiSelect =
    do_QueryInterface(mSelectControl);
  if (xulMultiSelect)
    return xulMultiSelect->GetSelectedCount(aSelectionCount);

  // For XUL single-select control/menulist
  PRInt32 index;
  mSelectControl->GetSelectedIndex(&index);
  if (index >= 0)
    *aSelectionCount = 1;
  return NS_OK;
}

NS_IMETHODIMP nsXULSelectableAccessible::AddChildToSelection(PRInt32 aIndex)
{
  PRBool isSelected;
  return ChangeSelection(aIndex, eSelection_Add, &isSelected);
}

NS_IMETHODIMP nsXULSelectableAccessible::RemoveChildFromSelection(PRInt32 aIndex)
{
  PRBool isSelected;
  return ChangeSelection(aIndex, eSelection_Remove, &isSelected);
}

NS_IMETHODIMP nsXULSelectableAccessible::IsChildSelected(PRInt32 aIndex, PRBool *aIsSelected)
{
  *aIsSelected = PR_FALSE;
  return ChangeSelection(aIndex, eSelection_GetState, aIsSelected);
}

NS_IMETHODIMP nsXULSelectableAccessible::ClearSelection()
{
  if (!mSelectControl) {
    return NS_ERROR_FAILURE;
  }
  nsCOMPtr<nsIDOMXULMultiSelectControlElement> xulMultiSelect =
    do_QueryInterface(mSelectControl);
  return xulMultiSelect ? xulMultiSelect->ClearSelection() : mSelectControl->SetSelectedIndex(-1);
}

NS_IMETHODIMP nsXULSelectableAccessible::SelectAllSelection(PRBool *aSucceeded)
{
  *aSucceeded = PR_TRUE;

  nsCOMPtr<nsIDOMXULMultiSelectControlElement> xulMultiSelect =
    do_QueryInterface(mSelectControl);
  if (xulMultiSelect)
    return xulMultiSelect->SelectAll();

  // otherwise, don't support this method
  *aSucceeded = PR_FALSE;
  return NS_ERROR_NOT_IMPLEMENTED;
}


// ------------------------ Menu Item -----------------------------

nsXULMenuitemAccessible::nsXULMenuitemAccessible(nsIDOMNode* aDOMNode, nsIWeakReference* aShell): 
nsAccessibleWrap(aDOMNode, aShell)
{ 
}

NS_IMETHODIMP nsXULMenuitemAccessible::Init()
{
  nsresult rv = nsAccessibleWrap::Init();
  nsXULMenupopupAccessible::GenerateMenu(mDOMNode);
  return rv;
}

NS_IMETHODIMP nsXULMenuitemAccessible::GetState(PRUint32 *_retval)
{
  nsAccessible::GetState(_retval);

  // Focused?
  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(mDOMNode));
  if (!element)
    return NS_ERROR_FAILURE;
  PRBool isFocused = PR_FALSE;
  element->HasAttribute(NS_LITERAL_STRING("_moz-menuactive"), &isFocused); 
  if (isFocused)
    *_retval |= STATE_FOCUSED;

  // Has Popup?
  nsAutoString tagName;
  element->GetLocalName(tagName);
  if (tagName.EqualsLiteral("menu")) {
    *_retval |= STATE_HASPOPUP;
    PRBool isOpen;
    element->HasAttribute(NS_LITERAL_STRING("open"), &isOpen);
    *_retval |= isOpen ? STATE_EXPANDED : STATE_COLLAPSED;
  }

  nsAutoString menuItemType;
  element->GetAttribute(NS_LITERAL_STRING("type"), menuItemType); 

  if (!menuItemType.IsEmpty()) {
    // Checkable?
    if (menuItemType.EqualsIgnoreCase("radio") ||
        menuItemType.EqualsIgnoreCase("checkbox"))
      *_retval |= STATE_CHECKABLE;

    // Checked?
    nsAutoString checkValue;
    element->GetAttribute(NS_LITERAL_STRING("checked"), checkValue);
    if (checkValue.EqualsLiteral("true")) {
      *_retval |= STATE_CHECKED;
    }
  }

  // Offscreen?
  // If parent or grandparent menuitem is offscreen, then we're offscreen too
  // We get it by replacing the current offscreen bit with the parent's
  nsCOMPtr<nsIAccessible> parentAccessible(GetParent());
  if (parentAccessible) {
    *_retval &= ~STATE_OFFSCREEN;  // clear the old OFFSCREEN bit
    *_retval |= (State(parentAccessible) & STATE_OFFSCREEN);  // or it with the parent's offscreen bit
  }

  return NS_OK;
}

NS_IMETHODIMP nsXULMenuitemAccessible::GetName(nsAString& _retval)
{
  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(mDOMNode));
  if (!element) {
    return NS_ERROR_FAILURE;
  }
  element->GetAttribute(NS_LITERAL_STRING("label"), _retval); 

  return NS_OK;
}

NS_IMETHODIMP nsXULMenuitemAccessible::GetDescription(nsAString& aDescription)
{
  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(mDOMNode));
  if (!element) {
    return NS_ERROR_FAILURE;
  }
  element->GetAttribute(NS_LITERAL_STRING("description"), aDescription);

  return NS_OK;
}

//return menu accesskey: N or Alt+F
NS_IMETHODIMP nsXULMenuitemAccessible::GetKeyboardShortcut(nsAString& _retval)
{
  static PRInt32 gMenuAccesskeyModifier = -1;  // magic value of -1 indicates unitialized state

  nsCOMPtr<nsIDOMElement> elt(do_QueryInterface(mDOMNode));
  if (elt) {
    nsAutoString accesskey;
    elt->GetAttribute(NS_LITERAL_STRING("accesskey"), accesskey);
    if (accesskey.IsEmpty())
      return NS_OK;

    nsCOMPtr<nsIAccessible> parentAccessible(GetParent());
    if (parentAccessible) {
      PRUint32 role;
      parentAccessible->GetRole(&role);
      if (role == ROLE_MENUBAR) {
        // If top level menu item, add Alt+ or whatever modifier text to string
        // No need to cache pref service, this happens rarely
        if (gMenuAccesskeyModifier == -1) {
          // Need to initialize cached global accesskey pref
          gMenuAccesskeyModifier = 0;
          nsCOMPtr<nsIPrefBranch> prefBranch(do_GetService(NS_PREFSERVICE_CONTRACTID));
          if (prefBranch)
            prefBranch->GetIntPref("ui.key.menuAccessKey", &gMenuAccesskeyModifier);
        }
        nsAutoString propertyKey;
        switch (gMenuAccesskeyModifier) {
          case nsIDOMKeyEvent::DOM_VK_CONTROL: propertyKey.AssignLiteral("VK_CONTROL"); break;
          case nsIDOMKeyEvent::DOM_VK_ALT: propertyKey.AssignLiteral("VK_ALT"); break;
          case nsIDOMKeyEvent::DOM_VK_META: propertyKey.AssignLiteral("VK_META"); break;
        }
        if (!propertyKey.IsEmpty())
          nsAccessible::GetFullKeyName(propertyKey, accesskey, _retval);
      }
    }
    if (_retval.IsEmpty())
      _retval = accesskey;
    return NS_OK;
  }
  return NS_ERROR_FAILURE;
}

//return menu shortcut: Ctrl+F or Ctrl+Shift+L
NS_IMETHODIMP nsXULMenuitemAccessible::GetKeyBinding(nsAString& _retval)
{
  nsCOMPtr<nsIDOMElement> elt(do_QueryInterface(mDOMNode));
  if (elt) {
    nsAutoString accelText;
    elt->GetAttribute(NS_LITERAL_STRING("acceltext"), accelText);
    if (accelText.IsEmpty())
      return NS_OK;

    _retval = accelText;
    return NS_OK;
  }
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsXULMenuitemAccessible::GetRole(PRUint32 *aRole)
{
  *aRole = ROLE_MENUITEM;
  if (mParent && Role(mParent) == ROLE_COMBOBOX_LIST) {
    *aRole = ROLE_COMBOBOX_LISTITEM;
    return NS_OK;
  }
  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(mDOMNode));
  if (!element)
    return NS_ERROR_FAILURE;
  nsAutoString menuItemType;
  element->GetAttribute(NS_LITERAL_STRING("type"), menuItemType);
  if (menuItemType.EqualsIgnoreCase("radio"))
    *aRole = ROLE_RADIO_MENU_ITEM;
  else if (menuItemType.EqualsIgnoreCase("checkbox"))
    *aRole = ROLE_CHECK_MENU_ITEM;
  else { // Fortunately, radio/checkbox menuitems don't typically have children
    PRInt32 childCount;
    GetChildCount(&childCount);
    if (childCount > 0) {
      *aRole = ROLE_PARENT_MENUITEM;
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsXULMenuitemAccessible::GetAllowsAnonChildAccessibles(PRBool *aAllowsAnonChildren)
{
  // That indicates we don't walk anonymous children for menuitems
  *aAllowsAnonChildren = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP nsXULMenuitemAccessible::DoAction(PRUint8 index)
{
  if (index == eAction_Click) {   // default action
    DoCommand();
    return NS_OK;
  }

  return NS_ERROR_INVALID_ARG;
}

/** select us! close combo box if necessary*/
NS_IMETHODIMP nsXULMenuitemAccessible::GetActionName(PRUint8 aIndex, nsAString& aName)
{
  if (aIndex == eAction_Click) {
    aName.AssignLiteral("click"); 
    return NS_OK;
  }
  return NS_ERROR_INVALID_ARG;
}

NS_IMETHODIMP nsXULMenuitemAccessible::GetNumActions(PRUint8 *_retval)
{
  *_retval = 1;
  return NS_OK;
}


// ------------------------ Menu Separator ----------------------------

nsXULMenuSeparatorAccessible::nsXULMenuSeparatorAccessible(nsIDOMNode* aDOMNode, nsIWeakReference* aShell): 
nsXULMenuitemAccessible(aDOMNode, aShell)
{ 
}

NS_IMETHODIMP nsXULMenuSeparatorAccessible::GetState(PRUint32 *_retval)
{
  // Isn't focusable, but can be offscreen
  nsXULMenuitemAccessible::GetState(_retval);
  *_retval &= STATE_OFFSCREEN;

  return NS_OK;
}

NS_IMETHODIMP nsXULMenuSeparatorAccessible::GetName(nsAString& _retval)
{
  _retval.Truncate();
  return NS_OK;
}

NS_IMETHODIMP nsXULMenuSeparatorAccessible::GetRole(PRUint32 *_retval)
{
  *_retval = ROLE_SEPARATOR;
  return NS_OK;
}

NS_IMETHODIMP nsXULMenuSeparatorAccessible::DoAction(PRUint8 index)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP nsXULMenuSeparatorAccessible::GetActionName(PRUint8 aIndex, nsAString& aName)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP nsXULMenuSeparatorAccessible::GetNumActions(PRUint8 *_retval)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}
// ------------------------ Menu Popup -----------------------------

nsXULMenupopupAccessible::nsXULMenupopupAccessible(nsIDOMNode* aDOMNode, nsIWeakReference* aShell): 
  nsXULSelectableAccessible(aDOMNode, aShell)
{ 
  // May be the anonymous <menupopup> inside <menulist> (a combobox)
  nsCOMPtr<nsIDOMNode> parentNode;
  aDOMNode->GetParentNode(getter_AddRefs(parentNode));
  mSelectControl = do_QueryInterface(parentNode);
}

NS_IMETHODIMP nsXULMenupopupAccessible::GetState(PRUint32 *_retval)
{
  // We are onscreen if our parent is active
  *_retval = 0;
  PRBool isActive = PR_FALSE;

  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(mDOMNode));
  element->HasAttribute(NS_LITERAL_STRING("menuactive"), &isActive);
  if (!isActive) {
    nsCOMPtr<nsIAccessible> parent(GetParent());
    nsCOMPtr<nsIDOMNode> parentNode;
    nsCOMPtr<nsIAccessNode> accessNode(do_QueryInterface(parent));
    if (accessNode) 
      accessNode->GetDOMNode(getter_AddRefs(parentNode));
    element = do_QueryInterface(parentNode);
    if (element)
      element->HasAttribute(NS_LITERAL_STRING("open"), &isActive);
  }

  if (!isActive)
    *_retval |= STATE_OFFSCREEN;

  return NS_OK;
}

already_AddRefed<nsIDOMNode>
nsXULMenupopupAccessible::FindInNodeList(nsIDOMNodeList *aNodeList, 
                                         nsIAtom *aAtom, PRUint32 aNameSpaceID)
{
  PRUint32 numChildren;
  if (!aNodeList || NS_FAILED(aNodeList->GetLength(&numChildren))) {
    return nsnull;
  }
  nsCOMPtr<nsIDOMNode> childNode;
  for (PRUint32 childIndex = 0; childIndex < numChildren; childIndex++) {
    aNodeList->Item(childIndex, getter_AddRefs(childNode));
    nsCOMPtr<nsIContent> content = do_QueryInterface(childNode);
    if (content && content->NodeInfo()->Equals(aAtom, kNameSpaceID_XUL)) {
      nsIDOMNode *matchNode = childNode;
      NS_ADDREF(matchNode);
      return matchNode;
    }
  }
  return nsnull;
}

void nsXULMenupopupAccessible::GenerateMenu(nsIDOMNode *aNode)
{
  // Set menugenerated="true" on the menupopup node to generate the
  // sub-menu items if they have not been generated
  nsCOMPtr<nsIDOMNodeList> nodeList;
  aNode->GetChildNodes(getter_AddRefs(nodeList));

  nsCOMPtr<nsIDOMNode> menuPopup = FindInNodeList(nodeList, nsAccessibilityAtoms::menupopup,
                                                  kNameSpaceID_XUL);
  nsCOMPtr<nsIDOMElement> popupElement(do_QueryInterface(menuPopup));
  if (popupElement) {
    nsAutoString attr;
    popupElement->GetAttribute(NS_LITERAL_STRING("menugenerated"), attr);
    if (!attr.EqualsLiteral("true")) {
      popupElement->SetAttribute(NS_LITERAL_STRING("menugenerated"), NS_LITERAL_STRING("true"));
    }
  }
}

NS_IMETHODIMP nsXULMenupopupAccessible::GetName(nsAString& _retval)
{
  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(mDOMNode));
  NS_ASSERTION(element, "No element for popup node!");

  while (element) {
    element->GetAttribute(NS_LITERAL_STRING("label"), _retval);
    if (!_retval.IsEmpty())
      return NS_OK;
    nsCOMPtr<nsIDOMNode> parentNode, node(do_QueryInterface(element));
    if (!node)
      return NS_ERROR_FAILURE;
    node->GetParentNode(getter_AddRefs(parentNode));
    element = do_QueryInterface(parentNode);
  }

  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsXULMenupopupAccessible::GetRole(PRUint32 *aRole)
{
  if (mParent && Role(mParent) == ROLE_COMBOBOX) {
    *aRole = ROLE_COMBOBOX_LIST;
  }
  else {
    *aRole = ROLE_MENUPOPUP;
  }
  return NS_OK;
}

// ------------------------ Menu Bar -----------------------------

nsXULMenubarAccessible::nsXULMenubarAccessible(nsIDOMNode* aDOMNode, nsIWeakReference* aShell): 
  nsAccessibleWrap(aDOMNode, aShell)
{ 
}

NS_IMETHODIMP nsXULMenubarAccessible::GetState(PRUint32 *_retval)
{
  nsresult rv = nsAccessible::GetState(_retval);
  *_retval &= ~STATE_FOCUSABLE; // Menu bar iteself is not actually focusable
  return rv;
}


NS_IMETHODIMP nsXULMenubarAccessible::GetName(nsAString& _retval)
{
  _retval.AssignLiteral("Application");

  return NS_OK;
}

NS_IMETHODIMP nsXULMenubarAccessible::GetRole(PRUint32 *_retval)
{
  *_retval = ROLE_MENUBAR;
  return NS_OK;
}

