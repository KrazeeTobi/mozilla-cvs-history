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

#ifndef nspr_nec_defs_h___
#define nspr_nec_defs_h___
 
/*
 * Internal configuration macros
 */

#define PR_LINKER_ARCH  "nec"
#define _PR_SI_SYSNAME "NEC"
#define _PR_SI_ARCHITECTURE "mips"
#define PR_DLL_SUFFIX		".so"
 
#define _PR_STACK_VMBASE        0x50000000
#define _MD_DEFAULT_STACK_SIZE  65536L
#define _MD_MMAP_FLAGS          MAP_PRIVATE

#undef  HAVE_STACK_GROWING_UP
#define HAVE_DLL
#define USE_DLFCN
#define NEED_TIME_R
#define NEED_STRFTIME_LOCK
#define _PR_POLL_AVAILABLE
#define _PR_USE_POLL
#define _PR_NO_LARGE_FILES
#define _PR_STAT_HAS_ST_ATIM_UNION
 
#include <ucontext.h>
#include <sys/regset.h>
 
#define PR_NUM_GCREGS   NGREG
#define PR_CONTEXT_TYPE ucontext_t
 
#define CONTEXT(_thread) (&(_thread)->md.context)
 
#define _MD_GET_SP(_t)    (_t)->md.context.uc_mcontext.gregs[CXT_SP]
 
/*
** Initialize the thread context preparing it to execute "e(o,a)"
*/
#define _MD_INIT_CONTEXT(thread, _sp, _main, status)               \
{                                                                   \
    *status = PR_TRUE; \
    getcontext(CONTEXT(thread));                                    \
    CONTEXT(thread)->uc_stack.ss_sp = (char*) (thread)->stack->stackBottom; \
    CONTEXT(thread)->uc_stack.ss_size = (thread)->stack->stackSize; \
    _MD_GET_SP(thread) = (greg_t) _sp - 64;             \
    makecontext(CONTEXT(thread), _main, 0);              \
}
 
#define _MD_SWITCH_CONTEXT(_thread)      \
    if (!getcontext(CONTEXT(_thread))) { \
        (_thread)->md.errcode = errno;      \
        _PR_Schedule();                  \
    }
 
/*
** Restore a thread context, saved by _MD_SWITCH_CONTEXT
*/
#define _MD_RESTORE_CONTEXT(_thread)   \
{                                      \
    ucontext_t *uc = CONTEXT(_thread); \
    uc->uc_mcontext.gregs[CXT_V0] = 1; \
    uc->uc_mcontext.gregs[CXT_A3] = 0; \
    errno = (_thread)->md.errcode;     \
    _MD_SET_CURRENT_THREAD(_thread);   \
    setcontext(uc);                    \
}

/* Machine-dependent (MD) data structures */

struct _MDThread {
    PR_CONTEXT_TYPE context;
    int id;
    int errcode;
};

struct _MDThreadStack {
    PRInt8 notused;
};

struct _MDLock {
    PRInt8 notused;
};

struct _MDSemaphore {
    PRInt8 notused;
};

struct _MDCVar {
    PRInt8 notused;
};

struct _MDSegment {
    PRInt8 notused;
};

/*
 * md-specific cpu structure field
 */
#define _PR_MD_MAX_OSFD FD_SETSIZE

struct _MDCPU_Unix {
    PRCList ioQ;
    PRUint32 ioq_timeout;
    PRInt32 ioq_max_osfd;
    PRInt32 ioq_osfd_cnt;
#ifndef _PR_USE_POLL
    fd_set fd_read_set, fd_write_set, fd_exception_set;
    PRInt16 fd_read_cnt[_PR_MD_MAX_OSFD],fd_write_cnt[_PR_MD_MAX_OSFD],
				fd_exception_cnt[_PR_MD_MAX_OSFD];
#else
	struct pollfd *ioq_pollfds;
	int ioq_pollfds_size;
#endif	/* _PR_USE_POLL */
};

#define _PR_IOQ(_cpu)			((_cpu)->md.md_unix.ioQ)
#define _PR_ADD_TO_IOQ(_pq, _cpu) PR_APPEND_LINK(&_pq.links, &_PR_IOQ(_cpu))
#define _PR_FD_READ_SET(_cpu)		((_cpu)->md.md_unix.fd_read_set)
#define _PR_FD_READ_CNT(_cpu)		((_cpu)->md.md_unix.fd_read_cnt)
#define _PR_FD_WRITE_SET(_cpu)		((_cpu)->md.md_unix.fd_write_set)
#define _PR_FD_WRITE_CNT(_cpu)		((_cpu)->md.md_unix.fd_write_cnt)
#define _PR_FD_EXCEPTION_SET(_cpu)	((_cpu)->md.md_unix.fd_exception_set)
#define _PR_FD_EXCEPTION_CNT(_cpu)	((_cpu)->md.md_unix.fd_exception_cnt)
#define _PR_IOQ_TIMEOUT(_cpu)		((_cpu)->md.md_unix.ioq_timeout)
#define _PR_IOQ_MAX_OSFD(_cpu)		((_cpu)->md.md_unix.ioq_max_osfd)
#define _PR_IOQ_OSFD_CNT(_cpu)		((_cpu)->md.md_unix.ioq_osfd_cnt)
#define _PR_IOQ_POLLFDS(_cpu)		((_cpu)->md.md_unix.ioq_pollfds)
#define _PR_IOQ_POLLFDS_SIZE(_cpu)	((_cpu)->md.md_unix.ioq_pollfds_size)

#define _PR_IOQ_MIN_POLLFDS_SIZE(_cpu)	32

struct _MDCPU {
	struct _MDCPU_Unix md_unix;
};

#define _MD_INIT_LOCKS()
#define _MD_NEW_LOCK(lock) PR_SUCCESS
#define _MD_FREE_LOCK(lock)
#define _MD_LOCK(lock)
#define _MD_UNLOCK(lock)
#define _MD_INIT_IO()
#define _MD_IOQ_LOCK()
#define _MD_IOQ_UNLOCK()

#define _MD_EARLY_INIT          _MD_EarlyInit
#define _MD_FINAL_INIT			_PR_UnixInit
#define _MD_INIT_RUNNING_CPU(cpu) _MD_unix_init_running_cpu(cpu)
#define _MD_INIT_THREAD         _MD_InitializeThread
#define _MD_EXIT_THREAD(thread)
#define _MD_CLEAN_THREAD(_thread)

#define _MD_SELECT _select
#define _MD_POLL _poll

#define _MD_GET_INTERVAL                  _PR_UNIX_GetInterval
#define _MD_INTERVAL_PER_SEC              _PR_UNIX_TicksPerSecond
 
#endif /* nspr_nec_defs_h___ */
