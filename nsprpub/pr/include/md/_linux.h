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

#ifndef nspr_linux_defs_h___
#define nspr_linux_defs_h___

#include "prthread.h"

/*
 * Internal configuration macros
 */

#define PR_LINKER_ARCH	"linux"
#define _PR_SI_SYSNAME  "LINUX"
#ifdef __powerpc__
#define _PR_SI_ARCHITECTURE "ppc"
#elif defined(__alpha)
#define _PR_SI_ARCHITECTURE "alpha"
#elif defined(__mc68000__)
#define _PR_SI_ARCHITECTURE "m68k"
#elif defined(__sparc__)
#define _PR_SI_ARCHITECTURE "sparc"
#elif defined(__i386__)
#define _PR_SI_ARCHITECTURE "x86"
#elif defined(__mips__)
#define _PR_SI_ARCHITECTURE "mips"
#elif defined(__arm__)
#define _PR_SI_ARCHITECTURE "arm"
#else
#error "Unknown CPU architecture"
#endif
#define PR_DLL_SUFFIX		".so"

#define _PR_VMBASE              0x30000000
#define _PR_STACK_VMBASE	0x50000000
#define _MD_DEFAULT_STACK_SIZE	65536L
#define _MD_MMAP_FLAGS          MAP_PRIVATE

#undef	HAVE_STACK_GROWING_UP

/*
 * Elf linux supports dl* functions
 */
#define HAVE_DLL
#define USE_DLFCN

#define USE_SETJMP
#if defined(__GLIBC__) && __GLIBC__ >= 2
#define _PR_POLL_AVAILABLE
#endif
#undef _PR_USE_POLL
#define _PR_STAT_HAS_ONLY_ST_ATIME
#if defined(__alpha)
#define _PR_HAVE_LARGE_OFF_T
#else
#define _PR_NO_LARGE_FILES
#endif

#ifdef _PR_PTHREADS

extern void _MD_CleanupBeforeExit(void);
#define _MD_CLEANUP_BEFORE_EXIT _MD_CleanupBeforeExit

#else  /* ! _PR_PTHREADS */

#include <setjmp.h>

#define PR_CONTEXT_TYPE	sigjmp_buf

#define CONTEXT(_th) ((_th)->md.context)

#ifdef __powerpc__
/*
 * PowerPC based MkLinux
 *
 * On the PowerPC, the new style jmp_buf isn't used until glibc
 * 2.1.
 */
#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 1
#define _MD_GET_SP(_t) (_t)->md.context[0].__jmpbuf[JB_GPR1]
#else
#define _MD_GET_SP(_t) (_t)->md.context[0].__jmpbuf[0].__misc[0]
#endif /* __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 1 */
#define _MD_SET_FP(_t, val)
#define _MD_GET_SP_PTR(_t) &(_MD_GET_SP(_t))
#define _MD_GET_FP_PTR(_t) ((void *) 0)
/* aix = 64, macos = 70 */
#define PR_NUM_GCREGS  64

#elif defined(__alpha)
/* Alpha based Linux */

#if defined(__GLIBC__) && __GLIBC__ >= 2
#define _MD_GET_SP(_t) (_t)->md.context[0].__jmpbuf[JB_SP]
#define _MD_SET_FP(_t, val)
#define _MD_GET_SP_PTR(_t) &(_MD_GET_SP(_t))
#define _MD_GET_FP_PTR(_t) ((void *) 0)
#define _MD_SP_TYPE long int
#else
#define _MD_GET_SP(_t) (_t)->md.context[0].__jmpbuf[0].__sp
#define _MD_SET_FP(_t, val)
#define _MD_GET_SP_PTR(_t) &(_MD_GET_SP(_t))
#define _MD_GET_FP_PTR(_t) ((void *) 0)
#define _MD_SP_TYPE __ptr_t
#endif /* defined(__GLIBC__) && __GLIBC__ >= 2 */

/* XXX not sure if this is correct, or maybe it should be 17? */
#define PR_NUM_GCREGS 9

#elif defined(__mc68000__)
/* m68k based Linux */

/*
 * On the m68k, glibc still uses the old style sigjmp_buf, even
 * in glibc 2.0.7.
 */
#if defined(__GLIBC__) && __GLIBC__ >= 2
#define _MD_GET_SP(_t) (_t)->md.context[0].__jmpbuf[0].__sp
#define _MD_SET_FP(_t, val)
#define _MD_GET_SP_PTR(_t) &(_MD_GET_SP(_t))
#define _MD_GET_FP_PTR(_t) ((void *) 0)
#define _MD_SP_TYPE int
#else
#define _MD_GET_SP(_t) (_t)->md.context[0].__jmpbuf[0].__sp
#define _MD_SET_FP(_t, val)
#define _MD_GET_SP_PTR(_t) &(_MD_GET_SP(_t))
#define _MD_GET_FP_PTR(_t) ((void *) 0)
#define _MD_SP_TYPE __ptr_t
#endif /* defined(__GLIBC__) && __GLIBC__ >= 2 */

