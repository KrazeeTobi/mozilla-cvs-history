/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
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
 * Copyright (C) 2000 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 *   Johnny Stenback <jst@netscape.com> (original author)
 */

#ifndef nsDOMClassInfo_h___
#define nsDOMClassInfo_h___

#include "nsIDOMClassInfo.h"
#include "nsIXPCScriptable.h"
#include "jsapi.h"
#include "nsIScriptSecurityManager.h"

class nsIPluginInstance;

struct nsDOMClassInfoData;
typedef void (*GetDOMClassIIDsFnc)(nsVoidArray& aArray);

class nsDOMClassInfo : public nsIXPCScriptable,
                       public nsIClassInfo
{
public:
  nsDOMClassInfo(nsDOMClassInfoID aID);
  virtual ~nsDOMClassInfo();

  NS_DECL_NSIXPCSCRIPTABLE

  NS_DECL_ISUPPORTS

  NS_DECL_NSICLASSINFO

  // Helper method that returns a *non* refcounted pointer to a
  // helper. So please note, don't release this pointer, if you do,
  // you better make sure you've addreffed before release.
  //
  // Whaaaaa! I wanted to name this method GetClassInfo, but nooo,
  // some of Microsoft devstudio's headers #defines GetClassInfo to
  // GetClassInfoA so I can't, those $%#@^! bastards!!! What gives
  // them the right to do that?

  static nsISupports* GetClassInfoInstance(nsDOMClassInfoID aID,
                                           GetDOMClassIIDsFnc aGetIIDsFptr,
                                           const char *aName);

  static void ShutDown();

  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsDOMClassInfo(aID);
  }

  static nsresult WrapNative(JSContext *cx, JSObject *scope,
                             nsISupports *native, const nsIID& aIID,
                             jsval *vp);

protected:
  static nsresult Init();

  nsDOMClassInfoID mID;

  static nsIXPConnect *sXPConnect;
  static nsIScriptSecurityManager *sSecMan;

  // nsIXPCScriptable code
  static nsresult DefineStaticJSStrings(JSContext *cx);

  static PRBool sIsInitialized;

  static JSString *sTop_id;
  static JSString *sScrollbars_id;
  static JSString *sLocation_id;
  static JSString *sComponents_id;
  static JSString *s_content_id;
  static JSString *sContent_id;
  static JSString *sSidebar_id;
  static JSString *sPrompter_id;
  static JSString *sMenubar_id;
  static JSString *sToolbar_id;
  static JSString *sLocationbar_id;
  static JSString *sPersonalbar_id;
  static JSString *sStatusbar_id;
  static JSString *sDirectories_id;
  static JSString *sControllers_id;
  static JSString *sLength_id;
  static JSString *sOnmousedown_id;
  static JSString *sOnmouseup_id;
  static JSString *sOnclick_id;
  static JSString *sOncontextmenu_id;
  static JSString *sOnmouseover_id;
  static JSString *sOnmouseout_id;
  static JSString *sOnkeydown_id;
  static JSString *sOnkeyup_id;
  static JSString *sOnkeypress_id;
  static JSString *sOnmousemove_id;
  static JSString *sOnfocus_id;
  static JSString *sOnblur_id;
  static JSString *sOnsubmit_id;
  static JSString *sOnreset_id;
  static JSString *sOnchange_id;
  static JSString *sOnselect_id;
  static JSString *sOnload_id;
  static JSString *sOnunload_id;
  static JSString *sOnabort_id;
  static JSString *sOnerror_id;
  static JSString *sOnpaint_id;
  static JSString *sOnresize_id;
  static JSString *sOnscroll_id;
};

typedef nsDOMClassInfo nsDOMGenericSH;


// EventProp scriptable helper, this class should be the base class of
// all objects that should support things like
// obj.onclick=function{...}

class nsEventRecieverSH : public nsDOMGenericSH
{
protected:
  nsEventRecieverSH(nsDOMClassInfoID aID) : nsDOMGenericSH(aID)
  {
  }

  virtual ~nsEventRecieverSH()
  {
  }

  static PRBool ReallyIsEventName(JSString *jsstr, jschar aFirstChar);

  static inline PRBool IsEventName(JSString *jsstr)
  {
    jschar *str = ::JS_GetStringChars(jsstr);

    if (str[0] == 'o' && str[1] == 'n') {
      return ReallyIsEventName(jsstr, str[2]);
    }

    return PR_FALSE;
  }

