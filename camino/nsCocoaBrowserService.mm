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

#import "nsCocoaBrowserService.h"
#import "DownloadFactories.h"
#import "CHBrowserView.h"

#include "nsIWindowWatcher.h"
#include "nsIWebBrowserChrome.h"
#include "nsCRT.h"
#include "nsString.h"
#include "nsIGenericFactory.h"
#include "nsIComponentRegistrar.h"
#include "nsEmbedAPI.h"
#include "nsIDownload.h"
#include "nsIExternalHelperAppService.h"

nsAlertController* nsCocoaBrowserService::sController = nsnull;
nsCocoaBrowserService* nsCocoaBrowserService::sSingleton = nsnull;
PRUint32 nsCocoaBrowserService::sNumBrowsers = 0;
PRBool nsCocoaBrowserService::sCanTerminate = PR_FALSE;

// This method should return a nsModuleComponentInfo array of
// application-provided XPCOM components to register.  The implementation
// is in AppComponents.mm.
extern const nsModuleComponentInfo* GetAppModuleComponentInfo(int* outNumComponents);

// nsCocoaBrowserService implementation
nsCocoaBrowserService::nsCocoaBrowserService()
{
  NS_INIT_ISUPPORTS();
}

nsCocoaBrowserService::~nsCocoaBrowserService()
{
}

NS_IMPL_ISUPPORTS3(nsCocoaBrowserService,
                   nsIWindowCreator,
                   nsIFactory, 
                   nsIHelperAppLauncherDialog)

/* static */
nsresult
nsCocoaBrowserService::InitEmbedding()
{
  sNumBrowsers++;
  
  if (sSingleton)
    return NS_OK;

  sSingleton = new nsCocoaBrowserService();
  if (!sSingleton)
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(sSingleton);
  
  nsCOMPtr<nsIComponentRegistrar> cr;
  NS_GetComponentRegistrar(getter_AddRefs(cr));
  if ( !cr )
    return NS_ERROR_FAILURE;

  // Register as the window creator
  nsCOMPtr<nsIWindowWatcher> watcher(do_GetService("@mozilla.org/embedcomp/window-watcher;1"));
  if (!watcher) 
    return NS_ERROR_FAILURE;
  watcher->SetWindowCreator(sSingleton);

  // Register application-provided Gecko components.  This includes our security dialog implementation.

  int numComponents;
  const nsModuleComponentInfo* componentInfo = GetAppModuleComponentInfo(&numComponents);
  nsresult rv;
  for (int i = 0; i < numComponents; ++i) {
    nsCOMPtr<nsIGenericFactory> componentFactory;
    rv = NS_NewGenericFactory(getter_AddRefs(componentFactory), &(componentInfo[i]));
    if (NS_FAILED(rv)) {
      NS_ASSERTION(PR_FALSE, "Unable to create factory for component");
      continue;
    }

    rv = cr->RegisterFactory(componentInfo[i].mCID,
                             componentInfo[i].mDescription,
                             componentInfo[i].mContractID,
                             componentFactory);
    NS_ASSERTION(NS_SUCCEEDED(rv), "Unable to register factory for component");
  }

  // replace the external helper app dialog with our own
  #define NS_HELPERAPPLAUNCHERDIALOG_CID \
          {0xf68578eb, 0x6ec2, 0x4169, {0xae, 0x19, 0x8c, 0x62, 0x43, 0xf0, 0xab, 0xe1}}
  static NS_DEFINE_CID(kHelperDlgCID, NS_HELPERAPPLAUNCHERDIALOG_CID);
  rv = cr->RegisterFactory(kHelperDlgCID, NS_IHELPERAPPLAUNCHERDLG_CLASSNAME, NS_IHELPERAPPLAUNCHERDLG_CONTRACTID,
                            sSingleton);
  
  // replace the downloader with our own which does not rely on the xpfe downlaod manager
  nsCOMPtr<nsIFactory> downloadFactory;
  rv = NewDownloadListenerFactory(getter_AddRefs(downloadFactory));
  if (NS_FAILED(rv)) return rv;
  
  static NS_DEFINE_CID(kDownloadCID, NS_DOWNLOAD_CID);
  rv = cr->RegisterFactory(kDownloadCID, "Download", NS_DOWNLOAD_CONTRACTID, downloadFactory);

  return rv;
}

