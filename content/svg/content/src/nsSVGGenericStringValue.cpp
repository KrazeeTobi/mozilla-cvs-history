/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ----- BEGIN LICENSE BLOCK -----
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Mozilla SVG project.
 *
 * The Initial Developer of the Original Code is 
 * Crocodile Clips Ltd..
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *    Alex Fritze <alex.fritze@crocodile-clips.com> (original author)
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
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ----- END LICENSE BLOCK ----- */

#include "nsSVGValue.h"

////////////////////////////////////////////////////////////////////////
// nsSVGGenericStringValue implementation

class nsSVGGenericStringValue : public nsSVGValue
{
protected:
  friend nsresult
  NS_CreateSVGGenericStringValue(const nsAReadableString& aValue, nsISVGValue** aResult);
  
  nsSVGGenericStringValue(const nsAReadableString& aValue);
  virtual ~nsSVGGenericStringValue();
  
public:
  NS_DECL_ISUPPORTS

  // nsISVGValue interface: 
  NS_IMETHOD SetValueString(const nsAReadableString& aValue);
  NS_IMETHOD GetValueString(nsAWritableString& aValue);

protected:
  nsString mValue;
};


nsresult
NS_CreateSVGGenericStringValue(const nsAReadableString& aValue,
                               nsISVGValue** aResult)
{
  NS_PRECONDITION(aResult != nsnull, "null ptr");
  if (! aResult) return NS_ERROR_NULL_POINTER;
  
  *aResult = (nsISVGValue*) new nsSVGGenericStringValue(aValue);
  if(!*aResult) return NS_ERROR_OUT_OF_MEMORY;
  
  NS_ADDREF(*aResult);
  return NS_OK;
}

nsSVGGenericStringValue::nsSVGGenericStringValue(const nsAReadableString& aValue)
{
  NS_INIT_ISUPPORTS();
  mValue = aValue;
}

nsSVGGenericStringValue::~nsSVGGenericStringValue()
{
}


// nsISupports methods:

NS_IMPL_ISUPPORTS1(nsSVGGenericStringValue, nsISVGValue);


// nsISVGValue methods:

NS_IMETHODIMP
nsSVGGenericStringValue::SetValueString(const nsAReadableString& aValue)
{
  WillModify();
  mValue = aValue;
  DidModify();
  return NS_OK;
}

NS_IMETHODIMP
nsSVGGenericStringValue::GetValueString(nsAWritableString& aValue)
{
  aValue = mValue;
  return NS_OK;
}


