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

#include "nsCOMPtr.h"
#include "nsFileSpec.h"
#include "nsSpecialSystemDirectory.h"
#include "nsIChromeRegistry.h"
#include "nsIRDFDataSource.h"
#include "nsIRDFObserver.h"
#include "nsIRDFRemoteDataSource.h"
#include "nsCRT.h"
#include "rdf.h"
#include "nsIServiceManager.h"
#include "nsIRDFService.h"
#include "nsRDFCID.h"
#include "nsIRDFResource.h"
#include "nsIRDFDataSource.h"
#include "nsIRDFContainer.h"
#include "nsHashtable.h"
#include "nsString.h"
#include "nsXPIDLString.h"
#include "nsISimpleEnumerator.h"
#include "nsNeckoUtil.h"

static NS_DEFINE_CID(kRDFServiceCID, NS_RDFSERVICE_CID);
static NS_DEFINE_CID(kRDFXMLDataSourceCID, NS_RDFXMLDATASOURCE_CID);

#define CHROME_NAMESPACE_URI "http://chrome.mozilla.org/rdf#"
DEFINE_RDF_VOCAB(CHROME_NAMESPACE_URI, CHROME, chrome);
DEFINE_RDF_VOCAB(CHROME_NAMESPACE_URI, CHROME, skin);
DEFINE_RDF_VOCAB(CHROME_NAMESPACE_URI, CHROME, content);
DEFINE_RDF_VOCAB(CHROME_NAMESPACE_URI, CHROME, locale);
DEFINE_RDF_VOCAB(CHROME_NAMESPACE_URI, CHROME, base);
DEFINE_RDF_VOCAB(CHROME_NAMESPACE_URI, CHROME, main);
DEFINE_RDF_VOCAB(CHROME_NAMESPACE_URI, CHROME, archive);
DEFINE_RDF_VOCAB(CHROME_NAMESPACE_URI, CHROME, theme);
DEFINE_RDF_VOCAB(CHROME_NAMESPACE_URI, CHROME, name);

// This nasty function should disappear when we land Necko completely and 
// change chrome://global/skin/foo to chrome://skin@global/foo
//
void BreakProviderAndRemainingFromPath(const char* i_path, char** o_provider, char** o_remaining);

////////////////////////////////////////////////////////////////////////////////
class nsOverlayEnumerator : public nsISimpleEnumerator
{
public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSISIMPLEENUMERATOR

    nsOverlayEnumerator(nsISimpleEnumerator *aArcs);
    virtual ~nsOverlayEnumerator();

private:
    nsCOMPtr<nsISimpleEnumerator> mArcs;
};

NS_IMPL_ISUPPORTS1(nsOverlayEnumerator, nsISimpleEnumerator)

nsOverlayEnumerator::nsOverlayEnumerator(nsISimpleEnumerator *aArcs)
{
  NS_INIT_REFCNT();
  mArcs = aArcs;
}

nsOverlayEnumerator::~nsOverlayEnumerator()
{
}

NS_IMETHODIMP nsOverlayEnumerator::HasMoreElements(PRBool *aIsTrue)
{
  return mArcs->HasMoreElements(aIsTrue);
}

NS_IMETHODIMP nsOverlayEnumerator::GetNext(nsISupports **aResult)
{
  nsresult rv;
  *aResult = nsnull;

  if (!mArcs)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsISupports> supports;
  mArcs->GetNext(getter_AddRefs(supports));

  nsCOMPtr<nsIRDFLiteral> value = do_QueryInterface(supports, &rv);
  if (NS_FAILED(rv))
    return NS_OK;

  const PRUnichar* valueStr;
  rv = value->GetValueConst(&valueStr);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsIURL> url;
  
  rv = nsComponentManager::CreateInstance("component://netscape/network/standard-url",
                                          nsnull,
                                          NS_GET_IID(nsIURL),
                                          getter_AddRefs(url));

  if (NS_FAILED(rv))
    return NS_OK;

  nsCAutoString str(valueStr);
  url->SetSpec(str);
  
  nsCOMPtr<nsISupports> sup;
  sup = do_QueryInterface(url, &rv);
  if (NS_FAILED(rv))
    return NS_OK;

  *aResult = sup;
  NS_ADDREF(*aResult);

  return NS_OK;
}