  nsresult RegisterCompileHandler(nsIXPConnectWrappedNative *wrapper,
                                  JSContext *cx, JSObject *obj, jsval id,
                                  PRBool compile, PRBool *did_compile);

public:
  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);
  NS_IMETHOD SetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp,
                         PRBool *_retval);
};


// Window scirptable helper

class nsWindowSH : public nsEventRecieverSH
{
protected:
  nsWindowSH(nsDOMClassInfoID aID) : nsEventRecieverSH(aID)
  {
  }

  virtual ~nsWindowSH()
  {
  }

  static nsresult GlobalResolve(nsISupports *aNative, JSContext *cx,
                                JSObject *obj, JSString *str, PRUint32 flags,
                                PRBool *did_resolve);
  static nsresult DefineInterfaceProperty(JSContext *cx, JSObject *obj,
                                          JSString *str);

  nsresult doCheckReadAccess(JSContext *cx, JSObject *obj, jsval id,
                             nsISupports *native);
  nsresult doCheckWriteAccess(JSContext *cx, JSObject *obj, jsval id,
                              nsISupports *native);

public:
  NS_IMETHOD PreCreate(nsISupports *nativeObj, JSContext *cx,
                       JSObject *globalObj, JSObject **parentObj);
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD SetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);
  NS_IMETHOD Finalize(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                      JSObject *obj);

  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsWindowSH(aID);
  }
};


// DOM Node helper, this class deals with setting the parent for the
// wrappers

class nsNodeSH : public nsEventRecieverSH
{
protected:
  nsNodeSH(nsDOMClassInfoID aID) : nsEventRecieverSH(aID)
  {
  }

  virtual ~nsNodeSH()
  {
  }

public:
  NS_IMETHOD PreCreate(nsISupports *nativeObj, JSContext *cx,
                       JSObject *globalObj, JSObject **parentObj);
  NS_IMETHOD AddProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsNodeSH(aID);
  }
};


// Element helper

class nsElementSH : public nsNodeSH
{
protected:
  nsElementSH(nsDOMClassInfoID aID) : nsNodeSH(aID)
  {
  }

  virtual ~nsElementSH()
  {
  }

public:
  NS_IMETHOD PostCreate(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                    JSObject *obj);

  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsElementSH(aID);
  }
};


// NodeList scriptable helper

class nsArraySH : public nsDOMClassInfo
{
protected:
  nsArraySH(nsDOMClassInfoID aID) : nsDOMClassInfo(aID)
  {
  }

  virtual ~nsArraySH()
  {
  }

  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

public:
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsArraySH(aID);
  }
};


// NamedArray helper

class nsNamedArraySH : public nsArraySH
{
protected:
  nsNamedArraySH(nsDOMClassInfoID aID) : nsArraySH(aID)
  {
  }

  virtual ~nsNamedArraySH()
  {
  }

  virtual nsresult GetNamedItem(nsISupports *aNative, nsAReadableString& aName,
                                nsISupports **aResult) = 0;

public:
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
};


// NamedNodeMap helper

class nsNamedNodeMapSH : public nsNamedArraySH
{
protected:
  nsNamedNodeMapSH(nsDOMClassInfoID aID) : nsNamedArraySH(aID)
  {
  }

  virtual ~nsNamedNodeMapSH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

  // Override nsNamedArraySH::GetNamedItem()
  virtual nsresult GetNamedItem(nsISupports *aNative, nsAReadableString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsNamedNodeMapSH(aID);
  }
};


// HTMLCollection helper

class nsHTMLCollectionSH : public nsNamedArraySH
{
protected:
  nsHTMLCollectionSH(nsDOMClassInfoID aID) : nsNamedArraySH(aID)
  {
  }

  virtual ~nsHTMLCollectionSH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

  // Override nsNamedArraySH::GetNamedItem()
  virtual nsresult GetNamedItem(nsISupports *aNative, nsAReadableString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsHTMLCollectionSH(aID);
  }
};


// FomrControlList helper

class nsFormControlListSH : public nsHTMLCollectionSH
{
protected:
  nsFormControlListSH(nsDOMClassInfoID aID) : nsHTMLCollectionSH(aID)
  {
  }

  virtual ~nsFormControlListSH()
  {
  }

  // Override nsNamedArraySH::GetNamedItem() since our NamedItem() can
  // return either a nsIDOMNode or a nsIHTMLCollection
  virtual nsresult GetNamedItem(nsISupports *aNative, nsAReadableString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsFormControlListSH(aID);
  }
};


