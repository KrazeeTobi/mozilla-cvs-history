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

#include "libimg.h"
#include "nsImageNet.h"
#include "ilINetContext.h"
#include "ilIURL.h"
#include "ilINetReader.h"
#include "ilErrors.h"
#include "nsIStreamListener.h"
#include "nsIInputStream.h"
#include "nsIURL.h"
#include "nsILoadGroup.h"
#include "nsIChannel.h"
#include "nsCOMPtr.h"
#include "nsWeakPtr.h"
#include "prprf.h"


#include "nsITimer.h"
#include "nsVoidArray.h"
#include "nsString.h"
#include "prmem.h"
#include "plstr.h"

#include "nsNetUtil.h"

#include "nsCURILoader.h"
#include "nsIURIContentListener.h"
#include "nsIHttpChannel.h"
#include "nsIStreamConverterService.h"
#include "nsIPref.h"

#include "nsMimeTypes.h"

static NS_DEFINE_CID(kStreamConvServiceCID, NS_STREAMCONVERTERSERVICE_CID);
static NS_DEFINE_IID(kIURLIID, NS_IURL_IID);
static NS_DEFINE_CID(kPrefCID, NS_PREF_CID);

PRLogModuleInfo *image_net_context_async_log_module = NULL;

#define IMAGE_BUF_SIZE 4096

class ImageConsumer;

class ImageNetContextImpl : public ilINetContext {
public:
  ImageNetContextImpl(ImgCachePolicy aReloadPolicy,
                      nsISupports * aLoadContext,
                      nsReconnectCB aReconnectCallback,
                      void* aReconnectArg);
  virtual ~ImageNetContextImpl();

  NS_DECL_ISUPPORTS

  virtual ilINetContext* Clone();

  virtual ImgCachePolicy GetReloadPolicy();
  virtual ImgCachePolicy SetReloadPolicy(ImgCachePolicy ReloadPolicy);


  virtual void AddReferer(ilIURL *aUrl);

  virtual void Interrupt();

  virtual ilIURL* CreateURL(const char *aUrl, 
			    ImgCachePolicy aReloadMethod);

  virtual PRBool IsLocalFileURL(char *aAddress);
#ifdef NU_CACHE
  virtual PRBool IsURLInCache(ilIURL *aUrl);
#else /* NU_CACHE */
  virtual PRBool IsURLInMemCache(ilIURL *aUrl);

  virtual PRBool IsURLInDiskCache(ilIURL *aUrl);
#endif

  virtual int GetURL (ilIURL * aUrl, ImgCachePolicy aLoadMethod,
		      ilINetReader *aReader, PRBool IsAnimationLoop);

  virtual int GetContentLength(ilIURL * aURL);

  nsresult RemoveRequest(ImageConsumer *aConsumer);
  nsresult RequestDone(ImageConsumer *aConsumer, nsIRequest* request,
                   nsISupports* ctxt, nsresult status, const PRUnichar* aMsg);

  nsVoidArray *mRequests; // WEAK references to |ImageConsumer|s
  ImgCachePolicy mReloadPolicy;
  nsWeakPtr mLoadContext;
  nsReconnectCB mReconnectCallback;
  void* mReconnectArg;
};

class ImageConsumer : public nsIStreamListener, public nsIURIContentListener, public nsIInterfaceRequestor
{
public:
  NS_DECL_ISUPPORTS
  
  ImageConsumer(ilIURL *aURL, ImageNetContextImpl *aContext);
  
  // nsIRequestObserver methods:
  NS_DECL_NSIREQUESTOBSERVER
  NS_DECL_NSISTREAMLISTENER
  NS_DECL_NSIURICONTENTLISTENER
  NS_DECL_NSIINTERFACEREQUESTOR

  void SetKeepPumpingData(nsIRequest* request, nsISupports* context) {
    NS_ADDREF(request);
    NS_IF_RELEASE(mRequest);
    mRequest = request;

    NS_IF_ADDREF(context);
    NS_IF_RELEASE(mUserContext);
    mUserContext = context;
  }
  
  void Interrupt();