class nsChromeRegistry : public nsIChromeRegistry
{
public:
    NS_DECL_ISUPPORTS

    // nsIChromeRegistry methods:
    NS_DECL_NSICHROMEREGISTRY

    // nsChromeRegistry methods:
    nsChromeRegistry();
    virtual ~nsChromeRegistry();

    static PRUint32 gRefCnt;
    static nsIRDFService* gRDFService;
    static nsIRDFResource* kCHROME_chrome;
    static nsIRDFResource* kCHROME_skin;
    static nsIRDFResource* kCHROME_content;
    static nsIRDFResource* kCHROME_locale;
    static nsIRDFResource* kCHROME_base;
    static nsIRDFResource* kCHROME_main;
    static nsIRDFResource* kCHROME_archive;
    static nsIRDFResource* kCHROME_name;
    static nsIRDFResource* kCHROME_theme;
    static nsSupportsHashtable *mDataSourceTable;

protected:
    NS_IMETHOD GetOverlayDataSource(nsIURI *aChromeURL, nsIRDFDataSource **aResult);
    NS_IMETHOD InitializeDataSource(nsString &aPackage,
                                    nsString &aProvider,
                                    nsIRDFDataSource **aResult);
    nsresult GetPackageTypeResource(const nsString& aChromeType, nsIRDFResource** aResult);
    nsresult GetChromeResource(nsIRDFDataSource *aDataSource,
                               nsString& aResult, nsIRDFResource* aChromeResource,
                               nsIRDFResource* aProperty);
    NS_IMETHOD RemoveOverlay(nsIRDFDataSource *aDataSource, nsIRDFResource *aResource);
    NS_IMETHOD RemoveOverlays(nsAutoString aPackage,
                              nsAutoString aProvider,
                              nsIRDFContainer *aContainer,
                              nsIRDFDataSource *aDataSource);
private:
    NS_IMETHOD ReallyRemoveOverlayFromDataSource(const PRUnichar *aDocURI, char *aOverlayURI);
    NS_IMETHOD LoadDataSource(const nsCAutoString &aFileName, nsIRDFDataSource **aResult);

};

PRUint32 nsChromeRegistry::gRefCnt  ;
nsIRDFService* nsChromeRegistry::gRDFService = nsnull;
nsSupportsHashtable* nsChromeRegistry::mDataSourceTable = nsnull;

nsIRDFResource* nsChromeRegistry::kCHROME_chrome = nsnull;
nsIRDFResource* nsChromeRegistry::kCHROME_skin = nsnull;
nsIRDFResource* nsChromeRegistry::kCHROME_content = nsnull;
nsIRDFResource* nsChromeRegistry::kCHROME_locale = nsnull;
nsIRDFResource* nsChromeRegistry::kCHROME_base = nsnull;
nsIRDFResource* nsChromeRegistry::kCHROME_main = nsnull;
nsIRDFResource* nsChromeRegistry::kCHROME_archive = nsnull;
nsIRDFResource* nsChromeRegistry::kCHROME_name = nsnull;
nsIRDFResource* nsChromeRegistry::kCHROME_theme = nsnull;

////////////////////////////////////////////////////////////////////////////////

