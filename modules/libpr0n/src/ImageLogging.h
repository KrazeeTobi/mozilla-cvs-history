/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 * The Original Code is mozilla.org code.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 2001 Netscape Communications Corporation.
 * All Rights Reserved.
 * 
 * Contributor(s):
 *   Stuart Parmenter <pavlov@netscape.com>
 */

#include "prlog.h"

#include "nsString.h"

#if defined(PR_LOGGING)

extern PRLogModuleInfo *gImgLog;

class LogScope {
public:
  LogScope(void *from, const nsAReadableCString &fn) :
    mFrom(from), mFunc(fn)
  {
    PR_LOG(gImgLog, PR_LOG_DEBUG, ("[this=%p] %s {ENTER}\n",
                                   mFrom, mFunc.get()));
  }

  ~LogScope() {
    PR_LOG(gImgLog, PR_LOG_DEBUG, ("[this=%p] %s {EXIT}\n",
                                   mFrom, mFunc.get()));
  }

private:
  void *mFrom;
  nsCAutoString mFunc;
};


#define LOG_SCOPE(s) \
  LogScope LOG_SCOPE_TMP_VAR##__LINE__(this, NS_LITERAL_CSTRING(s))

#else
#define LOG_SCOPE(s)
#define gImgLog
#endif
