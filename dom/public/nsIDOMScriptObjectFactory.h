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

#ifndef nsIDOMScriptObjectFactory_h__
#define nsIDOMScriptObjectFactory_h__

#include "nsISupports.h"
#include "nsString.h"

#define NS_IDOM_SCRIPT_OBJECT_FACTORY_IID   \
{ 0xa6cf9064, 0x15b3, 0x11d2,               \
  { 0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32 } }

class nsIScriptContext;

class nsIDOMScriptObjectFactory : public nsISupports {
public:  
  static const nsIID& GetIID() { static nsIID iid = NS_IDOM_SCRIPT_OBJECT_FACTORY_IID; return iid; }

  NS_IMETHOD    NewScriptAttr(nsIScriptContext *aContext, 
                              nsISupports *aAttribute, 
                              nsISupports *aParent, 
                              void** aReturn)=0;

  NS_IMETHOD    NewScriptComment(nsIScriptContext *aContext, 
                                 nsISupports *aComment,
                                 nsISupports *aParent, 
                                 void** aReturn)=0;

  NS_IMETHOD    NewScriptDocument(const nsString &aDocType,
                                  nsIScriptContext *aContext, 
                                  nsISupports *aDocFrag, 
                                  nsISupports *aParent, 
                                  void** aReturn)=0;

  NS_IMETHOD    NewScriptDocumentFragment(nsIScriptContext *aContext, 
                                          nsISupports *aDocFrag, 
                                          nsISupports *aParent, 
                                          void** aReturn)=0;

  NS_IMETHOD    NewScriptDocumentType(nsIScriptContext *aContext, 
                                      nsISupports *aDocType, 
                                      nsISupports *aParent, 
                                      void** aReturn)=0;
  
  NS_IMETHOD    NewScriptDOMImplementation(nsIScriptContext *aContext, 
                                           nsISupports *aDOM, 
                                           nsISupports *aParent, 
                                           void** aReturn)=0;

  NS_IMETHOD    NewScriptCharacterData(PRUint16 aNodeType,
                                       nsIScriptContext *aContext, 
                                       nsISupports *aData, 
                                       nsISupports *aParent, 
                                       void** aReturn)=0;
  
  NS_IMETHOD    NewScriptElement(const nsString &aTagName, 
                                 nsIScriptContext *aContext, 
                                 nsISupports *aElement, 
                                 nsISupports *aParent, 
                                 void **aReturn)=0;

  NS_IMETHOD    NewScriptXMLElement(const nsString &aTagName, 
                                    nsIScriptContext *aContext, 
                                    nsISupports *aElement, 
                                    nsISupports *aParent, 
                                    void **aReturn)=0;

  NS_IMETHOD    NewScriptHTMLCollection(nsIScriptContext *aContext, 
                                        nsISupports *aCollection, 
                                        nsISupports *aParent, 
                                        void** aReturn)=0;
  
  NS_IMETHOD    NewScriptNamedNodeMap(nsIScriptContext *aContext, 
                                      nsISupports *aNodeMap, 
                                      nsISupports *aParent, 
                                      void** aReturn)=0;

  NS_IMETHOD    NewScriptNodeList(nsIScriptContext *aContext, 
                                  nsISupports *aNodeList, 
                                  nsISupports *aParent, 
                                  void** aReturn)=0;

  NS_IMETHOD    NewScriptProcessingInstruction(nsIScriptContext *aContext, 
                                               nsISupports *aPI, 
                                               nsISupports *aParent, 
                                               void** aReturn)=0;

  NS_IMETHOD    NewScriptEntity(nsIScriptContext *aContext, 
                                nsISupports *aPI, 
                                nsISupports *aParent, 
                                void** aReturn)=0;

  NS_IMETHOD    NewScriptNotation(nsIScriptContext *aContext, 
                                  nsISupports *aPI, 
                                  nsISupports *aParent, 
                                  void** aReturn)=0;

  NS_IMETHOD    NewScriptCrypto(nsIScriptContext *aContext, 
                                nsISupports *aPI, 
                                nsISupports *aParent, 
                                void** aReturn)=0;

  NS_IMETHOD    NewScriptCRMFObject(nsIScriptContext *aContext, 
                                    nsISupports *aPI, 
                                    nsISupports *aParent, 
                                    void** aReturn)=0;

  NS_IMETHOD    NewScriptPkcs11(nsIScriptContext *aContext, 
                                nsISupports *aPI, 
                                nsISupports *aParent, 
                                void** aReturn)=0;
};

#endif /* nsIDOMScriptObjectFactory_h__ */
