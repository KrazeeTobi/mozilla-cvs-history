/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 *   Gagan Saksena <gagan@netscape.com> (original author)
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *   Mike Shaver <shaver@zeroknowledge.com>
 *   Christopher Blizzard <blizzard@mozilla.org>
 *   Darin Fisher <darin@netscape.com>
 */

#include "nspr.h"
#include "prlong.h"
#include "nsHTTPChannel.h"
#include "netCore.h"
#include "nsIHTTPEventSink.h"
#include "nsIHTTPProtocolHandler.h"
#include "nsHTTPRequest.h"
#include "nsHTTPResponse.h"
#include "nsIInterfaceRequestor.h"
#include "nsIChannel.h"
#include "nsIInputStream.h"
#include "nsIStreamListener.h"
#include "nsIIOService.h"
#include "nsXPIDLString.h"
#include "nsHTTPAtoms.h"
#include "nsNetUtil.h"
#include "nsIHttpNotify.h"
#include "nsINetModRegEntry.h"
#include "nsIServiceManager.h"
#include "nsINetModuleMgr.h"
#include "nsIEventQueueService.h"
#include "nsCExternalHandlerService.h"
#include "nsIMIMEService.h"
#include "nsIEnumerator.h"
#include "nsAuthEngine.h"
#include "nsIScriptSecurityManager.h"
#include "nsIProxy.h"
#include "nsMimeTypes.h"
#include "nsIPrompt.h"
#include "nsIThread.h"
#include "nsIEventQueueService.h"
#include "nsIProxyObjectManager.h"
#include "nsIWalletService.h"
#include "netCore.h"

#ifdef MOZ_NEW_CACHE
static NS_DEFINE_CID(kStreamListenerTeeCID, NS_STREAMLISTENERTEE_CID);
#else
#include "nsINetDataCacheManager.h"
#include "nsINetDataCache.h"
#endif

// FIXME - Temporary include.  Delete this when cache is enabled on all 
// platforms
#include "nsIPref.h"
#include "nsIAuthenticator.h"

static NS_DEFINE_CID(kNetModuleMgrCID, NS_NETMODULEMGR_CID);
static NS_DEFINE_CID(kIOServiceCID, NS_IOSERVICE_CID);
static NS_DEFINE_CID(kWalletServiceCID, NS_WALLETSERVICE_CID);
static NS_DEFINE_CID(kProxyObjectManagerCID, NS_PROXYEVENT_MANAGER_CID);
static NS_DEFINE_CID(kEventQueueServiceCID, NS_EVENTQUEUESERVICE_CID);

#if defined(PR_LOGGING)
extern PRLogModuleInfo* gHTTPLog;
#endif

#define LOG(args) PR_LOG(gHTTPLog, PR_LOG_DEBUG, args)


nsHTTPChannel::nsHTTPChannel(nsIURI *aURL, nsHTTPHandler *aHandler)
    : mHandler(dont_QueryInterface(aHandler))
    , mState(HS_IDLE)
    , mStatus(NS_OK)
    , mRequest(nsnull)
    , mResponse(nsnull)
    , mCachedResponse(nsnull)
    , mHTTPServerListener(nsnull)
    , mURI(dont_QueryInterface(aURL))
    , mLoadAttributes(LOAD_NORMAL)
    , mLoadGroup(nsnull)
    , mAuthRealm(nsnull)
    , mProxyType(nsnull)
    , mProxy(nsnull)
    , mProxyPort(-1)
    , mPipelinedRequest(nsnull)
    , mPipeliningAllowed(PR_TRUE)
    , mConnected(PR_FALSE)
    , mApplyConversion(PR_TRUE)
    , mOpenHasEventQueue(PR_TRUE)
    , mCachedContentIsAvailable(PR_FALSE)
    , mCachedContentIsValid(PR_FALSE)
    , mFiredOnHeadersAvailable(PR_FALSE)
    , mAuthTriedWithPrehost(PR_FALSE)
    , mProxyTransparent(PR_FALSE)
{
    NS_INIT_ISUPPORTS();

#ifndef MOZ_NEW_CACHE
    NS_NewISupportsArray(getter_AddRefs(mStreamAsFileObserverArray));
#endif

#if defined(PR_LOGGING)
    nsXPIDLCString urlCString; 
    mURI->GetSpec(getter_Copies(urlCString));
  
    LOG(("Creating nsHTTPChannel [this=%x] for URI: %s.\n", 
        this, (const char *)urlCString));
#endif

    PRBool isHTTPS = PR_FALSE;
    if (NS_SUCCEEDED(mURI->SchemeIs(nsIURI::HTTPS, &isHTTPS)) && isHTTPS)
        mLoadAttributes |= nsIChannel::INHIBIT_PERSISTENT_CACHING; 
}

nsHTTPChannel::~nsHTTPChannel()
{
    LOG(("Deleting nsHTTPChannel [this=%x].\n", this));

#ifdef NS_DEBUG
    NS_RELEASE(mRequest);
#else
    NS_IF_RELEASE(mRequest);
#endif
    NS_IF_RELEASE(mResponse);
    NS_IF_RELEASE(mCachedResponse);

    mHandler         = 0;
    mEventSink       = 0;
    mPrompter        = 0;
    mResponseContext = 0;
    mLoadGroup       = 0;

    CRTFREEIF(mAuthRealm);
    CRTFREEIF(mProxy);
    CRTFREEIF(mProxyType);
}

#ifdef MOZ_NEW_CACHE
NS_IMPL_THREADSAFE_ISUPPORTS7(nsHTTPChannel,
                              nsIHTTPChannel,
                              nsIChannel,
                              nsIInterfaceRequestor,
                              nsIProgressEventSink,
                              nsIProxy,
                              nsIRequest,
                              nsICacheListener);
                              //nsICachingChannel);
#else
NS_IMPL_THREADSAFE_ISUPPORTS7(nsHTTPChannel,
                              nsIHTTPChannel,
                              nsIChannel,
                              nsIInterfaceRequestor,
                              nsIProgressEventSink,
                              nsIProxy,
                              nsIRequest,
                              nsIStreamAsFile);
#endif

////////////////////////////////////////////////////////////////////////////////
// nsIRequest methods:

NS_IMETHODIMP
nsHTTPChannel::GetName(PRUnichar **result)
{
    return mRequest->GetName(result);
}

NS_IMETHODIMP
nsHTTPChannel::IsPending(PRBool *result)
{
    return mRequest->IsPending(result);
}

