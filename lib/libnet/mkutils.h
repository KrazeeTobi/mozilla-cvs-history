/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
#ifndef MKUTILS_H
#define MKUTILS_H

/* Used throughout netlib. */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

/* use the fancy new mail reading stuff
 */
#undef NEW_MAIL_STUFF

#include "net.h"
#include "xp.h"                 /* The cross-platform API */
#include "mktrace.h"
#include "mkselect.h"           /* needed by all files in netlib */

#define CONST const             /* "const" only exists in STDC? */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#ifdef XP_UNIX
#include <unistd.h>
#include <signal.h>
#endif /* XP_UNIX */
/*#include "version.h"*/

#ifndef NSPR20
#if defined(__sun)
# include "sunos4.h"
#endif /* __sun */
#endif /* NSPR20 */

#define FREE_AND_CLEAR(x) NET_f_a_c(&x) 

XP_BEGIN_PROTOS
typedef struct WritePostDataData WritePostDataData;
extern void NET_free_write_post_data_object(struct WritePostDataData *obj);

PUBLIC void 
NET_SetDNSExpirationPref(int32 n);
MODULE_PRIVATE int PR_CALLBACK
NET_DNSExpirationPrefChanged(const char * newpref, void * data);

extern int NET_URL_Type (CONST char *URL);
extern void del_front_spaces (char *string);
extern void NET_f_a_c (char **obj);

extern void NET_ParseContentTypeHeader(MWContext *context, char *value, URL_Struct *URL_s, XP_Bool is_http);

/*
 * This function takes an error code and associated error data
 * and creates a string containing a textual description of
 * what the error is and why it happened.
 *
 * The returned string is allocated and thus should be freed
 * once it has been used.
 *
 * This function is defined in mkmessag.c.
 */
extern char * NET_ExplainErrorDetails (int code, ...);

/* Find an Author's mail address
 *
 * THE EMAIL ADDRESS IS CORRUPTED
 *
 * For example, returns "montulli@netscape.com" if given any of
 *      " Lou Montullism <montulli@netscape.com> "
 *  or  " montulli@netscape.com ( Lou "The Stud :)" Montulli ) "
 *  or  "montulli@netscape.com"
*/
extern char * NET_EmailAddress (char *email);

/* this function is called repeatably to write
 * the post data body onto the network.
 *
 * Returns negative on fatal error
 * Returns zero when done
 * Returns positive if not yet completed
 */
extern int
NET_WritePostData(MWContext  *context,
				  URL_Struct *URL_s,
                  PRFileDesc     *sock,
                  void      **write_post_data_data,
                  XP_Bool     add_crlf_to_line_endings);

extern char * NET_SpaceToPlus(char * string);

/* returns true if the functions thinks the string contains
 * HTML
 */
extern Bool NET_ContainsHTML(char * string, int32 length);

/* try to make sure that this is a fully qualified
 * email address including a host and domain
 */
extern Bool NET_IsFQDNMailAddress(const char * string);

extern int NET_UUEncode(unsigned char *src, unsigned char *dst, int srclen);

extern void NET_Progress(MWContext *context, char *msg);

/*
#define AUTH_SKEY_DEFINED 
*/
#ifdef AUTH_SKEY_DEFINED
/* MD4 SKEY 64-bit password crunching and hashing function
 * see skey.h for more details information.
 */
extern int keycrunch(char *result, char *seed, char *passwd);
extern void f(char *x);
#endif

/* returns Base64 encoded string
 * caller must free
 */
extern char * NET_Base64Encode(char *src, int32 srclen);

/* returns Base64 decoded string
 * caller must free
 */
extern char * NET_Base64Decode(char *src, int32 srclen);

/* a stub function for URN protocol converter.
 * right now we only proxy URN's
 * if URN's ever get worked on move this to another file
 */
extern void NET_InitURNProtocol(void);

/* a stub function for NFS protocol converter.
 * right now we only proxy NFS's
 * if NFS ever gets worked on move this to another file
 */
extern void NET_InitNFSProtocol(void);

/* a stub function for WAIS protocol converter.
 * right now we only proxy WAIS 
 * if WAIS ever gets worked on move this to another file
 */
extern void NET_InitWAISProtocol(void);

XP_END_PROTOS
#ifndef FREE
#define FREE(obj)    XP_FREE(obj)
#endif
#ifndef FREEIF
#define FREEIF(obj)  {if(obj) {XP_FREE(obj); obj=0;}}
#endif

/* A utility function to fetch a file from cache right away, 
 * and update it (from the original server) after its used.
 * Used in/Required by Mr. R.D.F. Guha 
 *
 * Note that if the item is not in the cache, this defaults
 * to retrieving it from the server.
 *
 * Variables and returns : see NET_GetURL in include/net.h
 *
 */
extern int
NET_GetURLQuick (URL_Struct * URL_s,
        FO_Present_Types output_format,
        MWContext * context,
        Net_GetUrlExitFunc*	exit_routine);

#endif /* MKUTILS_H */


