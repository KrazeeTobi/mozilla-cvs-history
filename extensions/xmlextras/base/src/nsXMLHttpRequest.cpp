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
 * Portions created by the Initial Developer are Copyright (C) 1998
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

#include "nsXMLHttpRequest.h"
#include "nsISimpleEnumerator.h"
#include "nsIXPConnect.h"
#include "nsIUnicodeEncoder.h"
#include "nsIServiceManager.h"
#include "nsICharsetConverterManager.h"
#include "nsLayoutCID.h"
#include "nsIDOMDOMImplementation.h"
#include "nsIPrivateDOMImplementation.h"
#include "nsXPIDLString.h"
#include "nsReadableUtils.h"
#include "nsPrintfCString.h"
#include "nsCRT.h"
#include "nsIURI.h"
#include "nsILoadGroup.h"
#include "nsNetUtil.h"
#include "nsIUploadChannel.h"
#include "nsIDOMSerializer.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsIDOMEventReceiver.h"
#include "nsIEventListenerManager.h"
#include "nsGUIEvent.h"
#include "nsIPrivateDOMEvent.h"
#include "prprf.h"
#include "nsIDOMEventListener.h"
#include "nsIJSContextStack.h"
#include "nsIScriptSecurityManager.h"
#include "nsWeakPtr.h"
#include "nsICharsetAlias.h"
#include "nsIScriptGlobalObject.h"
#include "nsIDOMClassInfo.h"
#include "nsIDOMElement.h"
#include "nsIDOMWindow.h"
#include "nsIVariant.h"
#include "nsIParser.h"
#include "nsLoadListenerProxy.h"
#include "nsIWindowWatcher.h"
#include "nsIAuthPrompt.h"
#include "nsIStringStream.h"

static const char* kLoadAsData = "loadAsData";
#define LOADSTR NS_LITERAL_STRING("load")
#define ERRORSTR NS_LITERAL_STRING("error")

// CIDs
static NS_DEFINE_CID(kCharsetAliasCID, NS_CHARSETALIAS_CID);
static NS_DEFINE_CID(kIDOMDOMImplementationCID, NS_DOM_IMPLEMENTATION_CID);
static NS_DEFINE_CID(kCharsetConverterManagerCID, NS_ICHARSETCONVERTERMANAGER_CID);
static NS_DEFINE_CID(kEventQueueServiceCID, NS_EVENTQUEUESERVICE_CID);

// State
#define XML_HTTP_REQUEST_UNINITIALIZED    1 // 0
#define XML_HTTP_REQUEST_OPENED           2 // 1 aka LOADING
#define XML_HTTP_REQUEST_LOADED           4 // 2
#define XML_HTTP_REQUEST_INTERACTIVE      8 // 3
#define XML_HTTP_REQUEST_COMPLETED       16 // 4
#define XML_HTTP_REQUEST_SENT            32 // Internal, LOADING in IE and external view
#define XML_HTTP_REQUEST_STOPPED         64 // Internal, INTERACTIVE in IE and external view
// The above states are mutually exclusing, change with ChangeState() only.
// The states below can be combined.
#define XML_HTTP_REQUEST_ABORTED        128 // Internal
#define XML_HTTP_REQUEST_ASYNC          256 // Internal
#define XML_HTTP_REQUEST_PARSEBODY      512 // Internal
#define XML_HTTP_REQUEST_XSITEENABLED  1024 // Internal
#define XML_HTTP_REQUEST_SYNCLOOPING   2048 // Internal

#define XML_HTTP_REQUEST_LOADSTATES         \
  (XML_HTTP_REQUEST_UNINITIALIZED |         \
   XML_HTTP_REQUEST_OPENED |                \
   XML_HTTP_REQUEST_LOADED |                \
   XML_HTTP_REQUEST_INTERACTIVE |           \
   XML_HTTP_REQUEST_COMPLETED |             \
   XML_HTTP_REQUEST_SENT |                  \
   XML_HTTP_REQUEST_STOPPED)

static nsIScriptContext *
GetCurrentContext()
{
  // Get JSContext from stack.
  nsCOMPtr<nsIJSContextStack> stack =
    do_GetService("@mozilla.org/js/xpc/ContextStack;1");

  if (!stack) {
    return nsnull;
  }

  JSContext *cx;

  if (NS_FAILED(stack->Peek(&cx)) || !cx) {
    return nsnull;
  }

  return GetScriptContextFromJSContext(cx);
}

/**
 * Gets the nsIDocument given the script context. Will return nsnull on failure.
 *
 * @param aScriptContext the script context to get the document for; can be null
 *
 * @return the document associated with the script context
 */
static already_AddRefed<nsIDocument>
GetDocumentFromScriptContext(nsIScriptContext *aScriptContext)
{
  if (!aScriptContext)
    return nsnull;

  nsCOMPtr<nsIDOMWindow> window =
    do_QueryInterface(aScriptContext->GetGlobalObject());
  nsIDocument *doc = nsnull;
  if (window) {
    nsCOMPtr<nsIDOMDocument> domdoc;
    window->GetDocument(getter_AddRefs(domdoc));
    (void) CallQueryInterface(domdoc, &doc);
  }
  return doc;
}

/////////////////////////////////////////////
//
//
/////////////////////////////////////////////

nsXMLHttpRequest::nsXMLHttpRequest()
  : mState(XML_HTTP_REQUEST_UNINITIALIZED)
{
}

nsXMLHttpRequest::~nsXMLHttpRequest()
{
  if (mState & (XML_HTTP_REQUEST_STOPPED | 
                XML_HTTP_REQUEST_SENT | 
                XML_HTTP_REQUEST_INTERACTIVE)) {
    Abort();
  }    
  
  NS_ABORT_IF_FALSE(!(mState & XML_HTTP_REQUEST_SYNCLOOPING), "we rather crash than hang");
  mState &= ~XML_HTTP_REQUEST_SYNCLOOPING;
}


// QueryInterface implementation for nsXMLHttpRequest
NS_INTERFACE_MAP_BEGIN(nsXMLHttpRequest)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIXMLHttpRequest)
  NS_INTERFACE_MAP_ENTRY(nsIXMLHttpRequest)
  NS_INTERFACE_MAP_ENTRY(nsIJSXMLHttpRequest)
  NS_INTERFACE_MAP_ENTRY(nsIDOMLoadListener)
  NS_INTERFACE_MAP_ENTRY(nsIDOMEventTarget)
  NS_INTERFACE_MAP_ENTRY(nsIRequestObserver)
  NS_INTERFACE_MAP_ENTRY(nsIStreamListener)
  NS_INTERFACE_MAP_ENTRY(nsIHttpEventSink)
  NS_INTERFACE_MAP_ENTRY(nsIInterfaceRequestor)
  NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
  NS_INTERFACE_MAP_ENTRY_EXTERNAL_DOM_CLASSINFO(XMLHttpRequest)
