/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

#include "nsISupports.h"
#include "nsVoidArray.h"
#include "prprf.h"
#include "prlog.h"
#include "plstr.h"
#include <stdlib.h>
#include "nsCOMPtr.h"
#include "nsCRT.h"
#include <math.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(linux) && defined(__GLIBC__) && (defined(__i386) || defined(PPC))
#include <setjmp.h>

//
// On glibc 2.1, the Dl_info api defined in <dlfcn.h> is only exposed
// if __USE_GNU is defined.  I suppose its some kind of standards
// adherence thing.
//
#if (__GLIBC_MINOR__ >= 1)
#define __USE_GNU
#endif

#include <dlfcn.h>
#endif

#ifdef HAVE_LIBDL
#include <dlfcn.h>
#endif

#if defined(XP_MAC) && !TARGET_CARBON
#include "macstdlibextras.h"
#endif

////////////////////////////////////////////////////////////////////////////////

NS_COM void 
NS_MeanAndStdDev(double n, double sumOfValues, double sumOfSquaredValues,
                 double *meanResult, double *stdDevResult)
{
  double mean = 0.0, var = 0.0, stdDev = 0.0;
  if (n > 0.0 && sumOfValues >= 0) {
    mean = sumOfValues / n;
    double temp = (n * sumOfSquaredValues) - (sumOfValues * sumOfValues);
    if (temp < 0.0 || n <= 1)
      var = 0.0;
    else
      var = temp / (n * (n - 1));
    // for some reason, Windows says sqrt(0.0) is "-1.#J" (?!) so do this:
    stdDev = var != 0.0 ? sqrt(var) : 0.0;
  }
  *meanResult = mean;
  *stdDevResult = stdDev;
}

////////////////////////////////////////////////////////////////////////////////

#ifdef NS_BUILD_REFCNT_LOGGING
#include "plhash.h"

#if defined(NS_MT_SUPPORTED)
#include "prlock.h"

static PRLock* gTraceLock;

#define LOCK_TRACELOG()   PR_Lock(gTraceLock)
#define UNLOCK_TRACELOG() PR_Unlock(gTraceLock)
#else /* ! NT_MT_SUPPORTED */
#define LOCK_TRACELOG()
#define UNLOCK_TRACELOG()
#endif /* ! NS_MT_SUPPORTED */

static PLHashTable* gBloatView;
static PLHashTable* gTypesToLog;
static PLHashTable* gObjectsToLog;
static PLHashTable* gSerialNumbers;
static PRInt32 gNextSerialNumber;

static PRBool gLogging;
static PRBool gLogToLeaky;
static PRBool gLogLeaksOnly;

static void (*leakyLogAddRef)(void* p, int oldrc, int newrc);
static void (*leakyLogRelease)(void* p, int oldrc, int newrc);

static PRBool gInitialized = PR_FALSE;
static FILE *gBloatLog = nsnull;
static FILE *gRefcntsLog = nsnull;
static FILE *gAllocLog = nsnull;
static FILE *gLeakyLog = nsnull;

#define XPCOM_REFCNT_TRACK_BLOAT  0x1
#define XPCOM_REFCNT_LOG_ALL      0x2
#define XPCOM_REFCNT_LOG_SOME     0x4
#define XPCOM_REFCNT_LOG_TO_LEAKY 0x8

// Should only use this on NS_LOSING_ARCHITECTURE...
#define XPCOM_REFCNT_LOG_CALLS    0x10
#define XPCOM_REFCNT_LOG_NEW      0x20

////////////////////////////////////////////////////////////////////////////////

struct GatherArgs {
  nsTraceRefcntStatFunc func;
  void* closure;
};

class BloatEntry {
public:
  BloatEntry(const char* className, PRUint32 classSize)
    : mClassName(className), mClassSize(classSize) { 
    Clear(&mNewStats);
    Clear(&mAllStats);
    mTotalLeaked = 0;
  }

  ~BloatEntry() {}

  PRUint32 GetClassSize() { return (PRUint32)mClassSize; }
  const char* GetClassName() { return mClassName; }

  static void Clear(nsTraceRefcntStats* stats) {
    stats->mAddRefs = 0;
    stats->mReleases = 0;
    stats->mCreates = 0;
    stats->mDestroys = 0;
    stats->mRefsOutstandingTotal = 0;
    stats->mRefsOutstandingSquared = 0;
    stats->mObjsOutstandingTotal = 0;
    stats->mObjsOutstandingSquared = 0;
  }

  void Accumulate() {
      mAllStats.mAddRefs += mNewStats.mAddRefs;
      mAllStats.mReleases += mNewStats.mReleases;
      mAllStats.mCreates += mNewStats.mCreates;
      mAllStats.mDestroys += mNewStats.mDestroys;
      mAllStats.mRefsOutstandingTotal += mNewStats.mRefsOutstandingTotal;
      mAllStats.mRefsOutstandingSquared += mNewStats.mRefsOutstandingSquared;
      mAllStats.mObjsOutstandingTotal += mNewStats.mObjsOutstandingTotal;
      mAllStats.mObjsOutstandingSquared += mNewStats.mObjsOutstandingSquared;
      Clear(&mNewStats);
  }

  void AddRef(nsrefcnt refcnt) {
    mNewStats.mAddRefs++;
    if (refcnt == 1) {
      Ctor();
    }
    AccountRefs();
  }

  void Release(nsrefcnt refcnt) {
    mNewStats.mReleases++;
    if (refcnt == 0) {
      Dtor();
    }
    AccountRefs();
  }

  void Ctor() {
    mNewStats.mCreates++;
    AccountObjs();
  }

  void Dtor() {
    mNewStats.mDestroys++;
    AccountObjs();
  }

  void AccountRefs() {
    PRInt32 cnt = (mNewStats.mAddRefs - mNewStats.mReleases);
    mNewStats.mRefsOutstandingTotal += cnt;
    mNewStats.mRefsOutstandingSquared += cnt * cnt;
  }

  void AccountObjs() {
    PRInt32 cnt = (mNewStats.mCreates - mNewStats.mDestroys);
    mNewStats.mObjsOutstandingTotal += cnt;
    mNewStats.mObjsOutstandingSquared += cnt * cnt;
  }

