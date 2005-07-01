/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Netscape Portable Runtime (NSPR).
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998-2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Peter Naulls <peter@chocky.org>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef nspr_riscos_defs_h___
#define nspr_riscos_defs_h___

/*
** Internal configuration macros
*/
#define PR_LINKER_ARCH		"riscos"
#define _PR_SI_SYSNAME		"RISCOS"
#define _PR_SI_ARCHITECTURE	"arm"
#define PR_DLL_SUFFIX		".a"

#define _PR_POLL_AVAILABLE
#define _PR_USE_POLL
#define _PR_HAVE_SOCKADDR_LEN
#undef HAVE_BSD_FLOCK
#define _PR_NO_LARGE_FILES
#define _PR_STAT_HAS_ONLY_ST_ATIME
#define _PR_HAVE_POSIX_SEMAPHORES

#define uint unsigned int

#undef atoll

#include <sys/select.h>
#include <sys/poll.h>
#include <kernel.h>


#undef  HAVE_STACK_GROWING_UP
#undef	HAVE_DLL
#undef	USE_DLFCN
#define NEED_STRFTIME_LOCK
#define NEED_TIME_R
#define PT_NO_SIGTIMEDWAIT

#ifndef HAVE_STRERROR
#define HAVE_STRERROR
#endif

#define USE_SETJMP

#include <setjmp.h>

#define _SETJMP			setjmp
#define _LONGJMP		longjmp
#define _PR_CONTEXT_TYPE	jmp_buf
#define _PR_NUM_GCREGS		_JBLEN
#define _MD_GET_SP(_t)		(_t)->md.context[7]

#define CONTEXT(_th)		((_th)->md.context)


/*
** Initialize the thread context preparing it to execute _main.
*/
#define _MD_INIT_CONTEXT(_thread, _sp, _main, status)	\
{							\
    *status = PR_TRUE;					\
    if(_SETJMP(CONTEXT(_thread))) (*_main)();		\
    _MD_GET_SP(_thread) = (int) ((_sp) - 128);		\
}

#define _MD_SWITCH_CONTEXT(_thread)	\
    if (!_SETJMP(CONTEXT(_thread))) {	\
	(_thread)->md.errcode = errno;	\
	_PR_Schedule();			\
    }

/*
** Restore a thread context, saved by _MD_SWITCH_CONTEXT
*/
#define _MD_RESTORE_CONTEXT(_thread)	\
{					\
    errno = (_thread)->md.errcode;	\
    _MD_SET_CURRENT_THREAD(_thread);	\
    _LONGJMP(CONTEXT(_thread), 1);	\
}

/*
** Machine-dependent (MD) data structures.
*/
struct _MDThread {
    _PR_CONTEXT_TYPE context;
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
** md-specific cpu structure field
*/
#define _PR_MD_MAX_OSFD		FD_SETSIZE

struct _MDCPU_Unix {
    PRCList ioQ;
    PRUint32 ioq_timeout;
    PRInt32 ioq_max_osfd;
    PRInt32 ioq_osfd_cnt;
#ifndef _PR_USE_POLL
    fd_set fd_read_set, fd_write_set, fd_exception_set;
    PRInt16 fd_read_cnt[_PR_MD_MAX_OSFD], fd_write_cnt[_PR_MD_MAX_OSFD], fd_exception_cnt[_PR_MD_MAX_OSFD];
#else
    struct pollfd *ioq_pollfds;
    int ioq_pollfds_size;
#endif
};

#define _PR_IOQ(_cpu)	/*	*/	((_cpu)->md.md_unix.ioQ)
#define _PR_ADD_TO_IOQ(_pq, _cpu)	PR_APPEND_LINK(&_pq.links, &_PR_IOQ(_cpu))
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
#define _MD_NEW_LOCK(lock)		PR_SUCCESS
#define _MD_FREE_LOCK(lock)
#define _MD_LOCK(lock)
#define _MD_UNLOCK(lock)
#define _MD_INIT_IO()
#define _MD_IOQ_LOCK()
#define _MD_IOQ_UNLOCK()

#define _MD_GET_INTERVAL		_PR_UNIX_GetInterval
#define _MD_INTERVAL_PER_SEC		_PR_UNIX_TicksPerSecond
#define _MD_EARLY_INIT			_MD_EarlyInit
#define _MD_FINAL_INIT			_PR_UnixInit
#define _MD_INIT_RUNNING_CPU(cpu)	_MD_unix_init_running_cpu(cpu)
#define _MD_INIT_THREAD			_MD_InitializeThread
#define _MD_EXIT_THREAD(thread)
#define	_MD_SUSPEND_THREAD(thread)
#define	_MD_RESUME_THREAD(thread)
#define _MD_CLEAN_THREAD(_thread)

/*
** We wrapped the select() call.  _MD_SELECT refers to the built-in,
** unwrapped version.
*/
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#define _MD_SELECT		select

#endif /* nspr_riscos_defs_h___ */
