/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is the Netscape Portable Runtime (NSPR).
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are 
 * Copyright (C) 1998-2000 Netscape Communications Corporation.  All
 * Rights Reserved.
 * 
 * Contributor(s):
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

/*
 * OS/2 interval timers
 *
 */

#include "primpl.h"

static PRBool useHighResTimer = PR_FALSE;
PRIntervalTime _os2_ticksPerSec = -1;
PRIntn _os2_bitShift = 0;
PRInt32 _os2_highMask = 0;
   
void
_PR_MD_INTERVAL_INIT()
{
    ULONG timerFreq = 0; /* OS/2 high-resolution timer frequency in Hz */
    APIRET rc = DosTmrQueryFreq(&timerFreq);
    if (NO_ERROR == rc) {
        useHighResTimer = PR_TRUE;
        PR_ASSERT(timerFreq != 0);
        while (timerFreq > PR_INTERVAL_MAX) {
            timerFreq >>= 1;
            _os2_bitShift++;
            _os2_highMask = (_os2_highMask << 1)+1;
        }

        _os2_ticksPerSec = timerFreq;
        PR_ASSERT(_os2_ticksPerSec > PR_INTERVAL_MIN);
    }
}

PRIntervalTime
_PR_MD_GET_INTERVAL()
{
    if (useHighResTimer) {
        QWORD timestamp;
        PRInt32 top;
        APIRET rc = DosTmrQueryTime(&timestamp);
        if (NO_ERROR != rc) {
            return -1;
        }
        /* Sadly, nspr requires the interval to range from 1000 ticks per
         * second to only 100000 ticks per second. DosTmrQueryTime is too
         * high resolution...
         */
        top = timestamp.ulHi & _os2_highMask;
        top = top << (32 - _os2_bitShift);
        timestamp.ulLo = timestamp.ulLo >> _os2_bitShift;   
        timestamp.ulLo = timestamp.ulLo + top; 
        return (PRUint32)timestamp.ulLo;
    } else {
        ULONG msCount = -1;
        DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, &msCount, sizeof(msCount));
        return msCount;
    }
}

PRIntervalTime
_PR_MD_INTERVAL_PER_SEC()
{
    if (useHighResTimer) {
        return _os2_ticksPerSec;
    } else {
        return 1000;
    }
}