  static PRIntn DumpEntry(PLHashEntry *he, PRIntn i, void *arg) {
    BloatEntry* entry = (BloatEntry*)he->value;
    if (entry) {
      entry->Accumulate();
      NS_STATIC_CAST(nsVoidArray*, arg)->AppendElement(entry);
    }
    return HT_ENUMERATE_NEXT;
  }

  static PRIntn TotalEntries(PLHashEntry *he, PRIntn i, void *arg) {
    BloatEntry* entry = (BloatEntry*)he->value;
    if (entry && nsCRT::strcmp(entry->mClassName, "TOTAL") != 0) {
      entry->Total((BloatEntry*)arg);
    }
    return HT_ENUMERATE_NEXT;
  }

  void Total(BloatEntry* total) {
    total->mAllStats.mAddRefs += mNewStats.mAddRefs + mAllStats.mAddRefs;
    total->mAllStats.mReleases += mNewStats.mReleases + mAllStats.mReleases;
    total->mAllStats.mCreates += mNewStats.mCreates + mAllStats.mCreates;
    total->mAllStats.mDestroys += mNewStats.mDestroys + mAllStats.mDestroys;
    total->mAllStats.mRefsOutstandingTotal += mNewStats.mRefsOutstandingTotal + mAllStats.mRefsOutstandingTotal;
    total->mAllStats.mRefsOutstandingSquared += mNewStats.mRefsOutstandingSquared + mAllStats.mRefsOutstandingSquared;
    total->mAllStats.mObjsOutstandingTotal += mNewStats.mObjsOutstandingTotal + mAllStats.mObjsOutstandingTotal;
    total->mAllStats.mObjsOutstandingSquared += mNewStats.mObjsOutstandingSquared + mAllStats.mObjsOutstandingSquared;
    PRInt32 count = (mNewStats.mCreates + mAllStats.mCreates);
    total->mClassSize += mClassSize * count;    // adjust for average in DumpTotal
    total->mTotalLeaked += (PRInt32)(mClassSize *
                                     ((mNewStats.mCreates + mAllStats.mCreates)
                                      -(mNewStats.mDestroys + mAllStats.mDestroys)));
  }

  nsresult DumpTotal(PRUint32 nClasses, FILE* out) {
    mClassSize /= mAllStats.mCreates;
    return Dump(-1, out, nsTraceRefcnt::ALL_STATS);
  }

  static PRIntn DestroyEntry(PLHashEntry *he, PRIntn i, void *arg) {
    BloatEntry* entry = (BloatEntry*)he->value;
    if (entry) {
      delete entry;
    }
    return HT_ENUMERATE_REMOVE | HT_ENUMERATE_NEXT;
  }

  static PRIntn GatherEntry(PLHashEntry *he, PRIntn i, void *arg) {
    BloatEntry* entry = (BloatEntry*)he->value;
    GatherArgs* ga = (GatherArgs*) arg;
    if (arg && entry && ga->func) {
      PRBool stop = (*ga->func)(entry->mClassName, (PRUint32)entry->mClassSize,
                                &entry->mNewStats, &entry->mAllStats,
                                ga->closure);
      if (stop) {
        return HT_ENUMERATE_STOP;
      }
    }
    return HT_ENUMERATE_NEXT;
  }

  static PRBool HaveLeaks(nsTraceRefcntStats* stats) {
    return ((stats->mAddRefs != stats->mReleases) ||
            (stats->mCreates != stats->mDestroys));
  }

  static nsresult PrintDumpHeader(FILE* out, const char* msg) {
    fprintf(out, "\n== BloatView: %s\n\n", msg);
    fprintf(out, 
        "     |<----------------Class--------------->|<-----Bytes------>|<----------------Objects---------------->|<--------------References-------------->|\n");
    fprintf(out,
        "                                              Per-Inst   Leaked    Total      Rem      Mean       StdDev     Total      Rem      Mean       StdDev\n");
    return NS_OK;
  }

  nsresult Dump(PRIntn i, FILE* out, nsTraceRefcnt::StatisticsType type) {
    nsTraceRefcntStats* stats = (type == nsTraceRefcnt::NEW_STATS) ? &mNewStats : &mAllStats;
    if (gLogLeaksOnly && !HaveLeaks(stats)) {
      return NS_OK;
    }

    double meanRefs, stddevRefs;
    NS_MeanAndStdDev(stats->mAddRefs + stats->mReleases, 
                     stats->mRefsOutstandingTotal, 
                     stats->mRefsOutstandingSquared,
                     &meanRefs, &stddevRefs);

    double meanObjs, stddevObjs;
    NS_MeanAndStdDev(stats->mCreates + stats->mDestroys,
                     stats->mObjsOutstandingTotal, 
                     stats->mObjsOutstandingSquared,
                     &meanObjs, &stddevObjs);

    if ((stats->mAddRefs - stats->mReleases) != 0 ||
        stats->mAddRefs != 0 ||
        meanRefs != 0 ||
        stddevRefs != 0 ||
        (stats->mCreates - stats->mDestroys) != 0 ||
        stats->mCreates != 0 ||
        meanObjs != 0 ||
        stddevObjs != 0) {
      fprintf(out, "%4d %-40.40s %8d %8d %8d %8d (%8.2f +/- %8.2f) %8d %8d (%8.2f +/- %8.2f)\n",
              i+1, mClassName,
              (PRInt32)mClassSize,
              (nsCRT::strcmp(mClassName, "TOTAL"))
                  ?(PRInt32)((stats->mCreates - stats->mDestroys) * mClassSize)
                  :mTotalLeaked,
              stats->mCreates,
              (stats->mCreates - stats->mDestroys),
              meanObjs,
              stddevObjs, 
              stats->mAddRefs,
              (stats->mAddRefs - stats->mReleases),
              meanRefs,
              stddevRefs);
    }
    return NS_OK;
  }

protected:
  const char*   mClassName;
  double        mClassSize;     // this is stored as a double because of the way we compute the avg class size for total bloat
  PRInt32       mTotalLeaked; // used only for TOTAL entry
  nsTraceRefcntStats mNewStats;
  nsTraceRefcntStats mAllStats;
};

static void
RecreateBloatView()
{
  gBloatView = PL_NewHashTable(256, 
                               PL_HashString,
                               PL_CompareStrings,
                               PL_CompareValues,
                               NULL, NULL);
}