// Document helper, for document.location and document.on*

class nsDocumentSH : public nsNodeSH
{
public:
  nsDocumentSH(nsDOMClassInfoID aID) : nsNodeSH(aID)
  {
  }

  virtual ~nsDocumentSH()
  {
  }

public:
  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD SetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsDocumentSH(aID);
  }
};


// HTMLDocument helper

class nsHTMLDocumentSH : public nsDocumentSH
{
protected:
  nsHTMLDocumentSH(nsDOMClassInfoID aID) : nsDocumentSH(aID)
  {
  }

  virtual ~nsHTMLDocumentSH()
  {
  }

public:
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsHTMLDocumentSH(aID);
  }
};


// HTMLFormElement helper

class nsHTMLFormElementSH : public nsElementSH
{
protected:
  nsHTMLFormElementSH(nsDOMClassInfoID aID) : nsElementSH(aID)
  {
  }

  virtual ~nsHTMLFormElementSH()
  {
  }

public:
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp,
                         PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsHTMLFormElementSH(aID);
  }
};


// Base helper for external HTML object (such as a plugin or an
// applet)

class nsHTMLExternalObjSH : public nsElementSH
{
protected:
  nsHTMLExternalObjSH(nsDOMClassInfoID aID) : nsElementSH(aID)
  {
  }

  virtual ~nsHTMLExternalObjSH()
  {
  }

  nsresult GetPluginInstance(nsIXPConnectWrappedNative *aWrapper,
                             nsIPluginInstance **aResult);

  virtual nsresult GetPluginJSObject(JSContext *cx, JSObject *obj,
                                     nsIPluginInstance *plugin_inst,
                                     JSObject **plugin_obj,
                                     JSObject **plugin_proto) = 0;

public:
  NS_IMETHOD PostCreate(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj);
};


// HTMLEmbed/ObjectElement helper

class nsHTMLPluginObjElementSH : public nsHTMLExternalObjSH
{
protected:
  nsHTMLPluginObjElementSH(nsDOMClassInfoID aID) : nsHTMLExternalObjSH(aID)
  {
  }

  virtual ~nsHTMLPluginObjElementSH()
  {
  }

  virtual nsresult GetPluginJSObject(JSContext *cx, JSObject *obj,
                                     nsIPluginInstance *plugin_inst,
                                     JSObject **plugin_obj,
                                     JSObject **plugin_proto);

public:
  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsHTMLPluginObjElementSH(aID);
  }
};


// HTMLAppletElement helper

class nsHTMLAppletElementSH : public nsHTMLExternalObjSH
{
protected:
  nsHTMLAppletElementSH(nsDOMClassInfoID aID) : nsHTMLExternalObjSH(aID)
  {
  }

  virtual ~nsHTMLAppletElementSH()
  {
  }

  virtual nsresult GetPluginJSObject(JSContext *cx, JSObject *obj,
                                     nsIPluginInstance *plugin_inst,
                                     JSObject **plugin_obj,
                                     JSObject **plugin_proto);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsHTMLAppletElementSH(aID);
  }
};


// HTMLOptionCollection helper

class nsHTMLOptionCollectionSH : public nsHTMLCollectionSH
{
protected:
  nsHTMLOptionCollectionSH(nsDOMClassInfoID aID) : nsHTMLCollectionSH(aID)
  {
  }

  virtual ~nsHTMLOptionCollectionSH()
  {
  }

public:
  NS_IMETHOD SetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsHTMLOptionCollectionSH(aID);
  }
};


// Plugin helper

class nsPluginSH : public nsNamedArraySH
{
protected:
  nsPluginSH(nsDOMClassInfoID aID) : nsNamedArraySH(aID)
  {
  }

  virtual ~nsPluginSH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

  // Override nsNamedArraySH::GetNamedItem()
  virtual nsresult GetNamedItem(nsISupports *aNative, nsAReadableString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsPluginSH(aID);
  }
};


// PluginArray helper

class nsPluginArraySH : public nsNamedArraySH
{
protected:
  nsPluginArraySH(nsDOMClassInfoID aID) : nsNamedArraySH(aID)
  {
  }

  virtual ~nsPluginArraySH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

  // Override nsNamedArraySH::GetNamedItem()
  virtual nsresult GetNamedItem(nsISupports *aNative, nsAReadableString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsPluginArraySH(aID);
  }
};


// MimeTypeArray helper