nsChromeRegistry::nsChromeRegistry()
{
	NS_INIT_REFCNT();


  gRefCnt++;
  if (gRefCnt == 1) {
      nsresult rv;
      rv = nsServiceManager::GetService(kRDFServiceCID,
                                        NS_GET_IID(nsIRDFService),
                                        (nsISupports**)&gRDFService);
      NS_ASSERTION(NS_SUCCEEDED(rv), "unable to get RDF service");

      // get all the properties we'll need:
      rv = gRDFService->GetResource(kURICHROME_chrome, &kCHROME_chrome);
      NS_ASSERTION(NS_SUCCEEDED(rv), "unable to get resource");

      rv = gRDFService->GetResource(kURICHROME_skin, &kCHROME_skin);
      NS_ASSERTION(NS_SUCCEEDED(rv), "unable to get resource");

      rv = gRDFService->GetResource(kURICHROME_content, &kCHROME_content);
      NS_ASSERTION(NS_SUCCEEDED(rv), "unable to get resource");

      rv = gRDFService->GetResource(kURICHROME_locale, &kCHROME_locale);
      NS_ASSERTION(NS_SUCCEEDED(rv), "unable to get resource");

      rv = gRDFService->GetResource(kURICHROME_base, &kCHROME_base);
      NS_ASSERTION(NS_SUCCEEDED(rv), "unable to get resource");

      rv = gRDFService->GetResource(kURICHROME_main, &kCHROME_main);
      NS_ASSERTION(NS_SUCCEEDED(rv), "unable to get resource");

      rv = gRDFService->GetResource(kURICHROME_archive, &kCHROME_archive);
      NS_ASSERTION(NS_SUCCEEDED(rv), "unable to get resource");

      rv = gRDFService->GetResource(kURICHROME_name, &kCHROME_name);
      NS_ASSERTION(NS_SUCCEEDED(rv), "unable to get resource");

      rv = gRDFService->GetResource(kURICHROME_theme, &kCHROME_theme);
      NS_ASSERTION(NS_SUCCEEDED(rv), "unable to get resource");
  }
}

nsChromeRegistry::~nsChromeRegistry()
{
    --gRefCnt;
    if (gRefCnt == 0) {

        // release all the properties:
        NS_IF_RELEASE(kCHROME_chrome);
        NS_IF_RELEASE(kCHROME_skin);
        NS_IF_RELEASE(kCHROME_content);
        NS_IF_RELEASE(kCHROME_locale);
        NS_IF_RELEASE(kCHROME_base);
        NS_IF_RELEASE(kCHROME_main);
        NS_IF_RELEASE(kCHROME_archive);
        NS_IF_RELEASE(kCHROME_theme);
        NS_IF_RELEASE(kCHROME_name);
        delete mDataSourceTable;
       
        if (gRDFService) {
            nsServiceManager::ReleaseService(kRDFServiceCID, gRDFService);
            gRDFService = nsnull;
        }
    }
}

NS_IMPL_ISUPPORTS1(nsChromeRegistry, nsIChromeRegistry)