  static void KeepPumpingStream(nsITimer *aTimer, void *aClosure);

protected:
  virtual ~ImageConsumer();
  ilIURL *mURL;
  PRBool mInterrupted;
  ImageNetContextImpl *mContext;
  nsIInputStream *mStream;
  nsCOMPtr<nsITimer> mTimer;
  PRBool mFirstRead;
  char *mBuffer;
  PRInt32 mStatus;
  nsIRequest* mRequest;
  nsISupports* mUserContext;
  PRBool mIsMulti;
};

ImageConsumer::ImageConsumer(ilIURL *aURL, ImageNetContextImpl *aContext)
{
  NS_INIT_REFCNT();
  mURL = aURL;
  NS_ADDREF(mURL);
  mContext = aContext;
  NS_ADDREF(mContext);
  mInterrupted = PR_FALSE;
  mFirstRead = PR_TRUE;
  mStream = nsnull;
  mBuffer = nsnull;
  mStatus = 0;
  mRequest = nsnull;
  mUserContext = nsnull;
  mIsMulti = PR_FALSE;
}

NS_IMPL_THREADSAFE_ADDREF(ImageConsumer)
NS_IMPL_THREADSAFE_RELEASE(ImageConsumer)

NS_INTERFACE_MAP_BEGIN(ImageConsumer)
   NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIURIContentListener)
   NS_INTERFACE_MAP_ENTRY(nsIStreamListener)
   NS_INTERFACE_MAP_ENTRY(nsIRequestObserver)
   NS_INTERFACE_MAP_ENTRY(nsIInterfaceRequestor)
   NS_INTERFACE_MAP_ENTRY(nsIURIContentListener)
NS_INTERFACE_MAP_END

NS_IMETHODIMP ImageConsumer::GetInterface(const nsIID & aIID, void * *aInstancePtr)
{
   NS_ENSURE_ARG_POINTER(aInstancePtr);
   return QueryInterface(aIID, aInstancePtr);
}

// nsIURIContentListener support

NS_IMETHODIMP 
ImageConsumer::OnStartURIOpen(nsIURI* aURI, PRBool* aAbortOpen)
{
   return NS_OK;
}

NS_IMETHODIMP
ImageConsumer::GetProtocolHandler(nsIURI *aURI, nsIProtocolHandler **aProtocolHandler)
{
  *aProtocolHandler = nsnull;
  return NS_OK;
}

NS_IMETHODIMP 
ImageConsumer::GetParentContentListener(nsIURIContentListener** aParent)
{
  *aParent = nsnull;
  return NS_OK;
}

NS_IMETHODIMP 
ImageConsumer::SetParentContentListener(nsIURIContentListener* aParent)
{
  return NS_OK;
}

NS_IMETHODIMP 
ImageConsumer::GetLoadCookie(nsISupports ** aLoadCookie)
{
  nsCOMPtr<nsISupports> loadContext = do_QueryReferent(mContext->mLoadContext);
  *aLoadCookie = loadContext;
  NS_IF_ADDREF(*aLoadCookie);
  return NS_OK;
}

NS_IMETHODIMP 
ImageConsumer::SetLoadCookie(nsISupports * aLoadCookie)
{
  return NS_OK;
}

NS_IMETHODIMP 
ImageConsumer::IsPreferred(const char * aContentType,
                                nsURILoadCommand aCommand,
                                char ** aDesiredContentType,
                                PRBool * aCanHandleContent)

{
  return CanHandleContent(aContentType, aCommand, 
                          aDesiredContentType, aCanHandleContent);
}

NS_IMETHODIMP 
ImageConsumer::CanHandleContent(const char * aContentType,
                                nsURILoadCommand aCommand,
                                char ** aDesiredContentType,
                                PRBool * aCanHandleContent)

{
  // if we had a webshell or doc shell around, we'd pass this call
  // through to it...but we don't =(
  if (!nsCRT::strcasecmp(aContentType, "message/rfc822"))
    *aDesiredContentType = nsCRT::strdup("application/vnd.mozilla.xul+xml");
  // since we explicilty loaded the url, we always want to handle it!
  *aCanHandleContent = PR_TRUE;
    
  return NS_OK;
} 

