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
 * Author: John Gaunt (jgaunt@netscape.com)
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

// NOTE: alphabetically ordered
#include "nsIDocument.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMXULButtonElement.h"
#include "nsIDOMXULCheckboxElement.h"
#include "nsIDOMXULDescriptionElement.h"
#include "nsIDOMXULDocument.h"
#include "nsIDOMXULLabelElement.h"
#include "nsIDOMXULSelectCntrlEl.h"
#include "nsIDOMXULSelectCntrlItemEl.h"
#include "nsReadableUtils.h"
#include "nsString.h"
#include "nsXULFormControlAccessible.h"

/**
  * XUL Button: can contain arbitrary HTML content
  */

/**
  * Default Constructor
  */
nsXULButtonAccessible::nsXULButtonAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsFormControlAccessible(aNode, aShell)
{ 
}

/**
  * Only one actions available
  */
NS_IMETHODIMP nsXULButtonAccessible::GetAccNumActions(PRUint8 *_retval)
{
  *_retval = eSingle_Action;
  return NS_OK;;
}

/**
  * Return the name of our only action
  */
NS_IMETHODIMP nsXULButtonAccessible::GetAccActionName(PRUint8 index, nsAWritableString& _retval)
{
  if (index == eAction_Click) {
    _retval = NS_LITERAL_STRING("press");
    return NS_OK;
  }
  return NS_ERROR_INVALID_ARG;
}

/**
  * Tell the button to do it's action
  */
NS_IMETHODIMP nsXULButtonAccessible::AccDoAction(PRUint8 index)
{
  if (index == 0) {
    nsCOMPtr<nsIDOMXULButtonElement> buttonElement(do_QueryInterface(mDOMNode));
    if ( buttonElement )
    {
      buttonElement->DoCommand();
      return NS_OK;
    }
    return NS_ERROR_FAILURE;
  }
  return NS_ERROR_INVALID_ARG;
}

/**
  * We are a pushbutton
  */
NS_IMETHODIMP nsXULButtonAccessible::GetAccRole(PRUint32 *_retval)
{
  *_retval = ROLE_PUSHBUTTON;
  return NS_OK;
}

/**
  * Possible states: focused, focusable, unavailable(disabled)
  */
NS_IMETHODIMP nsXULButtonAccessible::GetAccState(PRUint32 *_retval)
{
  // get focus and disable status from base class
  nsFormControlAccessible::GetAccState(_retval);
  *_retval |= STATE_FOCUSABLE;

  // Buttons can be checked -- they simply appear pressed in rather than checked
  nsCOMPtr<nsIDOMXULButtonElement> xulButtonElement(do_QueryInterface(mDOMNode));
  if (xulButtonElement) {
    PRBool checked = PR_FALSE;
    PRInt32 checkState = 0;
    xulButtonElement->GetChecked(&checked);
    if (checked) {
      *_retval |= STATE_PRESSED;
      xulButtonElement->GetCheckState(&checkState);
      if (checkState == nsIDOMXULButtonElement::CHECKSTATE_MIXED)  
        *_retval |= STATE_MIXED;
    }
  }

  return NS_OK;
}

/**
  * XUL checkbox
  */

/**
  * Default Constructor
  */
nsXULCheckboxAccessible::nsXULCheckboxAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsFormControlAccessible(aNode, aShell)
{ 
}

/**
  * We are a CheckButton
  */
NS_IMETHODIMP nsXULCheckboxAccessible::GetAccRole(PRUint32 *_retval)
{
  *_retval = ROLE_CHECKBUTTON;
  return NS_OK;
}

/**
  * Only one action available
  */
NS_IMETHODIMP nsXULCheckboxAccessible::GetAccNumActions(PRUint8 *_retval)
{
  *_retval = eSingle_Action;
  return NS_OK;
}

/**
  * Return the name of our only action
  */
NS_IMETHODIMP nsXULCheckboxAccessible::GetAccActionName(PRUint8 index, nsAWritableString& _retval)
{
  if (index == eAction_Click) {
    // check or uncheck
    PRUint32 state;
    GetAccState(&state);

    if (state & STATE_CHECKED)
      _retval = NS_LITERAL_STRING("uncheck");
    else
      _retval = NS_LITERAL_STRING("check");

    return NS_OK;
  }
  return NS_ERROR_INVALID_ARG;
}