static BloatEntry*
GetBloatEntry(const char* aTypeName, PRUint32 aInstanceSize)
{
  if (!gBloatView) {
    RecreateBloatView();
  }
  BloatEntry* entry = NULL;
  if (gBloatView) {
    entry = (BloatEntry*)PL_HashTableLookup(gBloatView, aTypeName);
    if (entry == NULL) {
      entry = new BloatEntry(aTypeName, aInstanceSize);
      PLHashEntry* e = PL_HashTableAdd(gBloatView, aTypeName, entry);
      if (e == NULL) {
        delete entry;
        entry = NULL;
      }
    }
    else {
      NS_ASSERTION(aInstanceSize == 0 || entry->GetClassSize() == aInstanceSize, "bad size recorded");
    }
  }
  return entry;
}

static PRIntn DumpSerialNumbers(PLHashEntry* aHashEntry, PRIntn aIndex, void* aClosure)
{
  fprintf((FILE*) aClosure, "%d\n", PRInt32(aHashEntry->value));
  return HT_ENUMERATE_NEXT;
}


#endif /* NS_BUILD_REFCNT_LOGGING */

nsresult
nsTraceRefcnt::DumpStatistics(StatisticsType type, FILE* out)
{
  nsresult rv = NS_OK;
#ifdef NS_BUILD_REFCNT_LOGGING
  if (gBloatLog == nsnull || gBloatView == nsnull) {
    return NS_ERROR_FAILURE;
  }
  if (out == nsnull) {
    out = gBloatLog;
  }

  LOCK_TRACELOG();

  PRBool wasLogging = gLogging;
  gLogging = PR_FALSE;  // turn off logging for this method
  
  const char* msg;
  if (type == NEW_STATS) {
    if (gLogLeaksOnly)
      msg = "NEW (incremental) LEAK STATISTICS";
    else
      msg = "NEW (incremental) LEAK AND BLOAT STATISTICS";
  }
  else {
    if (gLogLeaksOnly)
      msg = "ALL (cumulative) LEAK STATISTICS";
    else
      msg = "ALL (cumulative) LEAK AND BLOAT STATISTICS";
  }
  rv = BloatEntry::PrintDumpHeader(out, msg);
  if (NS_FAILED(rv)) goto done;

  {
    BloatEntry total("TOTAL", 0);
    PL_HashTableEnumerateEntries(gBloatView, BloatEntry::TotalEntries, &total);
    total.DumpTotal(gBloatView->nentries, out);

    nsVoidArray entries;
    PL_HashTableEnumerateEntries(gBloatView, BloatEntry::DumpEntry, &entries);

    printf("nsTraceRefcnt::DumpStatistics: %d entries\n", entries.Count());

    // Sort the entries alphabetically by classname.
    PRInt32 i, j;
    for (i = entries.Count() - 1; i >= 1; --i) {
      for (j = i - 1; j >= 0; --j) {
        BloatEntry* left  = NS_STATIC_CAST(BloatEntry*, entries[i]);
        BloatEntry* right = NS_STATIC_CAST(BloatEntry*, entries[j]);

        if (PL_strcmp(left->GetClassName(), right->GetClassName()) < 0) {
          entries.ReplaceElementAt(right, i);
          entries.ReplaceElementAt(left, j);
        }
      }
    }

    // Enumerate from back-to-front, so things come out in alpha order
    for (i = 0; i < entries.Count(); ++i) {
      BloatEntry* entry = NS_STATIC_CAST(BloatEntry*, entries[i]);
      entry->Dump(i, out, type);
    }
  }

  if (gSerialNumbers) {
    fprintf(out, "\n\nSerial Numbers of Leaked Objects:\n");
    PL_HashTableEnumerateEntries(gSerialNumbers, DumpSerialNumbers, out);
  }

done:
  gLogging = wasLogging;
  UNLOCK_TRACELOG();
#endif
  return rv;
}

void
nsTraceRefcnt::ResetStatistics()
{
#ifdef NS_BUILD_REFCNT_LOGGING
  LOCK_TRACELOG();
  if (gBloatView) {
    PL_HashTableEnumerateEntries(gBloatView, BloatEntry::DestroyEntry, 0);
    PL_HashTableDestroy(gBloatView);
    gBloatView = nsnull;
  }
  UNLOCK_TRACELOG();
#endif
}

void
nsTraceRefcnt::GatherStatistics(nsTraceRefcntStatFunc aFunc,
                                void* aClosure)
{
#ifdef NS_BUILD_REFCNT_LOGGING
  LOCK_TRACELOG();

  if (gBloatView) {
    GatherArgs ga;
    ga.func = aFunc;
    ga.closure = aClosure;
    PL_HashTableEnumerateEntries(gBloatView, BloatEntry::GatherEntry,
                                 (void*) &ga);
  }

  UNLOCK_TRACELOG();
#endif
}

#ifdef NS_BUILD_REFCNT_LOGGING
static PRBool LogThisType(const char* aTypeName)
{
  void* he = PL_HashTableLookup(gTypesToLog, aTypeName);
  return nsnull != he;
}

static PRInt32 GetSerialNumber(void* aPtr, PRBool aCreate)
{
#ifdef GC_LEAK_DETECTOR
  // need to disguise this pointer, so the table won't keep the object alive.
  aPtr = (void*) ~PLHashNumber(aPtr);
#endif
  PLHashEntry** hep = PL_HashTableRawLookup(gSerialNumbers, PLHashNumber(aPtr), aPtr);
  if (hep && *hep) {
    return PRInt32((*hep)->value);
  }
  else if (aCreate) {
    PL_HashTableRawAdd(gSerialNumbers, hep, PLHashNumber(aPtr), aPtr, (void*)(++gNextSerialNumber));
    return gNextSerialNumber;
  }
  else {
    return 0;
  }
}

static void RecycleSerialNumberPtr(void* aPtr)
{
#ifdef GC_LEAK_DETECTOR
  // need to disguise this pointer, so the table won't keep the object alive.
  aPtr = (void*) ~PLHashNumber(aPtr);
#endif
  PL_HashTableRemove(gSerialNumbers, aPtr);
}

static PRBool LogThisObj(PRInt32 aSerialNumber)
{
  return nsnull != PL_HashTableLookup(gObjectsToLog, (const void*)(aSerialNumber));
}

