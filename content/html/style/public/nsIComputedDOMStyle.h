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
 * Portions created by the Initial Developer are Copyright (C) 1999
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
#ifndef nsIComputedDOMStyle_h___
#define nsIComputedDOMStyle_h___

#include "nsIDOMCSSStyleDeclaration.h"

class nsIDOMElement;
class nsIPresShell;

#define NS_ICOMPUTEDDOMSTYLE_IID \
 { 0x5f0197a1, 0xa873, 0x44e5, \
    {0x96, 0x31, 0xac, 0xd6, 0xca, 0xb4, 0xf1, 0xe0 } }

class nsIComputedDOMStyle : public nsIDOMCSSStyleDeclaration
{
public:
  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICOMPUTEDDOMSTYLE_IID)

  NS_IMETHOD Init(nsIDOMElement *aElement, const nsAString& aPseudoElt,
                  nsIPresShell *aPresShell) = 0;
};

extern NS_EXPORT nsresult 
NS_NewComputedDOMStyle(nsIComputedDOMStyle** aComputedStyle);

#endif /* nsIComputedDOMStyle_h___ */
