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
 */


#include "nsFTPChannel.h"
#include "nsIStreamListener.h"
#include "nsIServiceManager.h"
#include "nsCExternalHandlerService.h"
#include "nsIMIMEService.h"
#include "nsNetUtil.h"
#include "nsMimeTypes.h"

static NS_DEFINE_CID(kIOServiceCID, NS_IOSERVICE_CID);
 
#if defined(PR_LOGGING)
extern PRLogModuleInfo* gFTPLog;
#endif /* PR_LOGGING */

// There are two transport connections established for an 
// ftp connection. One is used for the command channel , and
// the other for the data channel. The command channel is the first
// connection made and is used to negotiate the second, data, channel.
// The data channel is driven by the command channel and is either
// initiated by the server (PORT command) or by the client (PASV command).
// Client initiation is the most common case and is attempted first.

nsFTPChannel::nsFTPChannel()
    : mLoadFlags(LOAD_NORMAL),
      mSourceOffset(0),
      mAmount(0),
      mContentLength(-1),
      mFTPState(nsnull),
      mLock(nsnull),
      mStatus(NS_OK),
      mCanceled(PR_FALSE)
{
    NS_INIT_REFCNT();
}

nsFTPChannel::~nsFTPChannel()
{
#if defined(PR_LOGGING)
    nsXPIDLCString spec;
    mURL->GetSpec(getter_Copies(spec));
    PR_LOG(gFTPLog, PR_LOG_ALWAYS, ("~nsFTPChannel() for %s", (const char*)spec));
#endif
    NS_IF_RELEASE(mFTPState);
    if (mLock) PR_DestroyLock(mLock);
}

NS_IMPL_THREADSAFE_ISUPPORTS7(nsFTPChannel,
                              nsIChannel,
                              nsIFTPChannel,
                              nsIRequest,
                              nsIInterfaceRequestor, 
                              nsIProgressEventSink,
                              nsIStreamListener,
                              nsIRequestObserver);

nsresult
nsFTPChannel::Init(nsIURI* uri)
{
    nsresult rv = NS_OK;

    // setup channel state
    mURL = uri;

    rv = mURL->GetHost(getter_Copies(mHost));
    if (NS_FAILED(rv)) return rv;

    if (!mLock) {
        mLock = PR_NewLock();
        if (!mLock) return NS_ERROR_OUT_OF_MEMORY;
    }
    return NS_OK;
}


NS_METHOD
nsFTPChannel::Create(nsISupports* aOuter, const nsIID& aIID, void* *aResult)
{
    nsFTPChannel* fc;
    NS_NEWXPCOM(fc, nsFTPChannel);
    if (!fc) return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(fc);
    nsresult rv = fc->QueryInterface(aIID, aResult);
    NS_RELEASE(fc);
    return rv;
}

////////////////////////////////////////////////////////////////////////////////
// nsIRequest methods:

// The FTP channel doesn't maintain any connection state. Nor does it
// interpret the protocol. The FTP connection thread is responsible for
// these things and thus, the FTP channel simply calls through to the 
// FTP connection thread using an xpcom proxy object to make the
// cross thread call.

NS_IMETHODIMP
nsFTPChannel::GetName(PRUnichar* *result)
{
    nsresult rv;
    nsXPIDLCString urlStr;
    rv = mURL->GetSpec(getter_Copies(urlStr));
    if (NS_FAILED(rv)) return rv;
    nsString name;
    name.AppendWithConversion(urlStr);
    *result = name.ToNewUnicode();
    return *result ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}