NS_IMETHODIMP 
ImageConsumer::DoContent(const char * aContentType,
                      nsURILoadCommand aCommand,
                      nsIRequest * aOpenedChannel,
                      nsIStreamListener ** aContentHandler,
                      PRBool * aAbortProcess)
{
  nsresult rv = NS_OK;
  if (aAbortProcess)
    *aAbortProcess = PR_FALSE;

  nsAutoString contentType; contentType.AssignWithConversion(aContentType);

  if (contentType.EqualsWithConversion(MULTIPART_MIXED_REPLACE)
      || contentType.EqualsWithConversion(MULTIPART_MIXED)) {
    // if we're getting multipart data, we have to convert it.
    // so wedge the converter inbetween us and the consumer.
    mIsMulti= PR_TRUE;

    nsCOMPtr<nsIStreamConverterService> convServ = do_GetService(kStreamConvServiceCID, &rv);
    if (NS_FAILED(rv)) return rv;

    nsAutoString astrix; astrix.AssignWithConversion("*/*");
    return convServ->AsyncConvertData(contentType.get(),
                                   astrix.get(),
                                   NS_STATIC_CAST(nsIStreamListener*, this),
                                   nsnull /*a context?*/, aContentHandler);
  }

  QueryInterface(NS_GET_IID(nsIStreamListener), (void **) aContentHandler);
  return rv;
}


NS_IMETHODIMP
ImageConsumer::OnStartRequest(nsIRequest* aRequest, nsISupports* aContext)
{
    nsCOMPtr<nsIChannel> channel = do_QueryInterface(aRequest);

  PRUint32 httpStatus;
  if (mInterrupted) {
    mStatus = MK_INTERRUPTED;
    return NS_ERROR_ABORT;
  }

  mBuffer = (char *)PR_MALLOC(IMAGE_BUF_SIZE);
  if (mBuffer == nsnull) {
    mStatus = MK_IMAGE_LOSSAGE;
    return NS_ERROR_ABORT;
  }

  nsCOMPtr<nsIHttpChannel> pHTTPCon(do_QueryInterface(channel));
  if (pHTTPCon) {
    pHTTPCon->GetResponseStatus(&httpStatus);
    if (httpStatus == 404) {
      mStatus = MK_IMAGE_LOSSAGE;
      return NS_ERROR_ABORT;
    }
  }

  ilINetReader *reader = mURL->GetReader(); //ptn test: nsCOMPtr??
  /*nsresult err=*/ reader->FlushImgBuffer(); //flush current data in buffer before starting 

  nsresult rv = NS_OK;
  char* aContentType = NULL;
  rv = channel->GetContentType(&aContentType); //nsCRT alloc's str
  if (NS_FAILED(rv)) {
      if(aContentType){
          nsCRT::free(aContentType);
      }
      aContentType = nsCRT::strdup("unknown");        
  }
  if(nsCRT::strlen(aContentType) > 50){
      //somethings wrong. mimetype string shouldn't be this big.
      //protect us from the user.
      nsCRT::free(aContentType);
      aContentType = nsCRT::strdup("unknown"); 
  }

  if (reader->StreamCreated(mURL, aContentType) != PR_TRUE) {
    mStatus = MK_IMAGE_LOSSAGE;
    reader->StreamAbort(mStatus);
    NS_RELEASE(reader);
    nsCRT::free(aContentType);
    return NS_ERROR_ABORT;
  }
  nsCRT::free(aContentType);
  NS_RELEASE(reader);
    
  return NS_OK;
}

#define IMAGE_BUF_SIZE 4096