NS_INTERFACE_MAP_END


NS_IMPL_ADDREF(nsXMLHttpRequest)
NS_IMPL_RELEASE(nsXMLHttpRequest)


/* void addEventListener (in string type, in nsIDOMEventListener
   listener); */
NS_IMETHODIMP
nsXMLHttpRequest::AddEventListener(const nsAString& type,
                                   nsIDOMEventListener *listener,
                                   PRBool useCapture)
{
  NS_ENSURE_ARG(listener);
  nsresult rv;

  if (type.Equals(LOADSTR)) {
    if (!mLoadEventListeners) {
      rv = NS_NewISupportsArray(getter_AddRefs(mLoadEventListeners));
      NS_ENSURE_SUCCESS(rv, rv);
    }
    mLoadEventListeners->AppendElement(listener);
  }
  else if (type.Equals(ERRORSTR)) {
    if (!mErrorEventListeners) {
      rv = NS_NewISupportsArray(getter_AddRefs(mErrorEventListeners));
      NS_ENSURE_SUCCESS(rv, rv);
    }
    mErrorEventListeners->AppendElement(listener);
  }
  else {
    return NS_ERROR_INVALID_ARG;
  }
  mScriptContext = GetCurrentContext();

  return NS_OK;
}

/* void removeEventListener (in string type, in nsIDOMEventListener
   listener); */
NS_IMETHODIMP 
nsXMLHttpRequest::RemoveEventListener(const nsAString & type,
                                      nsIDOMEventListener *listener,
                                      PRBool useCapture)
{
  NS_ENSURE_ARG(listener);

  if (type.Equals(LOADSTR)) {
    if (mLoadEventListeners) {
      mLoadEventListeners->RemoveElement(listener);
    }
  }
  else if (type.Equals(ERRORSTR)) {
    if (mErrorEventListeners) {
      mErrorEventListeners->RemoveElement(listener);
    }
  }
  else {
    return NS_ERROR_INVALID_ARG;
  }

  return NS_OK;
}

/* boolean dispatchEvent (in nsIDOMEvent evt); */
NS_IMETHODIMP
nsXMLHttpRequest::DispatchEvent(nsIDOMEvent *evt, PRBool *_retval)
{
  // Ignored

  return NS_OK;
}

/* attribute nsIOnReadystatechangeHandler onreadystatechange; */
NS_IMETHODIMP 
nsXMLHttpRequest::GetOnreadystatechange(nsIOnReadystatechangeHandler * *aOnreadystatechange)
{
  NS_ENSURE_ARG_POINTER(aOnreadystatechange);

  *aOnreadystatechange = mOnReadystatechangeListener;
  NS_IF_ADDREF(*aOnreadystatechange);

  return NS_OK;
}

NS_IMETHODIMP 
nsXMLHttpRequest::SetOnreadystatechange(nsIOnReadystatechangeHandler * aOnreadystatechange)
{
  mOnReadystatechangeListener = aOnreadystatechange;

  mScriptContext = GetCurrentContext();

  return NS_OK;
}


/* attribute nsIDOMEventListener onload; */
NS_IMETHODIMP 
nsXMLHttpRequest::GetOnload(nsIDOMEventListener * *aOnLoad)
{
  NS_ENSURE_ARG_POINTER(aOnLoad);

  *aOnLoad = mOnLoadListener;
  NS_IF_ADDREF(*aOnLoad);

  return NS_OK;
}

NS_IMETHODIMP 
nsXMLHttpRequest::SetOnload(nsIDOMEventListener * aOnLoad)
{
  mOnLoadListener = aOnLoad;

  mScriptContext = GetCurrentContext();

  return NS_OK;
}

/* attribute nsIDOMEventListener onerror; */
NS_IMETHODIMP 
nsXMLHttpRequest::GetOnerror(nsIDOMEventListener * *aOnerror)
{
  NS_ENSURE_ARG_POINTER(aOnerror);

  *aOnerror = mOnErrorListener;
  NS_IF_ADDREF(*aOnerror);

  return NS_OK;
}

NS_IMETHODIMP 
nsXMLHttpRequest::SetOnerror(nsIDOMEventListener * aOnerror)
{
  mOnErrorListener = aOnerror;

  mScriptContext = GetCurrentContext();

  return NS_OK;
}

/* readonly attribute nsIChannel channel; */
NS_IMETHODIMP nsXMLHttpRequest::GetChannel(nsIChannel **aChannel)
{
  NS_ENSURE_ARG_POINTER(aChannel);
  *aChannel = mChannel;
  NS_IF_ADDREF(*aChannel);

  return NS_OK;
}

/* readonly attribute nsIDOMDocument responseXML; */
NS_IMETHODIMP nsXMLHttpRequest::GetResponseXML(nsIDOMDocument **aResponseXML)
{
  NS_ENSURE_ARG_POINTER(aResponseXML);
  *aResponseXML = nsnull;
  if ((XML_HTTP_REQUEST_COMPLETED & mState) && mDocument) {
    *aResponseXML = mDocument;
    NS_ADDREF(*aResponseXML);
  }

  return NS_OK;
}

/*
 * This piece copied from nsXMLDocument, we try to get the charset
 * from HTTP headers.
 */
nsresult
nsXMLHttpRequest::DetectCharset(nsACString& aCharset)
{
  aCharset.Truncate();
  nsresult rv;
  nsCAutoString charsetVal;
  rv = mChannel->GetContentCharset(charsetVal);
  if (NS_SUCCEEDED(rv)) {
    nsCOMPtr<nsICharsetAlias> calias(do_GetService(kCharsetAliasCID,&rv));
    if(NS_SUCCEEDED(rv) && calias) {
      rv = calias->GetPreferred(charsetVal, aCharset);
    }
  }
  return rv;
}

