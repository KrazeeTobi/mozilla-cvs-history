/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
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
 * The Original Code is the Mozilla browser.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications, Inc.  Portions created by Netscape are
 * Copyright (C) 1999, Mozilla.  All Rights Reserved.
 * 
 * Author:
 *   Adam Lock <adamlock@netscape.com>
 *
 * Contributor(s):
 */

#include "nsEmbedAPI.h"

#ifndef WIN32
#error This file is for Win32!
#endif

#ifdef MOZ_SUPPORTS_EMBEDDING_EVENT_PROCESSING

#include "nsWidgetsCID.h"
#include "nsITimer.h"
#include "nsITimerQueue.h"

static NS_DEFINE_CID(kTimerManagerCID, NS_TIMERMANAGER_CID);

nsresult NS_DoIdleEmbeddingStuff()
{
    nsresult rv = NS_ERROR_FAILURE;
    nsCOMPtr<nsITimerQueue> timerQueue(do_GetService(kTimerManagerCID, &rv));
    if (NS_FAILED(rv))
    {
        return NS_ERROR_FAILURE;
    }

    if (timerQueue->HasReadyTimers(NS_PRIORITY_LOWEST))
    {
        MSG wmsg;
        do
        {
          timerQueue->FireNextReadyTimer(NS_PRIORITY_LOWEST);
        } while (timerQueue->HasReadyTimers(NS_PRIORITY_LOWEST) &&
                 !::PeekMessage(&wmsg, NULL, 0, 0, PM_NOREMOVE));
    }
    return NS_OK;
}

nsresult NS_HandleEmbeddingEvent(nsEmbedNativeEvent &aEvent, PRBool &aWasHandled)
{
    aWasHandled = PR_FALSE;
    return NS_OK;
}

#endif /* MOZ_SUPPORTS_EMBEDDING_EVENT_PROCESSING */
