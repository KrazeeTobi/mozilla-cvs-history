/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "msgCore.h" // for pre-compiled headers
//#include "nsIMsgIdentity.h"
#include "nsIMsgAccountManager.h"
//#include "nsIPop3IncomingServer.h"
#include "nsMsgMailSession.h"
#include "nsMsgLocalCID.h"
#include "nsMsgBaseCID.h"
#include "nsCOMPtr.h"
#include "nsMsgFolderCache.h"
#include "nsIFileLocator.h"
#include "nsFileLocations.h"
#include "nsIMsgStatusFeedback.h"

NS_IMPL_ISUPPORTS(nsMsgMailSession, nsCOMTypeInfo<nsIMsgMailSession>::GetIID());

static NS_DEFINE_CID(kMsgAccountManagerCID, NS_MSGACCOUNTMANAGER_CID);
static NS_DEFINE_CID(kMsgFolderCacheCID, NS_MSGFOLDERCACHE_CID);
static NS_DEFINE_IID(kIFileLocatorIID,      NS_IFILELOCATOR_IID);
static NS_DEFINE_CID(kFileLocatorCID,       NS_FILELOCATOR_CID);
 
//static NS_DEFINE_CID(kMsgIdentityCID, NS_MSGIDENTITY_CID);
//static NS_DEFINE_CID(kPop3IncomingServerCID, NS_POP3INCOMINGSERVER_CID);
//static NS_DEFINE_CID(kPrefCID, NS_PREF_CID);
    

nsMsgMailSession::nsMsgMailSession():
  mRefCnt(0),
  m_accountManager(0),
  m_msgFolderCache(0)
{

	NS_INIT_REFCNT();
}


nsMsgMailSession::~nsMsgMailSession()
{
	Shutdown();
}

nsresult nsMsgMailSession::Init()
{
	nsresult rv = NS_NewISupportsArray(getter_AddRefs(mListeners));

	return rv;

}

nsresult nsMsgMailSession::Shutdown()
{
  if(m_accountManager)
  {
	  if (m_msgFolderCache)
		m_accountManager->WriteToFolderCache(m_msgFolderCache);
	  m_accountManager->CloseCachedConnections();
	  m_accountManager->UnloadAccounts();
  }

  NS_IF_RELEASE(m_accountManager);

  NS_IF_RELEASE(m_msgFolderCache);

  return NS_OK;
}

// nsIMsgMailSession
nsresult nsMsgMailSession::GetCurrentIdentity(nsIMsgIdentity ** aIdentity)
{
  nsresult rv;

  nsCOMPtr<nsIMsgAccountManager> accountManager;
  rv = GetAccountManager(getter_AddRefs(accountManager));
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIMsgAccount> defaultAccount;
  rv = accountManager->GetDefaultAccount(getter_AddRefs(defaultAccount));
  if (NS_FAILED(rv)) return rv;
  
  rv = defaultAccount->GetDefaultIdentity(aIdentity);
  if (NS_SUCCEEDED(rv))
  	NS_ADDREF(*aIdentity);
  
  return rv;
}

nsresult nsMsgMailSession::GetCurrentServer(nsIMsgIncomingServer ** aServer)
{
  nsresult rv=NS_ERROR_UNEXPECTED;

  nsCOMPtr<nsIMsgAccountManager> accountManager;
  rv = GetAccountManager(getter_AddRefs(accountManager));
  if (NS_FAILED(rv)) return rv;
  
  nsCOMPtr<nsIMsgAccount> defaultAccount;
  rv = accountManager->GetDefaultAccount(getter_AddRefs(defaultAccount));
  if (NS_FAILED(rv)) return rv;

  //if successful aServer will be addref'd by GetIncomingServer
  rv = defaultAccount->GetIncomingServer(aServer);

  return rv;
}

nsresult nsMsgMailSession::GetAccountManager(nsIMsgAccountManager* *aAM)
{
  NS_ENSURE_ARG_POINTER(aAM);

  nsresult rv;
  
  NS_WITH_SERVICE(nsIMsgAccountManager, accountManager, kMsgAccountManagerCID, &rv);
  if (NS_FAILED(rv)) return rv;
  accountManager->LoadAccounts();
    
  *aAM = accountManager;
  NS_IF_ADDREF(*aAM);
  return NS_OK;
}

