/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * Sun Microsystems, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Original Author: 
 *   Paul Sandoz   <paul.sandoz@sun.com>
 *
 * Contributor(s):
 *   Seth Spitzer <sspitzer@netscape.com>
 *   Dan Mosedale <dmose@netscape.com>
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

#include "nsAbLDAPDirectoryQuery.h"
#include "nsAbBoolExprToLDAPFilter.h"
#include "nsAbLDAPProperties.h"
#include "nsILDAPErrors.h"
#include "nsILDAPOperation.h"
#include "nsAbUtils.h"

#include "nsIAuthPrompt.h"
#include "nsIStringBundle.h"
#include "nsXPIDLString.h"
#include "nsAutoLock.h"
#include "nsIProxyObjectManager.h"
#include "prprf.h"
#include "nsIWindowWatcher.h"
#include "nsIDOMWindow.h"
#include "nsICategoryManager.h"

class nsAbQueryLDAPMessageListener : public nsILDAPMessageListener
{
public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSILDAPMESSAGELISTENER

    nsAbQueryLDAPMessageListener (
        nsAbLDAPDirectoryQuery* directoryQuery,
        PRInt32 contextID,
        nsILDAPURL* url,
        nsILDAPConnection* connection,
        nsIAbDirectoryQueryArguments* queryArguments,
        nsIAbDirectoryQueryResultListener* queryListener,
        PRInt32 resultLimit = -1,
        PRInt32 timeOut = 0);
    virtual ~nsAbQueryLDAPMessageListener ();

protected:
    nsresult OnLDAPMessageBind (nsILDAPMessage *aMessage);
    nsresult OnLDAPMessageSearchEntry (nsILDAPMessage *aMessage,
            nsIAbDirectoryQueryResult** result);
    nsresult OnLDAPMessageSearchResult (nsILDAPMessage *aMessage,
            nsIAbDirectoryQueryResult** result);

    nsresult QueryResultStatus (nsISupportsArray* properties,
            nsIAbDirectoryQueryResult** result, PRUint32 resultStatus);

protected:
    friend class nsAbLDAPDirectoryQuery;
    nsresult Cancel ();
    nsresult Initiate ();

protected:
    nsAbLDAPDirectoryQuery* mDirectoryQuery;
    PRInt32 mContextID;
    nsCOMPtr<nsILDAPURL> mUrl;
    nsCOMPtr<nsILDAPConnection> mConnection;
    nsCOMPtr<nsIAbDirectoryQueryArguments> mQueryArguments;
    nsCOMPtr<nsIAbDirectoryQueryResultListener> mQueryListener;
    PRInt32 mResultLimit;
    PRInt32 mTimeOut;

    PRBool mBound;
    PRBool mFinished;
    PRBool mInitialized;
    PRBool mCanceled;

    nsCOMPtr<nsILDAPOperation> mSearchOperation;

    PRLock* mLock;
};


NS_IMPL_THREADSAFE_ISUPPORTS1(nsAbQueryLDAPMessageListener, nsILDAPMessageListener)

nsAbQueryLDAPMessageListener::nsAbQueryLDAPMessageListener (
        nsAbLDAPDirectoryQuery* directoryQuery,
        PRInt32 contextID,
        nsILDAPURL* url,
        nsILDAPConnection* connection,
        nsIAbDirectoryQueryArguments* queryArguments,
        nsIAbDirectoryQueryResultListener* queryListener,
        PRInt32 resultLimit,
        PRInt32 timeOut) :
    mDirectoryQuery (directoryQuery),
    mContextID (contextID),
    mUrl (url),
    mConnection (connection),
    mQueryArguments (queryArguments),
    mQueryListener (queryListener),
    mResultLimit (resultLimit),
    mTimeOut (timeOut),
    mBound (PR_FALSE),
    mFinished (PR_FALSE),
    mInitialized(PR_FALSE),
    mCanceled (PR_FALSE),
    mLock(0)

{
    NS_INIT_ISUPPORTS();

    NS_ADDREF(mDirectoryQuery);
}

