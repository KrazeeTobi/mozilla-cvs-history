/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
#include "nsCOMPtr.h"
#include "nsIMsgMailSession.h"
#include "nsIMsgIdentity.h"
#include "nsMsgCompPrefs.h"
#include "nsMsgBaseCID.h"
#include "nsIPref.h"

static NS_DEFINE_CID(kCMsgMailSessionCID, NS_MSGMAILSESSION_CID); 
static NS_DEFINE_CID(kPrefCID, NS_PREF_CID);

nsMsgCompPrefs::nsMsgCompPrefs()
{
	nsresult res;

	m_organization = nsnull;
	m_userFullName = nsnull;
	m_userEmail = nsnull;
	m_replyTo = nsnull;
	m_composeHTML = PR_TRUE;
	m_wrapColumn = 72;

	// get the current identity from the mail session....
	NS_WITH_SERVICE(nsIMsgMailSession, mailSession, kCMsgMailSessionCID, &res); 
	if (NS_SUCCEEDED(res) && mailSession)
	{
		nsCOMPtr<nsIMsgIdentity> identity;
		res = mailSession->GetCurrentIdentity(getter_AddRefs(identity));
		if (NS_SUCCEEDED(res) && identity)
		{
			identity->GetOrganization(&m_organization);

			identity->GetFullName(&m_userFullName);
				
			identity->GetEmail(&m_userEmail);

			identity->GetReplyTo(&m_replyTo);

			identity->GetComposeHtml(&m_composeHTML);
			NS_WITH_SERVICE(nsIPref, prefs, kPrefCID, &res); 
			if (NS_SUCCEEDED(res) && prefs) 
			{
				res = prefs->GetIntPref("mail.wraplength", &m_wrapColumn);
			}
		}
		else
			NS_ASSERTION(0, "no current identity found for this user (a)....");
	}
	else
		NS_ASSERTION(0, "no current identity found for this user (b)....");
}

nsMsgCompPrefs::~nsMsgCompPrefs()
{
	PR_FREEIF(m_organization);
	PR_FREEIF(m_userFullName);
	PR_FREEIF(m_userEmail);
	PR_FREEIF(m_replyTo);
}
