
#include "prlog.h"
#include "javaDOMGlobals.h"
#include "nsIDocumentLoader.h"
#include "nsIDocumentLoaderObserver.h"
#include "nsIServiceManager.h"
#include "nsCURILoader.h"

#include "nsIJavaDOM.h"
#include "org_mozilla_dom_DOMAccessorImpl.h"

static NS_DEFINE_IID(kDocLoaderServiceCID, NS_DOCUMENTLOADER_SERVICE_CID);
static NS_DEFINE_IID(kJavaDOMCID, NS_JAVADOM_CID);

/*
 * Class:     org_mozilla_dom_DOMAccessorImpl
 * Method:    register
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_mozilla_dom_DOMAccessorImpl_register
  (JNIEnv *env, jclass jthis)
{
  if (!JavaDOMGlobals::log) {
    JavaDOMGlobals::Initialize(env);
  }
  nsresult rv = NS_OK; 
  NS_WITH_SERVICE(nsIDocumentLoader, docLoaderService, kDocLoaderServiceCID, &rv);
  if (NS_FAILED(rv) || !docLoaderService) {
    PR_LOG(JavaDOMGlobals::log, PR_LOG_ERROR, 
	   ("DOMAccessor::register: GetService(JavaDOM) failed: %x\n", 
	    rv));
  } else {
    NS_WITH_SERVICE(nsIDocumentLoaderObserver, javaDOM, kJavaDOMCID, &rv);
    if (NS_FAILED(rv) || !javaDOM) {
      PR_LOG(JavaDOMGlobals::log, PR_LOG_ERROR, 
	     ("DOMAccessor::register: GetService(JavaDOM) failed: %x\n", 
	      rv));
    } else {
      rv = docLoaderService->AddObserver((nsIDocumentLoaderObserver*)javaDOM);
      if (NS_FAILED(rv)) {
	PR_LOG(JavaDOMGlobals::log, PR_LOG_ERROR, 
	       ("DOMAccessor::register: AddObserver(JavaDOM) failed x\n", 
		rv));
      }
    }
    nsServiceManager::ReleaseService(kDocLoaderServiceCID, docLoaderService);
  }
}

/*
 * Class:     org_mozilla_dom_DOMAccessorImpl
 * Method:    unregister
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_mozilla_dom_DOMAccessorImpl_unregister
  (JNIEnv *, jclass jthis)
{
  PR_LOG(JavaDOMGlobals::log, PR_LOG_DEBUG, 
	 ("DOMAccessor::unregister: unregistering %x\n", jthis));

  nsresult rv = NS_OK;
  NS_WITH_SERVICE(nsIDocumentLoader, docLoaderService, kDocLoaderServiceCID, &rv);
  if (NS_FAILED(rv) || !docLoaderService) {
    PR_LOG(JavaDOMGlobals::log, PR_LOG_ERROR, 
	   ("DOMAccessor::unregister: GetService(DocLoaderService) failed %x\n", 
	    rv));
  } else {
    NS_WITH_SERVICE(nsIDocumentLoaderObserver, javaDOM, kJavaDOMCID, &rv);
    if (NS_FAILED(rv) || !javaDOM) {
      PR_LOG(JavaDOMGlobals::log, PR_LOG_ERROR, 
	     ("DOMAccessor::unregister: GetService(JavaDOM) failed %x\n", 
	      rv));
    } else {
      rv = docLoaderService->RemoveObserver((nsIDocumentLoaderObserver*)javaDOM);
      if (NS_FAILED(rv)) {
	PR_LOG(JavaDOMGlobals::log, PR_LOG_ERROR, 
	       ("DOMAccessor::unregister: RemoveObserver(JavaDOM) failed x\n", 
		rv));
      }
    }
    nsServiceManager::ReleaseService(kDocLoaderServiceCID, docLoaderService);
  }
}

/*
 * Class:     org_mozilla_dom_DOMAccessorImpl
 * Method:    createElement
 * Signature: (J)Lorg/w3c/dom/Element;
 */
JNIEXPORT jobject JNICALL Java_org_mozilla_dom_DOMAccessorImpl_getElementByHandle
  (JNIEnv *env, jclass jthis, jlong p)
{ 
  if (!JavaDOMGlobals::log) {
    JavaDOMGlobals::Initialize(env);
  }
  nsIDOMNode *node = (nsIDOMNode*)p;
  return JavaDOMGlobals::CreateNodeSubtype(env, node);
}
