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

#ifndef nsIScriptGlobalObject_h__
#define nsIScriptGlobalObject_h__

#include "nsISupports.h"
#include "nsIScriptContext.h"
#include "nsGUIEvent.h"
#include "jsapi.h"

class nsIScriptContext;
class nsIDOMDocument;
class nsIDOMEvent;
class nsIPresContext;
class nsIDocShell;
class nsIDOMWindowInternal;
class nsIScriptGlobalObjectOwner;

#define NS_ISCRIPTGLOBALOBJECT_IID \
{ 0x2b16fc80, 0xfa41, 0x11d1,  \
{ 0x9b, 0xc3, 0x00, 0x60, 0x08, 0x8c, 0xa6, 0xb3} }

/**
 * The JavaScript specific global object. This often used to store
 * per-window global state.
 */

class nsIScriptGlobalObject : public nsISupports {
public:
  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISCRIPTGLOBALOBJECT_IID)

  NS_IMETHOD       SetContext(nsIScriptContext *aContext)=0;
  NS_IMETHOD       GetContext(nsIScriptContext **aContext)=0;
  NS_IMETHOD       SetNewDocument(nsIDOMDocument *aDocument)=0;
  NS_IMETHOD       SetDocShell(nsIDocShell *aDocShell)=0;
  NS_IMETHOD       GetDocShell(nsIDocShell **aDocShell)=0;
  NS_IMETHOD       SetOpenerWindow(nsIDOMWindowInternal *aOpener)=0;

    /**
   * Let the script global object know who its owner is.
   * The script global object should not addref the owner. It
   * will be told when the owner goes away.
   * @return NS_OK if the method is successful
   */
  NS_IMETHOD SetGlobalObjectOwner(nsIScriptGlobalObjectOwner* aOwner) = 0;

  /**
   * Get the owner of the script global object. The method
   * addrefs the returned reference according to regular
   * XPCOM rules, even though the internal reference itself
   * is a "weak" reference.
   */
  NS_IMETHOD GetGlobalObjectOwner(nsIScriptGlobalObjectOwner** aOwner) = 0;

  NS_IMETHOD HandleDOMEvent(nsIPresContext* aPresContext, 
                            nsEvent* aEvent, 
                            nsIDOMEvent** aDOMEvent,
                            PRUint32 aFlags,
                            nsEventStatus* aEventStatus)=0;

  NS_IMETHOD_(JSObject *) GetGlobalJSObject() = 0;

  /**
   * Called when the global JSObject is finalized
   */

  NS_IMETHOD OnFinalize(JSObject *aJSObject) = 0;
};

#endif