////////////////////////////////////////////////////////////////////////////////
// nsIChromeRegistry methods:
/*
    The ConvertChromeURL takes a chrome URL and converts it into a resource: or 
    an HTTP: url type with certain rules. Here are the current portions of a 
    chrome: url that make up the chrome-

            chrome://global/skin/foo?bar
            \------/ \----/\---/ \-----/
                |       |     |     |
                |       |     |     `-- RemainingPortion
                |       |     |
                |       |     `-- Provider 
                |       |
                |       `-- Package
                |
                '-- Always "chrome://"


    Sometime in future when Necko lands completely this will change to the 
    following syntax-

            chrome://skin@global/foo?bar

    This will make the parsing simpler and quicker (since the URL parsing already
    takes this into account)

*/
NS_IMETHODIMP
nsChromeRegistry::ConvertChromeURL(nsIURI* aChromeURL)
{
    nsresult rv = NS_OK;
    NS_ASSERTION(aChromeURL, "null url!");
    if (!aChromeURL)
        return NS_ERROR_NULL_POINTER;

#ifdef NS_DEBUG
    //Ensure that we got a chrome url!
    nsXPIDLCString scheme;
    rv = aChromeURL->GetScheme(getter_Copies(scheme));
    if (NS_FAILED(rv)) return rv;
    NS_ASSERTION((0 == PL_strncmp(scheme, "chrome", 6)), 
        "Bad scheme URL in chrome URL conversion!");
    if (0 != PL_strncmp(scheme, "chrome", 6))
        return NS_ERROR_FAILURE;
#endif

    // Obtain the package, provider and remaining from the URL
    nsXPIDLCString package, provider, remaining;

#if 0 // This change happens when we switch to using chrome://skin@global/foo..
    rv = aChromeURL->GetHost(getter_Copies(package));
    if (NS_FAILED(rv)) return rv;
    rv = aChromeURL->GetPreHost(getter_Copies(provider));
    if (NS_FAILED(rv)) return rv;
    rv = aChromeURL->GetPath(getter_Copies(remaining));
    if (NS_FAILED(rv)) return rv;
#else // For now however...

    rv = aChromeURL->GetHost(getter_Copies(package));
    if (NS_FAILED(rv)) return rv;
    nsXPIDLCString tempPath;
    rv = aChromeURL->GetPath(getter_Copies(tempPath));
    if (NS_FAILED(rv)) return rv;

    BreakProviderAndRemainingFromPath(
        (const char*)tempPath, 
        getter_Copies(provider), 
        getter_Copies(remaining));

#endif

    // Construct the lookup string-
    // which is basically chrome:// + package + provider
    
    nsAutoString lookup("chrome://");

    lookup += package; // no trailing slash here
    
    NS_ASSERTION(*provider == '/', "No leading slash here!");
    
    //definitely have a leading slash...
    if (*provider != '/')
        lookup += '/';
    lookup += provider; 
    
    // end it on a slash if none is present
    if (lookup.CharAt(lookup.Length()-1) != '/')
        lookup += '/';

    // Get the chromeResource from this lookup string
    nsCOMPtr<nsIRDFResource> chromeResource;
    if (NS_FAILED(rv = GetPackageTypeResource(lookup, getter_AddRefs(chromeResource)))) {
        NS_ERROR("Unable to retrieve the resource corresponding to the chrome skin or content.");
        return rv;
    }
    
    // Using this chrome resource get the three basic things of a chrome entry-
    // base, name, main. and don't bail if they don't exist.

    nsAutoString base, name, main;


    nsCOMPtr<nsIRDFDataSource> dataSource;
    nsString packageStr(package), providerStr(provider);
    InitializeDataSource(packageStr, providerStr, getter_AddRefs(dataSource));


    rv = GetChromeResource(dataSource, name, chromeResource, kCHROME_name);
    if (NS_FAILED (rv)) {
        if (PL_strcmp(provider,"/locale") == 0)
          name = "en-US";
        else
          name = "default";
    }

    rv = GetChromeResource(dataSource, base, chromeResource, kCHROME_base);
    if (NS_FAILED(rv))
    {
        // No base entry was found, default it to our cache.
        base = "resource:/chrome/";
        base += package;
        if ((base.CharAt(base.Length()-1) != '/') && *provider != '/')
            base += '/';
        base += provider;
        if (base.CharAt(base.Length()-1) != '/') 
            base += '/';
        if (name.Length())
            base += name;
        if (base.CharAt(base.Length()-1) != '/')
            base += '/';
    }

    NS_ASSERTION(base.CharAt(base.Length()-1) == '/', "Base doesn't end in a slash!");
    if ('/' != base.CharAt(base.Length()-1))
        base += '/';
  
    // Now we construct our finalString
    nsAutoString finalString(base);

    if (!remaining || (0 == PL_strlen(remaining)))
    {
        rv = GetChromeResource(dataSource, main, chromeResource, kCHROME_main);
        if (NS_FAILED(rv))
        {
            //we'd definitely need main for an empty remaining
            //NS_ERROR("Unable to retrieve the main file registry entry for a chrome URL.");
            //return rv;
            main = package;
            if (PL_strcmp(provider, "/skin") == 0)
              main += ".css";
            else if (PL_strcmp(provider, "/content") == 0)
              main += ".xul";
            else if (PL_strcmp(provider, "/locale") == 0)
              main += ".dtd";
        }
        finalString += main;
    }
    else
        finalString += remaining;

    char* finalURI = finalString.ToNewCString();
    aChromeURL->SetSpec(finalURI);

    nsCRT::free(finalURI);
    return NS_OK;
}

