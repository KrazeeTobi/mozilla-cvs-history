/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
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

#include "nsDebug.h"
#include "prlog.h"

#if defined(_WIN32)
#include <windows.h>
#endif

/**
 * Implementation of the nsDebug methods. Note that this code is
 * always compiled in, in case some other module that uses it is
 * compiled with debugging even if this library is not.
 */

static PRLogModuleInfo* gDebugLog;

static void InitLog(void)
{
  if (0 == gDebugLog) {
    gDebugLog = PR_NewLogModule("nsDebug");
    gDebugLog->level = PR_LOG_DEBUG;
  }
}

NS_COM void nsDebug::Abort(const char* aFile, PRIntn aLine)
{
  InitLog();
  PR_LOG(gDebugLog, PR_LOG_ERROR,
         ("Abort: at file %s, line %d", aFile, aLine));
  PR_LogFlush();
#if defined(_WIN32)
  long* __p = (long*) 0x7;
  *__p = 0x7;
#endif
}

NS_COM void nsDebug::Break(const char* aFile, PRIntn aLine)
{
  InitLog();
  PR_LOG(gDebugLog, PR_LOG_ERROR,
         ("Break: at file %s, line %d", aFile, aLine));
  PR_LogFlush();
  //XXX this works on win32 only for now. For all the other platforms call Abort
#if defined(_WIN32)
  ::DebugBreak();
#else
  Abort(aFile, aLine);
#endif
}

NS_COM void nsDebug::PreCondition(const char* aStr, const char* aExpr,
                                  const char* aFile, PRIntn aLine)
{
  InitLog();
  PR_LOG(gDebugLog, PR_LOG_ERROR,
         ("PreCondition: \"%s\" (%s) at file %s, line %d", aStr, aExpr,
          aFile, aLine));
  Break(aFile, aLine);
}

NS_COM void nsDebug::PostCondition(const char* aStr, const char* aExpr,
                                   const char* aFile, PRIntn aLine)
{
  InitLog();
  PR_LOG(gDebugLog, PR_LOG_ERROR,
         ("PostCondition: \"%s\" (%s) at file %s, line %d", aStr, aExpr,
          aFile, aLine));
  Break(aFile, aLine);
}

NS_COM void nsDebug::Assertion(const char* aStr, const char* aExpr,
                               const char* aFile, PRIntn aLine)
{
  InitLog();
  PR_LOG(gDebugLog, PR_LOG_ERROR,
         ("Assertion: \"%s\" (%s) at file %s, line %d", aStr, aExpr,
          aFile, aLine));
  Break(aFile, aLine);
}

NS_COM void nsDebug::NotYetImplemented(const char* aMessage,
                                       const char* aFile, PRIntn aLine)
{
  InitLog();
  PR_LOG(gDebugLog, PR_LOG_ERROR,
         ("NotYetImplemented: \"%s\" at file %s, line %d", aMessage,
          aFile, aLine));
  Break(aFile, aLine);
}

NS_COM void nsDebug::NotReached(const char* aMessage,
                                const char* aFile, PRIntn aLine)
{
  InitLog();
  PR_LOG(gDebugLog, PR_LOG_ERROR,
         ("NotReached: \"%s\" at file %s, line %d", aMessage, aFile, aLine));
  Break(aFile, aLine);
}

NS_COM void nsDebug::Error(const char* aMessage,
                           const char* aFile, PRIntn aLine)
{
  InitLog();
  PR_LOG(gDebugLog, PR_LOG_ERROR,
         ("Error: \"%s\" at file %s, line %d", aMessage, aFile, aLine));
  Break(aFile, aLine);
}

NS_COM void nsDebug::Warning(const char* aMessage,
                             const char* aFile, PRIntn aLine)
{
  InitLog();
  PR_LOG(gDebugLog, PR_LOG_ERROR,
         ("Warning: \"%s\" at file %s, line %d", aMessage, aFile, aLine));
}