NS_IMETHODIMP
nsFTPChannel::IsPending(PRBool *result) {
    nsAutoLock lock(mLock);
    NS_NOTREACHED("nsFTPChannel::IsPending");
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsFTPChannel::GetStatus(nsresult *status)
{
    *status = mStatus;
    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::Cancel(nsresult status) {

    PR_LOG(gFTPLog, 
           PR_LOG_DEBUG, 
           ("nsFTPChannel::Cancel() called [this=%x, status=%x, mCanceled=%d]\n", 
            this, status, mCanceled));

    NS_ASSERTION(NS_FAILED(status), "shouldn't cancel with a success code");
    nsAutoLock lock(mLock);
    
    if (mCanceled) 
        return NS_OK;

    mCanceled = PR_TRUE;

    mStatus = status;

    if (mFTPState) 
        return mFTPState->Cancel(status);

    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::Suspend(void) {

    PR_LOG(gFTPLog, PR_LOG_DEBUG, 
           ("nsFTPChannel::Suspend() called [this=%x]\n", this));

    nsAutoLock lock(mLock);
    if (mFTPState) {
        return mFTPState->Suspend();
    }
    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::Resume(void) {

    PR_LOG(gFTPLog, PR_LOG_DEBUG, 
           ("nsFTPChannel::Resume() called [this=%x]\n", this));

    nsAutoLock lock(mLock);
    if (mFTPState) {
        return mFTPState->Resume();
    }
    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// nsIChannel methods:

NS_IMETHODIMP
nsFTPChannel::GetOriginalURI(nsIURI* *aURL)
{
    *aURL = mOriginalURI ? mOriginalURI : mURL;
    NS_ADDREF(*aURL);
    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::SetOriginalURI(nsIURI* aURL)
{
    mOriginalURI = aURL;
    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::GetURI(nsIURI* *aURL)
{
    *aURL = mURL;
    NS_ADDREF(*aURL);
    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::Open(nsIInputStream **result)
{
    NS_NOTREACHED("nsFTPChannel::Open");
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsFTPChannel::AsyncOpen(nsIStreamListener *listener, nsISupports *ctxt)
{
    nsresult rv;

    PR_LOG(gFTPLog, PR_LOG_DEBUG, ("nsFTPChannel::AsyncOpen() called\n"));

    mListener = listener;
    mUserContext = ctxt;

    // Add this request to the load group
    if (mLoadGroup) {
        rv = mLoadGroup->AddRequest(this, nsnull);
        if (NS_FAILED(rv)) return rv;
    }

    ////////////////////////////////
    //// setup the channel thread
    if (!mFTPState) {
        NS_NEWXPCOM(mFTPState, nsFtpState);
        if (!mFTPState) return NS_ERROR_OUT_OF_MEMORY;
        NS_ADDREF(mFTPState);
    }
    rv = mFTPState->Init(this, mPrompter, mAuthPrompter, mFTPEventSink);
    if (NS_FAILED(rv)) return rv;

    rv = mFTPState->Connect();
    
    return rv;
}

NS_IMETHODIMP
nsFTPChannel::GetLoadFlags(PRUint32 *aLoadFlags)
{
    *aLoadFlags = mLoadFlags;
    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::SetLoadFlags(PRUint32 aLoadFlags)
{
    mLoadFlags = aLoadFlags;
    return NS_OK;
}

// FTP does not provide a file typing mechanism. We fallback to file
// extension mapping.

NS_IMETHODIMP
nsFTPChannel::GetContentType(char* *aContentType) {
    nsresult rv = NS_OK;

    if (!aContentType) return NS_ERROR_NULL_POINTER;

    nsAutoLock lock(mLock);
    *aContentType = nsnull;
    if (mContentType.IsEmpty()) {

        nsCOMPtr<nsIMIMEService> MIMEService (do_GetService(NS_MIMESERVICE_CONTRACTID, &rv));
        if (NS_FAILED(rv)) return rv;
        rv = MIMEService->GetTypeFromURI(mURL, aContentType);
        if (NS_SUCCEEDED(rv)) {
            mContentType = *aContentType;
        } else {
            mContentType = UNKNOWN_CONTENT_TYPE;
            rv = NS_OK;
        }
    }

    if (!*aContentType) {
        *aContentType = mContentType.ToNewCString();
    }

    if (!*aContentType) return NS_ERROR_OUT_OF_MEMORY;
    PR_LOG(gFTPLog, PR_LOG_DEBUG, ("nsFTPChannel::GetContentType() returned %s\n", *aContentType));
    return rv;
}

NS_IMETHODIMP
nsFTPChannel::SetContentType(const char *aContentType)
{
    nsAutoLock lock(mLock);
    mContentType = aContentType;
    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::GetContentLength(PRInt32 *aContentLength)
{
    nsAutoLock lock(mLock);
    *aContentLength = mContentLength;
    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::SetContentLength(PRInt32 aContentLength)
{
    nsAutoLock lock(mLock);
    mContentLength = aContentLength;
    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::GetLoadGroup(nsILoadGroup* *aLoadGroup)
{
    *aLoadGroup = mLoadGroup;
    NS_IF_ADDREF(*aLoadGroup);
    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::SetLoadGroup(nsILoadGroup* aLoadGroup)
{
    mLoadGroup = aLoadGroup;
    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::GetOwner(nsISupports* *aOwner)
{
    *aOwner = mOwner.get();
    NS_IF_ADDREF(*aOwner);
    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::SetOwner(nsISupports* aOwner)
{
    mOwner = aOwner;
    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::GetNotificationCallbacks(nsIInterfaceRequestor* *aNotificationCallbacks)
{
    NS_IF_ADDREF(*aNotificationCallbacks = mCallbacks);
    return NS_OK;
}

NS_IMETHODIMP
nsFTPChannel::SetNotificationCallbacks(nsIInterfaceRequestor* aNotificationCallbacks)
{
    mCallbacks = aNotificationCallbacks;

    // FIX these should be proxies!
    if (mCallbacks) {
        (void)mCallbacks->GetInterface(NS_GET_IID(nsIProgressEventSink), 
                                       getter_AddRefs(mEventSink));

        (void)mCallbacks->GetInterface(NS_GET_IID(nsIPrompt),
                                       getter_AddRefs(mPrompter));
        NS_ASSERTION ( mPrompter, "Channel doesn't have a prompt!!!" );

        (void)mCallbacks->GetInterface(NS_GET_IID(nsIFTPEventSink),
                                       getter_AddRefs(mFTPEventSink));
        
        (void)mCallbacks->GetInterface(NS_GET_IID(nsIAuthPrompt),
                                       getter_AddRefs(mAuthPrompter));
        NS_ASSERTION ( mAuthPrompter, "Channel doesn't have an auth prompt!!!" );
    }
    return NS_OK;
}

NS_IMETHODIMP 
nsFTPChannel::GetSecurityInfo(nsISupports * *aSecurityInfo)
{
    *aSecurityInfo = nsnull;
    return NS_OK;
}

// nsIInterfaceRequestor method
NS_IMETHODIMP
nsFTPChannel::GetInterface(const nsIID &anIID, void **aResult ) {
    // capture the progress event sink stuff. pass the rest through.
    if (anIID.Equals(NS_GET_IID(nsIProgressEventSink))) {
        *aResult = NS_STATIC_CAST(nsIProgressEventSink*, this);
        NS_ADDREF(this);
        return NS_OK;
    } else {
        return mCallbacks ? mCallbacks->GetInterface(anIID, aResult) : NS_ERROR_NO_INTERFACE;
    }
}

// nsIProgressEventSink methods
NS_IMETHODIMP
nsFTPChannel::OnStatus(nsIRequest *request, nsISupports *aContext,
                       nsresult aStatus, const PRUnichar* aStatusArg)
{
    if (aStatus == NS_NET_STATUS_CONNECTED_TO)
    {
        // The state machine needs to know that the data connection
        // was successfully started so that it can issue data commands
        // securely.
        NS_ASSERTION(mFTPState, "ftp state is null.");
        (void) mFTPState->DataConnectionEstablished();
    }

    if (!mEventSink)
        return NS_OK;

    return mEventSink->OnStatus(this, mUserContext, aStatus,
                                NS_ConvertASCIItoUCS2(mHost).get());
}

NS_IMETHODIMP
nsFTPChannel::OnProgress(nsIRequest *request, nsISupports* aContext,
                                  PRUint32 aProgress, PRUint32 aProgressMax) {
    if (!mEventSink)
        return NS_OK;

    return mEventSink->OnProgress(this, mUserContext, 
                                  aProgress, (PRUint32) mContentLength);
}


// nsIRequestObserver methods.
NS_IMETHODIMP
nsFTPChannel::OnStopRequest(nsIRequest *request, nsISupports* aContext,
                            nsresult aStatus)
{
    PR_LOG(gFTPLog, 
           PR_LOG_DEBUG, 
           ("nsFTPChannel::OnStopRequest() called [this=%x, request=%x, aStatus=%x]\n", 
            this, request, aStatus));

    nsresult rv = NS_OK;
    
    mStatus = aStatus;
    
    if (mObserver) {
        rv = mObserver->OnStopRequest(this, mUserContext, aStatus);
        if (NS_FAILED(rv)) return rv;
    }

    if (mListener) {
        rv = mListener->OnStopRequest(this, mUserContext, aStatus);
        if (NS_FAILED(rv)) return rv;
    }
    if (mLoadGroup) {
        rv = mLoadGroup->RemoveRequest(this, nsnull, aStatus);
        if (NS_FAILED(rv)) return rv;
    }
    return rv;
}

NS_IMETHODIMP
nsFTPChannel::OnStartRequest(nsIRequest *request, nsISupports *aContext) 
{
    PR_LOG(gFTPLog, 
           PR_LOG_DEBUG, 
           ("nsFTPChannel::OnStartRequest() called [this=%x, request=%x]\n", 
            this, request));
   
    nsresult rv = NS_OK;
    if (mObserver) {
        rv = mObserver->OnStartRequest(this, mUserContext);
        if (NS_FAILED(rv)) return rv;
    }

    if (mListener) {
        rv = mListener->OnStartRequest(this, mUserContext);
        if (NS_FAILED(rv)) return rv;
    }
    return rv;
}


// nsIStreamListener method
NS_IMETHODIMP
nsFTPChannel::OnDataAvailable(nsIRequest *request, nsISupports* aContext,
                               nsIInputStream *aInputStream, PRUint32 aSourceOffset,
                               PRUint32 aLength) {
    return mListener->OnDataAvailable(this, mUserContext, aInputStream, aSourceOffset, aLength);
}