NS_IMETHODIMP nsChromeRegistry::GetOverlayDataSource(nsIURI *aChromeURL, nsIRDFDataSource **aResult)
{
  *aResult = nsnull;

  nsresult rv;

  if (!mDataSourceTable)
    return NS_OK;

  // Obtain the package, provider and remaining from the URL
  nsXPIDLCString package, provider, remaining;

#if 0 // This change happens when we switch to using chrome://skin@global/foo..
  rv = aChromeURL->GetHost(getter_Copies(package));
  if (NS_FAILED(rv)) return rv;
  rv = aChromeURL->GetPreHost(getter_Copies(provider));
  if (NS_FAILED(rv)) return rv;
  rv = aChromeURL->GetPath(getter_Copies(remaining));
  if (NS_FAILED(rv)) return rv;
#else // For now however...

  rv = aChromeURL->GetHost(getter_Copies(package));
  if (NS_FAILED(rv)) return rv;
  nsXPIDLCString tempPath;
  rv = aChromeURL->GetPath(getter_Copies(tempPath));
  if (NS_FAILED(rv)) return rv;

  BreakProviderAndRemainingFromPath(
      (const char*)tempPath, 
      getter_Copies(provider), 
      getter_Copies(remaining));

#endif

  nsCAutoString overlayFile;

  // Retrieve the mInner data source.
  overlayFile = "resource:/chrome/";
  overlayFile += package;
  overlayFile += provider; // provider already has a / in the front of it
  overlayFile += "/";
  overlayFile += "overlays.rdf";

  nsStringKey skey(overlayFile);
  nsCOMPtr<nsISupports> supports = getter_AddRefs(NS_STATIC_CAST(nsISupports*, mDataSourceTable->Get(&skey)));
  if (supports)
  {
    nsCOMPtr<nsIRDFDataSource> dataSource = do_QueryInterface(supports, &rv);
    if (NS_SUCCEEDED(rv))
    {
      *aResult = dataSource;
      NS_ADDREF(*aResult);
    }
  }
  return NS_OK;
}


NS_IMETHODIMP nsChromeRegistry::GetOverlays(nsIURI *aChromeURL, nsISimpleEnumerator **aResult)
{
  *aResult = nsnull;

  nsresult rv;

  if (!mDataSourceTable)
    return NS_OK;

  nsCOMPtr<nsIRDFDataSource> dataSource;
  GetOverlayDataSource(aChromeURL, getter_AddRefs(dataSource));

  if (dataSource)
  {
    nsCOMPtr<nsIRDFContainer> container;
    rv = nsComponentManager::CreateInstance("component://netscape/rdf/container",
                                            nsnull,
                                            NS_GET_IID(nsIRDFContainer),
                                            getter_AddRefs(container));
    if (NS_FAILED(rv))
      return NS_OK;
 
    char *lookup;
    aChromeURL->GetSpec(&lookup);

    // Get the chromeResource from this lookup string
    nsCOMPtr<nsIRDFResource> chromeResource;
    if (NS_FAILED(rv = GetPackageTypeResource(lookup, getter_AddRefs(chromeResource)))) {
        NS_ERROR("Unable to retrieve the resource corresponding to the chrome skin or content.");
        return rv;
    }
    nsAllocator::Free(lookup);

    if (NS_FAILED(container->Init(dataSource, chromeResource)))
      return NS_OK;

    nsCOMPtr<nsISimpleEnumerator> arcs;
    if (NS_FAILED(container->GetElements(getter_AddRefs(arcs))))
      return NS_OK;

    *aResult = new nsOverlayEnumerator(arcs);

    NS_ADDREF(*aResult);
  }

  return NS_OK;
}

