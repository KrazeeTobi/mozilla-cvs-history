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
/* AUTO-GENERATED. DO NOT EDIT!!! */

#include "jsapi.h"
#include "nsJSUtils.h"
#include "nsDOMError.h"
#include "nscore.h"
#include "nsIServiceManager.h"
#include "nsIScriptContext.h"
#include "nsIScriptSecurityManager.h"
#include "nsIJSScriptObject.h"
#include "nsIScriptObjectOwner.h"
#include "nsIScriptGlobalObject.h"
#include "nsCOMPtr.h"
#include "nsDOMPropEnums.h"
#include "nsString.h"
#include "nsIDOMHTMLFieldSetElement.h"
#include "nsIDOMHTMLFormElement.h"


static NS_DEFINE_IID(kIScriptObjectOwnerIID, NS_ISCRIPTOBJECTOWNER_IID);
static NS_DEFINE_IID(kIJSScriptObjectIID, NS_IJSSCRIPTOBJECT_IID);
static NS_DEFINE_IID(kIScriptGlobalObjectIID, NS_ISCRIPTGLOBALOBJECT_IID);
static NS_DEFINE_IID(kIHTMLFieldSetElementIID, NS_IDOMHTMLFIELDSETELEMENT_IID);
static NS_DEFINE_IID(kIHTMLFormElementIID, NS_IDOMHTMLFORMELEMENT_IID);

//
// HTMLFieldSetElement property ids
//
enum HTMLFieldSetElement_slots {
  HTMLFIELDSETELEMENT_FORM = -1
};

/***********************************************************************/
//
// HTMLFieldSetElement Properties Getter
//
PR_STATIC_CALLBACK(JSBool)
GetHTMLFieldSetElementProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
  nsIDOMHTMLFieldSetElement *a = (nsIDOMHTMLFieldSetElement*)nsJSUtils::nsGetNativeThis(cx, obj);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == a) {
    return JS_TRUE;
  }

  if (JSVAL_IS_INT(id)) {
    nsresult rv;
    NS_WITH_SERVICE(nsIScriptSecurityManager, secMan,
                    NS_SCRIPTSECURITYMANAGER_PROGID, &rv);
    if (NS_FAILED(rv)) {
      return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECMAN_ERR);
    }
    switch(JSVAL_TO_INT(id)) {
      case HTMLFIELDSETELEMENT_FORM:
      {
        rv = secMan->CheckScriptAccess(cx, obj, NS_DOM_PROP_HTMLFIELDSETELEMENT_FORM, PR_FALSE);
        if (NS_FAILED(rv)) {
          return nsJSUtils::nsReportError(cx, obj, rv);
        }
        nsIDOMHTMLFormElement* prop;
        nsresult result = NS_OK;
        result = a->GetForm(&prop);
        if (NS_SUCCEEDED(result)) {
          // get the js object
          nsJSUtils::nsConvertObjectToJSVal((nsISupports *)prop, cx, obj, vp);
        }
        else {
          return nsJSUtils::nsReportError(cx, obj, result);
        }
        break;
      }
      default:
        return nsJSUtils::nsCallJSScriptObjectGetProperty(a, cx, obj, id, vp);
    }
  }
  else {
    return nsJSUtils::nsCallJSScriptObjectGetProperty(a, cx, obj, id, vp);
  }

  return PR_TRUE;
}

/***********************************************************************/
//
// HTMLFieldSetElement Properties Setter
//
PR_STATIC_CALLBACK(JSBool)
SetHTMLFieldSetElementProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
  nsIDOMHTMLFieldSetElement *a = (nsIDOMHTMLFieldSetElement*)nsJSUtils::nsGetNativeThis(cx, obj);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == a) {
    return JS_TRUE;
  }

  if (JSVAL_IS_INT(id)) {
    nsresult rv;
    NS_WITH_SERVICE(nsIScriptSecurityManager, secMan,
                    NS_SCRIPTSECURITYMANAGER_PROGID, &rv);
    if (NS_FAILED(rv)) {
      return nsJSUtils::nsReportError(cx, obj, NS_ERROR_DOM_SECMAN_ERR);
    }
    switch(JSVAL_TO_INT(id)) {
      case 0:
      default:
        return nsJSUtils::nsCallJSScriptObjectSetProperty(a, cx, obj, id, vp);
    }
  }
  else {
    return nsJSUtils::nsCallJSScriptObjectSetProperty(a, cx, obj, id, vp);
  }

  return PR_TRUE;
}


//
// HTMLFieldSetElement finalizer
//
PR_STATIC_CALLBACK(void)
FinalizeHTMLFieldSetElement(JSContext *cx, JSObject *obj)
{
  nsJSUtils::nsGenericFinalize(cx, obj);
}


//
// HTMLFieldSetElement enumerate
//
PR_STATIC_CALLBACK(JSBool)
EnumerateHTMLFieldSetElement(JSContext *cx, JSObject *obj)
{
  return nsJSUtils::nsGenericEnumerate(cx, obj);
}


//
// HTMLFieldSetElement resolve
//
PR_STATIC_CALLBACK(JSBool)
ResolveHTMLFieldSetElement(JSContext *cx, JSObject *obj, jsval id)
{
  return nsJSUtils::nsGenericResolve(cx, obj, id);
}