static PRBool InitLog(const char* envVar, const char* msg, FILE* *result)
{
  const char* value = getenv(envVar);
  if (value) {
    if (nsCRT::strcmp(value, "1") == 0) {
      *result = stdout;
      fprintf(stdout, "### %s defined -- logging %s to stdout\n", 
              envVar, msg);
      return PR_TRUE;
    }
    else if (nsCRT::strcmp(value, "2") == 0) {
      *result = stderr;
      fprintf(stdout, "### %s defined -- logging %s to stderr\n", 
              envVar, msg);
      return PR_TRUE;
    }
    else {
      FILE *stream = ::fopen(value, "w");
      if (stream != NULL) {
        *result = stream;
        fprintf(stdout, "### %s defined -- logging %s to %s\n", 
                envVar, msg, value);
        return PR_TRUE;
      }
      else {
        fprintf(stdout, "### %s defined -- unable to log %s to %s\n", 
                envVar, msg, value);
        return PR_FALSE;
      }
    }
  }
  return PR_FALSE;
}


static PLHashNumber HashNumber(const void* aKey)
{
  return PLHashNumber(aKey);
}

static void InitTraceLog(void)
{
  if (gInitialized) return;
  gInitialized = PR_TRUE;

#if defined(XP_MAC) && !TARGET_CARBON
  // this can get called before Toolbox has been initialized.
  InitializeMacToolbox();
#endif

  PRBool defined;
  defined = InitLog("XPCOM_MEM_BLOAT_LOG", "bloat/leaks", &gBloatLog);
  if (!defined)
    gLogLeaksOnly = InitLog("XPCOM_MEM_LEAK_LOG", "leaks", &gBloatLog);
  if (defined || gLogLeaksOnly) {
    RecreateBloatView();
    if (NS_WARN_IF_FALSE(gBloatView, "out of memory")) {
      gBloatLog = nsnull;
      gLogLeaksOnly = PR_FALSE;
    }
  }

  (void)InitLog("XPCOM_MEM_REFCNT_LOG", "refcounts", &gRefcntsLog);

  (void)InitLog("XPCOM_MEM_ALLOC_LOG", "new/delete", &gAllocLog);

  defined = InitLog("XPCOM_MEM_LEAKY_LOG", "for leaky", &gLeakyLog);
  if (defined) {
    gLogToLeaky = PR_TRUE;
    void* p = nsnull;
    void* q = nsnull;
#ifdef HAVE_LIBDL
    p = dlsym(0, "__log_addref");
    q = dlsym(0, "__log_release");
#endif
    if (p && q) {
      leakyLogAddRef = (void (*)(void*,int,int)) p;
      leakyLogRelease = (void (*)(void*,int,int)) q;
    }
    else {
      gLogToLeaky = PR_FALSE;
      fprintf(stdout, "### ERROR: XPCOM_MEM_LEAKY_LOG defined, but can't locate __log_addref and __log_release symbols\n");
      fflush(stdout);
    }
  }

  const char* classes = getenv("XPCOM_MEM_LOG_CLASSES");
  if (classes) {
    // if XPCOM_MEM_LOG_CLASSES was set to some value, the value is interpreted
    // as a list of class names to track
    gTypesToLog = PL_NewHashTable(256,
                                  PL_HashString,
                                  PL_CompareStrings,
                                  PL_CompareValues,
                                  NULL, NULL);
    if (NS_WARN_IF_FALSE(gTypesToLog, "out of memory")) {
      fprintf(stdout, "### XPCOM_MEM_LOG_CLASSES defined -- unable to log specific classes\n");
    }
    else {
      fprintf(stdout, "### XPCOM_MEM_LOG_CLASSES defined -- only logging these classes: ");
      const char* cp = classes;
      for (;;) {
        char* cm = (char*) strchr(cp, ',');
        if (cm) {
          *cm = '\0';
        }
        PL_HashTableAdd(gTypesToLog, nsCRT::strdup(cp), (void*)1);
        fprintf(stdout, "%s ", cp);
        if (!cm) break;
        *cm = ',';
        cp = cm + 1;
      }
      fprintf(stdout, "\n");
    }

    gSerialNumbers = PL_NewHashTable(256,
                                     HashNumber,
                                     PL_CompareValues,
                                     PL_CompareValues,
                                     NULL, NULL);


  }

  const char* objects = getenv("XPCOM_MEM_LOG_OBJECTS");
  if (objects) {
    gObjectsToLog = PL_NewHashTable(256,
                                    HashNumber,
                                    PL_CompareValues,
                                    PL_CompareValues,
                                    NULL, NULL);

    if (NS_WARN_IF_FALSE(gObjectsToLog, "out of memory")) {
      fprintf(stdout, "### XPCOM_MEM_LOG_OBJECTS defined -- unable to log specific objects\n");
    }
    else if (! (gRefcntsLog || gAllocLog)) {
      fprintf(stdout, "### XPCOM_MEM_LOG_OBJECTS defined -- but neither XPCOM_MEM_REFCNT_LOG nor XPCOM_MEM_ALLOC_LOG is defined\n");
    }
    else {
      fprintf(stdout, "### XPCOM_MEM_LOG_OBJECTS defined -- only logging these objects: ");
      const char* cp = objects;
      for (;;) {
        char* cm = (char*) strchr(cp, ',');
        if (cm) {
          *cm = '\0';
        }
        PRInt32 serialno = 0;
        while (*cp) {
          serialno *= 10;
          serialno += *cp - '0';
          ++cp;
        }
        PL_HashTableAdd(gObjectsToLog, (const void*)serialno, (void*)1);
        fprintf(stdout, "%d ", serialno);
        if (!cm) break;
        *cm = ',';
        cp = cm + 1;
      }
      fprintf(stdout, "\n");
    }
  }


  if (gBloatLog || gRefcntsLog || gAllocLog || gLeakyLog) {
    gLogging = PR_TRUE;
  }

#if defined(NS_MT_SUPPORTED)
  gTraceLock = PR_NewLock();
#endif /* NS_MT_SUPPORTED */

}

#endif

#if defined(_WIN32) && defined(_M_IX86) // WIN32 x86 stack walking code
#include "imagehlp.h"
#include <stdio.h>