nsAbQueryLDAPMessageListener::~nsAbQueryLDAPMessageListener ()
{
    if (mLock)
        PR_DestroyLock (mLock);

    NS_RELEASE(mDirectoryQuery);
}

nsresult nsAbQueryLDAPMessageListener::Initiate ()
{
    if(mInitialized == PR_TRUE)
        return NS_OK;
                
    mLock = PR_NewLock ();
    if(!mLock)
    {
        return NS_ERROR_OUT_OF_MEMORY;
    }

    mInitialized = PR_TRUE;

    return NS_OK;
}

nsresult nsAbQueryLDAPMessageListener::Cancel ()
{
    nsresult rv;

    rv = Initiate();
    NS_ENSURE_SUCCESS(rv, rv);

    nsAutoLock lock(mLock);

    if (mFinished == PR_TRUE || mCanceled == PR_TRUE)
        return NS_OK;

    mCanceled = PR_TRUE;

    return NS_OK;
}

NS_IMETHODIMP nsAbQueryLDAPMessageListener::OnLDAPMessage(nsILDAPMessage *aMessage)
{
    nsresult rv;

    rv = Initiate();
    NS_ENSURE_SUCCESS(rv, rv);

    PRInt32 messageType;
    rv = aMessage->GetType(&messageType);
    NS_ENSURE_SUCCESS(rv, rv);

    PRBool cancelOperation = PR_FALSE;

    // Enter lock
    {
        nsAutoLock lock (mLock);

        if (mFinished == PR_TRUE)
            return NS_OK;

        if (messageType == nsILDAPMessage::RES_SEARCH_RESULT)
            mFinished = PR_TRUE;
        else if (mCanceled == PR_TRUE)
        {
            mFinished = PR_TRUE;
            cancelOperation = PR_TRUE;
        }
    }
    // Leave lock

    nsCOMPtr<nsIAbDirectoryQueryResult> queryResult;
    if (cancelOperation == PR_FALSE)
    {
        switch (messageType)
        {
        case nsILDAPMessage::RES_BIND:
            rv = OnLDAPMessageBind (aMessage);
            NS_ENSURE_SUCCESS(rv, rv);
            break;
        case nsILDAPMessage::RES_SEARCH_ENTRY:
            rv = OnLDAPMessageSearchEntry (aMessage, getter_AddRefs (queryResult));
            NS_ENSURE_SUCCESS(rv, rv);
            break;
        case nsILDAPMessage::RES_SEARCH_RESULT:
            rv = OnLDAPMessageSearchResult (aMessage, getter_AddRefs (queryResult));
            NS_ENSURE_SUCCESS(rv, rv);
        default:
            break;
        }
    }
    else
    {
        if (mSearchOperation)
            rv = mSearchOperation->Abandon ();

        rv = QueryResultStatus (nsnull, getter_AddRefs (queryResult), nsIAbDirectoryQueryResult::queryResultStopped);
    }

    if (queryResult)
        rv = mQueryListener->OnQueryItem (queryResult);

    return rv;
}

