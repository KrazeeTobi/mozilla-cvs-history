/* 
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is the mozilla.org LDAP XPCOM component.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are 
 * Copyright (C) 2000 Netscape Communications Corporation.  All
 * Rights Reserved.
 * 
 * Contributor(s): Dan Mosedale <dmose@mozilla.org>n
 * 
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License Version 2 or later (the
 * "GPL"), in which case the provisions of the GPL are applicable 
 * instead of those above.  If you wish to allow use of your 
 * version of this file only under the terms of the GPL and not to
 * allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by
 * the GPL.  If you do not delete the provisions above, a recipient
 * may use your version of this file under either the MPL or the
 * GPL.
 */

#include <errno.h>
#include <unistd.h>
#include "nspr.h"
#include "ldap.h"
#include "nsCOMPtr.h"
#include "nsIComponentManager.h"
#include "nsILDAPConnection.h"
#include "nsILDAPOperation.h"
#include "nsILDAPMessage.h"
#include "nsLDAPChannel.h"

NS_METHOD
lds(class nsLDAPChannel *chan, const char *url)
{
    nsCOMPtr<nsILDAPConnection> myConnection;
    nsCOMPtr<nsILDAPMessage> myMessage;
    nsCOMPtr<nsILDAPOperation> myOperation;
    PRInt32 returnCode;
    char *errString;
    nsresult rv;

    // create an LDAP connection
    //
    myConnection = do_CreateInstance("mozilla.network.ldapconnection", &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    // initialize it with the defaults
    // XXX should return reasonable err msg, not assert
    //	
    rv = myConnection->Init("nsdirectory.netscape.com", LDAP_PORT);
    NS_ENSURE_SUCCESS(rv, rv);

    // create and initialize an LDAP operation on the new connection
    //	
    myOperation = do_CreateInstance("mozilla.network.ldapoperation", &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = myOperation->SetConnection(myConnection);
    NS_ENSURE_SUCCESS(rv, rv);

#ifdef NO_URL_SEARCH
#ifdef DEBUG_dmose
    PR_fprintf(PR_STDERR, "starting bind\n");
#endif
  
  if ( !myOperation->SimpleBind(NULL, NULL) ) {
    (void)myConnection->GetErrorString(&errString);
    PR_fprintf(PR_STDERR, "ldap_simple_bind: %s\n", errString);
    return NS_ERROR_FAILURE;
  }

#ifdef DEBUG_dmose
  PR_fprintf(PR_STDERR, "waiting for bind to complete");
#endif

  myMessage = new nsLDAPMessage(myOperation);
  returnCode = myOperation->Result(0, &nullTimeval, myMessage);

  while (returnCode == 0 ) {
    returnCode =   
      myOperation->Result(0, &nullTimeval, myMessage);
    usleep(20);
#ifdef DEBUG_dmose
    PR_fprintf(PR_STDERR,".");
#endif
  }

  switch (returnCode) {
  case LDAP_RES_BIND:
    // success
    break;
  case -1:
    (void)myConnection->GetErrorString(&errString);
    PR_fprintf(PR_STDERR,
	     "myOperation->Result() [myOperation->SimpleBind]: %s: errno=%d\n",
	     errString, errno);
    ldap_memfree(errString); 
    return NS_ERROR_FAILURE;
    break;
  default:
    PR_fprintf(PR_STDERR, 
	       "\nmyOperation->Result() returned unexpected value: %d", 
	       returnCode);
    return NS_ERROR_FAILURE;
  }
#ifdef DEBUG_dmose
  PR_fprintf(PR_STDERR, "bound\n");
#endif

#endif 

    // start search
    //
    PR_fprintf(PR_STDERR, "starting search\n");

    myOperation = do_CreateInstance("mozilla.network.ldapoperation", &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = myOperation->SetConnection(myConnection);
    NS_ENSURE_SUCCESS(rv, rv);

    // XXX what about timeouts? 
    // XXX failure is a reasonable thing; don't assert
    //
    rv = myOperation->UrlSearch(url, PR_FALSE);
    NS_ENSURE_SUCCESS(rv,rv);

    // poll for results
    //
#ifdef DEBUG_dmose
    PR_fprintf(PR_STDERR, "polling search operation");
#endif
    returnCode = LDAP_SUCCESS;
    while ( returnCode != LDAP_RES_SEARCH_RESULT ) {

	PR_fprintf(PR_STDERR,".");

	// XXX is 0 the right value?
	//
	rv = myOperation->Result(LDAP_MSG_ONE, (PRIntervalTime)0,
				 getter_AddRefs(myMessage), &returnCode);

	switch (returnCode) {
	case -1: // something went wrong
	    (void)myConnection->GetErrorString(&errString);
#ifdef DEBUG
	    PR_fprintf(PR_STDERR,
		    "\nmyOperation->Result() [URLSearch]: %s: errno=%d\n",
		    errString, errno);
#endif
	    ldap_memfree(errString); 
	    return NS_ERROR_FAILURE;
	case 0: // nothing's been returned yet
	    break; 	

	default:
	    chan->OnLDAPMessage(myMessage, returnCode);
	    break;

	}
	myMessage = 0;

	PR_Sleep(200);
    }

    return NS_OK;
}