// Define these as static pointers so that we can load the DLL on the
// fly (and not introduce a link-time dependency on it). Tip o' the
// hat to Matt Pietrick for this idea. See:
//
//   http://msdn.microsoft.com/library/periodic/period97/F1/D3/S245C6.htm
//
typedef BOOL (__stdcall *SYMINITIALIZEPROC)(HANDLE, LPSTR, BOOL);
static SYMINITIALIZEPROC _SymInitialize;

typedef BOOL (__stdcall *SYMCLEANUPPROC)(HANDLE);
static SYMCLEANUPPROC _SymCleanup;

typedef BOOL (__stdcall *STACKWALKPROC)(DWORD,
                                        HANDLE,
                                        HANDLE,
                                        LPSTACKFRAME,
                                        LPVOID,
                                        PREAD_PROCESS_MEMORY_ROUTINE,
                                        PFUNCTION_TABLE_ACCESS_ROUTINE,
                                        PGET_MODULE_BASE_ROUTINE,
                                        PTRANSLATE_ADDRESS_ROUTINE);
static STACKWALKPROC _StackWalk;

typedef LPVOID (__stdcall *SYMFUNCTIONTABLEACCESSPROC)(HANDLE, DWORD);
static SYMFUNCTIONTABLEACCESSPROC _SymFunctionTableAccess;

typedef DWORD (__stdcall *SYMGETMODULEBASEPROC)(HANDLE, DWORD);
static SYMGETMODULEBASEPROC _SymGetModuleBase;

typedef BOOL (__stdcall *SYMGETSYMFROMADDRPROC)(HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL);
static SYMGETSYMFROMADDRPROC _SymGetSymFromAddr;

typedef DWORD ( __stdcall *SYMLOADMODULE)(HANDLE, HANDLE, PSTR, PSTR, DWORD, DWORD);
static SYMLOADMODULE _SymLoadModule;

static PRBool
EnsureImageHlpInitialized()
{
  static PRBool gInitialized = PR_FALSE;

  if (! gInitialized) {
    HMODULE module = ::LoadLibrary("IMAGEHLP.DLL");
    if (!module) return PR_FALSE;

    _SymInitialize = (SYMINITIALIZEPROC) ::GetProcAddress(module, "SymInitialize");
    if (!_SymInitialize) return PR_FALSE;

    _SymCleanup = (SYMCLEANUPPROC)GetProcAddress(module, "SymCleanup");
    if (!_SymCleanup) return PR_FALSE;

    _StackWalk = (STACKWALKPROC)GetProcAddress(module, "StackWalk");
    if (!_StackWalk) return PR_FALSE;

    _SymFunctionTableAccess = (SYMFUNCTIONTABLEACCESSPROC) GetProcAddress(module, "SymFunctionTableAccess");
    if (!_SymFunctionTableAccess) return PR_FALSE;

    _SymGetModuleBase = (SYMGETMODULEBASEPROC)GetProcAddress(module, "SymGetModuleBase");
    if (!_SymGetModuleBase) return PR_FALSE;

    _SymGetSymFromAddr = (SYMGETSYMFROMADDRPROC)GetProcAddress(module, "SymGetSymFromAddr");
    if (!_SymGetSymFromAddr) return PR_FALSE;

    _SymLoadModule = (SYMLOADMODULE)GetProcAddress(module, "SymLoadModule");
    if (!_SymLoadModule) return PR_FALSE;

    gInitialized = PR_TRUE;
  }

  return gInitialized;
} 

static PRBool
EnsureSymInitialized()
{  
  static PRBool gInitialized = PR_FALSE;

  if (! gInitialized) {
    if (! EnsureImageHlpInitialized())
      return PR_FALSE;
    gInitialized = _SymInitialize(GetCurrentProcess(), 0, TRUE);
  }
  return gInitialized;
}
/**
 * Walk the stack, translating PC's found into strings and recording the
 * chain in aBuffer. For this to work properly, the dll's must be rebased
 * so that the address in the file agrees with the address in memory.
 * Otherwise StackWalk will return FALSE when it hits a frame in a dll's
 * whose in memory address doesn't match it's in-file address.
 *
 * Fortunately, there is a handy dandy routine in IMAGEHLP.DLL that does
 * the rebasing and accordingly I've made a tool to use it to rebase the
 * DLL's in one fell swoop (see xpcom/tools/windows/rebasedlls.cpp).
 */

void
nsTraceRefcnt::WalkTheStack(FILE* aStream)
{
  HANDLE myProcess = ::GetCurrentProcess();
  HANDLE myThread = ::GetCurrentThread();
  BOOL ok;

  ok = EnsureSymInitialized();
  if (! ok)
    return;

  // Get the context information for this thread. That way we will
  // know where our sp, fp, pc, etc. are and can fill in the
  // STACKFRAME with the initial values.
  CONTEXT context;
  context.ContextFlags = CONTEXT_FULL;
  ok = GetThreadContext(myThread, &context);
  if (! ok)
    return;

  // Setup initial stack frame to walk from
  STACKFRAME frame;
  memset(&frame, 0, sizeof(frame));
  frame.AddrPC.Offset    = context.Eip;
  frame.AddrPC.Mode      = AddrModeFlat;
  frame.AddrStack.Offset = context.Esp;
  frame.AddrStack.Mode   = AddrModeFlat;
  frame.AddrFrame.Offset = context.Ebp;
  frame.AddrFrame.Mode   = AddrModeFlat;

  // Now walk the stack and map the pc's to symbol names
  int skip = 2;
  while (1) {
    ok = _StackWalk(IMAGE_FILE_MACHINE_I386,
                   myProcess,
                   myThread,
                   &frame,
                   &context,
                   0,                        // read process memory routine
                   _SymFunctionTableAccess,  // function table access routine
                   _SymGetModuleBase,        // module base routine
                   0);                       // translate address routine

    if (!ok) {
      LPVOID lpMsgBuf;
      FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL 
        );
      fprintf(aStream, "### ERROR: WalkStack: %s", lpMsgBuf);
      fflush(aStream);
      LocalFree( lpMsgBuf );
    }
    if (!ok || frame.AddrPC.Offset == 0)
      break;

    if (skip-- > 0)
      continue;

    char buf[sizeof(IMAGEHLP_SYMBOL) + 512];
    PIMAGEHLP_SYMBOL symbol = (PIMAGEHLP_SYMBOL) buf;
    symbol->SizeOfStruct = sizeof(buf);
    symbol->MaxNameLength = 512;

    DWORD displacement;
    ok = _SymGetSymFromAddr(myProcess,
                            frame.AddrPC.Offset,
                            &displacement,
                            symbol);

    if (ok) {
      fprintf(aStream, "%s+0x%08X\n", symbol->Name, displacement);
    }
    else {
      fprintf(aStream, "0x%08X\n", frame.AddrPC.Offset);
    }
  }
}