NS_IMETHODIMP nsAbQueryLDAPMessageListener::OnLDAPInit(nsresult aStatus)
{
    nsresult rv;
    nsXPIDLString passwd;

    // Make sure that the Init() worked properly
    NS_ENSURE_SUCCESS(aStatus, aStatus);

    // If mLogin is set, we're expected to use it to get a password.
    //
    if (!mDirectoryQuery->mLogin.IsEmpty()) {
// XXX hack until nsUTF8AutoString exists
#define nsUTF8AutoString nsCAutoString
        nsUTF8AutoString spec;
        PRBool status;

        // we're going to use the URL spec of the server as the "realm" for 
        // wallet to remember the password by / for.
        // 
        rv = mDirectoryQuery->mDirectoryUrl->GetSpec(spec);
        if (NS_FAILED(rv)) {
            NS_ERROR("nsAbQueryLDAPMessageListener::OnLDAPInit(): GetSpec"
                     " failed\n");
            return NS_ERROR_FAILURE;
        }

        // get the string bundle service
        //
        nsCOMPtr<nsIStringBundleService> 
            stringBundleSvc(do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv)); 
        if (NS_FAILED(rv)) {
            NS_ERROR("nsAbQueryLDAPMessageListener::OnLDAPInit():"
                     " error getting string bundle service");
            return rv;
        }

        // get the LDAP string bundle
        //
        nsCOMPtr<nsIStringBundle> ldapBundle;
        rv = stringBundleSvc->CreateBundle(
            "chrome://mozldap/locale/ldap.properties",
            getter_AddRefs(ldapBundle));
        if (NS_FAILED(rv)) {
            NS_ERROR("nsAbQueryLDAPMessageListener::OnLDAPInit():"
                     " error creating string bundle"
                     " chrome://mozldap/locale/ldap.properties");
            return rv;
        } 

        // get the title for the authentication prompt
        //
        nsXPIDLString authPromptTitle;
        rv = ldapBundle->GetStringFromName(
            NS_LITERAL_STRING("authPromptTitle").get(), 
            getter_Copies(authPromptTitle));
        if (NS_FAILED(rv)) {
            NS_ERROR("nsAbQueryLDAPMessageListener::OnLDAPInit():"
                     "error getting 'authPromptTitle' string from bundle "
                     "chrome://mozldap/locale/ldap.properties");
            return rv;
        }

        // get the host name for the auth prompt
        //
        nsCAutoString host;
        rv = mUrl->GetAsciiHost(host);
        if (NS_FAILED(rv)) {
            return NS_ERROR_FAILURE;
        }
        const PRUnichar *hostArray[1] = { NS_ConvertASCIItoUCS2(host).get() };

        // format the hostname into the authprompt text string
        //
        nsXPIDLString authPromptText;
        rv = ldapBundle->FormatStringFromName(
            NS_LITERAL_STRING("authPromptText").get(),
            hostArray, sizeof(hostArray),
            getter_Copies(authPromptText));
        if (NS_FAILED(rv)) {
            NS_ERROR("nsAbQueryLDAPMessageListener::OnLDAPInit():"
                     "error getting 'authPromptText' string from bundle "
                     "chrome://mozldap/locale/ldap.properties");
            return rv;
        }


        // get the window watcher service, so we can get an auth prompter
        //
        nsCOMPtr<nsIWindowWatcher> windowWatcherSvc = 
            do_GetService("@mozilla.org/embedcomp/window-watcher;1", &rv);
        if (NS_FAILED(rv)) {
            NS_ERROR("nsAbQueryLDAPMessageListener::OnLDAPInit():"
                     " couldn't get window watcher service.");
            return rv;
        }

        // get the addressbook window, as it will be used to parent the auth
        // prompter dialog
        //
        nsCOMPtr<nsIDOMWindow> abDOMWindow;
        rv = windowWatcherSvc->GetWindowByName(
            NS_LITERAL_STRING("addressbookWindow").get(), nsnull,
            getter_AddRefs(abDOMWindow));
        if (NS_FAILED(rv)) {
            NS_ERROR("nsAbQueryLDAPMessageListener::OnLDAPInit():"
                     " error getting addressbook Window");
            return rv;
        }

        // get the auth prompter itself
        //
        nsCOMPtr<nsIAuthPrompt> authPrompter;
        rv = windowWatcherSvc->GetNewAuthPrompter(
            abDOMWindow, getter_AddRefs(authPrompter));
        if (NS_FAILED(rv)) {
            NS_ERROR("nsAbQueryLDAPMessageListener::OnLDAPInit():"
                     " error getting auth prompter");
            return rv;
        }

        // get authentication password, prompting the user if necessary
        //
        rv = authPrompter->PromptPassword(
            authPromptTitle.get(), authPromptText.get(),
            NS_ConvertUTF8toUCS2(spec).get(),
            nsIAuthPrompt::SAVE_PASSWORD_PERMANENTLY, getter_Copies(passwd),
            &status);
        if (NS_FAILED(rv) || status == PR_FALSE) {
            return NS_ERROR_FAILURE;
        }
    }

    // Initiate the LDAP operation
    nsCOMPtr<nsILDAPOperation> ldapOperation =
        do_CreateInstance(NS_LDAPOPERATION_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsILDAPMessageListener> proxyListener;
    rv = NS_GetProxyForObject(NS_UI_THREAD_EVENTQ,
                     NS_GET_IID(nsILDAPMessageListener),
                     NS_STATIC_CAST(nsILDAPMessageListener *, this),
                     PROXY_SYNC | PROXY_ALWAYS,
                     getter_AddRefs(proxyListener));

    rv = ldapOperation->Init(mConnection, proxyListener);
    NS_ENSURE_SUCCESS(rv, rv);

    // Bind
    rv = ldapOperation->SimpleBind(passwd);
    NS_ENSURE_SUCCESS(rv, rv);

    return rv;
}