class nsMimeTypeArraySH : public nsNamedArraySH
{
protected:
  nsMimeTypeArraySH(nsDOMClassInfoID aID) : nsNamedArraySH(aID)
  {
  }

  virtual ~nsMimeTypeArraySH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

  // Override nsNamedArraySH::GetNamedItem()
  virtual nsresult GetNamedItem(nsISupports *aNative, nsAReadableString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsMimeTypeArraySH(aID);
  }
};


// History helper

class nsStringArraySH : public nsDOMClassInfo
{
protected:
  nsStringArraySH(nsDOMClassInfoID aID) : nsDOMClassInfo(aID)
  {
  }

  virtual ~nsStringArraySH()
  {
  }

  virtual nsresult GetStringAt(nsISupports *aNative, PRInt32 aIndex,
                               nsAWritableString& aResult) = 0;

public:
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
};


// History helper

class nsHistorySH : public nsStringArraySH
{
protected:
  nsHistorySH(nsDOMClassInfoID aID) : nsStringArraySH(aID)
  {
  }

  virtual ~nsHistorySH()
  {
  }

  virtual nsresult GetStringAt(nsISupports *aNative, PRInt32 aIndex,
                               nsAWritableString& aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsHistorySH(aID);
  }
};


// MediaList helper

class nsMediaListSH : public nsStringArraySH
{
protected:
  nsMediaListSH(nsDOMClassInfoID aID) : nsStringArraySH(aID)
  {
  }

  virtual ~nsMediaListSH()
  {
  }

  virtual nsresult GetStringAt(nsISupports *aNative, PRInt32 aIndex,
                               nsAWritableString& aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsMediaListSH(aID);
  }
};


// StyleSheetList helper

class nsStyleSheetListSH : public nsArraySH
{
protected:
  nsStyleSheetListSH(nsDOMClassInfoID aID) : nsArraySH(aID)
  {
  }

  virtual ~nsStyleSheetListSH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsStyleSheetListSH(aID);
  }
};


// CSSStyleDeclaration helper

class nsCSSStyleDeclSH : public nsStringArraySH
{
protected:
  nsCSSStyleDeclSH(nsDOMClassInfoID aID) : nsStringArraySH(aID)
  {
  }

  virtual ~nsCSSStyleDeclSH()
  {
  }

  virtual nsresult GetStringAt(nsISupports *aNative, PRInt32 aIndex,
                               nsAWritableString& aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsCSSStyleDeclSH(aID);
  }
};


// XMLHttpRequest helper

class nsXMLHttpRequestSH : public nsDOMGenericSH
{
protected:
  nsXMLHttpRequestSH(nsDOMClassInfoID aID) : nsDOMGenericSH(aID)
  {
  }

  virtual ~nsXMLHttpRequestSH()
  {
  }

public:
  NS_IMETHOD GetHelperForLanguage(PRUint32 language, nsISupports **_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoID aID)
  {
    return new nsXMLHttpRequestSH(aID);
  }
};


// Event handler 'this' translator class, this is called by XPConnect
// when a "function interface" (nsIDOMEventListener) is called, this
// class extracts 'this' fomr the first argument to the called
// function (nsIDOMEventListener::HandleEvent(in nsIDOMEvent)), this
// class will pass back nsIDOMEvent::currentTarget to be used as
// 'this'.

class nsEventListenerThisTranslator : public nsIXPCFunctionThisTranslator
{
public:
  nsEventListenerThisTranslator()
  {
    NS_INIT_ISUPPORTS();
  }

  virtual ~nsEventListenerThisTranslator()
  {
  }

  // nsISupports
  NS_DECL_ISUPPORTS

  // nsIXPCFunctionThisTranslator
  NS_DECL_NSIXPCFUNCTIONTHISTRANSLATOR
};


/**
 * nsIClassInfo helper macros
 */

#define NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(_class)                          \
  if (aIID.Equals(NS_GET_IID(nsIClassInfo))) {                                \
    foundInterface =                                                          \
      nsDOMClassInfo::GetClassInfoInstance(eDOMClassInfo_##_class##_id,       \
                                           Get##_class##IIDs,                 \
                                           #_class);                          \
    NS_ENSURE_TRUE(foundInterface, NS_ERROR_OUT_OF_MEMORY);                   \
                                                                              \
    *aInstancePtr = foundInterface;                                           \
                                                                              \
    return NS_OK;                                                             \
  } else

#endif /* nsDOMClassInfo_h___ */