/* XXX not sure if this is correct, or maybe it should be 17? */
#define PR_NUM_GCREGS 9

#elif defined(__sparc__)
/* Sparc */
#if defined(__GLIBC__) && __GLIBC__ >= 2
/*
 * FIXME: The setjmp/longjmp approach works in Red Hat 4.2 but
 * not in Red Hat 5.1 for Linux/Sparc.  We should talk to the
 * maintainer of Linux/Sparc glibc2 about this.  Or we can write
 * assembly code to save and restore thread contexts.  Before
 * this problem is solved, you can build the pthreads version.
 * Set the environment variable USE_PTHREADS to 1 before you
 * build.
 */
#error "Please see comments in pr/include/md/_linux.h"
#define _MD_GET_SP(_t) (_t)->md.context[0].__jmpbuf[JB_SP]
#define _MD_SET_FP(_t, val) ((_t)->md.context[0].__jmpbuf[JB_FP] = val)
#define _MD_GET_SP_PTR(_t) &(_MD_GET_SP(_t))
#define _MD_GET_FP_PTR(_t) (&(_t)->md.context[0].__jmpbuf[JB_FP])
#define _MD_SP_TYPE int
#else
#define _MD_GET_SP(_t) (_t)->md.context[0].__jmpbuf[0].__fp
#define _MD_SET_FP(_t, val)
#define _MD_GET_SP_PTR(_t) &(_MD_GET_SP(_t))
#define _MD_GET_FP_PTR(_t) ((void *) 0)
#define _MD_SP_TYPE __ptr_t
#endif /* defined(__GLIBC__) && __GLIBC__ >= 2 */

#elif defined(__i386__)
/* Intel based Linux */
#if defined(__GLIBC__) && __GLIBC__ >= 2
#define _MD_GET_SP(_t) (_t)->md.context[0].__jmpbuf[JB_SP]
#define _MD_SET_FP(_t, val) ((_t)->md.context[0].__jmpbuf[JB_BP] = val)
#define _MD_GET_SP_PTR(_t) &(_MD_GET_SP(_t))
#define _MD_GET_FP_PTR(_t) (&(_t)->md.context[0].__jmpbuf[JB_BP])
#define _MD_SP_TYPE int
#else
#define _MD_GET_SP(_t) (_t)->md.context[0].__jmpbuf[0].__sp
#define _MD_SET_FP(_t, val) ((_t)->md.context[0].__jmpbuf[0].__bp = val)
#define _MD_GET_SP_PTR(_t) &(_MD_GET_SP(_t))
#define _MD_GET_FP_PTR(_t) &((_t)->md.context[0].__jmpbuf[0].__bp)
#define _MD_SP_TYPE __ptr_t
#endif /* defined(__GLIBC__) && __GLIBC__ >= 2 */
#define PR_NUM_GCREGS   6

#elif defined(__mips__)
/* Linux/MIPS */
#if defined(__GLIBC__) && __GLIBC__ >= 2
#define _MD_GET_SP(_t) (_t)->md.context[0].__jmpbuf[0].__sp
#define _MD_SET_FP(_t, val) ((_t)->md.context[0].__jmpbuf[0].__fp = (val))
#define _MD_GET_SP_PTR(_t) &(_MD_GET_SP(_t))
#define _MD_GET_FP_PTR(_t) (&(_t)->md.context[0].__jmpbuf[0].__fp)
#define _MD_SP_TYPE __ptr_t
#else
#error "Linux/MIPS pre-glibc2 not supported yet"
#endif /* defined(__GLIBC__) && __GLIBC__ >= 2 */

#elif defined(__arm__)
/* ARM/Linux */
#if defined(__GLIBC__) && __GLIBC__ >= 2
#define _MD_GET_SP(_t) (_t)->md.context[0].__jmpbuf[20]
#define _MD_SET_FP(_t, val) ((_t)->md.context[0].__jmpbuf[19] = (val))
#define _MD_GET_SP_PTR(_t) &(_MD_GET_SP(_t))
#define _MD_GET_FP_PTR(_t) (&(_t)->md.context[0].__jmpbuf[19])
#define _MD_SP_TYPE __ptr_t
#else
#error "ARM/Linux pre-glibc2 not supported yet"
#endif /* defined(__GLIBC__) && __GLIBC__ >= 2 */

#else

#error "Unknown CPU architecture"

#endif /*__powerpc__*/