nsresult nsMsgMailSession::GetFolderCache(nsIMsgFolderCache* *aFolderCache)
{
  if (!aFolderCache) return NS_ERROR_NULL_POINTER;

  nsresult rv = NS_OK;

  if (!m_msgFolderCache)
  {
    rv = nsComponentManager::CreateInstance(kMsgFolderCacheCID,
                                            NULL,
                                            nsCOMTypeInfo<nsIMsgFolderCache>::GetIID(),
                                            (void **)&m_msgFolderCache);
    if (NS_FAILED(rv))
		return rv;

    nsCOMPtr <nsIFileSpec> cacheFile;
    NS_WITH_SERVICE(nsIFileLocator, locator, kFileLocatorCID, &rv);
    if (NS_FAILED(rv)) return rv;

    rv = locator->GetFileLocation(nsSpecialFileSpec::App_MessengerFolderCache50, getter_AddRefs(cacheFile));
    if (NS_FAILED(rv)) return rv;

    m_msgFolderCache->Init(cacheFile);

  }

  *aFolderCache = m_msgFolderCache;
  NS_IF_ADDREF(*aFolderCache);
  return rv;
}

nsresult nsMsgMailSession::GetTemporaryMsgStatusFeedback(nsIMsgStatusFeedback* *aMsgStatusFeedback)
{
  if (!aMsgStatusFeedback) return NS_ERROR_NULL_POINTER;
  
  *aMsgStatusFeedback = m_temporaryMsgStatusFeedback;
  NS_IF_ADDREF(*aMsgStatusFeedback);
  return NS_OK;
}


nsresult nsMsgMailSession::SetTemporaryMsgStatusFeedback(nsIMsgStatusFeedback* aMsgStatusFeedback)
{
  m_temporaryMsgStatusFeedback = do_QueryInterface(aMsgStatusFeedback);
  return NS_OK;
}


NS_IMETHODIMP nsMsgMailSession::AddFolderListener(nsIFolderListener * listener)
{
  mListeners->AppendElement(listener);
  return NS_OK;

}

NS_IMETHODIMP nsMsgMailSession::RemoveFolderListener(nsIFolderListener * listener)
{
  mListeners->RemoveElement(listener);
  return NS_OK;

}

NS_IMETHODIMP
nsMsgMailSession::NotifyFolderItemPropertyChanged(nsISupports *item,
                                                  const char *property,
                                                  const char* oldValue,
                                                  const char* newValue)
{
	nsresult rv;
	PRUint32 count;
	rv = mListeners->Count(&count);
	if (NS_FAILED(rv)) return rv;

	
	for(PRUint32 i = 0; i < count; i++)
	{
		nsCOMPtr<nsIFolderListener> listener = getter_AddRefs((nsIFolderListener*)mListeners->ElementAt(i));
		listener->OnItemPropertyChanged(item, property, oldValue, newValue);
	}

	return NS_OK;

}

NS_IMETHODIMP
nsMsgMailSession::NotifyFolderItemPropertyFlagChanged(nsISupports *item,
                                                      const char *property,
                                                      PRUint32 oldValue,
                                                      PRUint32 newValue)
{
	nsresult rv;
	PRUint32 count;
	rv = mListeners->Count(&count);
	if (NS_FAILED(rv)) return rv;

	
	for(PRUint32 i = 0; i < count; i++)
	{
		nsCOMPtr<nsIFolderListener> listener = getter_AddRefs((nsIFolderListener*)mListeners->ElementAt(i));
		listener->OnItemPropertyFlagChanged(item, property, oldValue, newValue);
	}

	return NS_OK;

}

NS_IMETHODIMP nsMsgMailSession::NotifyFolderItemAdded(nsIFolder *folder, nsISupports *item)
{
	nsresult rv;
	PRUint32 count;
	rv = mListeners->Count(&count);
	if (NS_FAILED(rv)) return rv;

	
	for(PRUint32 i = 0; i < count; i++)
	{
		nsCOMPtr<nsIFolderListener> listener = getter_AddRefs((nsIFolderListener*)mListeners->ElementAt(i));
		listener->OnItemAdded(folder, item);
	}

	return NS_OK;

}

NS_IMETHODIMP nsMsgMailSession::NotifyFolderItemDeleted(nsIFolder *folder, nsISupports *item)
{
	nsresult rv;
	PRUint32 count;
	rv = mListeners->Count(&count);
	if (NS_FAILED(rv)) return rv;

	
	for(PRUint32 i = 0; i < count; i++)
	{
		nsCOMPtr<nsIFolderListener> listener = getter_AddRefs((nsIFolderListener*)mListeners->ElementAt(i));
		listener->OnItemRemoved(folder, item);
	}
	return NS_OK;

}

NS_IMETHODIMP  nsMsgMailSession::NotifyFolderLoaded(nsIFolder *folder)
{

	nsresult rv;
	PRUint32 count;
	rv = mListeners->Count(&count);
	if (NS_FAILED(rv)) return rv;

	
	for(PRUint32 i = 0; i < count; i++)
	{
		nsCOMPtr<nsIFolderListener> listener = getter_AddRefs((nsIFolderListener*)mListeners->ElementAt(i));
		listener->OnFolderLoaded(folder);
	}
	return NS_OK;


}