nsresult nsAbQueryLDAPMessageListener::OnLDAPMessageBind (nsILDAPMessage *aMessage)
{
    if (mBound == PR_TRUE)
        return NS_OK;

    // see whether the bind actually succeeded
    //
    PRInt32 errCode;
    nsresult rv = aMessage->GetErrorCode(&errCode);
    NS_ENSURE_SUCCESS(rv, rv);

    if (errCode != nsILDAPErrors::SUCCESS) {

        // if the login failed, tell the wallet to forget this password
        //
        if ( errCode == nsILDAPErrors::INAPPROPRIATE_AUTH ||
             errCode == nsILDAPErrors::INVALID_CREDENTIALS ) {

            // make sure the wallet service has been created, and in doing so,
            // pass in a login-failed message to tell it to forget this passwd.
            //
            // apparently getting passwords stored in the wallet
            // doesn't require the service to be running, which is why
            // this might not exist yet.
            //
            rv = NS_CreateServicesFromCategory(
                "passwordmanager", mDirectoryQuery->mDirectoryUrl,
                "login-failed");
            if (NS_FAILED(rv)) {
                NS_ERROR("nsLDAPAutoCompleteSession::ForgetPassword(): error"
                         " creating password manager service");
                // not much to do at this point, though conceivably we could 
                // pop up a dialog telling the user to go manually delete
                // this password in the password manager.
            }
        } 

        // XXX this error should be propagated back to the UI somehow
        return NS_OK;
    }

    mSearchOperation = do_CreateInstance(NS_LDAPOPERATION_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIProxyObjectManager> proxyMgr = 
	    do_GetService(NS_XPCOMPROXY_CONTRACTID, &rv);
	  NS_ENSURE_SUCCESS(rv, rv);
    
    nsCOMPtr<nsILDAPMessageListener> proxyListener;
    rv = proxyMgr->GetProxyForObject( NS_UI_THREAD_EVENTQ, NS_GET_IID(nsILDAPMessageListener),
									this, PROXY_SYNC | PROXY_ALWAYS, getter_AddRefs(proxyListener));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = mSearchOperation->Init (mConnection, proxyListener);
    NS_ENSURE_SUCCESS(rv, rv);

    nsXPIDLCString dn;
    rv = mUrl->GetDn (getter_Copies (dn));
    NS_ENSURE_SUCCESS(rv, rv);

    PRInt32 scope;
    rv = mUrl->GetScope (&scope);
    NS_ENSURE_SUCCESS(rv, rv);

    nsXPIDLCString filter;
    rv = mUrl->GetFilter (getter_Copies (filter));
    NS_ENSURE_SUCCESS(rv, rv);

    CharPtrArrayGuard attributes;
    rv = mUrl->GetAttributes (attributes.GetSizeAddr (), attributes.GetArrayAddr ());
    NS_ENSURE_SUCCESS(rv, rv);

    // when we called SetSpec(), our spec contained UTF8 data
    // so when we call GetFilter(), it's not going to be ASCII.
    // it will be UTF8, so we need to convert from UTF8 to UCS2
    // see bug #124995
    rv = mSearchOperation->SearchExt (NS_ConvertUTF8toUCS2(dn).get(), scope,
            NS_ConvertUTF8toUCS2(filter).get(),
            attributes.GetSize (), attributes.GetArray (),
            mTimeOut, mResultLimit);
    NS_ENSURE_SUCCESS(rv, rv);

    mBound = PR_TRUE;
    return rv;
}

nsresult nsAbQueryLDAPMessageListener::OnLDAPMessageSearchEntry (nsILDAPMessage *aMessage,
        nsIAbDirectoryQueryResult** result)
{
    nsresult rv;
    nsCOMPtr<nsISupportsArray> propertyValues;

    CharPtrArrayGuard properties;
    rv = mQueryArguments->GetReturnProperties (properties.GetSizeAddr(), properties.GetArrayAddr());
    NS_ENSURE_SUCCESS(rv, rv);

    CharPtrArrayGuard attrs;
    rv = aMessage->GetAttributes(attrs.GetSizeAddr(), attrs.GetArrayAddr());
    NS_ENSURE_SUCCESS(rv, rv);

    nsCAutoString n;
    for (PRUint32 i = 0; i < properties.GetSize(); i++)
    {
        n.Assign (properties[i]);

        nsAbDirectoryQueryPropertyValue* _propertyValue = 0;
        if (n.Equals("card:nsIAbCard"))
        {
            // Meta property
            nsXPIDLString dn;
            rv = aMessage->GetDn (getter_Copies (dn));
            NS_ENSURE_SUCCESS(rv, rv);

            nsCOMPtr<nsIAbCard> card;
            rv = mDirectoryQuery->CreateCard (mUrl, NS_ConvertUCS2toUTF8(dn).get(), getter_AddRefs (card));
            NS_ENSURE_SUCCESS(rv, rv);

            PRBool hasSetCardProperty = PR_FALSE;
            rv = MozillaLdapPropertyRelator::createCardPropertyFromLDAPMessage (aMessage,
                    card,
                    &hasSetCardProperty);
            NS_ENSURE_SUCCESS(rv, rv);

            if (hasSetCardProperty == PR_FALSE)
                continue;

            _propertyValue = new nsAbDirectoryQueryPropertyValue(n.get (), card);
            if (_propertyValue == NULL)
                return NS_ERROR_OUT_OF_MEMORY;

        }
        else
        {
            const MozillaLdapPropertyRelation* p =
                MozillaLdapPropertyRelator::findLdapPropertyFromMozilla (n.get ());

            if (!p)
                continue;

            for (PRUint32 j = 0; j < attrs.GetSize(); j++)
            {
                if (nsCRT::strcasecmp (p->ldapProperty, attrs[j]) == 0)
                {
                    PRUnicharPtrArrayGuard vals;
                    rv = aMessage->GetValues (attrs[j], vals.GetSizeAddr(), vals.GetArrayAddr());
                    NS_ENSURE_SUCCESS(rv, rv);

                    if (vals.GetSize() == 0)
                        break;

                    _propertyValue = new nsAbDirectoryQueryPropertyValue(n.get (), vals[0]);
                    if (_propertyValue == NULL)
                        return NS_ERROR_OUT_OF_MEMORY;
                    break;
                }
            }
        }

        if (_propertyValue)
        {
            nsCOMPtr<nsIAbDirectoryQueryPropertyValue> propertyValue;
            propertyValue = _propertyValue;

            if (!propertyValues)
            {
                NS_NewISupportsArray(getter_AddRefs(propertyValues));
            }

            propertyValues->AppendElement (propertyValue);
        }
    }

    if (!propertyValues)
        return NS_OK;

    return QueryResultStatus (propertyValues, result,nsIAbDirectoryQueryResult::queryResultMatch);
}
nsresult nsAbQueryLDAPMessageListener::OnLDAPMessageSearchResult (nsILDAPMessage *aMessage,
        nsIAbDirectoryQueryResult** result)
{
    mDirectoryQuery->RemoveListener (mContextID);

    nsresult rv;
    PRInt32 errorCode;
    rv = aMessage->GetErrorCode(&errorCode);
    NS_ENSURE_SUCCESS(rv, rv);
    
    if (errorCode == nsILDAPErrors::SUCCESS || errorCode == nsILDAPErrors::SIZELIMIT_EXCEEDED)
        rv = QueryResultStatus (nsnull, result,nsIAbDirectoryQueryResult::queryResultComplete);
    else
        rv = QueryResultStatus (nsnull, result, nsIAbDirectoryQueryResult::queryResultError);

    return rv;
}
nsresult nsAbQueryLDAPMessageListener::QueryResultStatus (nsISupportsArray* properties,
        nsIAbDirectoryQueryResult** result, PRUint32 resultStatus)
{
    nsAbDirectoryQueryResult* _queryResult = new nsAbDirectoryQueryResult (
        mContextID,
        mQueryArguments,
        resultStatus,
        (resultStatus == nsIAbDirectoryQueryResult::queryResultMatch) ? properties : 0);

    if (!_queryResult)
        return NS_ERROR_OUT_OF_MEMORY;

    NS_IF_ADDREF(*result = _queryResult);
    return NS_OK;
}





NS_IMPL_THREADSAFE_ISUPPORTS1(nsAbLDAPDirectoryQuery, nsIAbDirectoryQuery)

nsAbLDAPDirectoryQuery::nsAbLDAPDirectoryQuery() :
    mInitialized(PR_FALSE),
    mCounter (1),
    mLock (nsnull)
{
    NS_INIT_ISUPPORTS();

}

nsAbLDAPDirectoryQuery::~nsAbLDAPDirectoryQuery()
{
    if(mLock)
        PR_DestroyLock (mLock);
}
nsresult nsAbLDAPDirectoryQuery::Initiate ()
{
    if(mInitialized == PR_TRUE)
        return NS_OK;

    mLock = PR_NewLock ();
    if(!mLock)
    {
        return NS_ERROR_OUT_OF_MEMORY;
    }

    mInitialized = PR_TRUE;

    return NS_OK;
}

NS_IMETHODIMP nsAbLDAPDirectoryQuery::DoQuery(nsIAbDirectoryQueryArguments* arguments,
        nsIAbDirectoryQueryResultListener* listener,
        PRInt32 resultLimit,
        PRInt32 timeOut,
        PRInt32* _retval)
{
    nsresult rv;

    rv = Initiate ();
    NS_ENSURE_SUCCESS(rv, rv);

    // Get the scope
    nsCAutoString scope;
    PRBool doSubDirectories;
    rv = arguments->GetQuerySubDirectories (&doSubDirectories);
    NS_ENSURE_SUCCESS(rv, rv);
    if (doSubDirectories == PR_TRUE)
        scope = "sub";
    else
        scope = "one";

    // Get the return attributes
    nsCAutoString returnAttributes;
    rv = getLdapReturnAttributes (arguments, returnAttributes);
    NS_ENSURE_SUCCESS(rv, rv);


    // Get the filter
    nsCOMPtr<nsISupports> supportsExpression;
    rv = arguments->GetExpression (getter_AddRefs (supportsExpression));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIAbBooleanExpression> expression (do_QueryInterface (supportsExpression, &rv));
    nsCString filter;
    rv = nsAbBoolExprToLDAPFilter::Convert (expression, filter);
    NS_ENSURE_SUCCESS(rv, rv);


    // Set up the search ldap url
    rv = GetLDAPURL (getter_AddRefs (mDirectoryUrl));
    NS_ENSURE_SUCCESS(rv, rv);
    
    nsCAutoString host;
    rv = mDirectoryUrl->GetAsciiHost(host);
    NS_ENSURE_SUCCESS(rv, rv);

    PRInt32 port;
    rv = mDirectoryUrl->GetPort(&port);
    NS_ENSURE_SUCCESS(rv, rv);

    nsXPIDLCString dn;
    rv = mDirectoryUrl->GetDn(getter_Copies (dn));
    NS_ENSURE_SUCCESS(rv, rv);

    PRUint32 options;
    rv = mDirectoryUrl->GetOptions(&options);
    NS_ENSURE_SUCCESS(rv,rv);

    nsCString ldapSearchUrlString;
    char* _ldapSearchUrlString = 
        PR_smprintf ("ldap%s://%s:%d/%s?%s?%s?%s",
                     (options & nsILDAPURL::OPT_SECURE) ? "s" : "",
                     host.get (),
                     port,
                     dn.get (),
                     returnAttributes.get (),
                     scope.get (),
                     filter.get ());
    if (!_ldapSearchUrlString)
        return NS_ERROR_OUT_OF_MEMORY;
    ldapSearchUrlString = _ldapSearchUrlString;
    PR_smprintf_free (_ldapSearchUrlString);

    nsCOMPtr<nsILDAPURL> url;
    url = do_CreateInstance(NS_LDAPURL_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = url->SetSpec(ldapSearchUrlString);
    NS_ENSURE_SUCCESS(rv, rv);

    // Get the ldap connection
    nsCOMPtr<nsILDAPConnection> ldapConnection;
    rv = GetLDAPConnection (getter_AddRefs (ldapConnection));
    NS_ENSURE_SUCCESS(rv, rv);

    // Initiate contextID for message listener
    PRInt32 contextID;
    // Enter lock
    {
        nsAutoLock lock (mLock);
        contextID = mCounter++;
    }
    // Exit lock


    // Initiate LDAP message listener
    nsCOMPtr<nsILDAPMessageListener> messageListener;
    nsAbQueryLDAPMessageListener* _messageListener =
        new nsAbQueryLDAPMessageListener (
                this,
                contextID,
                url,
                ldapConnection,
                arguments,
                listener,
                resultLimit,
                timeOut);
    if (_messageListener == NULL)
            return NS_ERROR_OUT_OF_MEMORY;
    messageListener = _messageListener;
    nsVoidKey key (NS_REINTERPRET_CAST(void *,contextID));

    // Enter lock
    {
        nsAutoLock lock1(mLock);
        mListeners.Put (&key, _messageListener);
    }
    // Exit lock

    *_retval = contextID;

    // Now lets initialize the LDAP connection properly. We'll kick
    // off the bind operation in the callback function, |OnLDAPInit()|.
    rv = ldapConnection->Init(host.get(), port, options, mLogin.get(),
                              messageListener);
    NS_ENSURE_SUCCESS(rv, rv);

    return rv;
}

/* void stopQuery (in long contextID); */
NS_IMETHODIMP nsAbLDAPDirectoryQuery::StopQuery(PRInt32 contextID)
{
    nsresult rv;

    rv = Initiate ();
    NS_ENSURE_SUCCESS(rv, rv);

    nsAbQueryLDAPMessageListener* _messageListener;  

    // Enter Lock
    {
        nsAutoLock lock(mLock);

        nsVoidKey key (NS_REINTERPRET_CAST(void *,contextID));

         _messageListener = (nsAbQueryLDAPMessageListener* )mListeners.Remove (&key);

        if (!_messageListener)
            return NS_OK;

    }
    // Exit Lock

    rv = _messageListener->Cancel ();

    return rv;
}

nsresult nsAbLDAPDirectoryQuery::RemoveListener (PRInt32 contextID)
{
    nsresult rv;

    rv = Initiate ();
    NS_ENSURE_SUCCESS(rv, rv);

    nsAutoLock lock(mLock);

    nsVoidKey key (NS_REINTERPRET_CAST(void *,contextID));

    mListeners.Remove (&key);

    return NS_OK;
}



nsresult nsAbLDAPDirectoryQuery::getLdapReturnAttributes (
    nsIAbDirectoryQueryArguments* arguments,
    nsCString& returnAttributes)
{
    nsresult rv;

    CharPtrArrayGuard properties;
    rv = arguments->GetReturnProperties (properties.GetSizeAddr(), properties.GetArrayAddr());
    NS_ENSURE_SUCCESS(rv, rv);

    nsCAutoString n;
    for (PRUint32 i = 0; i < properties.GetSize(); i++)
    {
        n.Assign (properties[i]);

        if (n.Equals("card:nsIAbCard"))
        {
            // Meta property
            // require all attributes
            //
            rv = MozillaLdapPropertyRelator::GetAllSupportedLDAPAttributes(returnAttributes);
            NS_ASSERTION(NS_SUCCEEDED(rv), "GetAllSupportedLDAPAttributes failed");
            break;
        }

        const MozillaLdapPropertyRelation* p=
            MozillaLdapPropertyRelator::findLdapPropertyFromMozilla (n.get ());
        if (!p)
            continue;

        if (i)
            returnAttributes.Append (",");

        returnAttributes.Append (p->ldapProperty);
    }

    return rv;
}