/*
** Initialize a thread context to run "_main()" when started
*/
#ifdef __powerpc__

#define _MD_INIT_CONTEXT(_thread, _sp, _main, status)  \
{  \
    *status = PR_TRUE;  \
    if (sigsetjmp(CONTEXT(_thread), 1)) {  \
        _main();  \
    }  \
    _MD_GET_SP(_thread) = (unsigned char*) ((_sp) - 128); \
	_thread->md.sp = _MD_GET_SP_PTR(_thread); \
	_thread->md.fp = _MD_GET_FP_PTR(_thread); \
    _MD_SET_FP(_thread, 0); \
}

#elif defined(__mips__)

#define _MD_INIT_CONTEXT(_thread, _sp, _main, status)  \
{  \
    *status = PR_TRUE;  \
    (void) sigsetjmp(CONTEXT(_thread), 1);  \
    _thread->md.context[0].__jmpbuf[0].__pc = (__ptr_t) _main;  \
    _MD_GET_SP(_thread) = (_MD_SP_TYPE) ((_sp) - 64); \
    _thread->md.sp = _MD_GET_SP_PTR(_thread); \
    _thread->md.fp = _MD_GET_FP_PTR(_thread); \
    _MD_SET_FP(_thread, 0); \
}

#else

#define _MD_INIT_CONTEXT(_thread, _sp, _main, status)  \
{  \
    *status = PR_TRUE;  \
    if (sigsetjmp(CONTEXT(_thread), 1)) {  \
        _main();  \
    }  \
    _MD_GET_SP(_thread) = (_MD_SP_TYPE) ((_sp) - 64); \
	_thread->md.sp = _MD_GET_SP_PTR(_thread); \
	_thread->md.fp = _MD_GET_FP_PTR(_thread); \
    _MD_SET_FP(_thread, 0); \
}

#endif /*__powerpc__*/

#define _MD_SWITCH_CONTEXT(_thread)  \
    if (!sigsetjmp(CONTEXT(_thread), 1)) {  \
	(_thread)->md.errcode = errno;  \
	_PR_Schedule();  \
    }

/*
** Restore a thread context, saved by _MD_SWITCH_CONTEXT
*/
#define _MD_RESTORE_CONTEXT(_thread) \
{   \
    errno = (_thread)->md.errcode;  \
    _MD_SET_CURRENT_THREAD(_thread);  \
    siglongjmp(CONTEXT(_thread), 1);  \
}

/* Machine-dependent (MD) data structures */

struct _MDThread {
    PR_CONTEXT_TYPE context;
	void *sp;
	void *fp;
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
#include <sys/time.h>  /* for FD_SETSIZE */
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

extern PRStatus _MD_InitializeThread(PRThread *thread);

#define _MD_INIT_RUNNING_CPU(cpu)       _MD_unix_init_running_cpu(cpu)
#define _MD_INIT_THREAD                 _MD_InitializeThread
#define _MD_EXIT_THREAD(thread)
#define _MD_SUSPEND_THREAD(thread)      _MD_suspend_thread
#define _MD_RESUME_THREAD(thread)       _MD_resume_thread
#define _MD_CLEAN_THREAD(_thread)

extern PRStatus _MD_CREATE_THREAD(
    PRThread *thread,
    void (*start) (void *),
    PRThreadPriority priority,
    PRThreadScope scope,
    PRThreadState state,
    PRUint32 stackSize);
extern void _MD_SET_PRIORITY(struct _MDThread *thread, PRUintn newPri);
extern PRStatus _MD_WAIT(PRThread *, PRIntervalTime timeout);
extern PRStatus _MD_WAKEUP_WAITER(PRThread *);
extern void _MD_YIELD(void);

#endif /* ! _PR_PTHREADS */

extern void _MD_EarlyInit(void);
extern PRIntervalTime _PR_UNIX_GetInterval(void);
extern PRIntervalTime _PR_UNIX_TicksPerSecond(void);

#define _MD_EARLY_INIT                  _MD_EarlyInit
#define _MD_FINAL_INIT					_PR_UnixInit
#define _MD_GET_INTERVAL                _PR_UNIX_GetInterval
#define _MD_INTERVAL_PER_SEC            _PR_UNIX_TicksPerSecond

/*
 * We wrapped the select() call.  _MD_SELECT refers to the built-in,
 * unwrapped version.
 */
#define _MD_SELECT __select

#ifdef _PR_POLL_AVAILABLE
#include <sys/poll.h>
extern int __syscall_poll(struct pollfd *ufds, unsigned long int nfds,
	int timeout);
#define _MD_POLL __syscall_poll
#endif

/* For writev() */
#include <sys/uio.h>

#endif /* nspr_linux_defs_h___ */