#elif defined(linux) && defined(__GLIBC__) && (defined(__i386) || defined(PPC)) // i386 or PPC Linux stackwalking code

void
nsTraceRefcnt::WalkTheStack(FILE* aStream)
{
  jmp_buf jb;
  setjmp(jb);

  // Stack walking code courtesy Kipp's "leaky".
#if defined(__i386) 
  u_long* bp = (u_long*) (jb[0].__jmpbuf[JB_BP]);
#elif defined(PPC)
  u_long* bp = (u_long*) (jb[0].__jmpbuf[JB_GPR1]);
#endif

  int skip = 2;
  for (;;) {
    u_long* nextbp = (u_long*) *bp++;
    u_long pc = *bp;
    if ((pc < 0x08000000) || (pc > 0x7fffffff) || (nextbp < bp)) {
      break;
    }
    if (--skip <= 0) {
      Dl_info info;
      int ok = dladdr((void*) pc, &info);
      if (ok < 0)
        break;

      const char * symbol = info.dli_sname;

      int len = strlen(symbol);
      if (! len)
        break; // XXX Lazy. We could look at the filename or something.

      char demangled[4096] = "\0";

      DemangleSymbol(symbol,demangled,sizeof(demangled));

      if (strlen(demangled)) {
        symbol = demangled;
        len = strlen(symbol);
      }

      PRUint32 off = (char*)pc - (char*)info.dli_saddr;
      fprintf(aStream, "%s+0x%08X\n", symbol, off);
    }
    bp = nextbp;
  }
}
#elif defined(XP_MAC)

/**
 * Stack walking code for the Mac OS.
 */

#include "gc_fragments.h"

#include <typeinfo>

extern "C" {
void MWUnmangle(const char *mangled_name, char *unmangled_name, size_t buffersize);
}

struct traceback_table {
	long zero;
	long magic;
	long reserved;
	long codeSize;
	short nameLength;
	char name[2];
};

static char* pc2name(long* pc, char name[], long size)
{
	name[0] = '\0';
	
	// make sure pc is instruction aligned (at least).
	if (UInt32(pc) == (UInt32(pc) & 0xFFFFFFFC)) {
		long instructionsToLook = 4096;
		long* instruction = (long*)pc;
		
		// look for the traceback table.
		while (instructionsToLook--) {
			if (instruction[0] == 0x4E800020 && instruction[1] == 0x00000000) {
				traceback_table* tb = (traceback_table*)&instruction[1];
				long nameLength = (tb->nameLength > --size ? size : tb->nameLength);
				memcpy(name, tb->name + 1, --nameLength);
				name[nameLength] = '\0';
				break;
			}
			++instruction;
		}
	}
	
	return name;
}

struct stack_frame {
	stack_frame*	next;				// savedSP
	void*			savedCR;
	void*			savedLR;
	void*			reserved0;
	void*			reserved1;
	void*			savedTOC;
};

static asm stack_frame* getStackFrame() 
{
	mr		r3, sp
	blr
}

NS_COM void
nsTraceRefcnt::WalkTheStack(FILE* aStream)
{
    stack_frame* currentFrame = getStackFrame();    // WalkTheStack's frame.
	currentFrame = currentFrame->next;	            // WalkTheStack's caller's frame.
	currentFrame = currentFrame->next;              // WalkTheStack's caller's caller's frame.
    
	while (true) {
		// LR saved at 8(SP) in each frame. subtract 4 to get address of calling instruction.
		void* pc = currentFrame->savedLR;

		// convert PC to name, unmangle it, and generate source location, if possible.
		static char symbol_name[1024], unmangled_name[1024], file_name[256]; UInt32 file_offset;
		
     	if (GC_address_to_source((char*)pc, symbol_name, file_name, &file_offset)) {
			MWUnmangle(symbol_name, unmangled_name, sizeof(unmangled_name));
     		fprintf(aStream, "%s[%s,%ld]\n", unmangled_name, file_name, file_offset);
     	} else {
 		    pc2name((long*)pc, symbol_name, sizeof(symbol_name));
			MWUnmangle(symbol_name, unmangled_name, sizeof(unmangled_name));
    		fprintf(aStream, "%s(0x%08X)\n", unmangled_name, pc);
     	}

		currentFrame = currentFrame->next;
		// the bottom-most frame is marked as pointing to NULL, or is ODD if a 68K transition frame.
		if (currentFrame == NULL || UInt32(currentFrame) & 0x1)
			break;
	}
}

#else // unsupported platform.

void
nsTraceRefcnt::WalkTheStack(FILE* aStream)
{
	fprintf(aStream, "write me, dammit!\n");
}

#endif

//----------------------------------------------------------------------

// This thing is exported by libiberty.a (-liberty)
// Yes, this is a gcc only hack
#if defined(MOZ_DEMANGLE_SYMBOLS)
extern "C" char * cplus_demangle(const char *,int);
#include <stdlib.h> // for free()
#endif // MOZ_DEMANGLE_SYMBOLS

#ifdef __linux__
NS_COM void 
nsTraceRefcnt::DemangleSymbol(const char * aSymbol, 
                              char * aBuffer,
                              int aBufLen)
{
  NS_ASSERTION(nsnull != aSymbol,"null symbol");
  NS_ASSERTION(nsnull != aBuffer,"null buffer");
  NS_ASSERTION(aBufLen >= 32 ,"pulled 32 out of you know where");

  aBuffer[0] = '\0';

#if defined(MOZ_DEMANGLE_SYMBOLS)
  /* See demangle.h in the gcc source for the voodoo */
  char * demangled = cplus_demangle(aSymbol,3);

  if (demangled)
  {
    strncpy(aBuffer,demangled,aBufLen);

    free(demangled);
  }
#endif // MOZ_DEMANGLE_SYMBOLS
}