NS_IMETHODIMP
ImageConsumer::OnDataAvailable(nsIRequest* request, nsISupports* aContext,
        nsIInputStream *pIStream, PRUint32 offset, PRUint32 length)
{
  PRUint32 max_read=0;
  PRUint32 bytes_read = 0;
  ilINetReader *reader = mURL->GetReader();

    nsCOMPtr<nsIChannel> channel = do_QueryInterface(request);

  if (mInterrupted || mStatus != 0) {
    mStatus = MK_INTERRUPTED;
    reader->StreamAbort(mStatus);
    NS_RELEASE(reader);
    return NS_ERROR_ABORT;
  }

  nsresult err = 0;
  PRUint32 nb = 0;
  char* uriStr = NULL;
  nsCOMPtr<nsIURI> uri;

  err = channel->GetURI(getter_AddRefs(uri));
  if (NS_SUCCEEDED(err) && uri) {
      err = uri->GetSpec(&uriStr);
      if (NS_FAILED(err)){
          /* if we can't get a file spec, it is bad, very bad.*/
          mStatus = MK_INTERRUPTED;
          reader->StreamAbort(mStatus);
          NS_RELEASE(reader);
          return NS_ERROR_ABORT;
      }
  }

  do {
    err = reader->WriteReady(&max_read); //max read is most decoder can handle
    if(NS_FAILED(err))   //note length tells how much we already have.
        break;

    if(max_read < 0){
            max_read = 128;
    }

    if (max_read > (length - bytes_read)) {
      max_read = length - bytes_read;
    }

    if (max_read > IMAGE_BUF_SIZE) {
      max_read = IMAGE_BUF_SIZE;
    }
    
    // make sure there's enough data available to decode the image.
    // put test into WriteReady
    if (mFirstRead && length < 4)
      break;

    err = pIStream->Read(mBuffer,
                         max_read, &nb);

    if (err == NS_BASE_STREAM_WOULD_BLOCK) {
      NS_ASSERTION(nb == 0, "Data will be lost.");
      err = NS_OK;
      break;
    }
    if (NS_FAILED(err) || nb == 0) {
      NS_ASSERTION(nb == 0, "Data will be lost.");
      break;
    }

    bytes_read += nb;
    if (mFirstRead == PR_TRUE) {
            
      err = reader->FirstWrite((const unsigned char *)mBuffer, nb, uriStr);
      mFirstRead = PR_FALSE; //? move after err chk?
      /* 
       * If FirstWrite(...) fails then the image type
       * cannot be determined and the il_container 
       * stream functions have not been initialized!
       */
      if (NS_FAILED(err)) {
        mStatus = MK_IMAGE_LOSSAGE;
        mInterrupted = PR_TRUE;
        if(uriStr)
	        nsCRT::free(uriStr);
	    NS_RELEASE(reader);
        return NS_ERROR_ABORT;
      }
    }

    err = reader->Write((const unsigned char *)mBuffer, (int32)nb);
    if(NS_FAILED(err)){
      mStatus = MK_IMAGE_LOSSAGE;
      mInterrupted = PR_TRUE;
      if(uriStr)
	      nsCRT::free(uriStr);
	  NS_RELEASE(reader);
      return NS_ERROR_ABORT;
    }
  } while(bytes_read < length);

  if (uriStr) {
	nsCRT::free(uriStr);
  }

  if (NS_FAILED(err)) {
    mStatus = MK_IMAGE_LOSSAGE;
    mInterrupted = PR_TRUE;
  }

  if (bytes_read < length) {
    // If we haven't emptied the stream, hold onto it, because
    // we will need to read from it subsequently and we don't
    // know if we'll get a OnDataAvailable call again.
    //
    // Addref the new stream before releasing the old one,
    // in case it is the same stream!
    NS_ADDREF(pIStream);
    NS_IF_RELEASE(mStream);
    mStream = pIStream;
  } else {
    NS_IF_RELEASE(mStream);
  }
  NS_RELEASE(reader);
  return NS_OK;
}

void
ImageConsumer::KeepPumpingStream(nsITimer *aTimer, void *aClosure)
{
  ImageConsumer *consumer = (ImageConsumer *)aClosure;

  consumer->OnStopRequest(consumer->mRequest, consumer->mUserContext,
                          NS_BINDING_SUCCEEDED);
}

