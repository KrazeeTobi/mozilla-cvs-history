/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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
 * Copyright (C) 1999 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

/* Implementation of xptiZipItem. */

#include "xptiprivate.h"

MOZ_DECL_CTOR_COUNTER(xptiZipItem)

xptiZipItem::xptiZipItem()
    :   mName(nsnull),
        mGuts(nsnull)
{
    MOZ_COUNT_CTOR(xptiZipItem);
    // empty
}

xptiZipItem::xptiZipItem(const char*     aName,
                         xptiWorkingSet* aWorkingSet,
                         XPTHeader*      aHeader /*= nsnull */)

    :   mName(aName),
        mGuts(nsnull)
{
    MOZ_COUNT_CTOR(xptiZipItem);

    NS_ASSERTION(aWorkingSet,"bad param");
    mName = XPT_STRDUP(aWorkingSet->GetStringArena(), aName);

    if(aHeader)
        SetHeader(aHeader);
}

xptiZipItem::xptiZipItem(const xptiZipItem& r, xptiWorkingSet* aWorkingSet,
                         PRBool cloneGuts)
    :   mName(nsnull),
        mGuts(nsnull)
{
    MOZ_COUNT_CTOR(xptiZipItem);

    NS_ASSERTION(aWorkingSet,"bad param");
    mName = XPT_STRDUP(aWorkingSet->GetStringArena(), r.mName);

    if(cloneGuts && r.mGuts)
        mGuts = r.mGuts->Clone();
}

xptiZipItem::~xptiZipItem()
{
    MOZ_COUNT_DTOR(xptiZipItem);

    if(mGuts)
        delete mGuts;
}

PRBool 
xptiZipItem::SetHeader(XPTHeader* aHeader)
{
    NS_ASSERTION(!mGuts,"bad state");
    NS_ASSERTION(aHeader,"bad param");

    mGuts = new xptiTypelibGuts(aHeader);
    if(mGuts && !mGuts->IsValid())
    {
        delete mGuts;
        mGuts = nsnull;
    } 
    return mGuts != nsnull;
}