nsresult
nsXMLHttpRequest::ConvertBodyToText(nsAString& aOutBuffer)
{
  // This code here is basically a copy of a similar thing in
  // nsScanner::Append(const char* aBuffer, PRUint32 aLen).
  // If we get illegal characters in the input we replace 
  // them and don't just fail.

  PRInt32 dataLen = mResponseBody.Length();
  if (!dataLen)
    return NS_OK;

  nsresult rv = NS_OK;

  nsCAutoString dataCharset;
  nsCOMPtr<nsIDocument> document(do_QueryInterface(mDocument));
  if (document) {
    dataCharset = document->GetDocumentCharacterSet();
  } else {
    if (NS_FAILED(DetectCharset(dataCharset)) || dataCharset.IsEmpty()) {
      // MS documentation states UTF-8 is default for responseText
      dataCharset.Assign(NS_LITERAL_CSTRING("UTF-8"));
    }
  }

  if (dataCharset.Equals(NS_LITERAL_CSTRING("ASCII"))) {
    CopyASCIItoUTF16(mResponseBody, aOutBuffer);

    return NS_OK;
  }

  nsCOMPtr<nsICharsetConverterManager> ccm =
    do_GetService(NS_CHARSETCONVERTERMANAGER_CONTRACTID, &rv);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsIUnicodeDecoder> decoder;
  rv = ccm->GetUnicodeDecoderRaw(dataCharset.get(),
                                 getter_AddRefs(decoder));
  if (NS_FAILED(rv))
    return rv;

  const char * inBuffer = mResponseBody.get();
  PRInt32 outBufferLength;
  rv = decoder->GetMaxLength(inBuffer, dataLen, &outBufferLength);
  if (NS_FAILED(rv))
    return rv;

  PRUnichar * outBuffer = 
    NS_STATIC_CAST(PRUnichar*, nsMemory::Alloc((outBufferLength + 1) *
                                               sizeof(PRUnichar)));
  if (!outBuffer) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  PRInt32 totalChars = 0,
          outBufferIndex = 0,
          outLen = outBufferLength;

  do {
    PRInt32 inBufferLength = dataLen;
    rv = decoder->Convert(inBuffer, 
                          &inBufferLength, 
                          &outBuffer[outBufferIndex], 
                          &outLen);
    totalChars += outLen;
    if (NS_FAILED(rv)) {
      // We consume one byte, replace it with U+FFFD
      // and try the conversion again.
      outBuffer[outBufferIndex + outLen++] = (PRUnichar)0xFFFD;
      outBufferIndex += outLen;
      outLen = outBufferLength - (++totalChars);

      decoder->Reset();

      if((inBufferLength + 1) > dataLen) {
        inBufferLength = dataLen;
      } else {
        inBufferLength++;
      }

      inBuffer = &inBuffer[inBufferLength];
      dataLen -= inBufferLength;
    }
  } while ( NS_FAILED(rv) && (dataLen > 0) );

  aOutBuffer.Assign(outBuffer, totalChars);

  return NS_OK;
}

/* readonly attribute AString responseText; */
NS_IMETHODIMP nsXMLHttpRequest::GetResponseText(nsAString& aResponseText)
{
  nsresult rv = NS_OK;

  aResponseText.Truncate();

  if (mState & (XML_HTTP_REQUEST_COMPLETED |
                XML_HTTP_REQUEST_INTERACTIVE)) {
    rv = ConvertBodyToText(aResponseText);
  }

  return rv;
}

/* readonly attribute unsigned long status; */
NS_IMETHODIMP 
nsXMLHttpRequest::GetStatus(PRUint32 *aStatus)
{
  nsCOMPtr<nsIHttpChannel> httpChannel(do_QueryInterface(mChannel));

  if (httpChannel) {
    return httpChannel->GetResponseStatus(aStatus);
  } 
  *aStatus = 0;

  return NS_OK;
}

/* readonly attribute AUTF8String statusText; */
NS_IMETHODIMP 
nsXMLHttpRequest::GetStatusText(nsACString& aStatusText)
{
  nsCOMPtr<nsIHttpChannel> httpChannel(do_QueryInterface(mChannel));

  aStatusText.Truncate();

  nsresult rv = NS_OK;

  if (httpChannel) {
    rv = httpChannel->GetResponseStatusText(aStatusText);
  }

  return rv;
}

/* void abort (); */
NS_IMETHODIMP 
nsXMLHttpRequest::Abort()
{
  if (mReadRequest) {
    mReadRequest->Cancel(NS_BINDING_ABORTED);
  }
  if (mChannel) {
    mChannel->Cancel(NS_BINDING_ABORTED);
  }
  mDocument = nsnull;
  mState |= XML_HTTP_REQUEST_ABORTED;

  ChangeState(XML_HTTP_REQUEST_COMPLETED);

  ClearEventListeners();

  ChangeState(XML_HTTP_REQUEST_UNINITIALIZED, PR_FALSE);  // IE seems to do it

  return NS_OK;
}

/* string getAllResponseHeaders (); */
NS_IMETHODIMP 
nsXMLHttpRequest::GetAllResponseHeaders(char **_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);
  *_retval = nsnull;
  nsCOMPtr<nsIHttpChannel> httpChannel(do_QueryInterface(mChannel));
  if (httpChannel) {
    nsHeaderVisitor *visitor = nsnull;
    NS_NEWXPCOM(visitor, nsHeaderVisitor);
    if (!visitor)
      return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(visitor);

    nsresult rv = httpChannel->VisitResponseHeaders(visitor);
    if (NS_SUCCEEDED(rv))
      *_retval = ToNewCString(visitor->Headers());

    NS_RELEASE(visitor);
    return rv;
  }
  
  return NS_OK;
}

/* ACString getResponseHeader (in AUTF8String header); */
NS_IMETHODIMP 
nsXMLHttpRequest::GetResponseHeader(const nsACString& header,
                                    nsACString& _retval)
{
  nsCOMPtr<nsIHttpChannel> httpChannel(do_QueryInterface(mChannel));
  nsresult rv = NS_OK;

  _retval.Truncate();

  if (httpChannel) {
    rv = httpChannel->GetResponseHeader(header, _retval);
  }

  return rv;
}

nsresult 
nsXMLHttpRequest::GetLoadGroup(nsILoadGroup **aLoadGroup)
{
  NS_ENSURE_ARG_POINTER(aLoadGroup);
  *aLoadGroup = nsnull;

  if (!mScriptContext) {
    mScriptContext = GetCurrentContext();
  }

  nsCOMPtr<nsIDocument> doc = GetDocumentFromScriptContext(mScriptContext);
  if (doc) {
    *aLoadGroup = doc->GetDocumentLoadGroup().get();  // already_AddRefed
  }

  return NS_OK;
}

nsIURI *
nsXMLHttpRequest::GetBaseURI()
{
  if (!mScriptContext) {
    mScriptContext = GetCurrentContext();
    if (!mScriptContext) {
      return nsnull;
    }
  }

  nsCOMPtr<nsIDocument> doc = GetDocumentFromScriptContext(mScriptContext);
  if (!doc) {
    nsnull;
  }

  return doc->GetBaseURI();
}

