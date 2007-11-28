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
 * The Original Code is Mozilla XForms support.
 *
 * The Initial Developer of the Original Code is
 * Olli Pettay.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Olli Pettay <Olli.Pettay@helsinki.fi> (original author)
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

#include "nsXFormsActionModuleBase.h"
#include "nsXFormsActionElement.h"

class nsXFormsRecalculateElement : public nsXFormsActionModuleBase
{
public:
  nsXFormsRecalculateElement();
protected:
  nsresult HandleSingleAction(nsIDOMEvent* aEvent,
                              nsIXFormsActionElement *aParentAction);
};

nsXFormsRecalculateElement::nsXFormsRecalculateElement() :
  nsXFormsActionModuleBase(PR_TRUE)
{
}

nsresult
nsXFormsRecalculateElement::HandleSingleAction(nsIDOMEvent* aEvent,
                                               nsIXFormsActionElement *aParentAction)
{
  nsCOMPtr<nsIModelElementPrivate> model = nsXFormsUtils::GetModel(mElement);
  NS_ENSURE_STATE(model);
  
  if (aParentAction) {
    aParentAction->SetRecalculate(model, PR_FALSE);
  }
  return model->Recalculate();
}

NS_HIDDEN_(nsresult)
NS_NewXFormsRecalculateElement(nsIXTFElement **aResult)
{
  *aResult = new nsXFormsRecalculateElement();
  if (!*aResult)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(*aResult);
  return NS_OK;
}