NS_IMETHODIMP nsChromeRegistry::LoadDataSource(const nsCAutoString &aFileName, nsIRDFDataSource **aResult)
{
    nsresult rv = nsComponentManager::CreateInstance(kRDFXMLDataSourceCID,
                                                     nsnull,
                                                     NS_GET_IID(nsIRDFDataSource),
                                                     (void**) aResult);
    if (NS_FAILED(rv)) return rv;


    nsCOMPtr<nsIRDFRemoteDataSource> remote = do_QueryInterface(*aResult);
    if (! remote)
        return NS_ERROR_UNEXPECTED;

    if (!mDataSourceTable)
      mDataSourceTable = new nsSupportsHashtable;

    rv = remote->Init(aFileName);
    if (NS_FAILED(rv)) {
      nsStringKey skey(aFileName);
      mDataSourceTable->Put(&skey, nsnull);
      return rv;
    }

    // We need to read this synchronously.
    rv = remote->Refresh(PR_TRUE);
    if (NS_FAILED(rv)) {
      nsStringKey skey(aFileName);
      mDataSourceTable->Put(&skey, nsnull);
      return rv;
    }

    nsCOMPtr<nsISupports> supports = do_QueryInterface(remote);
    nsStringKey skey(aFileName);
    mDataSourceTable->Put(&skey, (void*)supports.get());

    return NS_OK;
}