nsresult
nsXMLHttpRequest::CreateEvent(PRUint32 aMsg, nsIDOMEvent** aDOMEvent)
{
  nsresult rv;

  nsCOMPtr<nsIDOMEventReceiver> receiver(do_QueryInterface(mDocument));
  if (!receiver) {
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsIEventListenerManager> manager;
  rv = receiver->GetListenerManager(getter_AddRefs(manager));
  if (!manager) {
    return NS_ERROR_FAILURE;
  }

  nsEvent event(aMsg);
  rv = manager->CreateEvent(nsnull, &event,
                            NS_LITERAL_STRING("HTMLEvents"), 
                            aDOMEvent);
  if (NS_FAILED(rv)) {
    return rv;
  }

  nsCOMPtr<nsIPrivateDOMEvent> privevent(do_QueryInterface(*aDOMEvent));
  if (!privevent) {
    NS_IF_RELEASE(*aDOMEvent);
    return NS_ERROR_FAILURE;
  }
  privevent->SetTarget(this);
  privevent->SetCurrentTarget(this);
  privevent->SetOriginalTarget(this);

  return NS_OK;
}

void
nsXMLHttpRequest::NotifyEventListeners(nsIDOMEventListener* aHandler, nsISupportsArray* aListeners, nsIDOMEvent* aEvent)
{
  if (!aEvent)
    return;

  nsCOMPtr<nsIJSContextStack> stack;
  JSContext *cx = nsnull;

  if (mScriptContext) {
    stack = do_GetService("@mozilla.org/js/xpc/ContextStack;1");

    if (stack) {
      cx = (JSContext *)mScriptContext->GetNativeContext();

      if (cx) {
        stack->Push(cx);
      }
    }
  }

  if (aHandler) {
    aHandler->HandleEvent(aEvent);
  }

  if (aListeners) {
    PRUint32 index, count;

    aListeners->Count(&count);
    for (index = 0; index < count; index++) {
      nsCOMPtr<nsIDOMEventListener> listener = do_QueryElementAt(aListeners, index);

      if (listener) {
        listener->HandleEvent(aEvent);
      }
    }
  }

  if (cx) {
    stack->Pop(&cx);
  }
}

void
nsXMLHttpRequest::ClearEventListeners()
{
  mLoadEventListeners = nsnull;
  mErrorEventListeners = nsnull;
  mOnLoadListener = nsnull;
  mOnErrorListener = nsnull;
  mOnReadystatechangeListener = nsnull;
}

/* noscript void openRequest (in AUTF8String method, in AUTF8String url, in boolean async, in AString user, in AString password); */
NS_IMETHODIMP
nsXMLHttpRequest::OpenRequest(const nsACString& method, 
                              const nsACString& url, 
                              PRBool async, 
                              const nsAString& user, 
                              const nsAString& password)
{
  NS_ENSURE_ARG(!method.IsEmpty());
  NS_ENSURE_ARG(!url.IsEmpty());
  
  nsresult rv;
  nsCOMPtr<nsIURI> uri; 
  PRBool authp = PR_FALSE;

  if (mState & XML_HTTP_REQUEST_ABORTED) {
    // Proceed as normal
  } else if (mState & (XML_HTTP_REQUEST_OPENED | 
                       XML_HTTP_REQUEST_LOADED |
                       XML_HTTP_REQUEST_INTERACTIVE |
                       XML_HTTP_REQUEST_SENT |
                       XML_HTTP_REQUEST_STOPPED)) {
    // IE aborts as well
    Abort();
  
    // XXX We should probably send a warning to the JS console
    //     that load was aborted and event listeners were cleared
    //     since this looks like a situation that could happen
    //     by accident and you could spend a lot of time wondering
    //     why things didn't work.

    return NS_OK;
  }

  if (async) {
    mState |= XML_HTTP_REQUEST_ASYNC;
  } else {
    mState &= ~XML_HTTP_REQUEST_ASYNC;
  }

  rv = NS_NewURI(getter_AddRefs(uri), url, nsnull, GetBaseURI());
  if (NS_FAILED(rv)) return rv;

  if (!user.IsEmpty()) {
    nsCAutoString userpass;
    CopyUTF16toUTF8(user, userpass);
    if (!password.IsEmpty()) {
      userpass.Append(':');
      AppendUTF16toUTF8(password, userpass);
    }
    uri->SetUserPass(userpass);
    authp = PR_TRUE;
  }

  // When we are called from JS we can find the load group for the page,
  // and add ourselves to it. This way any pending requests
  // will be automatically aborted if the user leaves the page.
  nsCOMPtr<nsILoadGroup> loadGroup;
  GetLoadGroup(getter_AddRefs(loadGroup));

  // nsIRequest::LOAD_BACKGROUND prevents throbber from becoming active,
  // which in turn keeps STOP button from becoming active
  rv = NS_NewChannel(getter_AddRefs(mChannel), uri, nsnull, loadGroup, 
                     nsnull, nsIRequest::LOAD_BACKGROUND);
  if (NS_FAILED(rv)) return rv;
  
  //mChannel->SetAuthTriedWithPrehost(authp);

  nsCOMPtr<nsIHttpChannel> httpChannel(do_QueryInterface(mChannel));
  if (httpChannel) {
    rv = httpChannel->SetRequestMethod(method);
  }

  ChangeState(XML_HTTP_REQUEST_OPENED);

  return rv;
}

/* void open (in AUTF8String method, in AUTF8String url); */
NS_IMETHODIMP 
nsXMLHttpRequest::Open(const nsACString& method, const nsACString& url)
{
  nsresult rv;
  PRBool async = PR_TRUE;
  nsAutoString user, password;

  nsCOMPtr<nsIXPCNativeCallContext> cc;
  nsCOMPtr<nsIXPConnect> xpc(do_GetService(nsIXPConnect::GetCID(), &rv));
  if(NS_SUCCEEDED(rv)) {
    rv = xpc->GetCurrentNativeCallContext(getter_AddRefs(cc));
  }

  if (NS_SUCCEEDED(rv) && cc) {
    PRUint32 argc;
    rv = cc->GetArgc(&argc);
    if (NS_FAILED(rv)) return NS_ERROR_FAILURE;

    jsval* argv;
    rv = cc->GetArgvPtr(&argv);
    if (NS_FAILED(rv)) return NS_ERROR_FAILURE;

    JSContext* cx;
    rv = cc->GetJSContext(&cx);
    if (NS_FAILED(rv)) return NS_ERROR_FAILURE;

    nsCOMPtr<nsIURI> targetURI;
    rv = NS_NewURI(getter_AddRefs(targetURI), url, nsnull, GetBaseURI());
    if (NS_FAILED(rv)) return NS_ERROR_FAILURE;

    nsCOMPtr<nsIScriptSecurityManager> secMan = 
             do_GetService(NS_SCRIPTSECURITYMANAGER_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return NS_ERROR_FAILURE;

    rv = secMan->CheckConnect(cx, targetURI, "XMLHttpRequest","open");
    if (NS_FAILED(rv))
    {
      // Security check failed.
      return NS_OK;
    }

    // Find out if UniversalBrowserRead privileges are enabled
    // we will need this in case of a redirect
    PRBool crossSiteAccessEnabled;
    rv = secMan->IsCapabilityEnabled("UniversalBrowserRead",
                                     &crossSiteAccessEnabled);
    if (NS_FAILED(rv)) return rv;
    if (crossSiteAccessEnabled) {
      mState |= XML_HTTP_REQUEST_XSITEENABLED;
    } else {
      mState &= ~XML_HTTP_REQUEST_XSITEENABLED;
    }

    if (argc > 2) {
      JSBool asyncBool;
      ::JS_ValueToBoolean(cx, argv[2], &asyncBool);
      async = (PRBool)asyncBool;

      if (argc > 3) {
        JSString* userStr = ::JS_ValueToString(cx, argv[3]);

        if (userStr) {
          user.Assign((PRUnichar *)::JS_GetStringChars(userStr),
                      ::JS_GetStringLength(userStr));
        }

        if (argc > 4) {
          JSString* passwordStr = JS_ValueToString(cx, argv[4]);

          if (passwordStr) {
            password.Assign((PRUnichar *)::JS_GetStringChars(passwordStr),
                            ::JS_GetStringLength(passwordStr));
          }
        }
      }
    }
  }

  return OpenRequest(method, url, async, user, password);
}

nsresult
nsXMLHttpRequest::GetStreamForWString(const PRUnichar* aStr,
                                      PRInt32 aLength,
                                      nsIInputStream** aStream)
{
  nsresult rv;
  nsCOMPtr<nsIUnicodeEncoder> encoder;
  char* postData;

  // We want to encode the string as utf-8, so get the right encoder
  nsCOMPtr<nsICharsetConverterManager> charsetConv = 
           do_GetService(kCharsetConverterManagerCID, &rv);
  NS_ENSURE_SUCCESS(rv, NS_ERROR_FAILURE);
  
  rv = charsetConv->GetUnicodeEncoderRaw("UTF-8",
                                         getter_AddRefs(encoder));
  NS_ENSURE_SUCCESS(rv, NS_ERROR_FAILURE);
  
  // Convert to utf-8
  PRInt32 charLength;
  const PRUnichar* unicodeBuf = aStr;
  PRInt32 unicodeLength = aLength;
    
  rv = encoder->GetMaxLength(unicodeBuf, unicodeLength, &charLength);
  if (NS_FAILED(rv)) return NS_ERROR_FAILURE;
  

  // Allocate extra space for the trailing and leading CRLF
  postData = (char*)nsMemory::Alloc(charLength + 5);
  if (!postData) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  rv = encoder->Convert(unicodeBuf, 
                        &unicodeLength, postData+2, &charLength);
  if (NS_FAILED(rv)) {
    nsMemory::Free(postData);
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsIHttpChannel> httpChannel(do_QueryInterface(mChannel));
  if (!httpChannel) {
    return NS_ERROR_FAILURE;
  }
  
  // If no content type header was set by the client, we set it to text/xml.
  nsCAutoString header;
  if( NS_FAILED(httpChannel->GetRequestHeader(NS_LITERAL_CSTRING("Content-Type"), header)) )  
    httpChannel->SetRequestHeader(NS_LITERAL_CSTRING("Content-Type"),
                                  NS_LITERAL_CSTRING("text/xml"),
                                  PR_FALSE);

  // set the content length header
  httpChannel->SetRequestHeader(NS_LITERAL_CSTRING("Content-Length"),
                                nsPrintfCString("%d", charLength),
                                PR_FALSE);

  // Shove in the trailing and leading CRLF
  postData[0] = nsCRT::CR;
  postData[1] = nsCRT::LF;
  postData[2+charLength] = nsCRT::CR;
  postData[2+charLength+1] = nsCRT::LF;
  postData[2+charLength+2] = '\0';

  nsCOMPtr<nsIStringInputStream> inputStream(do_CreateInstance("@mozilla.org/io/string-input-stream;1", &rv));
  if (NS_SUCCEEDED(rv)) {
    rv = inputStream->AdoptData(postData, charLength +4);
    if (NS_SUCCEEDED(rv)) {
      return CallQueryInterface(inputStream, aStream);
    }
  }

  // If we got here then something went wrong before the stream was
  // adopted the buffer.
  nsMemory::Free(postData);
  return NS_ERROR_FAILURE;
}


/*
 * "Copy" from a stream.
 */
NS_METHOD
nsXMLHttpRequest::StreamReaderFunc(nsIInputStream* in,
                                   void* closure,
                                   const char* fromRawSegment,
                                   PRUint32 toOffset,
                                   PRUint32 count,
                                   PRUint32 *writeCount)
{
  nsXMLHttpRequest* xmlHttpRequest = NS_STATIC_CAST(nsXMLHttpRequest*, closure);
  if (!xmlHttpRequest || !writeCount) {
    NS_WARNING("XMLHttpRequest cannot read from stream: no closure or writeCount");
    return NS_ERROR_FAILURE;
  }

  // Copy for our own use
  xmlHttpRequest->mResponseBody.Append(fromRawSegment,count);

  nsresult rv = NS_OK;

  if (xmlHttpRequest->mState & XML_HTTP_REQUEST_PARSEBODY) {
    // Give the same data to the parser.

    // We need to wrap the data in a new lightweight stream and pass that
    // to the parser, because calling ReadSegments() recursively on the same
    // stream is not supported.
    nsCOMPtr<nsIInputStream> copyStream;
    rv = NS_NewByteInputStream(getter_AddRefs(copyStream), fromRawSegment, count);

    if (NS_SUCCEEDED(rv)) {
      NS_ASSERTION(copyStream, "NS_NewByteInputStream lied");
      rv = xmlHttpRequest->mXMLParserStreamListener->OnDataAvailable(xmlHttpRequest->mReadRequest,xmlHttpRequest->mContext,copyStream,toOffset,count);
    }
  }

  xmlHttpRequest->ChangeState(XML_HTTP_REQUEST_INTERACTIVE);

  if (NS_SUCCEEDED(rv)) {
    *writeCount = count;
  } else {
    *writeCount = 0;
  }

  return rv;
}

/* void onDataAvailable (in nsIRequest request, in nsISupports ctxt, in nsIInputStream inStr, in unsigned long sourceOffset, in unsigned long count); */
NS_IMETHODIMP 
nsXMLHttpRequest::OnDataAvailable(nsIRequest *request, nsISupports *ctxt, nsIInputStream *inStr, PRUint32 sourceOffset, PRUint32 count)
{
  NS_ENSURE_ARG_POINTER(inStr);

  NS_ABORT_IF_FALSE(mContext.get() == ctxt,"start context different from OnDataAvailable context");

  PRUint32 totalRead;
  return inStr->ReadSegments(nsXMLHttpRequest::StreamReaderFunc, (void*)this, count, &totalRead);
}

/* void onStartRequest (in nsIRequest request, in nsISupports ctxt); */
NS_IMETHODIMP 
nsXMLHttpRequest::OnStartRequest(nsIRequest *request, nsISupports *ctxt)
{
  // Don't do anything if we have been aborted
  if (mState & XML_HTTP_REQUEST_UNINITIALIZED)
    return NS_OK;

  mReadRequest = request;
  mContext = ctxt;
  mState |= XML_HTTP_REQUEST_PARSEBODY;
  ChangeState(XML_HTTP_REQUEST_LOADED);

  if (!mDocument) {
    // We were probably aborted. So no point in attempting to parse
    // incoming data.
    mState &= ~XML_HTTP_REQUEST_PARSEBODY;
  }

  if (mOverrideMimeType.IsEmpty()) {
    // If we are not overriding the mime type, we can gain a huge performance
    // win by not even trying to parse non-XML data. This also protects us
    // from the situation where we have an XML document and sink, but HTML (or other)
    // parser, which can produce unreliable results.
    nsCAutoString type;
    mChannel->GetContentType(type);
    if (type.Find("xml") == -1) {
      mState &= ~XML_HTTP_REQUEST_PARSEBODY;
    }
  } else {
    nsresult status;
    request->GetStatus(&status);
    nsCOMPtr<nsIChannel> channel = do_QueryInterface(request);
    if (channel && NS_SUCCEEDED(status)) {
      channel->SetContentType(mOverrideMimeType);
    }
  }
  
  if (mState & XML_HTTP_REQUEST_PARSEBODY) {
    nsCOMPtr<nsIStreamListener> listener;
    nsCOMPtr<nsILoadGroup> loadGroup;
    mChannel->GetLoadGroup(getter_AddRefs(loadGroup));
    
    nsCOMPtr<nsIDocument> document(do_QueryInterface(mDocument));
    if (!document) {
      return NS_ERROR_FAILURE;
    }

    nsresult rv = document->StartDocumentLoad(kLoadAsData, mChannel, 
                                              loadGroup, nsnull, 
                                              getter_AddRefs(listener),
                                              PR_TRUE);
    NS_ENSURE_SUCCESS(rv, rv);

    mXMLParserStreamListener = listener;
    return mXMLParserStreamListener->OnStartRequest(request, ctxt);
  }
  
  return NS_OK;
}

/* void onStopRequest (in nsIRequest request, in nsISupports ctxt, in nsresult status, in wstring statusArg); */
NS_IMETHODIMP 
nsXMLHttpRequest::OnStopRequest(nsIRequest *request, nsISupports *ctxt, nsresult status)
{
  // Don't do anything if we have been aborted
  if (mState & XML_HTTP_REQUEST_UNINITIALIZED)
    return NS_OK;

  nsresult rv = NS_OK;

  nsCOMPtr<nsIParser> parser;

  if (mState & XML_HTTP_REQUEST_PARSEBODY) { 
    NS_ASSERTION(mXMLParserStreamListener, "null parser stream in parsing mode!!");
    parser = do_QueryInterface(mXMLParserStreamListener);
    NS_ABORT_IF_FALSE(parser, "stream listener was expected to be a parser");
    rv = mXMLParserStreamListener->OnStopRequest(request, ctxt, status);
  }

  mXMLParserStreamListener = nsnull;
  mReadRequest = nsnull;
  mContext = nsnull;
  mChannel->SetNotificationCallbacks(nsnull);

  if (NS_FAILED(status)) {
    // This can happen if the server is unreachable. Other possible reasons
    // are that the user leaves the page or hits the ESC key.
    Error(nsnull);

    // By nulling out channel here we make it so that Send() can test for that
    // and throw. Also calling the various status methods/members will not throw.
    // This matches what IE does.
    mChannel = nsnull;
  } else if (!parser || parser->IsParserEnabled()) {
    // If we don't have a parser, we never attempted to parse the
    // incoming data, and we can proceed to call RequestCompleted().
    // Alternatively, if we do have a parser, its possible that we
    // have given it some data and this caused it to block e.g. by a
    // by a xml-stylesheet PI. In this case, we will have to wait till
    // it gets enabled again and RequestCompleted() must be called
    // later, when we get the load event from the document. If the
    // parser is enabled, it is not blocked and we can still go ahead
    // and call RequestCompleted() and expect everything to get
    // cleaned up immediately.
    RequestCompleted();
  } else {
    ChangeState(XML_HTTP_REQUEST_STOPPED, PR_FALSE);
  }

  mState &= ~XML_HTTP_REQUEST_SYNCLOOPING;

  return rv;
}

nsresult 
nsXMLHttpRequest::RequestCompleted()
{
  nsresult rv = NS_OK;

  mState &= ~XML_HTTP_REQUEST_SYNCLOOPING;

  // If we're uninitialized at this point, we encountered an error
  // earlier and listeners have already been notified. Also we do
  // not want to do this if we already completed.
  if (mState & (XML_HTTP_REQUEST_UNINITIALIZED |
                XML_HTTP_REQUEST_COMPLETED)) {
    return NS_OK;
  }

  // We need to create the event before nulling out mDocument
  nsCOMPtr<nsIDOMEvent> domevent;
  rv = CreateEvent(NS_PAGE_LOAD, getter_AddRefs(domevent));

  // We might have been sent non-XML data. If that was the case,
  // we should null out the document member. The idea in this
  // check here is that if there is no document element it is not
  // an XML document. We might need a fancier check...
  if (mDocument) {
    nsCOMPtr<nsIDOMElement> root;
    mDocument->GetDocumentElement(getter_AddRefs(root));
    if (!root) {
      mDocument = nsnull;
    }
  }

  ChangeState(XML_HTTP_REQUEST_COMPLETED);

  NotifyEventListeners(mOnLoadListener, mLoadEventListeners, domevent);
  ClearEventListeners();

  return rv;
}

/* void send (in nsIVariant aBody); */
NS_IMETHODIMP 
nsXMLHttpRequest::Send(nsIVariant *aBody)
{
  nsresult rv;

  // Return error if we're already processing a request
  if (XML_HTTP_REQUEST_SENT & mState) {
    return NS_ERROR_FAILURE;
  }
  
  // Make sure we've been opened
  if (!mChannel || !(XML_HTTP_REQUEST_OPENED & mState)) {
    return NS_ERROR_NOT_INITIALIZED;
  }

  // XXX We should probably send a warning to the JS console
  //     if there are no event listeners set and we are doing
  //     an asynchronous call.

  // Ignore argument if method is GET, there is no point in trying to
  // upload anything
  nsCAutoString method;
  nsCOMPtr<nsIHttpChannel> httpChannel(do_QueryInterface(mChannel));

  if (httpChannel) {
    httpChannel->GetRequestMethod(method); // If GET, method name will be uppercase
  }

  if (aBody && httpChannel && !method.Equals(NS_LITERAL_CSTRING("GET"))) {
    nsXPIDLString serial;
    nsCOMPtr<nsIInputStream> postDataStream;

    PRUint16 dataType;
    rv = aBody->GetDataType(&dataType);
    if (NS_FAILED(rv)) 
      return rv;

    switch (dataType) {
    case nsIDataType::VTYPE_INTERFACE:
    case nsIDataType::VTYPE_INTERFACE_IS:
      {
        nsCOMPtr<nsISupports> supports;
        nsID *iid;
        rv = aBody->GetAsInterface(&iid, getter_AddRefs(supports));
        if (NS_FAILED(rv)) 
          return rv;
        if (iid)
          nsMemory::Free(iid);

        // document?
        nsCOMPtr<nsIDOMDocument> doc(do_QueryInterface(supports));
        if (doc) {
          nsCOMPtr<nsIDOMSerializer> serializer(do_CreateInstance(NS_XMLSERIALIZER_CONTRACTID, &rv));
          if (NS_FAILED(rv)) return rv;  

          rv = serializer->SerializeToString(doc, serial);
          if (NS_FAILED(rv)) 
            return rv;
        } else {
          // nsISupportsString?
          nsCOMPtr<nsISupportsString> wstr(do_QueryInterface(supports));
          if (wstr) {
            wstr->GetData(serial);
          } else {
            // stream?
            nsCOMPtr<nsIInputStream> stream(do_QueryInterface(supports));
            if (stream) {
              postDataStream = stream;
            }
          }
        }
      }
      break;
    case nsIDataType::VTYPE_VOID:
    case nsIDataType::VTYPE_EMPTY:
      // Makes us act as if !aBody, don't upload anything
      break;
    case nsIDataType::VTYPE_EMPTY_ARRAY:
    case nsIDataType::VTYPE_ARRAY:
      // IE6 throws error here, so we do that as well
      return NS_ERROR_INVALID_ARG;
    default:
      // try variant string
      rv = aBody->GetAsWString(getter_Copies(serial));    
      if (NS_FAILED(rv)) 
        return rv;
      break;
    }

    if (serial) {
      // Convert to a byte stream
      rv = GetStreamForWString(serial.get(), 
                               nsCRT::strlen(serial.get()),
                               getter_AddRefs(postDataStream));
      if (NS_FAILED(rv)) 
        return rv;
    }

    if (postDataStream) {
      nsCOMPtr<nsIUploadChannel> uploadChannel(do_QueryInterface(httpChannel));
      NS_ASSERTION(uploadChannel, "http must support nsIUploadChannel");

      rv = uploadChannel->SetUploadStream(postDataStream, EmptyCString(), -1);
      // Reset the method to its original value
      if (httpChannel) {
        httpChannel->SetRequestMethod(method);
      }
    }
  }

  // Get and initialize a DOMImplementation
  nsCOMPtr<nsIDOMDOMImplementation> implementation =
    do_CreateInstance(kIDOMDOMImplementationCID, &rv);
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIPrivateDOMImplementation> privImpl =
    do_QueryInterface(implementation);
  if (privImpl) {
    privImpl->Init(GetBaseURI());
  }

  // Create an empty document from it (resets current document as well)
  const nsAString& emptyStr = EmptyString();
  rv = implementation->CreateDocument(emptyStr, 
                                      emptyStr, 
                                      nsnull, 
                                      getter_AddRefs(mDocument));
  if (NS_FAILED(rv)) return NS_ERROR_FAILURE;

  // Reset responseBody
  mResponseBody.Truncate();

  // Register as a load listener on the document
  nsCOMPtr<nsIDOMEventReceiver> target(do_QueryInterface(mDocument));
  if (target) {
    nsWeakPtr requestWeak(do_GetWeakReference(NS_STATIC_CAST(nsIXMLHttpRequest*, this)));
    nsLoadListenerProxy* proxy = new nsLoadListenerProxy(requestWeak);
    if (!proxy) return NS_ERROR_OUT_OF_MEMORY;

    // This will addref the proxy
    rv = target->AddEventListenerByIID(NS_STATIC_CAST(nsIDOMEventListener*, 
                                                      proxy), 
                                       NS_GET_IID(nsIDOMLoadListener));
    if (NS_FAILED(rv)) return NS_ERROR_FAILURE;
  }


  nsCOMPtr<nsIEventQueue> modalEventQueue;

  if (!(mState & XML_HTTP_REQUEST_ASYNC)) {
    if(!mEventQService) {
      mEventQService = do_GetService(kEventQueueServiceCID, &rv);
      NS_ENSURE_SUCCESS(rv, NS_ERROR_FAILURE);
    }

    mState |= XML_HTTP_REQUEST_SYNCLOOPING;

    rv = mEventQService->PushThreadEventQueue(getter_AddRefs(modalEventQueue));
    if (NS_FAILED(rv)) {
      return rv;
    }
  }

  if (!mScriptContext) {
    // We need a context to check if redirect (if any) is allowed
    mScriptContext = GetCurrentContext();
  }

  // Hook us up to listen to redirects and the like
  mChannel->SetNotificationCallbacks(this);

  // Start reading from the channel
  ChangeState(XML_HTTP_REQUEST_SENT);
  rv = mChannel->AsyncOpen(this, nsnull);

  if (NS_FAILED(rv)) {
    if (modalEventQueue) {
      mEventQService->PopThreadEventQueue(modalEventQueue);
    }
    return rv;
  }  

  // If we're synchronous, spin an event loop here and wait
  if (!(mState & XML_HTTP_REQUEST_ASYNC)) {
    while (mState & XML_HTTP_REQUEST_SYNCLOOPING) {
      modalEventQueue->ProcessPendingEvents();
    }

    mEventQService->PopThreadEventQueue(modalEventQueue);
  }
  
  if (!mChannel) {
    return NS_ERROR_FAILURE;
  }

  return NS_OK;
}

/* void setRequestHeader (in AUTF8String header, in AUTF8String value); */
NS_IMETHODIMP 
nsXMLHttpRequest::SetRequestHeader(const nsACString& header,
                                   const nsACString& value)
{
  if (!mChannel)             // open() initializes mChannel, and open()
    return NS_ERROR_FAILURE; // must be called before first setRequestHeader()

  if (!IsASCII(header) || !IsASCII(value)) {
    return NS_ERROR_INVALID_ARG;
  }

  nsCOMPtr<nsIHttpChannel> httpChannel(do_QueryInterface(mChannel));

  if (httpChannel) {
    // We need to set, not add to, the header.
    return httpChannel->SetRequestHeader(header, value, PR_FALSE);
  }

  return NS_OK;
}

/* readonly attribute long readyState; */
NS_IMETHODIMP 
nsXMLHttpRequest::GetReadyState(PRInt32 *aState)
{
  NS_ENSURE_ARG_POINTER(aState);
  // Translate some of our internal states for external consumers
  if (mState & XML_HTTP_REQUEST_UNINITIALIZED) {
    *aState = 0; // UNINITIALIZED
  } else  if (mState & (XML_HTTP_REQUEST_OPENED | XML_HTTP_REQUEST_SENT)) {
    *aState = 1; // LOADING
  } else if (mState & XML_HTTP_REQUEST_LOADED) {
    *aState = 2; // LOADED
  } else if (mState & (XML_HTTP_REQUEST_INTERACTIVE | XML_HTTP_REQUEST_STOPPED)) {
    *aState = 3; // INTERACTIVE
  } else if (mState & XML_HTTP_REQUEST_COMPLETED) {
    *aState = 4; // COMPLETED
  } else {
    NS_ERROR("Should not happen");
  }
  
  return NS_OK;
}

/* void   overrideMimeType(in AUTF8String mimetype); */
NS_IMETHODIMP
nsXMLHttpRequest::OverrideMimeType(const nsACString& aMimeType)
{
  // XXX Should we do some validation here?
  mOverrideMimeType.Assign(aMimeType);
  return NS_OK;
}


// nsIDOMEventListener
nsresult
nsXMLHttpRequest::HandleEvent(nsIDOMEvent* aEvent)
{
  return NS_OK;
}


// nsIDOMLoadListener
nsresult
nsXMLHttpRequest::Load(nsIDOMEvent* aEvent)
{
  // If we had an XML error in the data, the parser terminated and
  // we received the load event, even though we might still be
  // loading data into responseBody/responseText. We will delay
  // sending the load event until OnStopRequest(). In normal case
  // there is no harm done, we will get OnStopRequest() immediately
  // after the load event.
  //
  // However, if the data we were loading caused the parser to stop,
  // for example when loading external stylesheets, we can receive
  // the OnStopRequest() call before the parser has finished building
  // the document. In that case, we obviously should not fire the event
  // in OnStopRequest(). For those documents, we must wait for the load
  // event from the document to fire our RequestCompleted().
  if (mState & XML_HTTP_REQUEST_STOPPED) {
    RequestCompleted();
  }
  return NS_OK;
}

nsresult
nsXMLHttpRequest::Unload(nsIDOMEvent* aEvent)
{
  return NS_OK;
}

nsresult
nsXMLHttpRequest::BeforeUnload(nsIDOMEvent* aEvent)
{
  return NS_OK;
}

nsresult
nsXMLHttpRequest::Abort(nsIDOMEvent* aEvent)
{
  Abort();

  mState &= ~XML_HTTP_REQUEST_SYNCLOOPING;

  return NS_OK;
}

nsresult
nsXMLHttpRequest::Error(nsIDOMEvent* aEvent)
{
  // We need to create the event before nulling out mDocument
  nsCOMPtr<nsIDOMEvent> event(do_QueryInterface(aEvent));
  if (!event) {
    // There is no NS_PAGE_ERROR event but NS_SCRIPT_ERROR should be ok.
    CreateEvent(NS_SCRIPT_ERROR, getter_AddRefs(event));
  }

  mDocument = nsnull;
  ChangeState(XML_HTTP_REQUEST_COMPLETED);

  mState &= ~XML_HTTP_REQUEST_SYNCLOOPING;

  NotifyEventListeners(mOnErrorListener, mErrorEventListeners, event);
  ClearEventListeners();

  return NS_OK;
}

nsresult
nsXMLHttpRequest::ChangeState(PRUint32 aState, PRBool aBroadcast)
{
  // If we are setting one of the mutually exclusing states,
  // unset those state bits first.
  if (aState & XML_HTTP_REQUEST_LOADSTATES) {
    mState &= ~XML_HTTP_REQUEST_LOADSTATES;
  }
  mState |= aState;
  nsresult rv = NS_OK;
  if ((mState & XML_HTTP_REQUEST_ASYNC) &&
      (aState & XML_HTTP_REQUEST_LOADSTATES) && // Broadcast load states only
      aBroadcast && 
      mOnReadystatechangeListener) {
    nsCOMPtr<nsIJSContextStack> stack;
    JSContext *cx = nsnull;

    if (mScriptContext) {
      stack = do_GetService("@mozilla.org/js/xpc/ContextStack;1");

      if (stack) {
        cx = (JSContext *)mScriptContext->GetNativeContext();

        if (cx) {
          stack->Push(cx);
        }
      }
    }

    rv = mOnReadystatechangeListener->HandleEvent();

    if (cx) {
      stack->Pop(&cx);
    }
  }

  return rv;
}

/////////////////////////////////////////////////////
// nsIHttpEventSink methods:
//
NS_IMETHODIMP
nsXMLHttpRequest::OnRedirect(nsIHttpChannel *aHttpChannel, nsIChannel *aNewChannel)
{
  NS_ENSURE_ARG_POINTER(aNewChannel);

  if (mScriptContext && !(mState & XML_HTTP_REQUEST_XSITEENABLED)) {
    nsresult rv = NS_ERROR_FAILURE;

    nsCOMPtr<nsIJSContextStack> stack(do_GetService("@mozilla.org/js/xpc/ContextStack;1", & rv));
    if (NS_FAILED(rv))
      return rv;

    JSContext *cx = (JSContext *)mScriptContext->GetNativeContext();
    if (!cx)
      return NS_ERROR_UNEXPECTED;

    nsCOMPtr<nsIScriptSecurityManager> secMan = 
             do_GetService(NS_SCRIPTSECURITYMANAGER_CONTRACTID, &rv);
    if (NS_FAILED(rv))
      return rv;

    nsCOMPtr<nsIURI> newURI;
    rv = aNewChannel->GetURI(getter_AddRefs(newURI)); // The redirected URI
    if (NS_FAILED(rv)) 
      return rv;

    stack->Push(cx);

    rv = secMan->CheckSameOrigin(cx, newURI);

    stack->Pop(&cx);
    
    if (NS_FAILED(rv))
      return rv;
  }

  mChannel = aNewChannel;

  return NS_OK;
}

/////////////////////////////////////////////////////
// nsIInterfaceRequestor methods:
//
NS_IMETHODIMP
nsXMLHttpRequest::GetInterface(const nsIID & aIID, void **aResult)
{
  if (aIID.Equals(NS_GET_IID(nsIAuthPrompt))) {
    NS_ENSURE_ARG_POINTER(aResult);
    *aResult = nsnull;

    nsresult rv;
    nsCOMPtr<nsIWindowWatcher> ww(do_GetService(NS_WINDOWWATCHER_CONTRACTID, &rv));
    if (NS_FAILED(rv))
      return rv;

    nsCOMPtr<nsIAuthPrompt> prompt;
    rv = ww->GetNewAuthPrompter(nsnull, getter_AddRefs(prompt));
    if (NS_FAILED(rv))
      return rv;

    nsIAuthPrompt *p = prompt.get();
    NS_ADDREF(p);
    *aResult = p;
    return NS_OK;
  }

  return QueryInterface(aIID, aResult);
}


NS_IMPL_ISUPPORTS1(nsXMLHttpRequest::nsHeaderVisitor, nsIHttpHeaderVisitor)

NS_IMETHODIMP nsXMLHttpRequest::
nsHeaderVisitor::VisitHeader(const nsACString &header, const nsACString &value)
{
    mHeaders.Append(header);
    mHeaders.Append(": ");
    mHeaders.Append(value);
    mHeaders.Append('\n');
    return NS_OK;
}
