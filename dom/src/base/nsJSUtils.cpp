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
 *   Vidur Apparao <vidur@netscape.com>
 */

/**
 * This is not a generated file. It contains common utility functions 
 * invoked from the JavaScript code generated from IDL interfaces.
 * The goal of the utility functions is to cut down on the size of
 * the generated code itself.
 */

#include "nsJSUtils.h"
#include "jsapi.h"
#include "jsdbgapi.h"
#include "prprf.h"
#include "nscore.h"
#include "nsIScriptContext.h"
#include "nsIScriptObjectOwner.h"
#include "nsIScriptGlobalObject.h"
#include "nsString.h"
#include "nsIScriptNameSpaceManager.h"
#include "nsIComponentManager.h"
#include "nsIScriptEventListener.h"
#include "nsIServiceManager.h"
#include "nsIXPConnect.h"
#include "nsIDOMDOMException.h"
#include "nsDOMError.h"
#include "nsCOMPtr.h"
#include "nsIScriptSecurityManager.h"
#include "nsIJSNativeInitializer.h"

static NS_DEFINE_CID(kXPConnectCID, NS_XPCONNECT_CID);

static struct ResultMap 
{nsresult rv; const char* name; const char* format;} map[] = {
#define DOM_MSG_DEF(val, format) \
    {(val), #val, format},
#include "domerr.msg"
#undef DOM_MSG_DEF
    {0,0,0}   // sentinel to mark end of array
};

PRBool
nsJSUtils::NameAndFormatForNSResult(nsresult rv, const char** name,
                                    const char** format)
{
  for (ResultMap* p = map; p->name; p++)
  {
    if (rv == p->rv)
    {
      if (name) *name = p->name;
      if (format) *format = p->format;
      return PR_TRUE;
    }
  }
  return PR_FALSE;
}

JSBool
nsJSUtils::GetCallingLocation(JSContext* aContext, const char* *aFilename,
                              PRUint32 *aLineno)
{
  // Get the current filename and line number
  JSStackFrame* frame = nsnull;
  JSScript* script = nsnull;
  do {
    frame = ::JS_FrameIterator(aContext, &frame);

    if (frame) {
      script = ::JS_GetFrameScript(aContext, frame);
    }
  } while ((nsnull != frame) && (nsnull == script));

  if (script) {
    const char* filename = ::JS_GetScriptFilename(aContext, script);

    if (filename) {
      PRUint32 lineno = 0;
      jsbytecode* bytecode = ::JS_GetFramePC(aContext, frame);

      if (bytecode) {
        lineno = ::JS_PCToLineNumber(aContext, script, bytecode);
      }

      *aFilename = filename;
      *aLineno = lineno;
      return JS_TRUE;
    }
  }

  return JS_FALSE;
}

JSBool
nsJSUtils::ReportError(JSContext* aContext, JSObject* aObj, nsresult aResult,
                       const char* aMessage)
{
  const char* name = nsnull;
  const char* format = nsnull;

  // Get the name and message
  if (!aMessage)
    NameAndFormatForNSResult(aResult, &name, &format);
  else
    format = aMessage;

  const char* filename;
  PRUint32 lineno;
  char* location = nsnull;

  if (nsJSUtils::GetCallingLocation(aContext, &filename, &lineno))
    location = PR_smprintf("%s Line: %d", filename, lineno);
  
  nsCOMPtr<nsIDOMDOMException> exc;
  nsresult rv = NS_NewDOMException(getter_AddRefs(exc), 
                                   aResult,
                                   name, 
                                   format,
                                   location);
  
  if (location) {
    PR_smprintf_free(location);
  }
    
  if (NS_SUCCEEDED(rv)) {
    nsCOMPtr<nsIScriptObjectOwner> owner = do_QueryInterface(exc);
    if (owner) {
      nsCOMPtr<nsIScriptContext> scriptCX;
      GetStaticScriptContext(aContext, aObj, getter_AddRefs(scriptCX));
      if (scriptCX) {
        JSObject* obj;
        rv = owner->GetScriptObject(scriptCX, (void**)&obj);
        if (NS_SUCCEEDED(rv)) {
          ::JS_SetPendingException(aContext, OBJECT_TO_JSVAL(obj));
        }
      }
    }
  }

  return JS_FALSE;
}

void 
nsJSUtils::ConvertStringToJSVal(const nsString& aProp, JSContext* aContext,
                                jsval* aReturn)
{
  JSString *jsstring =
    ::JS_NewUCStringCopyN(aContext, NS_REINTERPRET_CAST(const jschar*,
                                                        aProp.get()),
                          aProp.Length());

  // set the return value
  *aReturn = STRING_TO_JSVAL(jsstring);
}

PRBool
nsJSUtils::ConvertJSValToXPCObject(nsISupports** aSupports, REFNSIID aIID,
                                   JSContext* aContext, jsval aValue)
{
  *aSupports = nsnull;
  if (JSVAL_IS_NULL(aValue)) {
    return JS_TRUE;
  }
  else if (JSVAL_IS_OBJECT(aValue)) {
    nsresult rv;
    NS_WITH_SERVICE(nsIXPConnect, xpc, kXPConnectCID, &rv);
    if (NS_FAILED(rv))
      return JS_FALSE;

    // WrapJS does all the work to recycle an existing wrapper and/or do a QI
    rv = xpc->WrapJS(aContext, JSVAL_TO_OBJECT(aValue), aIID,
                     (void**)aSupports);

    if (NS_FAILED(rv))
      return JS_FALSE;

    return JS_TRUE;
  }

  return JS_FALSE;
}

void 
nsJSUtils::ConvertJSValToString(nsAWritableString& aString,
                                JSContext* aContext, jsval aValue)
{
  JSString *jsstring;
  if ((jsstring = ::JS_ValueToString(aContext, aValue)) != nsnull) {
    aString.Assign(NS_REINTERPRET_CAST(const PRUnichar*,
                                       ::JS_GetStringChars(jsstring)),
                   ::JS_GetStringLength(jsstring));
  }
  else {
    aString.Truncate();
  }
}

PRBool
nsJSUtils::ConvertJSValToUint32(PRUint32* aProp, JSContext* aContext,
                                jsval aValue)
{
  uint32 temp;
  if (::JS_ValueToECMAUint32(aContext, aValue, &temp)) {
    *aProp = (PRUint32)temp;
  }
  else {
    ::JS_ReportError(aContext, "Parameter must be an integer");
    return JS_FALSE;
  }
  
  return JS_TRUE;
}

nsresult 
nsJSUtils::GetStaticScriptGlobal(JSContext* aContext, JSObject* aObj,
                                 nsIScriptGlobalObject** aNativeGlobal)
{
  nsISupports* supports;
  JSClass* clazz;
  JSObject* parent;
  JSObject* glob = aObj; // starting point for search

  if (!glob)
    return NS_ERROR_FAILURE;

  while ((parent = ::JS_GetParent(aContext, glob)))
    glob = parent;

#ifdef JS_THREADSAFE
  clazz = ::JS_GetClass(aContext, glob);
#else
  clazz = ::JS_GetClass(glob);
#endif

  if (!clazz ||
      !(clazz->flags & JSCLASS_HAS_PRIVATE) ||
      !(clazz->flags & JSCLASS_PRIVATE_IS_NSISUPPORTS) ||
      !(supports = (nsISupports*)::JS_GetPrivate(aContext, glob))) {
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsIXPConnectWrappedNative> wrapper(do_QueryInterface(supports));
  NS_ENSURE_TRUE(wrapper, NS_ERROR_UNEXPECTED);

  nsCOMPtr<nsISupports> native;
  wrapper->GetNative(getter_AddRefs(native));

  return CallQueryInterface(native, aNativeGlobal);
}

nsresult 
nsJSUtils::GetStaticScriptContext(JSContext* aContext, JSObject* aObj,
                                  nsIScriptContext** aScriptContext)
{
  nsCOMPtr<nsIScriptGlobalObject> nativeGlobal;
  GetStaticScriptGlobal(aContext, aObj, getter_AddRefs(nativeGlobal));
  if (!nativeGlobal)    
    return NS_ERROR_FAILURE;
  nsIScriptContext* scriptContext = nsnull;
  nativeGlobal->GetContext(&scriptContext);
  *aScriptContext = scriptContext;
  return scriptContext ? NS_OK : NS_ERROR_FAILURE;
}  

nsresult 
nsJSUtils::GetDynamicScriptGlobal(JSContext* aContext,
                                  nsIScriptGlobalObject** aNativeGlobal)
{
  nsCOMPtr<nsIScriptContext> scriptCX;
  GetDynamicScriptContext(aContext, getter_AddRefs(scriptCX));
  if (!scriptCX)
    return NS_ERROR_FAILURE;
  return scriptCX->GetGlobalObject(aNativeGlobal);
}  

nsresult 
nsJSUtils::GetDynamicScriptContext(JSContext *aContext,
                                   nsIScriptContext** aScriptContext)
{
  // XXX We rely on the rule that if any JSContext in our JSRuntime has a 
  // private set then that private *must* be a pointer to an nsISupports.
  nsISupports *supports = (nsIScriptContext*)::JS_GetContextPrivate(aContext);
  if (!supports)
      return nsnull;
  return supports->QueryInterface(NS_GET_IID(nsIScriptContext),
                                  (void**)aScriptContext);
}

JSBool PR_CALLBACK
nsJSUtils::CheckAccess(JSContext *cx, JSObject *obj, jsid id,
                       JSAccessMode mode, jsval *vp)
{
  if (mode == JSACC_WATCH) {
    jsval value, dummy;
    if (!::JS_IdToValue(cx, id, &value))
      return JS_FALSE;
    JSString *str = ::JS_ValueToString(cx, value);

    if (!str)
      return JS_FALSE;

    return ::JS_GetUCProperty(cx, obj, ::JS_GetStringChars(str),
                              ::JS_GetStringLength(str), &dummy);
  }

  return JS_TRUE;
}

nsIScriptSecurityManager * 
nsJSUtils::GetSecurityManager(JSContext *cx, JSObject *obj)
{
  if (!mCachedSecurityManager) {
    nsresult rv;
    NS_WITH_SERVICE(nsIScriptSecurityManager, secMan,
                    NS_SCRIPTSECURITYMANAGER_CONTRACTID, &rv);
    if (NS_FAILED(rv)) {
      nsJSUtils::ReportError(cx, obj, NS_ERROR_DOM_SECMAN_ERR);
      return nsnull;
    }
    mCachedSecurityManager = secMan;
    NS_ADDREF(mCachedSecurityManager);
  }
  return mCachedSecurityManager;
}

void
nsJSUtils::ClearCachedSecurityManager()
{
  if (mCachedSecurityManager) {
    NS_RELEASE(mCachedSecurityManager);
    mCachedSecurityManager = nsnull;
  }
}

nsIScriptSecurityManager *nsJSUtils::mCachedSecurityManager;