NS_IMETHODIMP
nsChromeRegistry::InitializeDataSource(nsString &aPackage,
                                       nsString &aProvider,
                                       nsIRDFDataSource **aResult)
{
    nsCAutoString chromeFile, overlayFile;

    // Retrieve the mInner data source.
    chromeFile = "resource:/chrome/";
    chromeFile += aPackage;
    chromeFile += aProvider; // provider already has a / in the front of it
    chromeFile += "/";
    overlayFile = chromeFile;
    chromeFile += "chrome.rdf";
    overlayFile += "overlays.rdf";

    if (mDataSourceTable)
    {
      // current.rdf and overlays.rdf are loaded in pairs and so if one is loaded, the other should be too.
      nsStringKey skey(chromeFile);
      nsCOMPtr<nsISupports> supports = getter_AddRefs(NS_STATIC_CAST(nsISupports*, mDataSourceTable->Get(&skey)));
      if (supports)
      {
        nsCOMPtr<nsIRDFDataSource> dataSource = do_QueryInterface(supports);
        if (dataSource)
        {
          *aResult = dataSource;
          NS_ADDREF(*aResult);
          return NS_OK;
        }
        return NS_ERROR_FAILURE;
      }
      else if (mDataSourceTable->Exists(&skey))
      	return NS_OK;
    }

   nsCOMPtr<nsIRDFDataSource> overlayDataSource;

   LoadDataSource(chromeFile, aResult);
   LoadDataSource(overlayFile, getter_AddRefs(overlayDataSource));

   return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////

nsresult
nsChromeRegistry::GetPackageTypeResource(const nsString& aChromeType,
                                           nsIRDFResource** aResult)
{
    nsresult rv = NS_OK;
    char* url = aChromeType.ToNewCString();
    if (NS_FAILED(rv = gRDFService->GetResource(url, aResult))) {
        NS_ERROR("Unable to retrieve a resource for this package type.");
        *aResult = nsnull;
        nsCRT::free(url);
        return rv;
    }
    nsCRT::free(url);
    return NS_OK;
}

nsresult 
nsChromeRegistry::GetChromeResource(nsIRDFDataSource *aDataSource,
                                    nsString& aResult, 
                                    nsIRDFResource* aChromeResource,
                                    nsIRDFResource* aProperty)
{
    if (!aDataSource)
        return NS_ERROR_FAILURE;

    nsresult rv;

    nsCOMPtr<nsIRDFNode> chromeBase;
    if (NS_FAILED(rv = aDataSource->GetTarget(aChromeResource, aProperty, PR_TRUE, getter_AddRefs(chromeBase)))) {
        NS_ERROR("Unable to obtain a base resource.");
        return rv;
    }

    if (chromeBase == nsnull)
        return NS_ERROR_FAILURE;

    nsCOMPtr<nsIRDFResource> resource;
    nsCOMPtr<nsIRDFLiteral> literal;

    if (NS_SUCCEEDED(rv = chromeBase->QueryInterface(NS_GET_IID(nsIRDFResource),
                                                     (void**) getter_AddRefs(resource)))) {
        nsXPIDLCString uri;
        resource->GetValue( getter_Copies(uri) );
        aResult = uri;
    }
    else if (NS_SUCCEEDED(rv = chromeBase->QueryInterface(NS_GET_IID(nsIRDFLiteral),
                                                      (void**) getter_AddRefs(literal)))) {
        nsXPIDLString s;
        literal->GetValue( getter_Copies(s) );
        aResult = s;
    }
    else {
        // This should _never_ happen.
        NS_ERROR("uh, this isn't a resource or a literal!");
        return NS_ERROR_UNEXPECTED;
    }

    return NS_OK;
}


nsresult
NS_NewChromeRegistry(nsIChromeRegistry** aResult)
{
    NS_PRECONDITION(aResult != nsnull, "null ptr");
    if (! aResult)
        return NS_ERROR_NULL_POINTER;

    nsChromeRegistry* chromeRegistry = new nsChromeRegistry();
    if (chromeRegistry == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(chromeRegistry);
    *aResult = chromeRegistry;
    return NS_OK;
}

////////////////////////////////////////////////////////////////////////

//
// Path = provider/remaining
// 
void BreakProviderAndRemainingFromPath(const char* i_path, char** o_provider, char** o_remaining)
{
    if (!i_path || !o_provider || !o_remaining)
        return;
    int len = PL_strlen(i_path);
    NS_ASSERTION(len>1, "path is messed up!");
    char* slash = PL_strchr(i_path+1, '/'); // +1 to skip the leading slash if any
    if (slash)
    {
        *o_provider = PL_strndup(i_path, (slash - i_path)); // dont include the trailing slash
        if (slash != (i_path + len-1)) // if that was not the last trailing slash...
        {
            // don't include the leading slash here as well...
            *o_remaining = PL_strndup(slash+1, len - (slash-i_path + 1)); 
        }
        else
            *o_remaining = nsnull;
    }
    else // everything is just the provider
        *o_provider = PL_strndup(i_path, len);
}




// theme stuff

NS_IMETHODIMP nsChromeRegistry::RefreshChrome()
{
  return NS_OK;
}

NS_IMETHODIMP nsChromeRegistry::ApplyTheme(const PRUnichar *themeFileName)
{
  nsCAutoString chromeFile = "resource:/chrome/themes.rdf";
  nsCOMPtr<nsIRDFDataSource> dataSource;
  LoadDataSource(chromeFile, getter_AddRefs(dataSource));

  return NS_OK;
}

NS_IMETHODIMP nsChromeRegistry::ReallyRemoveOverlayFromDataSource(const PRUnichar *aDocURI,
                                                                  char *aOverlayURI)
{
  nsresult rv;
  nsCOMPtr<nsIURL> url;
  
  rv = nsComponentManager::CreateInstance("component://netscape/network/standard-url",
                                          nsnull,
                                          NS_GET_IID(nsIURL),
                                          getter_AddRefs(url));

  if (NS_FAILED(rv))
    return NS_OK;

  nsCAutoString str(aDocURI);
  url->SetSpec(str);
  nsCOMPtr<nsIRDFDataSource> dataSource;
  GetOverlayDataSource(url, getter_AddRefs(dataSource));

  if (!dataSource)
    return NS_OK;

  nsCOMPtr<nsIRDFResource> resource;
  rv = GetPackageTypeResource(aDocURI,
                              getter_AddRefs(resource));

  if (NS_FAILED(rv))
    return NS_OK;

  nsCOMPtr<nsIRDFContainer> container;

  rv = nsComponentManager::CreateInstance("component://netscape/rdf/container",
                                          nsnull,
                                          NS_GET_IID(nsIRDFContainer),
                                          getter_AddRefs(container));
  if (NS_FAILED(rv))
    return NS_ERROR_FAILURE;
  
  if (NS_FAILED(container->Init(dataSource, resource)))
    return NS_ERROR_FAILURE;

  nsAutoString unistr(aOverlayURI);
  nsCOMPtr<nsIRDFLiteral> literal;
  gRDFService->GetLiteral(unistr.GetUnicode(), getter_AddRefs(literal));

  container->RemoveElement(literal, PR_TRUE);

  nsCOMPtr<nsIRDFRemoteDataSource> remote = do_QueryInterface(dataSource, &rv);
  if (NS_FAILED(rv))
    return NS_OK;
  
  remote->Flush();

  return NS_OK;
}

NS_IMETHODIMP nsChromeRegistry::RemoveOverlay(nsIRDFDataSource *aDataSource, nsIRDFResource *aResource)
{
  nsCOMPtr<nsIRDFContainer> container;
  nsresult rv;

  rv = nsComponentManager::CreateInstance("component://netscape/rdf/container",
                                          nsnull,
                                          NS_GET_IID(nsIRDFContainer),
                                          getter_AddRefs(container));
  if (NS_FAILED(rv))
    return NS_ERROR_FAILURE;
  
  if (NS_FAILED(container->Init(aDataSource, aResource)))
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsISimpleEnumerator> arcs;
  if (NS_FAILED(container->GetElements(getter_AddRefs(arcs))))
    return NS_ERROR_FAILURE;

  PRBool moreElements;
  arcs->HasMoreElements(&moreElements);
  
  char *value;
  aResource->GetValue(&value);

  while (moreElements)
  {
    nsCOMPtr<nsISupports> supports;
    arcs->GetNext(getter_AddRefs(supports));

    nsCOMPtr<nsIRDFLiteral> literal = do_QueryInterface(supports, &rv);

    if (NS_SUCCEEDED(rv))
    {
      const PRUnichar* valueStr;
      rv = literal->GetValueConst(&valueStr);
      if (NS_FAILED(rv))
        return rv;

      ReallyRemoveOverlayFromDataSource(valueStr, value);
    }
    arcs->HasMoreElements(&moreElements);
  }
  nsAllocator::Free(value);

  return NS_OK;
}


NS_IMETHODIMP nsChromeRegistry::RemoveOverlays(nsAutoString aPackage,
                                               nsAutoString aProvider,
                                               nsIRDFContainer *aContainer,
                                               nsIRDFDataSource *aDataSource)
{
  nsresult rv;

  nsCOMPtr<nsISimpleEnumerator> arcs;
  if (NS_FAILED(aContainer->GetElements(getter_AddRefs(arcs))))
    return NS_OK;

  PRBool moreElements;
  arcs->HasMoreElements(&moreElements);
  
  while (moreElements)
  {
    nsCOMPtr<nsISupports> supports;
    arcs->GetNext(getter_AddRefs(supports));

    nsCOMPtr<nsIRDFResource> resource = do_QueryInterface(supports, &rv);

    if (NS_SUCCEEDED(rv))
    {
      RemoveOverlay(aDataSource, resource);
    }

    arcs->HasMoreElements(&moreElements);
  }

  return NS_OK;
}

NS_IMETHODIMP nsChromeRegistry::ApplyThemeToPackage(const PRUnichar *aThemeFileName,
                                                    const PRUnichar *aPackageName,
                                                    const PRUnichar *aProviderName)
{
  nsresult rv;
  nsCOMPtr<nsIRDFDataSource> dataSource;
  nsAutoString package(aPackageName), provider("/");
  provider += aProviderName;

  if (NS_FAILED(InitializeDataSource(package, provider, getter_AddRefs(dataSource))))
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIRDFResource> resource;
  rv = GetPackageTypeResource("chrome:overlays",
                              getter_AddRefs(resource));
  if (NS_FAILED(rv))
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIRDFContainer> container;
  rv = nsComponentManager::CreateInstance("component://netscape/rdf/container",
                                          nsnull,
                                          NS_GET_IID(nsIRDFContainer),
                                          getter_AddRefs(container));
  if (NS_FAILED(rv))
    return NS_ERROR_FAILURE;
  
  if (NS_SUCCEEDED(container->Init(dataSource, resource)))
  {
    RemoveOverlays(package, provider, container, dataSource);
  }

  return NS_OK;
}