#else // __linux__

NS_COM void 
nsTraceRefcnt::DemangleSymbol(const char * aSymbol, 
                              char * aBuffer,
                              int aBufLen)
{
  NS_ASSERTION(nsnull != aSymbol,"null symbol");
  NS_ASSERTION(nsnull != aBuffer,"null buffer");

  // lose
  aBuffer[0] = '\0';
}
#endif // __linux__

//----------------------------------------------------------------------

NS_COM void
nsTraceRefcnt::LoadLibrarySymbols(const char* aLibraryName,
                                  void* aLibrayHandle)
{
#ifdef NS_BUILD_REFCNT_LOGGING
#if defined(_WIN32) && defined(_M_IX86) /* Win32 x86 only */
  if (!gInitialized)
    InitTraceLog();

  if (gAllocLog || gRefcntsLog) {
    fprintf(stdout, "### Loading symbols for %s\n", aLibraryName);
    fflush(stdout);

    HANDLE myProcess = ::GetCurrentProcess();    
    BOOL ok = EnsureSymInitialized();
    if (ok) {
      const char* baseName = aLibraryName;
      // just get the base name of the library if a full path was given:
      PRInt32 len = nsCRT::strlen(aLibraryName);
      for (PRInt32 i = len - 1; i >= 0; i--) {
        if (aLibraryName[i] == '\\') {
          baseName = &aLibraryName[i + 1];
          break;
        }
      }
      DWORD baseAddr = _SymLoadModule(myProcess,
                                     NULL,
                                     (char*)baseName,
                                     (char*)baseName,
                                     0,
                                     0);
      ok = (baseAddr != nsnull);
    }
    if (!ok) {
      LPVOID lpMsgBuf;
      FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL 
        );
      fprintf(stdout, "### ERROR: LoadLibrarySymbols for %s: %s\n",
              aLibraryName, lpMsgBuf);
      fflush(stdout);
      LocalFree( lpMsgBuf );
    }
  }
#endif
#endif
}

//----------------------------------------------------------------------

/*
 For consistency, and ease of munging the output, the following record format will be used:
 
 <TypeName> 0xADDRESS Verb [optional data]
 */
 
NS_COM void
nsTraceRefcnt::LogAddRef(void* aPtr,
                         nsrefcnt aRefCnt,
                         const char* aClazz,
                         PRUint32 classSize)
{
#ifdef NS_BUILD_REFCNT_LOGGING
  if (!gInitialized)
    InitTraceLog();
  if (gLogging) {
    LOCK_TRACELOG();

    if (gBloatLog) {
      BloatEntry* entry = GetBloatEntry(aClazz, classSize);
      if (entry) {
        entry->AddRef(aRefCnt);
      }
    }

    // Here's the case where neither NS_NEWXPCOM nor MOZ_COUNT_CTOR were used,
    // yet we still want to see creation information:
#ifndef NS_LOSING_ARCHITECTURE
    // (If we're on a losing architecture, don't do this because we'll be
    // using LogNewXPCOM instead to get file and line numbers.)
    PRBool loggingThisType = (!gTypesToLog || LogThisType(aClazz));
    PRInt32 serialno = 0;
    if (gSerialNumbers && loggingThisType) {
      serialno = GetSerialNumber(aPtr, aRefCnt == 1);
    }

    PRBool loggingThisObject = (!gObjectsToLog || LogThisObj(serialno));
    if (aRefCnt == 1 && gAllocLog && loggingThisType && loggingThisObject) {
      fprintf(gAllocLog, "\n<%s> 0x%08X %d Create\n",
              aClazz, PRInt32(aPtr), serialno);
      WalkTheStack(gAllocLog);
    }

    // (If we're on a losing architecture, don't do this because we'll be
    // using LogAddRefCall instead to get file and line numbers.)
    if (gRefcntsLog && loggingThisType && loggingThisObject) {
      if (gLogToLeaky) {
        (*leakyLogAddRef)(aPtr, aRefCnt - 1, aRefCnt);
      }
      else {        
          // Can't use PR_LOG(), b/c it truncates the line
          fprintf(gRefcntsLog,
                  "\n<%s> 0x%08X %d AddRef %d\n", aClazz, PRInt32(aPtr), serialno, aRefCnt);       
          WalkTheStack(gRefcntsLog);
          fflush(gRefcntsLog);
      }
    }
#endif

    UNLOCK_TRACELOG();
  }
#endif
}


NS_COM void
nsTraceRefcnt::LogRelease(void* aPtr,
                          nsrefcnt aRefCnt,
                          const char* aClazz)
{
#ifdef NS_BUILD_REFCNT_LOGGING
  if (!gInitialized)
    InitTraceLog();
  if (gLogging) {
    LOCK_TRACELOG();

    if (gBloatLog) {
      BloatEntry* entry = GetBloatEntry(aClazz, 0);
      if (entry) {
        entry->Release(aRefCnt);
      }
    }

#ifndef NS_LOSING_ARCHITECTURE
    // (If we're on a losing architecture, don't do this because we'll be
    // using LogReleaseCall instead to get file and line numbers.)
    PRBool loggingThisType = (!gTypesToLog || LogThisType(aClazz));
    PRInt32 serialno = 0;
    if (gSerialNumbers && loggingThisType) {
      serialno = GetSerialNumber(aPtr, PR_FALSE);
    }

    PRBool loggingThisObject = (!gObjectsToLog || LogThisObj(serialno));
    if (gRefcntsLog && loggingThisType && loggingThisObject) {
      if (gLogToLeaky) {
        (*leakyLogRelease)(aPtr, aRefCnt + 1, aRefCnt);
      }
      else {
          // Can't use PR_LOG(), b/c it truncates the line
          fprintf(gRefcntsLog,
                  "\n<%s> 0x%08X %d Release %d\n", aClazz, PRInt32(aPtr), serialno, aRefCnt);
          WalkTheStack(gRefcntsLog);
          fflush(gRefcntsLog);
      }
    }

    // Here's the case where neither NS_DELETEXPCOM nor MOZ_COUNT_DTOR were used,
    // yet we still want to see deletion information:

    // (If we're on a losing architecture, don't do this because we'll be
    // using LogDeleteXPCOM instead to get file and line numbers.)
    if (aRefCnt == 0 && gAllocLog && loggingThisType && loggingThisObject) {
      fprintf(gAllocLog,
              "\n<%s> 0x%08X %d Destroy\n",
              aClazz, PRInt32(aPtr), serialno);
      WalkTheStack(gAllocLog);
    }

    if (aRefCnt == 0 && gSerialNumbers && loggingThisType) {
      RecycleSerialNumberPtr(aPtr);
    }
#endif

    UNLOCK_TRACELOG();
  }
#endif
}