/* static */
void
nsCocoaBrowserService::BrowserClosed()
{
    sNumBrowsers--;
    if (sCanTerminate && sNumBrowsers == 0) {
        // The app is terminating *and* our count dropped to 0.
        NS_IF_RELEASE(sSingleton);
        NS_TermEmbedding();
#if DEBUG
        NSLog(@"Shutting down embedding!");
#endif
    }
}

/* static */
void
nsCocoaBrowserService::TermEmbedding()
{
    sCanTerminate = PR_TRUE;
    if (sNumBrowsers == 0) {
        NS_IF_RELEASE(sSingleton);
        NS_TermEmbedding();
#if DEBUG
        NSLog(@"Shutting down embedding.");
#endif
    }
    else {
#if DEBUG
        NSLog(@"Cannot yet shut down embedding.");
#endif
        // Otherwise we cannot yet terminate.  We have to let the death of the browser windows
        // induce termination.
    }
}

#define NS_ALERT_NIB_NAME "alert"

nsAlertController* 
nsCocoaBrowserService::GetAlertController()
{
  if (!sController) {
    NSBundle* bundle = [NSBundle bundleForClass:[CHBrowserView class]];
    [bundle loadNibFile:@NS_ALERT_NIB_NAME externalNameTable:nsnull withZone:[NSApp zone]];
  }
  return sController;
}

void
nsCocoaBrowserService::SetAlertController(nsAlertController* aController)
{
  // XXX When should the controller be released?
  sController = aController;
  [sController retain];
}

// nsIFactory implementation
NS_IMETHODIMP 
nsCocoaBrowserService::CreateInstance(nsISupports *aOuter, 
                                      const nsIID & aIID, 
                                      void **aResult)
{

  NS_ENSURE_ARG_POINTER(aResult);

  /*
  if (aIID.Equals(NS_GET_IID(nsIHelperAppLauncherDialog)))
  {
  }
  */

  return sSingleton->QueryInterface(aIID, aResult);
}

NS_IMETHODIMP 
nsCocoaBrowserService::LockFactory(PRBool lock)
{
  return NS_OK;
}


// Implementation of nsIWindowCreator
/* nsIWebBrowserChrome createChromeWindow (in nsIWebBrowserChrome parent, in PRUint32 chromeFlags); */
NS_IMETHODIMP 
nsCocoaBrowserService::CreateChromeWindow(nsIWebBrowserChrome *parent, 
                                          PRUint32 chromeFlags, 
                                          nsIWebBrowserChrome **_retval)
{
  if (!parent) {
#if DEBUG
    NSLog(@"Attempt to create a new browser window with a null parent.  Should not happen in Chimera.");
#endif
    return NS_ERROR_FAILURE;
  }
    
  nsCOMPtr<nsIWindowCreator> browserChrome(do_QueryInterface(parent));
  return browserChrome->CreateChromeWindow(parent, chromeFlags, _retval);
}


//    void show( in nsIHelperAppLauncher aLauncher, in nsISupports aContext );
NS_IMETHODIMP
nsCocoaBrowserService::Show(nsIHelperAppLauncher* inLauncher, nsISupports* inContext)
{
  return inLauncher->SaveToDisk(nsnull, PR_FALSE);
}

NS_IMETHODIMP
nsCocoaBrowserService::PromptForSaveToFile(nsISupports *aWindowContext, const PRUnichar *aDefaultFile, const PRUnichar *aSuggestedFileExtension, nsILocalFile **_retval)
{
  NSString* filename = [NSString stringWithPRUnichars:aDefaultFile];
  NSSavePanel *thePanel = [NSSavePanel savePanel];
  
  // Note: although the docs for NSSavePanel specifically state "path and filename can be empty strings, but
  // cannot be nil" if you want the last used directory to persist between calls to display the save panel
  // use nil for the path given to runModalForDirectory
  int runResult = [thePanel runModalForDirectory: nil file:filename];
  if (runResult == NSOKButton) {
    // NSLog(@"Saving to %@", [thePanel filename]);
    NSString *theName = [thePanel filename];
    return NS_NewNativeLocalFile(nsDependentCString([theName fileSystemRepresentation]), PR_FALSE, _retval);
  }

  return NS_ERROR_FAILURE;
}

/* void showProgressDialog (in nsIHelperAppLauncher aLauncher, in nsISupports aContext); */
NS_IMETHODIMP
nsCocoaBrowserService::ShowProgressDialog(nsIHelperAppLauncher *aLauncher, nsISupports *aContext)
{
  NSLog(@"nsCocoaBrowserService::ShowProgressDialog");
  return NS_OK;
}
