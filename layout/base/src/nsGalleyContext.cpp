/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
#include "nsPresContext.h"
#include "nsGfxCIID.h"
#include "nsLayoutAtoms.h"


class GalleyContext : public nsPresContext {
public:
  GalleyContext();
  ~GalleyContext();

  NS_IMETHOD GetMedium(nsIAtom** aMedium);
  NS_IMETHOD IsPaginated(PRBool* aResult);
  NS_IMETHOD GetPageDim(nsRect* aActualRect, nsRect* aAdjRect);
  NS_IMETHOD SetPageDim(nsRect* aRect);
};

GalleyContext::GalleyContext()
{
}

GalleyContext::~GalleyContext()
{
}

NS_IMETHODIMP
GalleyContext::GetMedium(nsIAtom** aResult)
{
  NS_PRECONDITION(nsnull != aResult, "null ptr");
  if (nsnull == aResult) {
    return NS_ERROR_NULL_POINTER;
  }
  *aResult = nsLayoutAtoms::screen;
  NS_ADDREF(*aResult);
  return NS_OK;
}

NS_IMETHODIMP
GalleyContext::IsPaginated(PRBool* aResult)
{
  NS_PRECONDITION(nsnull != aResult, "null ptr");
  if (nsnull == aResult) {
    return NS_ERROR_NULL_POINTER;
  }
  *aResult = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
GalleyContext::GetPageDim(nsRect* aActualRect, nsRect* aAdjRect)
{
  NS_ENSURE_ARG_POINTER(aActualRect);
  NS_ENSURE_ARG_POINTER(aAdjRect);
  aActualRect->SetRect(0, 0, 0, 0);
  aAdjRect->SetRect(0, 0, 0, 0);
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
GalleyContext::SetPageDim(nsRect* aPageDim)
{
  NS_ENSURE_ARG_POINTER(aPageDim);
  return NS_ERROR_FAILURE;
}

NS_LAYOUT nsresult
NS_NewGalleyContext(nsIPresContext** aInstancePtrResult)
{
  if (aInstancePtrResult == nsnull) {
    return NS_ERROR_NULL_POINTER;
  }

  GalleyContext *it = new GalleyContext();

  if (it == nsnull) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  return it->QueryInterface(NS_GET_IID(nsIPresContext), (void **) aInstancePtrResult);
}