NS_IMETHODIMP
nsHTTPChannel::GetStatus(nsresult *status)
{
    *status = mStatus;
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::Cancel(nsresult status)
{
    nsresult rv;

    //
    // If this channel is currently waiting for a transport to become available.
    // Notify the HTTPHandler that this request has been cancelled...
    //
    if (!mConnected && (HS_WAITING_FOR_OPEN == mState))
        rv = mHandler->CancelPendingChannel(this);

    mStatus = status;
    rv = mRequest->Cancel(status);

    if (mResponseDataListener) {
        nsIStreamListener *sl = mResponseDataListener;
        nsHTTPFinalListener *fl = NS_STATIC_CAST(nsHTTPFinalListener*, sl);
        fl->FireNotifications();
    }
    return rv;
}

NS_IMETHODIMP
nsHTTPChannel::Suspend()
{
    return mRequest->Suspend();
}

NS_IMETHODIMP
nsHTTPChannel::Resume()
{
    return mRequest->Resume();
}

////////////////////////////////////////////////////////////////////////////////
// nsIChannel methods:

NS_IMETHODIMP
nsHTTPChannel::GetOriginalURI(nsIURI **aURI)
{
    *aURI = mOriginalURI ? mOriginalURI : mURI;
    NS_IF_ADDREF(*aURI);
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::SetOriginalURI(nsIURI *aURI)
{
    mOriginalURI = aURI;
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::GetURI(nsIURI **aURI)
{
    *aURI = mURI;
    NS_IF_ADDREF(*aURI);
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::SetURI(nsIURI *aURI)
{
    mURI = aURI;
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::GetOpenHasEventQueue(PRBool *hasEventQueue)
{
    NS_ENSURE_ARG_POINTER(hasEventQueue);
    *hasEventQueue = mOpenHasEventQueue;
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::SetOpenHasEventQueue(PRBool hasEventQueue)
{
    mOpenHasEventQueue = hasEventQueue;
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::Open(nsIInputStream **aStream)
{
    nsresult rv;
    if (mConnected) return NS_ERROR_ALREADY_CONNECTED;

    nsCOMPtr<nsIStreamListener> listener;

    rv = NS_NewSyncStreamListener(aStream,
                                  getter_AddRefs(mBufOutputStream),
                                  getter_AddRefs(listener));

    if (NS_FAILED(rv))
        return rv;

    if (mOpenHasEventQueue) {
        rv = AsyncOpen(listener, nsnull);
        return rv;
    }

    nsSyncHelper *helper = new nsSyncHelper();
    if (!helper)
        return NS_ERROR_OUT_OF_MEMORY;

    rv = helper->Init(NS_STATIC_CAST(nsIChannel *, this), listener);
    if (NS_FAILED(rv))
        return rv;

    // spin up a new helper thread.  XXX HTTP IS NOT THREAD-SAFE !!!
    nsCOMPtr<nsIThread> helperThread;
    return NS_NewThread(getter_AddRefs(helperThread), NS_STATIC_CAST(nsIRunnable *, helper));
}

NS_IMETHODIMP
nsHTTPChannel::AsyncOpen(nsIStreamListener *aListener, nsISupports *aContext)
{
    nsresult rv = NS_OK;

    NS_ENSURE_ARG_POINTER(aListener);

    if (mResponseDataListener)
        return NS_ERROR_IN_PROGRESS;

    mResponseDataListener = new nsHTTPFinalListener(this,
                                                    aListener,
                                                    aContext);
    if (!mResponseDataListener)
        return NS_ERROR_OUT_OF_MEMORY;

    mResponseContext = aContext;

    // start loading the requested document
    Connect();
    
#ifndef MOZ_NEW_CACHE
    // If the data in the cache hasn't expired, then there's no need to talk
    // with the server.  Create a stream from the cache, synthesizing all the
    // various channel-related events.
    if (mCachedContentIsValid)
        ReadFromCache();
#endif

    return rv;
}

NS_IMETHODIMP
nsHTTPChannel::GetLoadAttributes(PRUint32 *aLoadAttributes)
{
    NS_ENSURE_ARG_POINTER(aLoadAttributes);
    *aLoadAttributes = mLoadAttributes;
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::SetLoadAttributes(PRUint32 aLoadAttributes)
{
    mLoadAttributes = aLoadAttributes;
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::GetContentType(char **aContentType)
{
    nsresult rv = NS_OK;

    NS_ENSURE_ARG_POINTER(aContentType);

    *aContentType = nsnull;

    //
    // If the content type has been returned by the server then return that...
    //
    if (mResponse) {
        rv = mResponse->GetContentType(aContentType);
        if (rv != NS_ERROR_NOT_AVAILABLE) return rv;
    }

    //
    // No response yet...  Try to determine the content-type based
    // on the file extension of the URI...
    //

    // We had to do this same hack in 4.x. Sometimes, we run an http url that
    // ends in special extensions like .dll, .exe, etc and the server doesn't
    // provide a specific content type for the document. In actuality the 
    // document is really text/html (sometimes). For these cases, we don't want
    // to ask the mime service for the content type because it will make 
    // incorrect conclusions based on the file extension. Instead, set the 
    // content type to unknown and allow our unknown content type decoder a
    // chance to sniff the data stream and conclude a content type. 
    // 
    nsCOMPtr<nsIURL> url = do_QueryInterface(mURI);
    PRBool performMimeServiceLookup = PR_TRUE;
    if (url) {
        nsXPIDLCString fileExt;
        url->GetFileExtension(getter_Copies(fileExt));
        if (fileExt && !nsCRT::strcasecmp(fileExt, "dll"))
            performMimeServiceLookup = PR_FALSE;
    }

    nsCOMPtr<nsIMIMEService> MIMEService = do_GetService(NS_MIMESERVICE_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv) && performMimeServiceLookup) {
        rv = MIMEService->GetTypeFromURI(mURI, aContentType);
        if (NS_SUCCEEDED(rv)) return rv;
        // XXX we should probably set the content-type for this http response 
        // at this stage too.
    }

    if (!*aContentType) 
        *aContentType = nsCRT::strdup(UNKNOWN_CONTENT_TYPE);

    if (!*aContentType)
        rv = NS_ERROR_OUT_OF_MEMORY;
    else
        rv = NS_OK;

    return rv;
}

NS_IMETHODIMP
nsHTTPChannel::SetContentType(const char *aContentType)
{
    nsresult rv;

    if (mResponse)
        rv = mResponse->SetContentType(aContentType);
    else
        //
        // Do not allow the content-type to be set until a response has been
        // received from the server...
        //
        rv = NS_ERROR_FAILURE;

    return rv;
}

NS_IMETHODIMP
nsHTTPChannel::GetContentLength(PRInt32 *aContentLength)
{
    if (!mResponse)
        return NS_ERROR_NOT_AVAILABLE;
    return mResponse->GetContentLength(aContentLength);
}

NS_IMETHODIMP
nsHTTPChannel::SetContentLength(PRInt32 aContentLength)
{
    NS_NOTREACHED("nsHTTPChannel::SetContentLength");
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsHTTPChannel::GetLoadGroup(nsILoadGroup **aLoadGroup)
{
    NS_ENSURE_ARG_POINTER(aLoadGroup);
    *aLoadGroup = mLoadGroup;
    NS_IF_ADDREF(*aLoadGroup);
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::SetLoadGroup(nsILoadGroup *aGroup)
{
    mLoadGroup = aGroup;
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::GetOwner(nsISupports **aOwner)
{
    NS_ENSURE_ARG_POINTER(aOwner);
    *aOwner = mOwner;
    NS_IF_ADDREF(*aOwner);
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::SetOwner(nsISupports *aOwner)
{
    mOwner = aOwner;
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::GetNotificationCallbacks(nsIInterfaceRequestor **aCallbacks)
{
    NS_ENSURE_ARG_POINTER(aCallbacks);
    *aCallbacks = mCallbacks;
    NS_IF_ADDREF(*aCallbacks);
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::SetNotificationCallbacks(nsIInterfaceRequestor *aCallbacks)
{
    nsresult rv = NS_OK;
    mCallbacks = aCallbacks;

    // Verify that the event sink is http
    if (mCallbacks) {
        mRealEventSink = do_GetInterface(mCallbacks);
        mRealPrompter = do_GetInterface(mCallbacks);
        mRealProgressEventSink = do_GetInterface(mCallbacks);
        
        rv = BuildNotificationProxies();
    }
    return rv;
}

nsresult
nsHTTPChannel::BuildNotificationProxies()
{
    nsresult rv = NS_OK;

    NS_WITH_SERVICE(nsIEventQueueService, eventQService, kEventQueueServiceCID, &rv); 
    if (NS_FAILED(rv)) return rv;
    
    nsCOMPtr<nsIEventQueue> eventQ;

    rv = eventQService->GetThreadEventQueue(NS_CURRENT_THREAD, getter_AddRefs(eventQ));
    if (NS_FAILED (rv)) return rv;
    
    NS_WITH_SERVICE(nsIProxyObjectManager, proxyManager, kProxyObjectManagerCID, &rv);
    if (NS_FAILED(rv)) return rv;

    if (mRealEventSink) {
        rv = proxyManager->GetProxyForObject(eventQ,
                                             NS_GET_IID(nsIHTTPEventSink),
                                             mRealEventSink,
                                             PROXY_ASYNC | PROXY_ALWAYS,
                                             getter_AddRefs(mEventSink));
        if (NS_FAILED(rv)) return rv;
    }

    if (mRealPrompter) {
        rv = proxyManager->GetProxyForObject(eventQ,
                                             NS_GET_IID(nsIPrompt),
                                             mRealPrompter,
                                             PROXY_SYNC | PROXY_ALWAYS,
                                             getter_AddRefs(mPrompter));
        if (NS_FAILED(rv)) return rv;
    }

    if (mRealProgressEventSink) {
        rv = proxyManager->GetProxyForObject(eventQ,
                                             NS_GET_IID(nsIProgressEventSink),
                                             mRealProgressEventSink,
                                             PROXY_SYNC | PROXY_ALWAYS,
                                             getter_AddRefs(mProgressEventSink));
        if (NS_FAILED(rv)) return rv;
    }
    return rv;
}

////////////////////////////////////////////////////////////////////////////////
// nsIHTTPChannel methods:

NS_IMETHODIMP
nsHTTPChannel::GetRequestHeader(nsIAtom *aHeader, char **aValue)
{
    NS_ENSURE_TRUE(mRequest, NS_ERROR_NOT_INITIALIZED);
    return mRequest->GetHeader(aHeader, aValue);
}

NS_IMETHODIMP
nsHTTPChannel::SetRequestHeader(nsIAtom *aHeader, const char *aValue)
{
    NS_ENSURE_TRUE(mRequest, NS_ERROR_NOT_INITIALIZED);
    return mRequest->SetHeader(aHeader, aValue);
}

NS_IMETHODIMP
nsHTTPChannel::GetRequestHeaderEnumerator(nsISimpleEnumerator **aResult)
{
    NS_ENSURE_TRUE(mRequest, NS_ERROR_NOT_INITIALIZED);
    return mRequest->GetHeaderEnumerator(aResult);
}

NS_IMETHODIMP
nsHTTPChannel::GetResponseHeader(nsIAtom *aHeader, char **aValue)
{
    if (mResponse)
        return mResponse->GetHeader(aHeader, aValue);
    else
        return NS_ERROR_FAILURE; // NS_ERROR_NO_RESPONSE_YET ? 
}


NS_IMETHODIMP
nsHTTPChannel::GetResponseHeaderEnumerator(nsISimpleEnumerator **aResult)
{
    nsresult rv;

    if (mResponse)
        rv = mResponse->GetHeaderEnumerator(aResult);
    else {
        *aResult = nsnull;
        rv = NS_ERROR_FAILURE; // NS_ERROR_NO_RESPONSE_YET ? 
    }
    return rv;
}

NS_IMETHODIMP
nsHTTPChannel::SetResponseHeader(nsIAtom *aHeader, const char *aValue)
{
    nsresult rv = NS_OK;

    if (mResponse) {
        // we need to set the header and ensure that 
        // observers are notified again.
        rv = mResponse->SetHeader(aHeader, aValue);
        if (NS_FAILED(rv)) return rv;

        rv = OnHeadersAvailable();
    }
    else
        rv = NS_ERROR_FAILURE;
    return rv;
}

NS_IMETHODIMP
nsHTTPChannel::GetResponseStatus(PRUint32 *aStatus)
{
    if (mResponse)
        return mResponse->GetStatus(aStatus);

    return NS_ERROR_FAILURE; // NS_ERROR_NO_RESPONSE_YET ? 
}

NS_IMETHODIMP
nsHTTPChannel::GetResponseString(char **aString) 
{
    if (mResponse)
        return mResponse->GetStatusString(aString);

    return NS_ERROR_FAILURE; // NS_ERROR_NO_RESPONSE_YET ? 
}

NS_IMETHODIMP
nsHTTPChannel::GetEventSink(nsIHTTPEventSink **aEventSink) 
{
    NS_ENSURE_ARG_POINTER(aEventSink);
    *aEventSink = mEventSink;
    NS_IF_ADDREF(*aEventSink);
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::GetResponseDataListener(nsIStreamListener **aListener)
{
    NS_ENSURE_ARG_POINTER(aListener);
    *aListener = mResponseDataListener;
    NS_IF_ADDREF(*aListener);
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::SetRequestMethod(nsIAtom *aMethod)
{
    return mRequest->SetMethod(aMethod);
}

NS_IMETHODIMP
nsHTTPChannel::GetRequestMethod(nsIAtom **aMethod)
{
    NS_ENSURE_ARG_POINTER(aMethod);
    *aMethod = mRequest->Method();
    NS_IF_ADDREF(*aMethod);
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::GetCharset(char **aString)
{
    if (!mResponse)
        return NS_ERROR_NOT_AVAILABLE;
    return mResponse->GetCharset(aString);
}

NS_IMETHODIMP
nsHTTPChannel::SetAuthTriedWithPrehost(PRBool aTried)
{
    mAuthTriedWithPrehost = aTried;
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::GetAuthTriedWithPrehost(PRBool *aTried)
{
    NS_ENSURE_ARG_POINTER(aTried);
    *aTried = mAuthTriedWithPrehost;
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::SetAuthRealm(const char *aAuthRealm)
{
    CRTFREEIF(mAuthRealm);
    if (aAuthRealm)
        mAuthRealm = nsCRT::strdup(aAuthRealm);
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::GetAuthRealm(char **aAuthRealm)
{
    NS_ENSURE_ARG_POINTER(aAuthRealm);
    *aAuthRealm = nsCRT::strdup(mAuthRealm);
    return (*aAuthRealm) ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}

// nsIInterfaceRequestor method
NS_IMETHODIMP
nsHTTPChannel::GetInterface(const nsIID &aIID, void **aResult)
{
    nsresult rv = NS_OK;

    *aResult = nsnull;

    // capture the progress event sink stuff. pass the rest through.
    if (aIID.Equals(NS_GET_IID(nsIProgressEventSink))) {
        *aResult = NS_STATIC_CAST(nsIProgressEventSink *, this);
        NS_ADDREF(this);
    } 
    else if (mCallbacks)
        rv = mCallbacks->GetInterface(aIID, aResult);
    else
        rv = NS_ERROR_NO_INTERFACE;

    return rv;
}


// nsIProgressEventSink methods
NS_IMETHODIMP
nsHTTPChannel::OnStatus(nsIRequest *request, nsISupports *aContext, 
                        nsresult aStatus, const PRUnichar *aStatusArg)
{
    nsresult rv = NS_OK;
    if (mProgressEventSink)
        rv = mProgressEventSink->OnStatus(this, aContext, aStatus, aStatusArg);
    return rv;
}

NS_IMETHODIMP
nsHTTPChannel::OnProgress(nsIRequest *request, nsISupports* aContext,
                          PRUint32 aProgress, PRUint32 aProgressMax)
{
    // These progreess notifications are coming from the raw socket and are not
    // as interesting as the progress of HTTP channel (pushing the real data
    // to the consumer)...
    //
    // HTTP pushes out progress via the response listener.
    // See ReportProgress(...)
    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// nsHTTPChannel methods:

nsresult nsHTTPChannel::Init() 
{
    // Set up a request object - later set to a clone of a default 
    // request from the handler. TODO
    mRequest = new nsHTTPRequest(mURI, mHandler);
    if (!mRequest)
         return NS_ERROR_OUT_OF_MEMORY;

    NS_ADDREF(mRequest);
    (void) mRequest->SetConnection(this);

    return NS_OK;
}


#ifdef MOZ_NEW_CACHE

nsresult
nsHTTPChannel::OpenCacheEntry()
{
    // make sure we're not abusing this function
    NS_ENSURE_TRUE(mHandler, NS_ERROR_NOT_INITIALIZED);
    NS_ENSURE_TRUE(!mCacheEntry, NS_ERROR_FAILURE);

    nsCOMPtr<nsICacheSession> session;
    nsresult rv = mHandler->GetCacheSession(getter_AddRefs(session));
    if (NS_FAILED(rv)) return rv;

    return session->AsyncOpenCacheEntry(mRequest->Spec(),
                                        nsICache::ACCESS_READ_WRITE,
                                        this);
}

#endif


// Create a cache entry for the channel's URL or retrieve an existing one.  If
// there's an existing cache entry for the current URL, confirm that it doesn't
// contain partially downloaded content.  Finally, check to see if the cache
// data has expired, in which case it may need to be revalidated with the
// server.
//
// We might call this function several times for the same request, i.e. once
// before a transport is requested and once again when it becomes available.
// In the interim, another HTTP request might have updated the cache entry, so
// we need to got through all the machinations each time.
//
nsresult
nsHTTPChannel::CheckCache()
{
    nsresult rv;
		
    // Be pessimistic: Assume cache entry has no useful data
    mCachedContentIsAvailable = mCachedContentIsValid = PR_FALSE;

    // For now, we handle only GET and HEAD requests
    nsCOMPtr<nsIAtom> httpMethod(mRequest->Method());
    nsIAtom *mP = httpMethod;

    if ((mP != nsHTTPAtoms::Get) && (mP != nsHTTPAtoms::Head))
        return NS_OK;

#ifdef MOZ_NEW_CACHE
    if (!mCacheEntry)
        return NS_OK; // failed to open a cache entry

    if (!(mCacheAccess & nsICache::ACCESS_READ))
        return NS_OK; // no data to read... must go to net

    // consider load attributes that effect storage policy
    if (mLoadAttributes & nsIChannel::CACHE_AS_FILE)
        mCacheEntry->SetStoragePolicy(nsICache::STORE_ON_DISK_AS_FILE);
    else if (mLoadAttributes & nsIChannel::INHIBIT_PERSISTENT_CACHING)
        mCacheEntry->SetStoragePolicy(nsICache::STORE_IN_MEMORY);
#else
    // If this is the first time we've been called for this channel,
    // retrieve an existing cache entry or create a new one.
    if (!mCacheEntry) {
        // Temporary code to disable cache on platforms where it is not 
        // known to work
        static NS_DEFINE_CID(kPrefServiceCID, NS_PREF_CID);
        static PRBool warnedCacheIsMissing = PR_FALSE;

        NS_WITH_SERVICE(nsIPref, prefs, kPrefServiceCID, &rv); 
        PRBool useCache = PR_FALSE;
        if (NS_SUCCEEDED(rv))
            prefs->GetBoolPref("browser.cache.enable", &useCache);
        if (!useCache)
            return NS_OK;

        // Get the cache manager service
        // TODO - we should cache this service
        NS_WITH_SERVICE(nsINetDataCacheManager, cacheManager,
                        NS_NETWORK_CACHE_MANAGER_CONTRACTID, &rv);
        if (rv == NS_ERROR_FACTORY_NOT_REGISTERED) {
            if (!warnedCacheIsMissing) {
                NS_WARNING("Unable to find network cache component. "
                           "Network performance will be diminished");
                warnedCacheIsMissing = PR_TRUE;   
            }
            return NS_OK; 
        }
        else if (NS_FAILED(rv))
            return rv;

        PRUint32 cacheFlags;
        if (mLoadAttributes & nsIChannel::CACHE_AS_FILE)
            cacheFlags = nsINetDataCacheManager::CACHE_AS_FILE;
        else if (mLoadAttributes & nsIChannel::INHIBIT_PERSISTENT_CACHING)
            cacheFlags = nsINetDataCacheManager::BYPASS_PERSISTENT_CACHE;
        else
            cacheFlags = 0;

        // Retrieve an existing cache entry or create a new one if none 
        // exists for the given URL.
        //
        // TODO - Pass in post data (or a hash of it) as the secondary key
        // argument to GetCachedNetData()
        nsXPIDLCString urlCString; 
        mURI->GetSpec(getter_Copies(urlCString));
        rv = cacheManager->GetCachedNetData(urlCString, 0, 0, cacheFlags,
                                            getter_AddRefs(mCacheEntry));
        if (NS_FAILED(rv)) return rv;

        NS_ASSERTION(mCacheEntry, "Cache manager must always return cache entry");

        if (!mCacheEntry)
            return NS_ERROR_FAILURE;
    }

    // Hook up stream as listener
    nsCOMPtr<nsIStreamAsFile> streamAsFile = do_QueryInterface(mCacheEntry);
    if (streamAsFile) {
        nsCOMPtr<nsIStreamAsFileObserver> observer;
        PRUint32 count = 0;
        mStreamAsFileObserverArray->Count(&count);
        for (PRUint32 i=0; i< count; i++) {
            mStreamAsFileObserverArray->GetElementAt(i, getter_AddRefs(observer));
            streamAsFile->AddObserver(observer);
        }
    }
#endif

    // Be pessimistic: Clear If-Modified-Since request header
    SetRequestHeader(nsHTTPAtoms::If_Modified_Since, 0);

    // If we can't use the cache data (because an end-to-end reload 
    // has been requested)
    // there's no point in inspecting it since it's about to be overwritten.
    if (mLoadAttributes & nsIChannel::FORCE_RELOAD)
        return NS_OK;

#ifdef MOZ_NEW_CACHE
    nsXPIDLCString cachedHeaders;
    rv = mCacheEntry->GetMetaDataElement("headers", getter_Copies(cachedHeaders));
    if (NS_FAILED(rv)) return rv;
#else
    // Due to architectural limitations in the cache manager, a cache entry can
    // not be accessed if it is currently being updated by another HTTP
    // request.  If that's the case, we ignore the cache entry for purposes of
    // both reading and writing.
    PRBool updateInProgress;
    mCacheEntry->GetUpdateInProgress(&updateInProgress);
    if (updateInProgress)
        return NS_OK;

    PRUint32 contentLength = 0;
    if (NS_FAILED(mCacheEntry->GetStoredContentLength(&contentLength)))
        return NS_OK; // not found in cache.
    PRBool partialFlag;
    mCacheEntry->GetPartialFlag(&partialFlag);

    // Check to see if there's content in the cache.  For now, we can't deal
    // with partial cache entries.
    if (!contentLength || partialFlag)
        return NS_OK;

    // Retrieve the HTTP headers from the cache
    nsXPIDLCString cachedHeaders;
    PRUint32 cachedHeadersLength;
    rv = mCacheEntry->GetAnnotation("HTTP headers", &cachedHeadersLength,
                                    getter_Copies(cachedHeaders));
    if (NS_FAILED(rv)) return rv;
    if (!cachedHeadersLength)
        return NS_ERROR_FAILURE;
#endif

    // Parse the cached HTTP headers
    NS_IF_RELEASE(mCachedResponse);
    mCachedResponse = new nsHTTPResponse;
    if (!mCachedResponse)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(mCachedResponse);
    nsSubsumeCStr cachedHeadersCStr(NS_CONST_CAST(char *, 
                                    NS_STATIC_CAST(const char *, cachedHeaders)),
                                    PR_FALSE);
    rv = mCachedResponse->ParseHeaders(cachedHeadersCStr);
    if (NS_FAILED(rv)) return rv;

    // If validation is inhibited, we'll just use whatever data is in
    // the cache, regardless of whether or not it has expired.
    if (mLoadAttributes & nsIChannel::VALIDATE_NEVER) {
        PR_LOG(gHTTPLog, PR_LOG_ALWAYS, 
              ("nsHTTPChannel::checkCache() [this=%x].\t"
               "obeying VALIDATE_NEVER\n", this));
        mCachedContentIsValid = PR_TRUE;
        return NS_OK;
     }
 
    // At this point, we know there is an existing, non-truncated entry in the
    // cache and that it has apparently valid HTTP headers.
    mCachedContentIsAvailable = PR_TRUE;

    // validate against expiration time
    PRBool mustRevalidate = PR_FALSE;
    PRTime expirationTime;
    mCacheEntry->GetExpirationTime(&expirationTime);
    if (!LL_IS_ZERO(expirationTime)) {
        PRTime now = PR_Now();
        if (LL_UCMP(now, >, expirationTime))
            mustRevalidate = PR_TRUE;
    }
  
    // If the must-revalidate directive is present in the cached response, data
    // must always be revalidated with the server, even if the user has
    // configured validation to be turned off.
    nsXPIDLCString header;
    mCachedResponse->GetHeader(nsHTTPAtoms::Cache_Control, 
                               getter_Copies(header));
    if (header) {
        PRInt32 offset;

        nsCAutoString cacheControlHeader(NS_STATIC_CAST(const char *, header));
        offset = cacheControlHeader.Find("must-revalidate", PR_TRUE);
        if (offset != kNotFound)
            mustRevalidate = PR_TRUE;
    }

    // XXX incorrect interpretation of the Vary header
    nsXPIDLCString varyHeader;
    mCachedResponse->GetHeader(nsHTTPAtoms::Vary, getter_Copies(varyHeader));
    if (varyHeader)
        mustRevalidate = PR_TRUE;
    
    // If the FORCE_VALIDATION flag is set, any cached data won't be used until
    // it's revalidated with the server, so there's no point in checking if it's
    // expired.
    PRBool doIfModifiedSince;
    if ((mLoadAttributes & nsIChannel::FORCE_VALIDATION) || mustRevalidate)
        doIfModifiedSince = PR_TRUE;
    else {
        doIfModifiedSince = PR_FALSE;

        // Determine if this is the first time that this cache entry has been
        // accessed in this session.
        PRBool firstAccessThisSession;
        PRTime sessionStartTime, lastAccessTime;
#ifdef MOZ_NEW_CACHE
        mCacheEntry->GetLastFetched(&lastAccessTime);
#else
        PRTime  lastUpdateTime;
        mCacheEntry->GetLastAccessTime(&lastAccessTime);
        mCacheEntry->GetLastUpdateTime(&lastUpdateTime);
#endif

        sessionStartTime = mHandler->GetSessionStartTime();
        if (LL_UCMP(sessionStartTime, > ,lastAccessTime))
            firstAccessThisSession = PR_TRUE;

#ifndef MOZ_NEW_CACHE  // XXX what is the logic here?
        else
            firstAccessThisSession = LL_UCMP(lastUpdateTime, >= ,lastAccessTime);
            //-dp- this should be > as == means that this is the second access for this session.
#endif 

        // Check to see if we can use the cache data without revalidating 
        // it with the server.
        PRBool useHeuristicExpiration = mLoadAttributes & 
            nsIChannel::VALIDATE_HEURISTICALLY;
        PRBool cacheEntryIsStale;
        cacheEntryIsStale = mCachedResponse->IsStale(useHeuristicExpiration);

        // If the content is stale, issue an if-modified-since request
        if (cacheEntryIsStale) {
            if (mLoadAttributes & nsIChannel::VALIDATE_ONCE_PER_SESSION)
                doIfModifiedSince = firstAccessThisSession;
            else
                // VALIDATE_ALWAYS || VALIDATE_HEURISTICALLY || default
                doIfModifiedSince = PR_TRUE;
        }
    }

    // If there is no if-modified header, we will refetch This will cause refetch of cgi's
    // on link click and url typein
    nsXPIDLCString lastModified;
    mCachedResponse->GetHeader(nsHTTPAtoms::Last_Modified,
                               getter_Copies(lastModified));

    if (doIfModifiedSince) {
        if (lastModified)
            SetRequestHeader(nsHTTPAtoms::If_Modified_Since, lastModified);

        // Add If-Match header
        nsXPIDLCString etag;
        mCachedResponse->GetHeader(nsHTTPAtoms::ETag, getter_Copies(etag));
        if (etag)
            SetRequestHeader(nsHTTPAtoms::If_None_Match, etag);
    }

    if (!lastModified)
        // This is most probably a cgi. Refetch unconditionally.
        mCachedContentIsValid = PR_FALSE;
    else
        mCachedContentIsValid = !doIfModifiedSince;

    return NS_OK;
}

// If the data in the cache hasn't expired, then there's no need to
// talk with the server, not even to do an if-modified-since.  This
// method creates a stream from the cache, synthesizing all the various
// channel-related events.
nsresult
nsHTTPChannel::ReadFromCache()
{
    nsresult rv;

    NS_ASSERTION(mCacheEntry && mCachedContentIsValid && mCachedResponse,
                 "Attempting to read from cache when content is not valid");
    if (!mCacheEntry || !mCachedContentIsValid || !mCachedResponse)
        return NS_ERROR_FAILURE;

    NS_ASSERTION(mResponseDataListener, 
        "Attempt to retrieve cache data before AsyncOpen or Open");
    if (!mResponseDataListener)
        return NS_ERROR_FAILURE;

    LOG(("nsHTTPChannel::ReadFromCache [this=%x].\tUsing cache copy for: %s\n",
        this, mRequest->Spec()));

#ifdef MOZ_NEW_CACHE
    // Get a transport to the cached data...
    rv = mCacheEntry->GetTransport(getter_AddRefs(mCacheTransport));
#else
    // Create a cache transport to read the cached response...
    rv = mCacheEntry->NewChannel(mLoadGroup, getter_AddRefs(mCacheChannel));
    if (NS_FAILED(rv)) return rv;

    // Propagate the load attributes of this channel into the cache channel.
    // This will ensure that notifications are suppressed if necessary.
    rv = mCacheChannel->SetLoadAttributes(mLoadAttributes);
    if (NS_FAILED(rv)) return rv;
#endif

    // Fake it so that HTTP headers come from cached versions
    SetResponse(mCachedResponse);
    
    // Create a listener that intercepts cache reads and fires off 
    // the appropriate events such as OnHeadersAvailable
    nsHTTPResponseListener* listener;
    listener = new nsHTTPCacheListener(this, mHandler);
    if (!listener)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(listener);

    listener->SetListener(mResponseDataListener);
    mConnected = PR_TRUE;

    // Fire all the normal events for header arrival
    FinishedResponseHeaders();

    // Pump the cache data downstream
#ifdef MOZ_NEW_CACHE
    rv = mCacheTransport->AsyncRead(listener, mResponseContext,
                                    0, ULONG_MAX, 0,
                                    getter_AddRefs(mCacheReadRequest));
#else
    rv = mCacheChannel->AsyncOpen(listener, mResponseContext);
#endif

    NS_RELEASE(listener);
    if (NS_FAILED(rv))
        ResponseCompleted(nsnull, rv, nsnull);
    return rv;
}

nsresult
nsHTTPChannel::CacheAbort(PRUint32 statusCode)
{
    nsresult rv = NS_OK;
    if (mCacheEntry) {
#ifdef MOZ_NEW_CACHE
        // Doom the cache entry.
        rv = mCacheEntry->Doom();
#else
        // Set the stored content length to zero
        rv = mCacheEntry->SetStoredContentLength(0);

        if (NS_FAILED(rv)) {
            // HACK HACK HACK
            // Set stored content length usually fails if there is no file. That would
            // mean this might be an entry never flushed to disk and we have an error.
            // We got two ways to go:
            //	a. Delete() this entry. But then deleting this entry from all the tables
            //     that it is in is hard and risky. APIs arent set for us to call this.
            //  b. Mark this entry as zero length. This is what we do for other entries
            //     that are already in cache and now we have an error. The only thing
            //     is to ensure we commit this to disk. If not, there would be no record
            //     associated with this recordID and that would cause all kinds of problems
            //     during evict. To make that happen, we toggle the updateInProgress flag.
            mCacheEntry->SetUpdateInProgress(PR_TRUE);
            mCacheEntry->SetUpdateInProgress(PR_FALSE);
        }
#endif

        // Release our reference to it.
        mCacheEntry = nsnull;
    }
    return rv;
}


// Cache the network response from the server, including both the content and
// the HTTP headers.
nsresult
nsHTTPChannel::CacheReceivedResponse(nsIStreamListener *aListener, 
                                     nsIStreamListener **aResult)
{
    nsresult rv;
    NS_ENSURE_ARG_POINTER(aListener);

    // If caching is disabled, there will be no cache entry
    if (!mCacheEntry)
        return NS_OK;

    nsCOMPtr<nsISupports> securityInfo;

#ifdef MOZ_NEW_CACHE
    // Store secure data in memory only
    rv = GetSecurityInfo(getter_AddRefs(securityInfo));
    if (NS_SUCCEEDED(rv) && securityInfo)
        ; // store in memory 
#else
    // ruslan/hack: don't cache secure connections in case of the persistent cache
    PRBool dontCache = (mCacheEntry->GetSecurityInfo(getter_AddRefs(securityInfo))
                    == NS_ERROR_NOT_IMPLEMENTED);

    if (NS_SUCCEEDED(GetSecurityInfo(getter_AddRefs(securityInfo))) &&
        securityInfo && dontCache)
        return NS_OK;
#endif

#ifdef MOZ_NEW_CACHE
    NS_ENSURE_TRUE(mCachedContentIsValid == PR_FALSE, NS_OK);
#else
    // XXX why is this check necessary?
    //
    // If the current response is itself from the cache rather than the network
    // server, don't allow it to overwrite itself.
    if (mCachedContentIsValid)
        return NS_OK;

    // Due to architectural limitations in the cache manager, a cache entry can
    // not be written to if it is currently being read or written.  In that
    // case, we silently cancel the update of the cache entry.
    PRBool inUse;
    rv = mCacheEntry->GetInUse(&inUse);
    if (NS_FAILED(rv)) return rv;
    if (inUse) {
#if defined(DEBUG_dp) || defined(DEBUG_neeti) || defined (DEBUG_gagan)
        NS_ASSERTION(inUse, "cacheEntry inUse. Cannot update with new content.");
#endif
        return NS_OK;
    }
#endif

    // The no-store directive within the 'Cache-Control:' header indicates
    // that we should not store the response in the cache
    nsXPIDLCString header;
    GetResponseHeader(nsHTTPAtoms::Cache_Control, getter_Copies(header));
    if (header) {
        PRInt32 offset;
        nsCAutoString cacheControlHeader(NS_STATIC_CAST(const char *, header));
        offset = cacheControlHeader.Find("no-store", PR_TRUE);
        if (offset != kNotFound)
            return NS_OK;
    }

    // Although 'Pragma:no-cache' is not a standard HTTP response header (it's
    // a request header), caching is inhibited when this header is present so
    // as to match existing Navigator behavior.
    GetResponseHeader(nsHTTPAtoms::Pragma, getter_Copies(header));
    if (header) {
        PRInt32 offset;
        nsCAutoString pragmaHeader(NS_STATIC_CAST(const char *, header));
        offset = pragmaHeader.Find("no-cache", PR_TRUE);
        if (offset != kNotFound)
            return NS_OK;
    }

#ifndef MOZ_NEW_CACHE
    // Inhibit any other HTTP requests from writing into this cache entry
    rv = mCacheEntry->SetUpdateInProgress(PR_TRUE);
    if (NS_FAILED(rv)) return rv;

    // For now, tell the cache that we don't support fetching of partial content
    // TODO - Set to true if server indicates that it supports byte ranges
    rv = mCacheEntry->SetAllowPartial(PR_FALSE);
    if (NS_FAILED(rv)) return rv;
#endif

    if (securityInfo)
        mCacheEntry->SetSecurityInfo(securityInfo);

    // Retrieve the value of the 'Last-Modified:' header, if present
    PRTime lastModified;
    PRBool lastModifiedHeaderIsPresent;
    rv = mResponse->ParseDateHeader(nsHTTPAtoms::Last_Modified, &lastModified,
                                    &lastModifiedHeaderIsPresent);

#ifndef MOZ_NEW_CACHE
    // Check for corrupted, missing or malformed 'LastModified:' header
    if (NS_SUCCEEDED(rv) && lastModifiedHeaderIsPresent && 
            !LL_IS_ZERO(lastModified)) {
        // Store value of 'Last-Modified:' header into cache entry, 
        // used for cache replacement policy decisions
        mCacheEntry->SetLastModifiedTime(lastModified);
    }
#endif

    // Retrieve the value of the 'Expires:' header, if present
    PRTime expires;
    PRBool expiresHeaderIsPresent;
    rv = mResponse->ParseDateHeader(nsHTTPAtoms::Expires, &expires, 
                                    &expiresHeaderIsPresent);

    // Check for corrupted 'Expires:' header
    if (NS_FAILED(rv)) return rv;

    // If there is an 'Expires:' header, tell the cache entry since it uses
    // that information for replacement policy decisions
    if (expiresHeaderIsPresent && !LL_IS_ZERO(expires))
        mCacheEntry->SetExpirationTime(expires);
    else {
        // If there is no expires header, set a "stale time" for the cache
        // entry, a heuristic time at which the cache entry is *likely* to be
        // out-of-date with respect to the version on the server.  The cache
        // uses this information for replacement policy decisions.  However, it
        // is not used for the purpose of deciding whether or not to validate
        // cache data with the origin server.

        // Get the value of the 'Date:' response header
        PRTime date;
        PRBool dateHeaderIsPresent;
        rv = mResponse->ParseDateHeader(nsHTTPAtoms::Date, &date, 
                &dateHeaderIsPresent);
        if (NS_FAILED(rv))
            return NS_ERROR_FAILURE;
        
        // Before proceeding, ensure that we don't have a bizarre 
        // last-modified time
        if (dateHeaderIsPresent && lastModifiedHeaderIsPresent && 
                LL_UCMP(date, >, lastModified)) {
            // The heuristic stale time is (Date + (Date - Last-Modified)/2)
            PRTime heuristicStaleTime;
            LL_SUB(heuristicStaleTime, date, lastModified);
            LL_USHR(heuristicStaleTime, heuristicStaleTime, 1);
            LL_ADD(heuristicStaleTime, heuristicStaleTime, date);
#ifdef MOZ_NEW_CACHE
            // XXX treat this as an estimate of the expiration time (for now)
            mCacheEntry->SetExpirationTime(heuristicStaleTime);
#else
            mCacheEntry->SetStaleTime(heuristicStaleTime);
#endif
        }
    }

    // Store the received HTTP headers in the cache
    nsCString allHeaders;
    rv = mResponse->EmitHeaders(allHeaders);
    if (NS_FAILED(rv)) return rv;
#ifdef MOZ_NEW_CACHE
    // Store all HTTP headers as a single meta data element
    rv = mCacheEntry->SetMetaDataElement("headers", allHeaders.get());
    if (NS_FAILED(rv)) return rv;

    rv = mCacheEntry->GetTransport(getter_AddRefs(mCacheTransport));
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIOutputStream> out;
    rv = mCacheTransport->OpenOutputStream(0, ULONG_MAX, 0, getter_AddRefs(out));
    if (NS_FAILED(rv)) return rv;

    // Mark entry valid inorder to allow simultaneous reading...
    rv = mCacheEntry->MarkValid();
    if (NS_FAILED(rv)) return rv;

    //
    // Insert a listener tee into the chain that copies data into the
    // transport's output stream.
    //
    nsCOMPtr<nsIStreamListenerTee> tee =
        do_CreateInstance(kStreamListenerTeeCID, &rv);
    if (NS_FAILED(rv)) return rv;

    rv = tee->Init(aListener, out);
    if (NS_FAILED(rv)) return rv;

    return CallQueryInterface(tee, aResult);
#else
    rv = mCacheEntry->SetAnnotation("HTTP headers", 
                                    allHeaders.Length() + 1,
                                    allHeaders.get());
    if (NS_FAILED(rv)) return rv;

    mCacheEntry->SetUpdateInProgress(PR_TRUE);
    // Store the HTTP content data in the cache too
    return mCacheEntry->InterceptAsyncRead(aListener, 0, aResult);
#endif
}

nsresult
nsHTTPChannel::Connect()
{
    // This function is the state machine for initiating HTTP transactions.
    //
    // States transistion in the following order:
    //
    // 1) idle (HS_IDLE)
    // 2) waiting for a cache entry (HS_WAITING_FOR_CACHE_ENTRY)
    // 3) waiting for a socket transport (HS_WAITING_FOR_OPEN)
    // 4) waiting for the server's response (HS_WAITING_FOR_RESPONSE)

    if (mConnected || (mState > HS_WAITING_FOR_OPEN))
        return NS_ERROR_ALREADY_CONNECTED;

    // Set up a new request observer and a response listener and pass 
    // to the transport
    nsresult rv = NS_OK;

    // If this is the first time, then add the channel to its load group
    if (mState == HS_IDLE) {
        if (mLoadGroup)
            mLoadGroup->AddRequest(this, nsnull);

#ifdef MOZ_NEW_CACHE
        rv = OpenCacheEntry();
        if (NS_SUCCEEDED(rv)) {
            mState = HS_WAITING_FOR_CACHE_ENTRY;
            // When the open succeeds, Connect will be re-entered.
            return NS_OK;
        }

        // Failed trying to open a cache entry for this channel.
        // XXX Fallback to skipping the cache altogether.
#else
        // See if there's a cache entry for the given URL
        CheckCache();

        // If the data in the cache is usable, i.e it hasn't expired, then
        // there's no need to request a socket transport.
        if (mCachedContentIsValid)
            return NS_OK;
#endif
    }

#ifdef MOZ_NEW_CACHE
    if (mState == HS_WAITING_FOR_CACHE_ENTRY) {
        // So, we "may" have a cache entry now...
        rv = CheckCache();
        if (NS_FAILED(rv))
            NS_NOTREACHED("CheckCache failed... what should I do?");

        // If the data in the cache is usable, i.e it hasn't expired, then
        // there's no need to request a socket transport.
        if (mCachedContentIsValid) {
            // The channel is being restarted by the HTTP protocol handler
            // and the cache data is usable, so start pumping the data from
            // the cache...
            return ReadFromCache();
        }
    }
#else
    // If this request was deferred because there was no available socket
    // transport, check the cache again since another HTTP request could have
    // filled in the cache entry while this request was pending.
    if (mState == HS_WAITING_FOR_OPEN) {
        CheckCache();

        // If the data in the cache is usable, i.e it hasn't expired, then
        // there's no need to request a socket transport.
        if (mCachedContentIsValid) {
            // The channel is being restarted by the HTTP protocol handler
            // and the cache data is usable, so start pumping the data from
            // the cache...
            return ReadFromCache();
        }
    }
#endif

    if (mState != HS_WAITING_FOR_OPEN) {
        // Check for any modules that want to set headers before we
        // send out a request.
        NS_WITH_SERVICE(nsINetModuleMgr, pNetModuleMgr, kNetModuleMgrCID, &rv);
        if (NS_FAILED(rv)) return rv;

        nsCOMPtr<nsISimpleEnumerator> pModules;
        rv = pNetModuleMgr->EnumerateModules(
                NS_NETWORK_MODULE_MANAGER_HTTP_REQUEST_CONTRACTID,
                getter_AddRefs(pModules));
        if (NS_FAILED(rv)) return rv;

        // Go through the external modules and notify each one.
        nsCOMPtr<nsISupports> supEntry;
        rv = pModules->GetNext(getter_AddRefs(supEntry));
        while (NS_SUCCEEDED(rv)) {
            nsCOMPtr<nsINetModRegEntry> entry = do_QueryInterface(supEntry, &rv);
            if (NS_FAILED(rv)) 
                return rv;

            nsCOMPtr<nsINetNotify> syncNotifier;
            entry->GetSyncProxy(getter_AddRefs(syncNotifier));
            nsCOMPtr<nsIHTTPNotify> pNotify = do_QueryInterface(syncNotifier, &rv);

            if (NS_SUCCEEDED(rv)) {
                // send off the notification, and block.
                // make the nsIHTTPNotify api call
                pNotify->ModifyRequest((nsISupports*)(nsIRequest*)this);
                // we could do something with the return code from the external
                // module, but what????            
            }
            rv = pModules->GetNext(getter_AddRefs(supEntry)); // go around again
        }

        // if using proxy...
        nsXPIDLCString requestSpec;
        rv = mRequest->GetOverrideRequestSpec(getter_Copies(requestSpec));
        // no one has overwritten this value as yet...
        if (!requestSpec && mProxy && *mProxy && !mProxyTransparent) {
            nsXPIDLCString strurl;
            if(NS_SUCCEEDED(mURI->GetSpec(getter_Copies(strurl))))
                mRequest->SetOverrideRequestSpec(strurl);
        }

        // Check to see if an authentication header is required
        nsAuthEngine* pAuthEngine = nsnull; 
        if (NS_SUCCEEDED(mHandler->GetAuthEngine(&pAuthEngine)) && pAuthEngine) {
            nsXPIDLCString authStr;
            if (NS_SUCCEEDED(pAuthEngine->GetAuthString(mURI, getter_Copies(authStr)))) {
                if (authStr && *authStr)
                    rv = mRequest->SetHeader(nsHTTPAtoms::Authorization, authStr);
            }

            if (mProxy && *mProxy && !mProxyTransparent) {
                nsXPIDLCString proxyAuthStr;
                if (NS_SUCCEEDED(pAuthEngine->GetProxyAuthString(mProxy, 
                                 mProxyPort,
                                 getter_Copies(proxyAuthStr)))) {
                    if (proxyAuthStr && *proxyAuthStr)
                        rv = mRequest->SetHeader(
                                nsHTTPAtoms::Proxy_Authorization, 
                                proxyAuthStr);
                }
            }
        }
    }

    nsHTTPPipelinedRequest *pReq;

    if (mState != HS_WAITING_FOR_OPEN)
        mHandler->GetPipelinedRequest(this, &pReq);
    else
        pReq = mPipelinedRequest;

    if (pReq) {
        if (mState != HS_WAITING_FOR_OPEN)
            pReq->AddToPipeline(mRequest);

        if (!mPipeliningAllowed)
            pReq->SetMustCommit(PR_TRUE);

        // if a request stream was provided from SetUploadStream, use it
        rv = pReq->WriteRequest(mRequestStream);

        if (NS_ERROR_BUSY == rv) {
            if (!mPipelinedRequest) {
                mPipelinedRequest = pReq;
                NS_RELEASE(pReq);
            }

            mState = HS_WAITING_FOR_OPEN;
            return NS_OK;
        }
        
        if (!mPipelinedRequest)
            NS_RELEASE(pReq);

        if (NS_FAILED(rv)) {
            mConnected = PR_TRUE;
            ResponseCompleted(mResponseDataListener, rv, nsnull);
            return rv;
        }
    }
    
    mState = HS_WAITING_FOR_RESPONSE;
    mConnected = PR_TRUE;

    return rv;
}


nsresult nsHTTPChannel::ReportProgress(PRUint32 aProgress,
                                       PRUint32 aProgressMax)
{
    nsresult rv = NS_OK;

    /*
     * Only fire progress notifications if the URI is being loaded in 
     * the forground...
     */
    if (!(nsIChannel::LOAD_BACKGROUND & mLoadAttributes) && mProgressEventSink)
         rv = mProgressEventSink->OnProgress(this, mResponseContext, 
                                             aProgress, aProgressMax);
    return rv;
}


nsresult nsHTTPChannel::Redirect(const char *aNewLocation, 
                                 nsIChannel **aResult, PRInt32 aStatusCode)
{
    nsresult rv;
    nsCOMPtr<nsIURI> newURI;
    nsCOMPtr<nsIChannel> channel;
    PRBool  checkSecurity = PR_TRUE;

    nsXPIDLCString proxyHost;
    PRInt32 proxyPort;

    *aResult = nsnull;

    //
    // Create a new URI using the Location header and the current URL 
    // as a base ...
    //
    NS_WITH_SERVICE(nsIIOService, serv, kIOServiceCID, &rv);
    if (NS_FAILED(rv)) return rv;

    if (aStatusCode == 305) { // Use-Proxy
        newURI = mURI;

        nsCOMPtr<nsIURI> tmpURI;
        rv = serv->NewURI(aNewLocation, mURI, getter_AddRefs(tmpURI));

        if (NS_FAILED(rv))
            return rv;

        tmpURI -> GetHost (getter_Copies (proxyHost));
        tmpURI -> GetPort (&proxyPort);

        if (proxyPort == -1)
            proxyPort = 80;
    }
    else {
        rv = serv->NewURI(aNewLocation, mURI, getter_AddRefs(newURI));
        if (NS_FAILED(rv)) return rv;


        //  removing redirect.xul for loop. bugs 44153, 56523, 58658
        // XXX This is broken
#if 0
        nsXPIDLCString spec1;
        nsXPIDLCString spec2;

        mURI   -> GetSpec (getter_Copies (spec1));
        newURI -> GetSpec (getter_Copies (spec2));

        if (!PL_strcmp (spec1, spec2))
        {
        // loop detected
        // ruslan/24884
        rv = serv->NewURI(LOOPING_REDIRECT_ERROR_URI, mURI, getter_AddRefs(newURI));
        if (NS_FAILED(rv)) return rv;

        checkSecurity = PR_FALSE;
        }
#endif
    }

    //
    // Move the Reference of the old location to the new one 
    // if the new one has none
    //
    nsXPIDLCString   newref;
    nsCOMPtr<nsIURL> newurl;

    newurl = do_QueryInterface(newURI, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = newurl->GetRef(getter_Copies(newref));
        if (NS_SUCCEEDED(rv) && !newref) {
            nsXPIDLCString   baseref;
            nsCOMPtr<nsIURL> baseurl;

            baseurl = do_QueryInterface(mURI, &rv);
            if (NS_SUCCEEDED(rv)) {
                rv = baseurl->GetRef(getter_Copies(baseref));
                if (NS_SUCCEEDED(rv) && baseref) {
                    // If the old URL had a reference and the new URL does not,
                    // then move it to the new URL...
                    newurl->SetRef(baseref);                
                }
            }
        }
    }

#if defined(PR_LOGGING)
    char *newURLSpec;
    nsresult log_rv;

    newURLSpec = nsnull;
    log_rv = newURI->GetSpec(&newURLSpec);
    if (NS_FAILED(log_rv))
        newURLSpec = nsCRT::strdup("?");
    LOG(("ProcessRedirect [this=%x].\tRedirecting to: %s.\n",
        this, newURLSpec));
    nsMemory::Free(newURLSpec);
#endif

    if (checkSecurity) {
        NS_WITH_SERVICE(nsIScriptSecurityManager, securityManager, 
        NS_SCRIPTSECURITYMANAGER_CONTRACTID, &rv);
        if (NS_FAILED(rv)) return rv;
        rv = securityManager->CheckLoadURI(mOriginalURI ? mOriginalURI : mURI,
                                           newURI,
                                           nsIScriptSecurityManager::DISALLOW_FROM_MAIL);
        if (NS_FAILED(rv)) return rv;
    }

    // Add in LOAD_REPLACE to loadattributes indicate that this is a redirect
    nsLoadFlags loadFlags = mLoadAttributes;
    loadFlags |= nsIChannel::LOAD_REPLACE;

    rv = NS_OpenURI(getter_AddRefs(channel), newURI, serv, mLoadGroup,
                    mCallbacks, loadFlags);
    if (NS_FAILED(rv)) return rv;
    rv = channel->SetOriginalURI(mOriginalURI);
    if (NS_FAILED(rv)) return rv;

    if (mLoadGroup) {
        nsCOMPtr<nsIRequest> tempRequest;
        rv = mLoadGroup->GetDefaultLoadRequest(getter_AddRefs(tempRequest));
        if (NS_SUCCEEDED(rv)) {
            nsCOMPtr<nsIChannel> tempChannel = do_QueryInterface(tempRequest);
            if (tempChannel == this)
                mLoadGroup->SetDefaultLoadRequest(channel);
        }
    }

    // Convey the referrer if one was used for this channel to the next one-
    nsXPIDLCString referrer;
    GetRequestHeader(nsHTTPAtoms::Referer, getter_Copies(referrer));

    if (referrer && *referrer) {
        nsCOMPtr<nsIHTTPChannel> httpChannel = do_QueryInterface(channel);
        if (httpChannel)
            httpChannel->SetRequestHeader(nsHTTPAtoms::Referer, referrer);
    }

    if (aStatusCode == 305) { // Use Proxy
        nsCOMPtr<nsIProxy> httpProxy = do_QueryInterface(channel);

        httpProxy->SetProxyHost(proxyHost);
        httpProxy->SetProxyPort(proxyPort);
    }

    if (mResponseDataListener) {
        // Start the redirect...
        nsIStreamListener   *sl = mResponseDataListener;
        nsHTTPFinalListener *fl = NS_STATIC_CAST(nsHTTPFinalListener*, sl);
        rv = channel->AsyncOpen(fl->GetListener(), mResponseContext);
        fl->Shutdown();
    }
    else
        rv = NS_ERROR_FAILURE;

    //
    // Fire the OnRedirect(...) notification.
    //
    if (mEventSink)
        mEventSink->OnRedirect((nsISupports*)(nsIRequest*)this, newURI);

    // Null out pointers that are no longer needed...
    //
    // This will prevent the OnStopRequest(...) notification from being fired
    // for the original URL...
    //
    mResponseDataListener = 0;

    NS_ADDREF(*aResult = channel);
    return rv;
}

nsresult nsHTTPChannel::ResponseCompleted(nsIStreamListener *aListener,
                                          nsresult aStatus, const PRUnichar* aStatusArg)
{
    nsresult rv = NS_OK;

    LOG(("nsHTTPChannel::ResponseComplete() [this=%x] "
         "mDataListenet=%x, Status=%o\n",
         this, (void*)mResponseDataListener, aStatus));

#if 0
    if (NS_FAILED (status) && !mResponse)
    {
        // ruslan: must have failed during connect phase

        mCachedContentIsValid = PR_TRUE;
        nsresult rv1 = ReadFromCache ();
        
        if (NS_SUCCEEDED (rv1))
        {
            status = NS_ERROR_GENERATE_SUCCESS (NS_ERROR_MODULE_NETWORK, NS_ERROR_GET_CODE (status));
            return NS_OK;
        }
    }
#endif

    {
        // ruslan: grab the security info before the transport disappears
        nsCOMPtr<nsISupports> secInfo;
        GetSecurityInfo(getter_AddRefs(secInfo)); // this will store it
    }

    //
    // Cache
    //----------------------------------------------------------------

#ifndef MOZ_NEW_CACHE
    // Release the cache transport. This would free the entry's channelCount enabling changes
    // to the cacheEntry
    mCacheChannel = nsnull;
#endif

    if (mCacheEntry) {
        if (NS_FAILED(aStatus)) {
            // The url failed. This could be a DNS failure or server error
            // We have covered the server errors elsewhere. On DNS error, do CacheAbort().
            CacheAbort(aStatus);
        }
        else {
            // The no-store directive within the 'Cache-Control:' header indicates
            // that we should not store the response in the cache
            nsXPIDLCString header;
            PRBool dontCache = PR_FALSE;

            GetResponseHeader(nsHTTPAtoms::Cache_Control, getter_Copies(header));
            if (header) {
                PRInt32 offset;
                // XXX readable string
                nsCAutoString cacheControlHeader(NS_STATIC_CAST(const char*, header));
                offset = cacheControlHeader.Find("no-store", PR_TRUE);
                if (offset != kNotFound)
                    dontCache = PR_TRUE;
            }

            // Although 'Pragma:no-cache' is not a standard HTTP response header (it's
            // a request header), caching is inhibited when this header is present so
            // as to match existing Navigator behavior.
            if (!dontCache) {
                GetResponseHeader(nsHTTPAtoms::Pragma, getter_Copies(header));
                if (header) {
                    PRInt32 offset;
                    // XXX readable string
                    nsCAutoString pragmaHeader(NS_STATIC_CAST(const char*, header));
                    offset = pragmaHeader.Find("no-cache", PR_TRUE);
                    if (offset != kNotFound)
                        dontCache = PR_TRUE;
                }
            }
    
#ifdef MOZ_NEW_CACHE 
            mCacheEntry->Doom();
#else
            if (dontCache)
                mCacheEntry->SetStoredContentLength(0);
#endif
        }
    }

    //
    // Call the consumer OnStopRequest(...) to end the request...
    //----------------------------------------------------------------
    mStatus = aStatus; // set the channel's status based on the respone status
    if (aListener) {
        rv = aListener->OnStopRequest(this, mResponseContext, aStatus, aStatusArg);

        if (NS_FAILED(rv))
            LOG(("nsHTTPChannel::OnStopRequest(...) [this=%x]."
                 "\tOnStopRequest to consumer failed! Status:%x\n", this, rv));
    }

#ifdef MOZ_NEW_CACHE
    mCacheReadRequest = 0;
    mCacheTransport = 0;
#endif

    // Release the cache entry as soon as we are done. This helps as it can
    // flush any cache records and do maintenance. But do this only after
    // stopRequest has been fired as the stopListeners could want to use
    // the cache entry like the plugin listener.
    mCacheEntry = 0;

    //
    // After the consumer has been notified, remove the channel from its 
    // load group...  This will trigger an OnStopRequest from the load group.
    //
    
    if (mLoadGroup)
        mLoadGroup->RemoveRequest(this, nsnull, aStatus, aStatusArg);


    // Null out pointers that are no longer needed...

    // rjc says: don't null out mResponseContext;
    // it needs to be valid for the life of the channel
    //  mResponseContext = 0;

    mResponseDataListener = 0;
    NS_IF_RELEASE(mCachedResponse);

    return rv;
}

nsresult nsHTTPChannel::SetResponse(nsHTTPResponse *aResp)
{ 
    NS_IF_RELEASE(mResponse);
    mResponse = aResp;
    NS_IF_ADDREF(mResponse);
    return NS_OK;
}

nsresult nsHTTPChannel::GetResponseContext(nsISupports **aContext)
{
    NS_ENSURE_ARG_POINTER(aContext);
    *aContext = mResponseContext;
    NS_IF_ADDREF(*aContext);
    return NS_OK;
}


nsresult nsHTTPChannel::Abort()
{
    // Disconnect the consumer from this response listener...  
    // This allows the entity that follows to be discarded 
    // without notifying the consumer...
    if (mHTTPServerListener)
        mHTTPServerListener->Abort();

    // Null out pointers that are no longer needed...
    //
    // This will prevent the OnStopRequest(...) notification from being fired
    // for the original URL...
    //
    mResponseDataListener = 0;

    return NS_OK;
}


nsresult nsHTTPChannel::OnHeadersAvailable()
{
    nsresult rv = NS_OK;

    // Notify the event sink that response headers are available...
    if (mEventSink) {
        rv = mEventSink->OnHeadersAvailable((nsISupports*)(nsIRequest*)this);
        if (NS_FAILED(rv)) return rv;
    }

    // Check for any modules that want to receive headers once they've arrived.
    NS_WITH_SERVICE(nsINetModuleMgr, pNetModuleMgr, kNetModuleMgrCID, &rv);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsISimpleEnumerator> pModules;
    rv = pNetModuleMgr->EnumerateModules(
            NS_NETWORK_MODULE_MANAGER_HTTP_RESPONSE_CONTRACTID, 
            getter_AddRefs(pModules));
    if (NS_FAILED(rv)) return rv;

    // Go through the external modules and notify each one.
    nsCOMPtr<nsISupports> supEntry;
    rv = pModules->GetNext(getter_AddRefs(supEntry));
    while (NS_SUCCEEDED(rv)) {
        nsCOMPtr<nsINetModRegEntry> entry = do_QueryInterface(supEntry, &rv);
        if (NS_FAILED(rv)) return rv;

        nsCOMPtr<nsINetNotify> syncNotifier;
        entry->GetSyncProxy(getter_AddRefs(syncNotifier));
        nsCOMPtr<nsIHTTPNotify> pNotify = do_QueryInterface(syncNotifier, &rv);

        if (NS_SUCCEEDED(rv)) {
            // send off the notification, and block.
            // make the nsIHTTPNotify api call
            pNotify->AsyncExamineResponse((nsISupports*)(nsIRequest*)this);
            // we could do something with the return code from the external
            // module, but what????            
        }
        rv = pModules->GetNext(getter_AddRefs(supEntry)); // go around again
    }
    return NS_OK;
}


nsresult 
nsHTTPChannel::Authenticate(const char *aChallenge, PRBool aProxyAuth)
{
    nsresult rv = NS_ERROR_FAILURE;
    nsIChannel* channel;
    if (!aChallenge)
        return NS_ERROR_NULL_POINTER;

    NS_WITH_SERVICE(nsIIOService, serv, kIOServiceCID, &rv);
    if (NS_FAILED(rv)) return rv;

    // Flush out existing records of this URI in authengine-
    nsAuthEngine* pEngine = nsnull;
    if (NS_SUCCEEDED(mHandler->GetAuthEngine(&pEngine)))
        rv = pEngine->SetAuthString(mURI, 0);
    
    // Determine the new username password combination to use 
    nsAutoString user, passwd;
    if (!mAuthTriedWithPrehost && !aProxyAuth) { // Proxy auth's never in prehost
        nsXPIDLCString prehost;
        
        // XXX utility routine to get user/pass from prehost?
        if (NS_SUCCEEDED(rv = mURI->GetPreHost(getter_Copies(prehost)))) {
            nsXPIDLCString newUserPass;
            if ((const char*)prehost) {
                // XXX should we be un-escaping?  4.x didn't.
                rv = serv->Unescape(prehost, getter_Copies(newUserPass));
                if (NS_FAILED(rv)) return rv;
                const char *colonBlow = strchr(newUserPass, ':');
                if (colonBlow) {
                    // "user:pass"
                    user.AssignWithConversion(newUserPass,
                                            colonBlow - (const char *)newUserPass);
                    passwd.AssignWithConversion(colonBlow + 1);
                }
                else // just "user"
                    user.AssignWithConversion(newUserPass);
            }
        }
    }

    nsCAutoString authType;
    nsXPIDLCString authString;
    nsCAutoString authLine;
    nsCOMPtr<nsIAuthenticator> auth;
    nsCAutoString authRealm;

    // multiple www-auth headers case
    // go thru each to see if we support that. 
    for (const char *eol = aChallenge-1; eol != 0;) {
        const char* bol = eol+1;
        eol = PL_strchr(bol, LF);
        if (eol)
            authLine.Assign(bol, eol-bol);
        else
            authLine.Assign(bol);

        PRInt32 space = authLine.FindChar(' ');
        if (space == -1) 
            authType = authLine;
        else
            authLine.Left(authType, space);

#if defined(DEBUG_shaver) || defined(DEBUG_gagan)
        fprintf(stderr, "Auth type: \"%s\"\n", authType.get());
#endif
        // normalize to lowercase
        char *authLower = authType.ToNewCString();
        for (int i = 0; authLower[i]; i++)
            authLower[i] = tolower(authLower[i]);

        auth = do_GetServiceFromCategory("http-auth", authLower, &rv);

        nsMemory::Free(authLower); // free before checking rv

        // do we support this auth type
        if (NS_SUCCEEDED(rv)) break;
    }
    
    if (NS_FAILED(rv)) // XXX report "Authentication-type not supported: %s"
        return NS_ERROR_FAILURE;

    nsXPIDLString userBuf, passwdBuf;
    // save me, waterson!
    *getter_Copies(userBuf) = user.ToNewUnicode();
    *getter_Copies(passwdBuf) = passwd.ToNewUnicode();

    PRUint32 interactionType;
    rv = auth->GetInteractionType(&interactionType);
    if (NS_FAILED(rv))
        interactionType = nsIAuthenticator::INTERACTION_STANDARD;

    // Couldnt get one from prehost
    if (!user.Length() &&
        interactionType == nsIAuthenticator::INTERACTION_STANDARD) {
        //
        // Throw a modal dialog box asking for 
        // username, password. Prefill (!?!)
        //
        PRBool retval = PR_FALSE;
        
        // TODO localize it!
        nsAutoString message; message.AssignWithConversion("Enter username for "); 
        // later on change to only show realm and then host's info. 
        // find the realm 
        PRBool foundRealm = PR_FALSE;
        const char *realm = authLine.get() + authType.Length();
        while (realm && nsCRT::IsAsciiSpace(*realm)) realm++;

        if (nsCRT::strncasecmp(realm, "realm", 5) == 0) {
            realm += 5;
            while (realm && (nsCRT::IsAsciiSpace(*realm) || *realm == '='))
                realm++;

            const char * end = realm;
            if (*realm == '"') {
                realm++; end++;
                while (end && *end != '"') end++;
            }
            else
                while (end && !nsCRT::IsAsciiSpace(*end) && *end != ',') end++;

            if (realm != end) {
                message.AppendWithConversion(realm, end - realm);
                foundRealm = PR_TRUE;

                // Remember this realm; we will set it as an attribute of the new channel.
                authRealm.Assign(realm, end - realm);
            }
        }

        // if we couldn't find the realm, just append the whole auth line
        if (!foundRealm)
            message.AppendWithConversion(authLine);

        // but, if we did find a realm and this is the first time trying to
        // authenticate this channel (indicated by mAuthRealm == NULL), then 
        // lookup the realm in the auth engine and try to authenticate using
        // the response.
        else if (!mAuthRealm) {
            // Get the authentication string for the realm.  If not found, then
            // authString will be NULL, and we will be forced to prompt.
            if (pEngine)
                pEngine->GetAuthStringForRealm(mURI, authRealm, getter_Copies(authString));

#ifdef DEBUG_darin
            fprintf(stderr, "\n>>>>> Authentication for Realm: [realm=%s, auth=%s]\n\n", (const char*) authRealm, (const char*) authString);
#endif
        }

        // Skip prompting if we already have an authentication string.
        if (!authString || !*authString) {
            // Delay this check until we absolutely would need the prompter
            if (!mPrompter) {
                NS_WARNING("Failed to prompt for username/password: nsHTTPChannel::mPrompter == NULL");
                return NS_ERROR_FAILURE;
            }

            // get the hostname
            nsXPIDLCString hostname;
            if (NS_SUCCEEDED(mURI->GetHost(getter_Copies(hostname)))) {
                // TODO localize
                message.Append(NS_LITERAL_STRING(" at "));
                message.AppendWithConversion(hostname);
            }

            // Get url
            nsXPIDLCString urlCString; 
            mURI->GetPrePath(getter_Copies(urlCString));
                
            nsAutoString prePath; // XXX i18n
            CopyASCIItoUCS2(nsLiteralCString(
                        NS_STATIC_CAST(const char*, urlCString)), prePath);
            rv = mPrompter->PromptUsernameAndPassword(
                    nsnull,
                    message.GetUnicode(),
                    prePath.GetUnicode(),
                    nsIPrompt::SAVE_PASSWORD_PERMANENTLY,
                    getter_Copies(userBuf),
                    getter_Copies(passwdBuf),
                    &retval);
            if (NS_FAILED(rv) || !retval) // let it go on if we cancelled auth...
                return rv;
        }
    }

    // Skip authentication if we already have an authentication string.
    if (!authString || !*authString) {
        if (!userBuf && interactionType == nsIAuthenticator::INTERACTION_STANDARD) {
            /* can't proceed without at least a username, can we? */
            return NS_ERROR_FAILURE;
        }

        if (NS_FAILED(rv) ||
            NS_FAILED(rv = auth->Authenticate(mURI, "http", authLine.get(),
                                              userBuf, passwdBuf, mPrompter,
                                              getter_Copies(authString))))
            return rv;
    }

#if defined(DEBUG_shaver) || defined(DEBUG_gagan)
    fprintf(stderr, "Auth string: %s\n", (const char *)authString);
#endif

    // Construct a new channel
    // For security/privacy purposes, a response to an authenticated request is
    // not cached, except perhaps in the memory cache.
    // XXX if we had username and passwd in user-auth, and the interaction
    // XXX was standard, then it's safe to cache, I think (shaver)
    mLoadAttributes |= nsIChannel::INHIBIT_PERSISTENT_CACHING;

    // This smells like a clone function... maybe there is a 
    // benefit in doing that, think. TODO.
    rv = NS_OpenURI(&channel, // delibrately...
                    mURI, serv, mLoadGroup, mCallbacks, mLoadAttributes);
    if (NS_FAILED(rv)) return rv; 
    rv = channel->SetOriginalURI(mOriginalURI);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIHTTPChannel> httpChannel = do_QueryInterface(channel);
    NS_ASSERTION(httpChannel, "Something terrible happened..!");
    if (!httpChannel) return rv;

    // Add the authentication header.
    httpChannel->SetRequestHeader(aProxyAuth ?
            nsHTTPAtoms::Proxy_Authorization : nsHTTPAtoms::Authorization, 
            authString);

    // Let it know that we have already tried prehost stuff...
    httpChannel->SetAuthTriedWithPrehost(PR_TRUE);

    // Let it know that we are trying to access a realm
    if (!authRealm.IsEmpty())
        httpChannel->SetAuthRealm(authRealm);

    if (mResponseDataListener) {
        // Fire the new request...
        nsIStreamListener   *sl = mResponseDataListener;
        nsHTTPFinalListener *fl = NS_STATIC_CAST (nsHTTPFinalListener*, sl);
        nsCOMPtr<nsIRequest> request;
        rv = channel->AsyncOpen(fl->GetListener (), mResponseContext);
    }
    else
        rv = NS_ERROR_FAILURE;

    // Abort the current response...  This will disconnect the consumer from
    // the response listener...  Thus allowing the entity that follows to
    // be discarded without notifying the consumer...
    Abort();

    return rv;
}

nsresult 
nsHTTPChannel::FinishedResponseHeaders(void)
{
    nsresult rv;

    if (mFiredOnHeadersAvailable)
        return NS_OK;

    rv = NS_OK;

    LOG(("nsHTTPChannel::FinishedResponseHeaders [this=%x].\n",
        this));

    // Notify the consumer that headers are available...
    OnHeadersAvailable();
    mFiredOnHeadersAvailable = PR_TRUE;

    // if its a head request dont call processStatusCode but wrap up instead
    nsCOMPtr<nsIAtom> method(mRequest->Method());
    if (method.get() == nsHTTPAtoms::Head)
        return ResponseCompleted(mResponseDataListener, NS_OK, nsnull);

    //
    // Check the status code to see if any special processing is necessary.
    //
    // If a redirect (ie. 30x) occurs, the mResponseDataListener is
    // released and a new request is issued...
    //
    return ProcessStatusCode();
}

nsresult
nsHTTPChannel::ProcessStatusCode(void)
{
    nsresult rv = NS_OK;
    PRUint32 statusCode, statusClass;

    statusCode = 0;
    mResponse->GetStatus(&statusCode);

    NS_ASSERTION(mHandler, "HTTP handler went away");
    nsAuthEngine* pEngine;
    // We know this auth challenge response was successful. Cache any
    // authentication so that future accesses can make use of it,
    // particularly other URLs loaded from the body of this response.
    if (NS_SUCCEEDED(mHandler->GetAuthEngine(&pEngine))) {
        nsXPIDLCString authString;
        if (statusCode != 407) {
            if (mProxy && *mProxy && !mProxyTransparent) {
                rv = GetRequestHeader(nsHTTPAtoms::Proxy_Authorization,
                                getter_Copies(authString));
                if (NS_FAILED(rv)) return rv;

                pEngine->SetProxyAuthString(
                        mProxy, mProxyPort, authString);
            }
            if (mAuthTriedWithPrehost) {
                if (statusCode != 401) {
                    rv = GetRequestHeader(nsHTTPAtoms::Authorization,
                            getter_Copies(authString));
#ifdef DEBUG_darin
                    fprintf(stderr, "\n>>>>> Auth Accepted!! [realm=%s, auth=%s]\n\n", mAuthRealm, (const char*) authString);
#endif
                    if (mAuthRealm)
                        pEngine->SetAuthStringForRealm(mURI, mAuthRealm, authString);
                    else
                        pEngine->SetAuthString(mURI, authString);
                }
                else { // clear out entry from single signon and our cache. 
#ifdef DEBUG_darin
                    rv = GetRequestHeader(nsHTTPAtoms::Authorization,
                            getter_Copies(authString));
                    fprintf(stderr, "\n>>>>> Auth Rejected!! [realm=%s, auth=%s]\n\n", mAuthRealm, (const char*) authString);
#endif
                    pEngine->SetAuthString(mURI, 0);

                    NS_WITH_SERVICE(nsIWalletService, walletService, 
                    kWalletServiceCID, &rv);

                    if (NS_SUCCEEDED(rv)) {
                        NS_WITH_SERVICE(nsIProxyObjectManager, pom, 
                        kProxyObjectManagerCID, &rv);

                        nsCOMPtr<nsIWalletService> pWalletService;
                        if (NS_SUCCEEDED(pom->GetProxyForObject(NS_UI_THREAD_EVENTQ, 
                                NS_GET_IID(nsIWalletService), walletService, 
                                PROXY_SYNC, getter_AddRefs(pWalletService)))) {
                            nsXPIDLCString uri;
                            if (NS_SUCCEEDED(mURI->GetSpec(getter_Copies(uri))))
                                pWalletService->SI_RemoveUser(uri, nsnull);
                        }
                    }
                }
            }
        }
    }

    nsCOMPtr<nsIStreamListener> listener = mResponseDataListener;

    statusClass = statusCode / 100;
    switch (statusClass) {
        //
        // Informational: 1xx
        //
    case 1:
        LOG(("ProcessStatusCode [this=%x].\tStatus - Informational: %d.\n",
            this, statusCode));
        break;

        //
        // Successful: 2xx
        //
    case 2:
        LOG(("ProcessStatusCode [this=%x].\tStatus - Successful: %d.\n",
            this, statusCode));
      
        // 200 (OK) and 203 (Non-authoritative success) results can be cached
        if ((statusCode == 200) || (statusCode == 203)) {
            nsCOMPtr<nsIStreamListener> listener2;
            rv = CacheReceivedResponse(listener, getter_AddRefs(listener2));
            if (NS_SUCCEEDED(rv) && listener2)
                listener = listener2;
        }

        if (statusCode == 204)
            LOG(("ProcessStatusCode [this=%x].\tStatus - Successful: %d.\n",
                this, statusCode));
        break;

        //
        // Redirection: 3xx
        //
    case 3:
        LOG(("ProcessStatusCode [this=%x].\tStatus - Redirection: %d.\n",
            this, statusCode));

        // A 304 (Not Modified) response to an if-modified-since request 
        // indicates that the cached response can be used.
        if (statusCode == 304) {
            rv = ProcessNotModifiedResponse(listener);
            if (NS_FAILED(rv)) return rv;
            break;
        }

        // 300 (Multiple choices) or 301 (Redirect) results can be cached
        if ((statusCode == 300) || (statusCode == 301)) {
            nsCOMPtr<nsIStreamListener> listener2;
            rv = CacheReceivedResponse(listener, getter_AddRefs(listener2));
            if (NS_SUCCEEDED(rv) && listener2)
                listener = listener2;
        }

        rv = ProcessRedirection(statusCode);
        break;

        //
        // Client Error: 4xx
        //
    case 4:
        LOG(("ProcessStatusCode [this=%x].\tStatus - Client Error: %d.\n",
            this, statusCode));
        if ((401 == statusCode) || (407 == statusCode))
            rv = ProcessAuthentication(statusCode);
        // Eliminate any cached entry when this happens
        CacheAbort(statusCode);
        break;

        //
        // Server Error: 5xo
        //
    case 5:
        LOG(("ProcessStatusCode [this=%x].\tStatus - Server Error: %d.\n",
            this, statusCode));
        break;

        //
        // Unknown Status Code category...
        //
    default:
        LOG(("ProcessStatusCode [this=%x].\tStatus - "
             "Unknown Status Code category: %d.\n", this, statusCode));
        break;
    }

    // If mResponseDataListener is null this means that the response has been
    // aborted...  So, do not update the response listener because this
    // is being discarded...
    if (mResponseDataListener && mHTTPServerListener)
        mHTTPServerListener->SetListener(listener);
    return rv;
}

nsresult
nsHTTPChannel::ProcessNotModifiedResponse(nsIStreamListener *aListener)
{
    // if there is no cached entry - bail right away
    if (!mCachedResponse)
        return NS_OK;

    nsresult rv;
    NS_ASSERTION(!mCachedContentIsValid, 
            "We should never have cached a 304 response");

    LOG(("nsHTTPChannel::ProcessNotModifiedResponse [this=%x].\t"
        "Using cache copy for: %s\n",
        this, mRequest->Spec()));

    // Orphan the current nsHTTPServerListener instance...  It will be
    // replaced with a nsHTTPCacheListener instance.
    NS_ASSERTION(mHTTPServerListener, "No nsHTTPServerResponse available!");
    if (mHTTPServerListener)
        mHTTPServerListener->Abort();

    // Update the cached headers with any more recent ones from the
    // server - see RFC2616 [13.5.3]
    nsCOMPtr<nsISimpleEnumerator> enumerator;
    rv = mResponse->GetHeaderEnumerator(getter_AddRefs(enumerator));
    if (NS_FAILED(rv)) return rv;

    mCachedResponse->UpdateHeaders(enumerator);

    // Store the updated HTTP headers in the cache
    // XXX: Should the Expires value be recaluclated too?
    nsCString allHeaders;
    rv = mCachedResponse->EmitHeaders(allHeaders);
    if (NS_FAILED(rv)) return rv;

#ifdef MOZ_NEW_CACHE
    rv = mCacheEntry->SetMetaDataElement("headers", allHeaders.get());
#else
    rv = mCacheEntry->SetAnnotation("HTTP headers", allHeaders.Length()+1, 
                                     allHeaders.get());
#endif
    if (NS_FAILED(rv)) return rv;

    // Fake it so that HTTP headers come from cached versions
    SetResponse(mCachedResponse);

#ifdef MOZ_NEW_CACHE
    rv = mCacheEntry->GetTransport(getter_AddRefs(mCacheTransport));
    if (NS_FAILED(rv)) return rv;
#else
    // Create a cache transport to read the cached response...
    rv = mCacheEntry->NewChannel(mLoadGroup, getter_AddRefs(mCacheChannel));
    if (NS_FAILED(rv)) return rv;

    // Set StreamAsFileObserver
    nsCOMPtr<nsIStreamAsFile> streamAsFile = do_QueryInterface(mCacheChannel);
    if (streamAsFile) {
        nsCOMPtr< nsIStreamAsFileObserver> observer;
        PRUint32 count = 0;
        mStreamAsFileObserverArray->Count(&count);
        for (PRUint32 i=0; i< count; i++) {
            mStreamAsFileObserverArray->GetElementAt(i, getter_AddRefs(observer));
            streamAsFile->AddObserver(observer);
        }
    }
#endif

    // Create a new HTTPCacheListener...
    nsHTTPResponseListener *cacheListener;
    cacheListener = new nsHTTPCacheListener(this, mHandler);
    if (!cacheListener)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(cacheListener);

    cacheListener->SetListener(aListener);
    mResponseDataListener = 0/* aListener */;

#ifdef MOZ_NEW_CACHE
    rv = mCacheTransport->AsyncRead(cacheListener, mResponseContext,
                                    0, ULONG_MAX, 0,
                                    getter_AddRefs(mCacheReadRequest));
#else
    rv = mCacheChannel->AsyncOpen(cacheListener, mResponseContext);
#endif
    if (NS_FAILED(rv))
        ResponseCompleted(cacheListener, rv, nsnull);
    NS_RELEASE(cacheListener);

    return rv;
}

nsresult
nsHTTPChannel::ProcessRedirection(PRInt32 aStatusCode)
{
    nsresult rv = NS_OK;
    nsXPIDLCString location;

    mResponse->GetHeader(nsHTTPAtoms::Location, getter_Copies(location));

    if ((location) && ((301 == aStatusCode) || (302 == aStatusCode) ||
                       (303 == aStatusCode) || (305 == aStatusCode))) {
        nsCOMPtr<nsIChannel> channel;

        rv = Redirect(location, getter_AddRefs(channel), aStatusCode);
        if (NS_FAILED(rv)) return rv;

        // Abort the current response...  This will disconnect the consumer from
        // the response listener...  Thus allowing the entity that follows to
        // be discarded without notifying the consumer...
        Abort();
    }
    return rv;
}

nsresult
nsHTTPChannel::ProcessAuthentication(PRInt32 aStatusCode)
{
    // 401 and 407 that's all we handle for now... 
    NS_ASSERTION((401 == aStatusCode) || (407 == aStatusCode),
                 "We don't handle other types of errors!"); 

    nsresult rv = NS_OK; // Let life go on...
    nsXPIDLCString challenge; // identifies the auth type and realm.

    if (401 == aStatusCode) {
        rv = GetResponseHeader(nsHTTPAtoms::WWW_Authenticate, 
                getter_Copies(challenge));
    }
    else if (407 == aStatusCode) {
        rv = GetResponseHeader(nsHTTPAtoms::Proxy_Authenticate, 
                getter_Copies(challenge));
    }
    else 
        return rv;

    // We can't send user-password without this challenge.
    if (NS_FAILED(rv) || !challenge || !*challenge)
        return rv;

    return Authenticate(challenge, (407 == aStatusCode));
}

// nsIProxy methods
NS_IMETHODIMP
nsHTTPChannel::GetProxyHost(char* *o_ProxyHost)
{
    return DupString(o_ProxyHost, mProxy);
}

NS_IMETHODIMP
nsHTTPChannel::SetProxyHost(const char* i_ProxyHost) 
{
    CRTFREEIF(mProxy);
    return DupString(&mProxy, i_ProxyHost);
}

NS_IMETHODIMP
nsHTTPChannel::GetProxyPort(PRInt32 *aPort)
{
    *aPort = mProxyPort;
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::SetProxyPort(PRInt32 i_ProxyPort) 
{
    mProxyPort = i_ProxyPort;
    return NS_OK;
}

NS_IMETHODIMP nsHTTPChannel::GetProxyType(char * *o_ProxyType)
{
    return DupString(o_ProxyType, mProxyType);
}

NS_IMETHODIMP nsHTTPChannel::SetProxyType(const char * i_ProxyType)
{
    if (i_ProxyType && nsCRT::strcasecmp(i_ProxyType, "socks") == 0)
        mProxyTransparent = PR_TRUE;
    else
        mProxyTransparent = PR_FALSE;
    CRTFREEIF(mProxyType);
    return DupString(&mProxyType, i_ProxyType);
}

NS_IMETHODIMP
nsHTTPChannel::SetProxyRequestURI(const char * i_Spec)
{
    return mRequest ? 
        mRequest->SetOverrideRequestSpec(i_Spec) : NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsHTTPChannel::GetProxyRequestURI(char * *o_Spec)
{
    return mRequest ? 
        mRequest->GetOverrideRequestSpec(o_Spec) : NS_ERROR_FAILURE;
}

NS_IMETHODIMP 
nsHTTPChannel::SetReferrer(nsIURI *referrer, PRUint32 referrerLevel)
{
    NS_ASSERTION(referrerLevel > 0, "No need to call SetReferrer with 0 level");
    if (referrerLevel == 0)
        return NS_OK;

    mReferrer = referrer;

    if (!referrer)
        return NS_ERROR_NULL_POINTER;

    // Referer - misspelled, but per the HTTP spec
    nsXPIDLCString spec;
    referrer->GetSpec(getter_Copies(spec));
    if (spec && (referrerLevel <= mHandler->ReferrerLevel())) {
        nsCAutoString ref(spec.get());
        nsXPIDLCString prehost; 
        referrer->GetPreHost(getter_Copies(prehost));
        if (prehost.get()) {
            PRUint32 prehostLocation = ref.Find(prehost.get(), PR_TRUE);
            PRInt32 remainingStart = prehostLocation +
                    PL_strlen(prehost.get()) + 1; // 1 for @
            ref = Substring(NS_READABLE_CAST(char, ref),
                    (PRUint32) 0,
                    (PRUint32) prehostLocation) +
                Substring(NS_READABLE_CAST(char, ref),
                    (PRUint32) remainingStart,
                    (PRUint32) ref.Length()-remainingStart);
        }

        if ((referrerLevel == nsIHTTPChannel::REFERRER_NON_HTTP) || 
                (0 == PL_strncasecmp(ref.get(), "http",4)))
            return SetRequestHeader(nsHTTPAtoms::Referer, ref.get());
    }
    return NS_OK; 
}

NS_IMETHODIMP
nsHTTPChannel::GetReferrer(nsIURI **aReferrer)
{
   NS_ENSURE_ARG_POINTER(aReferrer);

   *aReferrer = mReferrer;
   NS_IF_ADDREF(*aReferrer);
   return NS_OK;
}

nsresult DupString(char **aDest, const char *aSrc)
{
    if (!aDest)
        return NS_ERROR_NULL_POINTER;
    if (aSrc) {
        *aDest = nsCRT::strdup(aSrc);
        return (*aDest == nsnull) ? NS_ERROR_OUT_OF_MEMORY : NS_OK;
    }
    else {
        *aDest = nsnull;
        return NS_OK;
    }
}

NS_IMETHODIMP 
nsHTTPChannel::GetUsingProxy(PRBool *aUsingProxy)
{
    NS_ENSURE_ARG_POINTER(aUsingProxy);
    *aUsingProxy = (mProxy && *mProxy && !mProxyTransparent);
    return NS_OK;
}

NS_IMETHODIMP 
nsHTTPChannel::GetUsingTransparentProxy(PRBool *aUsingProxy)
{
    NS_ENSURE_ARG_POINTER(aUsingProxy);
    *aUsingProxy = (mProxy && *mProxy && mProxyTransparent);
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::GetPrompter(nsIPrompt **aPrompter)
{
    NS_ENSURE_ARG_POINTER(aPrompter);
    *aPrompter = mPrompter;
    NS_IF_ADDREF(*aPrompter);
    return NS_OK;
}

NS_IMETHODIMP 
nsHTTPChannel::GetSecurityInfo(nsISupports **aSecurityInfo)
{
    NS_ENSURE_ARG_POINTER(aSecurityInfo);

    *aSecurityInfo = nsnull;

    if (mRequest) {
        nsCOMPtr<nsITransport> trans;

        mRequest->GetTransport(getter_AddRefs(trans));
        if (trans)
            trans->GetSecurityInfo(getter_AddRefs(mSecurityInfo));
        else if (!mSecurityInfo && mCacheEntry)
            mCacheEntry->GetSecurityInfo(getter_AddRefs(mSecurityInfo));

        if (mSecurityInfo) {
            *aSecurityInfo = mSecurityInfo;
            NS_ADDREF (*aSecurityInfo);
        }
    }
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::GetUploadStream(nsIInputStream **aUploadStream)
{
    NS_ENSURE_TRUE(mRequest, NS_ERROR_NOT_INITIALIZED);
    return mRequest->GetUploadStream(aUploadStream);
}

NS_IMETHODIMP
nsHTTPChannel::SetUploadStream(nsIInputStream * aUploadStream) 
{
    NS_ENSURE_TRUE(mRequest, NS_ERROR_NOT_INITIALIZED);
    return mRequest->SetUploadStream(aUploadStream);
}


#ifndef MOZ_NEW_CACHE

NS_IMETHODIMP 
nsHTTPChannel::GetFile(nsIFile * *aFile)
{
	nsCOMPtr<nsIStreamAsFile> streamAsFile = do_QueryInterface(mCacheEntry);
	if (streamAsFile)
		return streamAsFile->GetFile(aFile);

	return NS_ERROR_FAILURE;
}

NS_IMETHODIMP 
nsHTTPChannel::AddObserver(nsIStreamAsFileObserver *aObserver) 
{
	nsCOMPtr<nsISupports> isupports = aObserver;
	PRInt32 index = 0;
	nsresult rv;
	mStreamAsFileObserverArray->GetIndexOf(isupports, &index);
	if (index == -1) {
		rv = mStreamAsFileObserverArray->AppendElement(isupports);
		if (NS_FAILED(rv)) return rv;
	}

	nsCOMPtr<nsIStreamAsFile> streamAsFile = do_QueryInterface(mCacheEntry);
	if (streamAsFile)
		return streamAsFile->AddObserver(aObserver);

	return NS_OK;
}

NS_IMETHODIMP 
nsHTTPChannel::RemoveObserver(nsIStreamAsFileObserver *aObserver)
{
	nsCOMPtr<nsISupports> isupports = aObserver;
	mStreamAsFileObserverArray->RemoveElement(isupports);
		
	nsCOMPtr<nsIStreamAsFile> streamAsFile = do_QueryInterface(mCacheEntry);
	if (streamAsFile)
		return streamAsFile->RemoveObserver(aObserver);
	
	return NS_OK;
}

#endif


NS_IMETHODIMP
nsHTTPChannel::GetApplyConversion(PRBool *aApplyConversion)
{
    NS_ENSURE_ARG_POINTER(aApplyConversion);
    *aApplyConversion = mApplyConversion;
    return NS_OK;
}

NS_IMETHODIMP
nsHTTPChannel::SetApplyConversion(PRBool aApplyConversion)
{
    mApplyConversion = aApplyConversion;
    return NS_OK;
}


#ifdef MOZ_NEW_CACHE

NS_IMETHODIMP
nsHTTPChannel::OnCacheEntryAvailable(nsICacheEntryDescriptor *entry,
                                     nsCacheAccessMode access,
                                     nsresult status)
{
    LOG(("nsHTTPChannel::OnCacheEntryAvailable [this=%x entry=%x "
         "access=%x status=%x]\n", this, entry, access, status));

    if (NS_SUCCEEDED(status)) {
        mCacheEntry = entry;
        mCacheAccess = access;
    }

    Connect(); // Advance to the next state
    return NS_OK;
}

#endif


// nsISupports implementation
NS_IMPL_THREADSAFE_ISUPPORTS3(nsSyncHelper,
                              nsIRunnable,
                              nsIStreamListener,
                              nsIStreamObserver);

// nsIRunnable implementation
NS_IMETHODIMP
nsSyncHelper::Run()
{
    nsresult rv = NS_OK;
    if (!mChannel || !mListener) return NS_ERROR_NOT_INITIALIZED;

    // create an event queue for this thread.
    
    nsCOMPtr<nsIEventQueueService> service =
        do_GetService(NS_EVENTQUEUESERVICE_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

    rv = service->CreateMonitoredThreadEventQueue();
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIEventQueue> currentThreadQ;
    rv = service->GetThreadEventQueue(NS_CURRENT_THREAD, 
                                      getter_AddRefs(currentThreadQ));
    if (NS_FAILED(rv)) return rv;
        
    // initiate the AsyncRead from this thread so events are
    // sent here for processing.
    
    rv = mChannel->AsyncOpen(this, nsnull);
    if (NS_FAILED(rv)) return rv;

    // process events until we're finished.
    PLEvent *event;
    while (mProcessing) {
        rv = currentThreadQ->WaitForEvent(&event);
        if (NS_FAILED(rv)) return rv;

        rv = currentThreadQ->HandleEvent(event);
        if (NS_FAILED(rv)) return rv;
    }

    rv = service->DestroyThreadEventQueue();
    if (NS_FAILED(rv)) return rv;

    // XXX make sure cleanup happens on the calling thread.
    return NS_OK;
}

// nsIStreamListener implementation
NS_IMETHODIMP
nsSyncHelper::OnDataAvailable(nsIRequest *request,
                              nsISupports *aContext,
                              nsIInputStream *aInStream,
                              PRUint32 aOffset, PRUint32 aCount)
{    
    return mListener->OnDataAvailable(request, aContext, aInStream,
                                      aOffset, aCount);
}


// nsIStreamObserver implementation
NS_IMETHODIMP
nsSyncHelper::OnStartRequest(nsIRequest *request, nsISupports *aContext)
{
    return mListener->OnStartRequest(request, aContext);
}

NS_IMETHODIMP
nsSyncHelper::OnStopRequest(nsIRequest *request, nsISupports *aContext,
                            nsresult aStatus, const PRUnichar* aStatusArg)
{
    mProcessing = PR_FALSE;
    return mListener->OnStopRequest(request, aContext, aStatus, aStatusArg);
}


// nsSyncHelper implementation
nsSyncHelper::nsSyncHelper()
{
    NS_INIT_REFCNT();
    mProcessing = PR_TRUE;
}

nsresult
nsSyncHelper::Init (nsIChannel* aChannel,
                   nsIStreamListener* aListener) 
{
    if (!aChannel || !aListener) 
        return NS_ERROR_FAILURE;

    mChannel  = aChannel;
    mListener = aListener;
    //mCallerEventQ = aEventQ;
    return NS_OK;
}
