/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */
/* AUTO-GENERATED. DO NOT EDIT!!! */

#ifndef nsIDOMHTMLLIElement_h__
#define nsIDOMHTMLLIElement_h__

#include "nsISupports.h"
#include "nsString.h"
#include "nsIScriptContext.h"
#include "nsIDOMHTMLElement.h"


#define NS_IDOMHTMLLIELEMENT_IID \
{ 0x6f765311,  0xee43, 0x11d1, \
 { 0x9b, 0xc3, 0x00, 0x60, 0x08, 0x8c, 0xa6, 0xb3 } } 

class nsIDOMHTMLLIElement : public nsIDOMHTMLElement {
public:

  NS_IMETHOD    GetType(nsString& aType)=0;
  NS_IMETHOD    SetType(const nsString& aType)=0;

  NS_IMETHOD    GetValue(PRInt32* aValue)=0;
  NS_IMETHOD    SetValue(PRInt32 aValue)=0;
};


#define NS_DECL_IDOMHTMLLIELEMENT   \
  NS_IMETHOD    GetType(nsString& aType);  \
  NS_IMETHOD    SetType(const nsString& aType);  \
  NS_IMETHOD    GetValue(PRInt32* aValue);  \
  NS_IMETHOD    SetValue(PRInt32 aValue);  \



#define NS_FORWARD_IDOMHTMLLIELEMENT(_to)  \
  NS_IMETHOD    GetType(nsString& aType) { return _to##GetType(aType); } \
  NS_IMETHOD    SetType(const nsString& aType) { return _to##SetType(aType); } \
  NS_IMETHOD    GetValue(PRInt32* aValue) { return _to##GetValue(aValue); } \
  NS_IMETHOD    SetValue(PRInt32 aValue) { return _to##SetValue(aValue); } \


extern nsresult NS_InitHTMLLIElementClass(nsIScriptContext *aContext, void **aPrototype);

extern "C" NS_DOM nsresult NS_NewScriptHTMLLIElement(nsIScriptContext *aContext, nsISupports *aSupports, nsISupports *aParent, void **aReturn);

#endif // nsIDOMHTMLLIElement_h__