NS_IMETHODIMP
ImageConsumer::OnStopRequest(nsIRequest* request, nsISupports* aContext, nsresult status)
{
  if (mTimer) {
    mTimer->Cancel();
    mTimer = nsnull;
  }

  if (NS_BINDING_SUCCEEDED != status) {
    mStatus = MK_INTERRUPTED;
  }

 
  // Since we're still holding on to the stream, there's still data
  // that needs to be read. So, pump the stream ourselves.
  if((mStream != nsnull) && (status == NS_BINDING_SUCCEEDED)) {
    PRUint32 str_length;
    nsresult err = mStream->Available(&str_length);
    if (NS_SUCCEEDED(err)) {
      NS_ASSERTION((str_length > 0), "No data left in the stream!");
      err = OnDataAvailable(request, aContext, mStream, 0, str_length);  // XXX fix offset
      if (NS_SUCCEEDED(err)) {
        // If we still have the stream, there's still data to be 
        // pumped, so we set a timer to call us back again.
        if (mStream) {
          SetKeepPumpingData(request, aContext);

          nsresult rv;
          mTimer = do_CreateInstance("@mozilla.org/timer;1", &rv);
          if (NS_FAILED(rv) ||
              (NS_OK != mTimer->Init(ImageConsumer::KeepPumpingStream, this, 0))) {
            mStatus = MK_IMAGE_LOSSAGE;
            NS_RELEASE(mStream);
          }
          else {
            return NS_OK;
          }
        }
      }
      else {
        mStatus = MK_IMAGE_LOSSAGE;
        NS_IF_RELEASE(mStream);
      }
    }
    else {
      mStatus = MK_IMAGE_LOSSAGE;
      NS_IF_RELEASE(mStream);
    }
  }

  ilINetReader *reader = mURL->GetReader();
  if (0 != mStatus) {
    reader->StreamAbort(mStatus);
  }
  else {
    reader->StreamComplete(mIsMulti);
  }

  if(mIsMulti)
        mFirstRead = PR_TRUE;  //reset to read new frame

  reader->NetRequestDone(mURL, mStatus);
  NS_RELEASE(reader);
  
  return mContext->RequestDone(this, request, aContext, status, nsnull);
}

void
ImageConsumer::Interrupt()
{
  mInterrupted = PR_TRUE;
}

ImageConsumer::~ImageConsumer()
{
  if (mTimer) {
    mTimer->Cancel();
    mTimer = nsnull;
  }
  if (mContext) {
    nsresult res = mContext->RemoveRequest(this);
    if (NS_SUCCEEDED(res)) {
      // The load was canceled.
      mStatus = MK_INTERRUPTED;
      if (mURL) {
        nsCOMPtr<ilINetReader> reader( dont_AddRef( mURL->GetReader() ) );
        reader->StreamAbort(mStatus);
        reader->NetRequestDone(mURL, mStatus);
      }
    }
    NS_RELEASE(mContext);
  }
  NS_IF_RELEASE(mURL);
  NS_IF_RELEASE(mStream);
  if (mBuffer != nsnull) {
    PR_DELETE(mBuffer);
  }
  NS_IF_RELEASE(mRequest);
  NS_IF_RELEASE(mUserContext);
}

ImageNetContextImpl::ImageNetContextImpl(ImgCachePolicy aReloadPolicy,
                                         nsISupports * aLoadContext,
                                         nsReconnectCB aReconnectCallback,
                                         void* aReconnectArg)
{
  NS_INIT_REFCNT();
  mRequests = nsnull;
  mLoadContext = getter_AddRefs(NS_GetWeakReference(aLoadContext));
  mReloadPolicy = aReloadPolicy;
  mReconnectCallback = aReconnectCallback;
  mReconnectArg = aReconnectArg;
}

ImageNetContextImpl::~ImageNetContextImpl()
{
  delete mRequests;
}

NS_IMPL_THREADSAFE_ISUPPORTS1(ImageNetContextImpl, ilINetContext)

ilINetContext* 
ImageNetContextImpl::Clone()
{
  ilINetContext *cx;
  nsCOMPtr<nsISupports> loadContext = do_QueryReferent(mLoadContext);

  //mReconnectArg is ImageGroup. If GetURL is triggered
  //by timer for animation, ImageGroup may have been unloaded
  //before timer kicks off.
  //mReconnectCallback=nsnull; mReconnectArg=nsnull;

  if (NS_NewImageNetContext(&cx, loadContext, mReconnectCallback, mReconnectArg) == NS_OK)
  {
    return cx;
  }
  else {
    return nsnull;
  }
}

ImgCachePolicy 
ImageNetContextImpl::GetReloadPolicy()
{
  return mReloadPolicy;
}

ImgCachePolicy 
ImageNetContextImpl::SetReloadPolicy(ImgCachePolicy reloadpolicy)
{
  mReloadPolicy=reloadpolicy;
  return mReloadPolicy;
}

void 
ImageNetContextImpl::AddReferer(ilIURL *aUrl)
{
}

void 
ImageNetContextImpl::Interrupt()
{
  if (mRequests != nsnull) {
    int i, count = mRequests->Count();
    for (i=0; i < count; i++) {
      ImageConsumer *ic = (ImageConsumer *)mRequests->ElementAt(i);
      ic->Interrupt();
    }
  }
}