/**
  * Tell the checkbox to do its only action -- check( or uncheck) itself
  */
NS_IMETHODIMP nsXULCheckboxAccessible::AccDoAction(PRUint8 index)
{
  if (index == eAction_Click) {
    PRBool checked = PR_FALSE;
    nsCOMPtr<nsIDOMXULCheckboxElement> xulCheckboxElement(do_QueryInterface(mDOMNode));
    if (xulCheckboxElement) {
      xulCheckboxElement->GetChecked(&checked);
      xulCheckboxElement->SetChecked(!checked);
      return NS_OK;
    }
    return NS_ERROR_FAILURE;
  }
  return NS_ERROR_INVALID_ARG;
}

/**
  * Possible states: focused, focusable, unavailable(disabled), checked
  */
NS_IMETHODIMP nsXULCheckboxAccessible::GetAccState(PRUint32 *_retval)
{
  // Get focus and disable status from base class
  nsFormControlAccessible::GetAccState(_retval);

  // Determine Checked state
  nsCOMPtr<nsIDOMXULCheckboxElement> xulCheckboxElement(do_QueryInterface(mDOMNode));
  if (xulCheckboxElement) {
    PRBool checked = PR_FALSE;
    xulCheckboxElement->GetChecked(&checked);
    if (checked) {
      *_retval |= STATE_CHECKED;
      PRInt32 checkState = 0;
      xulCheckboxElement->GetCheckState(&checkState);
      if (checkState == nsIDOMXULCheckboxElement::CHECKSTATE_MIXED)   
        *_retval |= STATE_MIXED;
    }
  }
  
  return NS_OK;
}

/**
  * XUL groupbox
  */

nsXULGroupboxAccessible::nsXULGroupboxAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsAccessible(aNode, aShell)
{ 
}

NS_IMETHODIMP nsXULGroupboxAccessible::GetAccRole(PRUint32 *_retval)
{
  *_retval = ROLE_GROUPING;
  return NS_OK;
}

NS_IMETHODIMP nsXULGroupboxAccessible::GetAccState(PRUint32 *_retval)
{
  // Groupbox doesn't support any states!
  *_retval = 0;

  return NS_OK;
}

NS_IMETHODIMP nsXULGroupboxAccessible::GetAccName(nsAWritableString& _retval)
{
  _retval.Assign(NS_LITERAL_STRING(""));  // Default name is blank 

  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(mDOMNode));
  if (element) {
    nsCOMPtr<nsIDOMNodeList> captions;
    element->GetElementsByTagName(NS_LITERAL_STRING("caption"), getter_AddRefs(captions));
    if (captions) {
      nsCOMPtr<nsIDOMNode> captionNode;
      captions->Item(0, getter_AddRefs(captionNode));
      if (captionNode) {
        element = do_QueryInterface(captionNode);
        NS_ASSERTION(element, "No nsIDOMElement for caption node!");
        element->GetAttribute(NS_LITERAL_STRING("label"), _retval) ;
      }
    }
  }
  return NS_OK;
}

/**
  * progressmeter
  */

nsXULProgressMeterAccessible::nsXULProgressMeterAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsAccessible(aNode, aShell)
{ 
}

NS_IMETHODIMP nsXULProgressMeterAccessible::GetAccRole(PRUint32 *_retval)
{
  *_retval = ROLE_PROGRESSBAR;
  return NS_OK;
}

/**
  * No states supported for progressmeter
  */
NS_IMETHODIMP nsXULProgressMeterAccessible::GetAccState(PRUint32 *_retval)
{
  *_retval =0;
  return NS_OK;
}

NS_IMETHODIMP nsXULProgressMeterAccessible::GetAccValue(nsAWritableString& _retval)
{
  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(mDOMNode));
  NS_ASSERTION(element, "No element for DOM node!");
  element->GetAttribute(NS_LITERAL_STRING("value"), _retval);
  _retval.Append(NS_LITERAL_STRING("%"));
  return NS_OK;
}
