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
#ifndef nsISelectElement_h___
#define nsISelectElement_h___

#include "nsISupports.h"

// IID for the nsISelect interface
#define NS_ISELECTELEMENT_IID    \
{ 0xa6cf90f6, 0x15b3, 0x11d2,    \
  { 0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32 } }

class nsIDOMHTMLOptionElement;
class nsIPresContext;
class nsIPresState;

/** 
 * This interface is used to notify a SELECT when OPTION
 * elements are added and removed from its subtree.
 * Note that the nsIDOMHTMLSelectElement and nsIContent 
 * interfaces are the ones to use to access and enumerate
 * OPTIONs within a SELECT element.
 */
class nsISelectElement : public nsISupports {
public:

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISELECTELEMENT_IID)

  /**
   * To be called when stuff is added under a child of
   * the select--but *before* they are actually added.
   */
  NS_IMETHOD WillAddOptions(nsIContent* aOptions,
                            nsIContent* aParent,
                            PRInt32 aContentIndex) = 0;

  /**
   * To be called when stuff is removed under a child of
   * the select--but *before* they are actually removed.
   */
  NS_IMETHOD WillRemoveOptions(nsIContent* aParent,
                               PRInt32 aContentIndex) = 0;

  /**
   * An OPTION element has been added to the SELECT's
   * subtree.
   */
  NS_IMETHOD AddOption(nsIContent* aContent) = 0;

  /**
   * An OPTION element has been deleted from the SELECT's
   * subtree.
   */
  NS_IMETHOD RemoveOption(nsIContent* aContent) = 0;

  /**
  * Indicates that we're done adding child content
  * to the select during document loading.
  */
  NS_IMETHOD DoneAddingContent(PRBool aIsDone) = 0;

  /**
  * Returns whether we're done adding child content
  * to the select during document loading.
  */
  NS_IMETHOD IsDoneAddingContent(PRBool * aIsDone) = 0;

  /**
  * Returns whether we're the option is selected
  */
  NS_IMETHOD IsOptionSelected(nsIDOMHTMLOptionElement* anOption,
                              PRBool* aIsSelected) = 0;

  /**
  * Sets an option selected or delselected
  */
  NS_IMETHOD SetOptionSelected(nsIDOMHTMLOptionElement* anOption,
                               PRBool aIsSelected) = 0;

  /**
   * Checks whether an option is disabled (even if it's part of an optgroup)
   */
  NS_IMETHOD IsOptionDisabled(PRInt32 aIndex, PRBool* aIsDisabled) = 0;

  /**
  * Sets multiple options (or just sets startIndex if select is single)
  */
  NS_IMETHOD SetOptionsSelectedByIndex(PRInt32 aStartIndex,
                                       PRInt32 aEndIndex,
                                       PRBool aIsSelected,
                                       PRBool aClearAll,
                                       PRBool aSetDisabled,
                                       PRBool* aChangedSomething) = 0;

  /**
  * Called when an option is disabled
  */
  NS_IMETHOD OnOptionDisabled(nsIDOMHTMLOptionElement* anOption) = 0;

  /**
   * Called to save/restore to/from pres. state
   */
  NS_IMETHOD SaveState(nsIPresContext* aPresContext,
                       nsIPresState** aState) = 0;
  NS_IMETHOD RestoreState(nsIPresContext* aPresContext,
                          nsIPresState* aState) = 0;
};

#endif // nsISelectElement_h___

