/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code, 
 * released March 31, 1998. 
 *
 * The Initial Developer of the Original Code is Netscape Communications 
 * Corporation.  Portions created by Netscape are 
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 *
 * Contributors:
 *     Douglas Turner <dougt@netscape.com>
 *     Daniel Veditz <dveditz@netscape.com>
 */

#include "nsIXPINotifier.h"
#include "nsTopProgressNotifier.h"

nsTopProgressNotifier::nsTopProgressNotifier()
{
    NS_INIT_ISUPPORTS();
    mNotifiers = new nsVector();
    mActive = 0;
}

nsTopProgressNotifier::~nsTopProgressNotifier()
{
    if (mNotifiers)
    {
        PRUint32 i=0;
        for (; i < mNotifiers->GetSize(); i++) 
        {
            nsIXPINotifier* element = (nsIXPINotifier*)mNotifiers->Get(i);
            delete element;
        }

        mNotifiers->RemoveAll();
        delete (mNotifiers);
    }
}


NS_IMPL_ISUPPORTS(nsTopProgressNotifier, nsIXPINotifier::GetIID());


long
nsTopProgressNotifier::RegisterNotifier(nsIXPINotifier * newNotifier)
{
    NS_IF_ADDREF( newNotifier );
    return mNotifiers->Add( newNotifier );
}


void
nsTopProgressNotifier::UnregisterNotifier(long id)
{
    nsIXPINotifier *item = (nsIXPINotifier*)mNotifiers->Get(id);
    NS_IF_RELEASE(item);
    mNotifiers->Set(id, NULL);
}



NS_IMETHODIMP
nsTopProgressNotifier::BeforeJavascriptEvaluation()
{
    if (mActive)
        mActive->BeforeJavascriptEvaluation();

    if (mNotifiers)
    {
        PRUint32 i=0;
        for (; i < mNotifiers->GetSize(); i++) 
        {
            nsIXPINotifier* element = (nsIXPINotifier*)mNotifiers->Get(i);
            if (element != NULL)
                element->BeforeJavascriptEvaluation();
        }
    }
    
    return NS_OK;
}

NS_IMETHODIMP
nsTopProgressNotifier::AfterJavascriptEvaluation(void)
{
    if (mActive)
        mActive->AfterJavascriptEvaluation();

    if (mNotifiers)
    {
        PRUint32 i=0;
        for (; i < mNotifiers->GetSize(); i++) 
        {
            nsIXPINotifier* element = (nsIXPINotifier*)mNotifiers->Get(i);
            if (element != NULL)
                element->AfterJavascriptEvaluation();
        }
    }
    return NS_OK;
}

NS_IMETHODIMP
nsTopProgressNotifier::InstallStarted(const char* UIPackageName)
{
    if (mActive)
        mActive->InstallStarted(UIPackageName);

    if (mNotifiers)
    {
        PRUint32 i=0;
        for (; i < mNotifiers->GetSize(); i++) 
        {
            nsIXPINotifier* element = (nsIXPINotifier*)mNotifiers->Get(i);
            if (element != NULL)
                element->InstallStarted(UIPackageName);
        }
    }
    return NS_OK;
}

NS_IMETHODIMP
nsTopProgressNotifier::ItemScheduled( const char* message )
{
    long rv = 0;

    if (mActive && mActive->ItemScheduled( message ) != 0 )
        rv = -1;

    if (mNotifiers)
    {
        PRUint32 i=0;
        for (; i < mNotifiers->GetSize(); i++) 
        {
            nsIXPINotifier* element = (nsIXPINotifier*)mNotifiers->Get(i);
            if (element != NULL)
                if (element->ItemScheduled( message ) != 0)
                    rv = -1;
        }
    }

    return rv;
}

NS_IMETHODIMP
nsTopProgressNotifier::InstallFinalization( const char* message, PRInt32 itemNum, PRInt32 totNum )
{
    if (mActive)
        mActive->InstallFinalization( message, itemNum, totNum );

    if (mNotifiers)
    {
        PRUint32 i=0;
        for (; i < mNotifiers->GetSize(); i++) 
        {
            nsIXPINotifier* element = (nsIXPINotifier*)mNotifiers->Get(i);
            if (element != NULL)
                element->InstallFinalization( message, itemNum, totNum );
        }
    }
    return NS_OK;
}

NS_IMETHODIMP
nsTopProgressNotifier::InstallAborted(void)
{
    if (mActive)
        mActive->InstallAborted();

    if (mNotifiers)
    {
        PRUint32 i=0;
        for (; i < mNotifiers->GetSize(); i++) 
        {
            nsIXPINotifier* element = (nsIXPINotifier*)mNotifiers->Get(i);
            if (element != NULL)
                element->InstallAborted();
        }
    }
   return NS_OK;
}

NS_IMETHODIMP
nsTopProgressNotifier::LogComment(const char* comment)
{
   if (mNotifiers)
    {
        PRUint32 i=0;
        for (; i < mNotifiers->GetSize(); i++) 
        {
            nsIXPINotifier* element = (nsIXPINotifier*)mNotifiers->Get(i);
            if (element != NULL)
                element->LogComment(comment);
        }
    }
   return NS_OK;
}