/***********************************************************************/
//
// class for HTMLFieldSetElement
//
JSClass HTMLFieldSetElementClass = {
  "HTMLFieldSetElement", 
  JSCLASS_HAS_PRIVATE | JSCLASS_PRIVATE_IS_NSISUPPORTS,
  JS_PropertyStub,
  JS_PropertyStub,
  GetHTMLFieldSetElementProperty,
  SetHTMLFieldSetElementProperty,
  EnumerateHTMLFieldSetElement,
  ResolveHTMLFieldSetElement,
  JS_ConvertStub,
  FinalizeHTMLFieldSetElement,
  nsnull,
  nsJSUtils::nsCheckAccess
};


//
// HTMLFieldSetElement class properties
//
static JSPropertySpec HTMLFieldSetElementProperties[] =
{
  {"form",    HTMLFIELDSETELEMENT_FORM,    JSPROP_ENUMERATE | JSPROP_READONLY},
  {0}
};


//
// HTMLFieldSetElement class methods
//
static JSFunctionSpec HTMLFieldSetElementMethods[] = 
{
  {0}
};


//
// HTMLFieldSetElement constructor
//
PR_STATIC_CALLBACK(JSBool)
HTMLFieldSetElement(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  return JS_FALSE;
}


//
// HTMLFieldSetElement class initialization
//
extern "C" NS_DOM nsresult NS_InitHTMLFieldSetElementClass(nsIScriptContext *aContext, void **aPrototype)
{
  JSContext *jscontext = (JSContext *)aContext->GetNativeContext();
  JSObject *proto = nsnull;
  JSObject *constructor = nsnull;
  JSObject *parent_proto = nsnull;
  JSObject *global = JS_GetGlobalObject(jscontext);
  jsval vp;

  if ((PR_TRUE != JS_LookupProperty(jscontext, global, "HTMLFieldSetElement", &vp)) ||
      !JSVAL_IS_OBJECT(vp) ||
      ((constructor = JSVAL_TO_OBJECT(vp)) == nsnull) ||
      (PR_TRUE != JS_LookupProperty(jscontext, JSVAL_TO_OBJECT(vp), "prototype", &vp)) || 
      !JSVAL_IS_OBJECT(vp)) {

    if (NS_OK != NS_InitHTMLElementClass(aContext, (void **)&parent_proto)) {
      return NS_ERROR_FAILURE;
    }
    proto = JS_InitClass(jscontext,     // context
                         global,        // global object
                         parent_proto,  // parent proto 
                         &HTMLFieldSetElementClass,      // JSClass
                         HTMLFieldSetElement,            // JSNative ctor
                         0,             // ctor args
                         HTMLFieldSetElementProperties,  // proto props
                         HTMLFieldSetElementMethods,     // proto funcs
                         nsnull,        // ctor props (static)
                         nsnull);       // ctor funcs (static)
    if (nsnull == proto) {
      return NS_ERROR_FAILURE;
    }

  }
  else if ((nsnull != constructor) && JSVAL_IS_OBJECT(vp)) {
    proto = JSVAL_TO_OBJECT(vp);
  }
  else {
    return NS_ERROR_FAILURE;
  }

  if (aPrototype) {
    *aPrototype = proto;
  }
  return NS_OK;
}


//
// Method for creating a new HTMLFieldSetElement JavaScript object
//
extern "C" NS_DOM nsresult NS_NewScriptHTMLFieldSetElement(nsIScriptContext *aContext, nsISupports *aSupports, nsISupports *aParent, void **aReturn)
{
  NS_PRECONDITION(nsnull != aContext && nsnull != aSupports && nsnull != aReturn, "null argument to NS_NewScriptHTMLFieldSetElement");
  JSObject *proto;
  JSObject *parent;
  nsIScriptObjectOwner *owner;
  JSContext *jscontext = (JSContext *)aContext->GetNativeContext();
  nsresult result = NS_OK;
  nsIDOMHTMLFieldSetElement *aHTMLFieldSetElement;

  if (nsnull == aParent) {
    parent = nsnull;
  }
  else if (NS_OK == aParent->QueryInterface(kIScriptObjectOwnerIID, (void**)&owner)) {
    if (NS_OK != owner->GetScriptObject(aContext, (void **)&parent)) {
      NS_RELEASE(owner);
      return NS_ERROR_FAILURE;
    }
    NS_RELEASE(owner);
  }
  else {
    return NS_ERROR_FAILURE;
  }

  if (NS_OK != NS_InitHTMLFieldSetElementClass(aContext, (void **)&proto)) {
    return NS_ERROR_FAILURE;
  }

  result = aSupports->QueryInterface(kIHTMLFieldSetElementIID, (void **)&aHTMLFieldSetElement);
  if (NS_OK != result) {
    return result;
  }

  // create a js object for this class
  *aReturn = JS_NewObject(jscontext, &HTMLFieldSetElementClass, proto, parent);
  if (nsnull != *aReturn) {
    // connect the native object to the js object
    JS_SetPrivate(jscontext, (JSObject *)*aReturn, aHTMLFieldSetElement);
  }
  else {
    NS_RELEASE(aHTMLFieldSetElement);
    return NS_ERROR_FAILURE; 
  }

  return NS_OK;
}
