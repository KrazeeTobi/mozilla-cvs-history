/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
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
/*
 * macos.h: bridge unix and Mac for  LBER/LDAP
 */
#ifndef __LDAP_MACOS_H__
#define __LDAP_MACOS_H__
#if 0
		/*------------------*/
		#define ntohl( l )	(l)
		#define htonl( l )	(l)
		#define ntohs( s )	(s)
		#define htons( s )	(s)
		/*------------------*/
#endif

#ifdef NO_GLOBALS

#ifdef macintosh	/* IUMagIDString declared in TextUtils.h under MPW */
#include <TextUtils.h>
#else /* macintosh */	/* IUMagIDString declared in Packages.h under ThinkC */
#include <Packages.h>
#endif /* macintosh */

#define strcasecmp( s1, s2 )	IUMagIDString( s1, s2, strlen( s1 ), \
					strlen( s2 ))
#else /* NO_GLOBALS */
int strcasecmp(  const char *s1,  const char *s2 );
int strncasecmp(  const char *s1,  const char *s2,  const long n );
#endif NO_GLOBALS

#include <Memory.h>	/* to get BlockMove() */

char *strdup( const char *s );

#ifndef isascii
#define isascii(c)	((unsigned)(c)<=0177)	/* for those who don't have this in ctype.h */
#endif isascii

/*
 * if we aren't supporting OpenTransport, we need to define some standard "errno" values here
 * the values should match those in OpenTransport.h
 */
#if !defined( SUPPORT_OPENTRANSPORT )
#define EHOSTUNREACH	65	/* No route to host */
#define EAGAIN		11	/* Resource temporarily unavailable */
#define EWOULDBLOCK	35	/* ditto */
#endif

int getopt(int nargc, char **nargv, char *ostr);

#include <time.h>
#include <stdlib.h>

#include "macsocket.h"
#include "tcp.h"
#if 0
		/*------------------*/
		#ifndef AF_INET
		/* these next few things are defined only for use by the LDAP socket I/O function callbacks */
		#define AF_INET			2
		#define SOCK_STREAM		1
		struct in_addr {
			unsigned long s_addr;
		};
		struct sockaddr_in {
			short			sin_family;
			unsigned short	sin_port;
			struct in_addr	sin_addr;
			char			sin_zero[8];
		};
		#endif
		/*------------------*/
#endif


#endif /* __LDAP_MACOS_H__ */