NS_COM nsrefcnt
nsTraceRefcnt::LogAddRefCall(void* aPtr,
                             nsrefcnt aNewRefcnt,
                             const char* aFile,
                             int aLine)
{
#ifdef NS_BUILD_REFCNT_LOGGING
#ifdef NS_LOSING_ARCHITECTURE
  if (!gInitialized)
    InitTraceLog();
  if (gRefcntsLog) {
    LOCK_TRACELOG();

    fprintf(gRefcntsLog, "\n<Call> 0x%08X AddRef %d=>%d in %s (line %d)\n",
            aPtr, aNewRefcnt-1, aNewRefcnt, aFile, aLine);
    WalkTheStack(gRefcntsLog);

    UNLOCK_TRACELOG();
  }
#endif
#endif
  return aNewRefcnt;
}

NS_COM nsrefcnt
nsTraceRefcnt::LogReleaseCall(void* aPtr,
                              nsrefcnt aNewRefcnt,
                              const char* aFile,
                              int aLine)
{
#ifdef NS_BUILD_REFCNT_LOGGING
#ifdef NS_LOSING_ARCHITECTURE
  if (!gInitialized)
    InitTraceLog();

  if (gRefcntsLog) {
    LOCK_TRACELOG();

    fprintf(gRefcntsLog, "\n<Call> 0x%08X Release %d=>%d in %s (line %d)\n",
            aPtr, aNewRefcnt+1, aNewRefcnt, aFile, aLine);
    WalkTheStack(gRefcntsLog);

    UNLOCK_TRACELOG();
  }
#endif
#endif
  return aNewRefcnt;
}

NS_COM void
nsTraceRefcnt::LogNewXPCOM(void* aPtr,
                           const char* aType,
                           PRUint32 aInstanceSize,
                           const char* aFile,
                           int aLine)
{
#ifdef NS_BUILD_REFCNT_LOGGING
#ifdef NS_LOSING_ARCHITECTURE
  if (!gInitialized)
    InitTraceLog();

  if (gAllocLog) {
    LOCK_TRACELOG();

    fprintf(gAllocLog, "\n<%s> 0x%08X NewXPCOM in %s (line %d)\n",
            aType, aPtr, aFile, aLine);
    WalkTheStack(gAllocLog);

    UNLOCK_TRACELOG();
  }
#endif
#endif
}

NS_COM void
nsTraceRefcnt::LogDeleteXPCOM(void* aPtr,
                              const char* aFile,
                              int aLine)
{
#ifdef NS_BUILD_REFCNT_LOGGING
#ifdef NS_LOSING_ARCHITECTURE
  if (!gInitialized)
    InitTraceLog();

  if (gAllocLog) {
    LOCK_TRACELOG();

    fprintf(gAllocLog, "\n<%s> 0x%08X Destroy in %s (line %d)\n",
            "?", aPtr, aFile, aLine);
    WalkTheStack(gAllocLog);

    UNLOCK_TRACELOG();
  }
#endif
#endif
}

NS_COM void
nsTraceRefcnt::LogCtor(void* aPtr,
                       const char* aType,
                       PRUint32 aInstanceSize)
{
#ifdef NS_BUILD_REFCNT_LOGGING
  if (!gInitialized)
    InitTraceLog();

  if (gLogging) {
    LOCK_TRACELOG();

    if (gBloatLog) {
      BloatEntry* entry = GetBloatEntry(aType, aInstanceSize);
      if (entry) {
        entry->Ctor();
      }
    }

#ifndef NS_LOSING_ARCHITECTURE
    PRBool loggingThisType = (!gTypesToLog || LogThisType(aType));
    PRInt32 serialno = 0;
    if (gSerialNumbers && loggingThisType) {
      serialno = GetSerialNumber(aPtr, PR_TRUE);
    }

    // (If we're on a losing architecture, don't do this because we'll be
    // using LogNewXPCOM instead to get file and line numbers.)
    PRBool loggingThisObject = (!gObjectsToLog || LogThisObj(serialno));
    if (gAllocLog && loggingThisType && loggingThisObject) {
      fprintf(gAllocLog, "\n<%s> 0x%08X %d Ctor (%d)\n",
             aType, PRInt32(aPtr), serialno, aInstanceSize);
      WalkTheStack(gAllocLog);
    }
#endif

    UNLOCK_TRACELOG();
  }
#endif
}

NS_COM void
nsTraceRefcnt::LogDtor(void* aPtr, const char* aType,
                       PRUint32 aInstanceSize)
{
#ifdef NS_BUILD_REFCNT_LOGGING
  if (!gInitialized)
    InitTraceLog();

  if (gLogging) {
    LOCK_TRACELOG();

    if (gBloatLog) {
      BloatEntry* entry = GetBloatEntry(aType, aInstanceSize);
      if (entry) {
        entry->Dtor();
      }
    }

#ifndef NS_LOSING_ARCHITECTURE
    PRBool loggingThisType = (!gTypesToLog || LogThisType(aType));
    PRInt32 serialno = 0;
    if (gSerialNumbers && loggingThisType) {
      serialno = GetSerialNumber(aPtr, PR_FALSE);
      RecycleSerialNumberPtr(aPtr);
    }

    PRBool loggingThisObject = (!gObjectsToLog || LogThisObj(serialno));

    // (If we're on a losing architecture, don't do this because we'll be
    // using LogDeleteXPCOM instead to get file and line numbers.)
    if (gAllocLog && loggingThisType && loggingThisObject) {
      fprintf(gAllocLog, "\n<%s> 0x%08X %d Dtor (%d)\n",
             aType, PRInt32(aPtr), serialno, aInstanceSize);
      WalkTheStack(gAllocLog);
    }
#endif

    UNLOCK_TRACELOG();
  }
#endif
}