ilIURL* 
ImageNetContextImpl::CreateURL(const char *aURL, 
                               ImgCachePolicy aReloadMethod)
{
  ilIURL *url;
 
  nsCOMPtr<nsISupports> loadContext (do_QueryReferent(mLoadContext)); 
  nsCOMPtr<nsILoadGroup> group (do_GetInterface(loadContext));
  if (NS_NewImageURL(&url, aURL, group) == NS_OK)
  {
    return url;
  }
  else {
    return nsnull;
  }
}

PRBool 
ImageNetContextImpl::IsLocalFileURL(char *aAddress)
{
  if (PL_strncasecmp(aAddress, "file:", 5) == 0) {
    return PR_TRUE;
  }
  else {
    return PR_FALSE;
  }
}

#ifdef NU_CACHE
PRBool 
ImageNetContextImpl::IsURLInCache(ilIURL *aUrl)
{
  return PR_TRUE;
}
#else /* NU_CACHE */
PRBool 
ImageNetContextImpl::IsURLInMemCache(ilIURL *aUrl)
{
  return PR_FALSE;
}

PRBool 
ImageNetContextImpl::IsURLInDiskCache(ilIURL *aUrl)
{
  return PR_TRUE;
}
#endif /* NU_CACHE */

int 
ImageNetContextImpl::GetContentLength (ilIURL * aURL)
{
    return -1;
}


int 
ImageNetContextImpl::GetURL (ilIURL * aURL, 
                             ImgCachePolicy aLoadMethod,
                             ilINetReader *aReader,
                             PRBool IsAnimationLoop)
{


  NS_PRECONDITION(nsnull != aURL, "null URL");
  NS_PRECONDITION(nsnull != aReader, "null reader");
  if (aURL == nsnull || aReader == nsnull) {
    return -1;
  }

  if (mRequests == nsnull) {
    mRequests = new nsVoidArray();
    if (mRequests == nsnull) {
      // XXX Should still call exit function
      return -1;
    }
  }

  int ret;
  nsresult rv;
  nsCOMPtr<nsIURI> nsurl = do_QueryInterface(aURL, &rv);
  if (NS_FAILED(rv)) return 0;

  aURL->SetReader(aReader);
  SetReloadPolicy(aLoadMethod); 
 
  
  // Find previously created ImageConsumer if possible
  ImageConsumer *ic = new ImageConsumer(aURL, this);
  if (ic == nsnull)
    return -1;
  NS_ADDREF(ic);
        
  // See if a reconnect is being done...(XXX: hack!)
  if (mReconnectCallback == nsnull
      || !(*mReconnectCallback)(mReconnectArg, ic)) {
    // first, create a channel for the protocol....
    nsCOMPtr<nsIChannel> channel;
    nsCOMPtr<nsISupports> loadContext (do_QueryReferent(mLoadContext)); 
    nsCOMPtr<nsILoadGroup> group (do_GetInterface(loadContext));
    nsCOMPtr<nsIInterfaceRequestor> sink(do_QueryInterface(loadContext));

   nsLoadFlags flags=0;   
   if(IsAnimationLoop)
       flags |= nsIRequest::LOAD_FROM_CACHE;

    rv = NS_OpenURI(getter_AddRefs(channel), nsurl, nsnull, group, sink, flags);
    if (NS_FAILED(rv)) goto error;

    nsCOMPtr<nsIHttpChannel> httpChannel = do_QueryInterface(channel);
    if (httpChannel)
    {
        // Get the defloadRequest from the loadgroup-
        nsCOMPtr<nsIRequest> defLoadRequest;
        if (NS_SUCCEEDED(group->GetDefaultLoadRequest(
                        getter_AddRefs(defLoadRequest))) && defLoadRequest)
        {
            nsCOMPtr<nsIChannel> reqchannel = do_QueryInterface(defLoadRequest);

            // Get the referrer from the loadchannel-
            nsCOMPtr<nsIURI> referrer;
            if (NS_SUCCEEDED(reqchannel->GetURI(getter_AddRefs(referrer))))
            {
                // Set the referrer-
                httpChannel->SetReferrer(referrer, nsIHttpChannel::REFERRER_INLINES);
            }
        }
    }


    rv = channel->GetLoadFlags(&flags);
    if (NS_FAILED(rv)) goto error;

   if (aURL->GetBackgroundLoad()) 
      flags |= nsIRequest::LOAD_BACKGROUND;
       
   (void)channel->SetLoadFlags(flags);

   nsCOMPtr<nsISupports> window (do_QueryInterface(NS_STATIC_CAST(nsIStreamListener *, ic)));

    // let's try uri dispatching...
    nsCOMPtr<nsIURILoader> pURILoader (do_GetService(NS_URI_LOADER_CONTRACTID, &rv));
    if (NS_SUCCEEDED(rv)) 
    {
      nsURILoadCommand loadCmd = nsIURILoader::viewNormal;
      PRBool bIsBackground = aURL->GetBackgroundLoad();
      if (bIsBackground) {
        loadCmd = nsIURILoader::viewNormalBackground;
      }


      rv = pURILoader->OpenURI(channel, loadCmd, window);
    }
    // rv = channel->AsyncOpen(ic, nsnull);
    if (NS_FAILED(rv)) goto error;
  }

  ret = mRequests->AppendElement((void *)ic) ? 0 : -1;
  NS_RELEASE(ic); // if nothing else is holding onto it, it will remove
                  // itself from mRequests
  return ret;

error:
  NS_RELEASE(ic);
  return -1;
}

nsresult
ImageNetContextImpl::RemoveRequest(ImageConsumer *aConsumer)
{
  nsresult rv = NS_OK;
  if (mRequests) {
    rv = mRequests->RemoveElement((void *)aConsumer)?NS_OK:NS_ERROR_FAILURE;
  }
  return rv;
}

nsresult
ImageNetContextImpl::RequestDone(ImageConsumer *aConsumer, nsIRequest* request,
                 nsISupports* ctxt, nsresult status, const PRUnichar* aMsg)
{
  RemoveRequest(aConsumer);
///  if (mLoadGroup)
///    return mLoadGroup->RemoveChannel(channel, ctxt, status, aMsg);
///  else
    return NS_OK;
}

extern "C" NS_GFX_(nsresult)
NS_NewImageNetContext(ilINetContext **aInstancePtrResult,
                      nsISupports * aLoadContext, 
                      nsReconnectCB aReconnectCallback,
                      void* aReconnectArg)
{
     
  PRUint32  necko_attribs;
  ImgCachePolicy imglib_attribs = USE_IMG_CACHE;
  nsLoadFlags defchan_attribs = nsIRequest::LOAD_NORMAL;

  NS_PRECONDITION(nsnull != aInstancePtrResult, "null ptr");
  if (nsnull == aInstancePtrResult) {
    return NS_ERROR_NULL_POINTER;
  }

  if(aLoadContext){
     nsCOMPtr<nsILoadGroup> group (do_GetInterface(aLoadContext));
     /*nsresult rv = */group->GetLoadFlags(&necko_attribs);
/*
Need code to check freshness of necko cache.
*/
     nsCOMPtr<nsIRequest> defLoadRequest; 
     nsCOMPtr<nsIChannel> channel;
     if (NS_SUCCEEDED(group->GetDefaultLoadRequest(
                        getter_AddRefs(defLoadRequest))) && defLoadRequest)
     {
         channel = do_QueryInterface(defLoadRequest);
         if (channel) channel->GetLoadFlags(&defchan_attribs);
     }

#if defined( DEBUG )
     if (image_net_context_async_log_module == NULL) {
    	image_net_context_async_log_module = PR_NewLogModule("IMAGENETCTXASYNC");
     }          
#endif
     if((nsIRequest::VALIDATE_ALWAYS & defchan_attribs)||   
        (nsIRequest::INHIBIT_PERSISTENT_CACHING & defchan_attribs)||
        (nsIRequest::LOAD_BYPASS_CACHE & defchan_attribs)) {
     		imglib_attribs = DONT_USE_IMG_CACHE;
#if defined( DEBUG )
		PR_LOG(image_net_context_async_log_module, 1, ("ImageNetContextAsync: NS_NewImageNetContext: DONT_USE_IMAGE_CACHE\n"));
#endif
     }
  }

  ilINetContext *cx = new ImageNetContextImpl( imglib_attribs,
                                              aLoadContext,
                                              aReconnectCallback,
                                              aReconnectArg);
  if (cx == nsnull) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  return cx->QueryInterface(NS_GET_IID(ilINetContext), (void **) aInstancePtrResult);

}
