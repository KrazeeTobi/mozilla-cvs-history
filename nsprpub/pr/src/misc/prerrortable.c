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
/*

Copyright 1987, 1988 by the Student Information Processing Board
	of the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this software
and its documentation for any purpose and without fee is
hereby granted, provided that the above copyright notice
appear in all copies and that both that copyright notice and
this permission notice appear in supporting documentation,
and that the names of M.I.T. and the M.I.T. S.I.P.B. not be
used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.
M.I.T. and the M.I.T. S.I.P.B. make no representations about
the suitability of this software for any purpose.  It is
provided "as is" without express or implied warranty.

*/

#include <string.h>
#ifdef SUNOS4
#include "md/sunos4.h"  /* for strerror */
#endif
#include <assert.h>
#include <errno.h>
#include "prmem.h"
#include "prerror.h"
#include "prerrorplugin.h"

#define	ERRCODE_RANGE	8	/* # of bits to shift table number */
#define	BITS_PER_CHAR	6	/* # bits to shift per character in name */

#ifdef NEED_SYS_ERRLIST
extern char const * const sys_errlist[];
extern const int sys_nerr;
#endif

/* List of error tables */
struct PRErrorTableList {
    struct PRErrorTableList *next;
    const struct PRErrorTable *table;
    struct PRErrorPluginTableRock *table_rock;
};
static struct PRErrorTableList * Table_List = (struct PRErrorTableList *) NULL;

/* Supported languages */
static const char * default_languages[] = { "i-default", "en", 0 };
static const char * const * plugin_languages = default_languages;

/* Plugin info */
static struct PRErrorPluginRock *plugin_rock = 0;
static PRErrorPluginLookupFn *plugin_lookup = 0;
static PRErrorPluginNewtableFn *plugin_newtable = 0;


static const char char_set[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_";

static const char *
error_table_name (PRErrorCode num)
{
    static char buf[6];	/* only used if internal code problems exist */

    long ch;
    int i;
    char *p;

    /* num = aa aaa abb bbb bcc ccc cdd ddd d?? ??? ??? */
    p = buf;
    num >>= ERRCODE_RANGE;
    /* num = ?? ??? ??? aaa aaa bbb bbb ccc ccc ddd ddd */
    num &= 077777777;
    /* num = 00 000 000 aaa aaa bbb bbb ccc ccc ddd ddd */
    for (i = 4; i >= 0; i--) {
	ch = (num >> BITS_PER_CHAR * i) & ((1 << BITS_PER_CHAR) - 1);
	if (ch != 0)
	    *p++ = char_set[ch-1];
    }
    *p = '\0';
    return(buf);
}



PR_IMPLEMENT(const char *)
PR_ErrorToString(PRErrorCode code, PRLanguageCode language)
{
    /* static buffer only used if code is using inconsistent error message
     * numbers, so just ignore the possible thread contention
     */
    static char buffer[25];

    const char *msg;
    int offset;
    PRErrorCode table_num;
    struct PRErrorTableList *et;
    int started = 0;
    char *cp;

    for (et = Table_List; et; et = et->next) {
	if (et->table->base <= code &&
	    et->table->base + et->table->n_msgs > code) {
	    /* This is the right table */
	    if (plugin_lookup) {
		msg = plugin_lookup(code, language, et->table,
		    plugin_rock, et->table_rock);
		if (msg) return msg;
	    }
    
	    return(et->table->msgs[code - et->table->base].en_text);
	}
    }

    if (code >= 0 && code < 256) {
	return strerror(code);
    }

    offset = (int) (code & ((1<<ERRCODE_RANGE)-1));
    table_num = code - offset;
    strcpy (buffer, "Unknown code ");
    if (table_num) {
	strcat(buffer, error_table_name (table_num));
	strcat(buffer, " ");
    }
    for (cp = buffer; *cp; cp++)
	;
    if (offset >= 100) {
	*cp++ = (char)('0' + offset / 100);
	offset %= 100;
	started++;
    }
    if (started || offset >= 10) {
	*cp++ = (char)('0' + offset / 10);
	offset %= 10;
    }
    *cp++ = (char)('0' + offset);
    *cp = '\0';
    return(buffer);
}

PR_IMPLEMENT(const char * const *)
PR_ErrorLanguages(void)
{
    return plugin_languages;
}

PR_IMPLEMENT(PRErrorCode)
PR_ErrorInstallTable(const struct PRErrorTable *table)
{
    struct PRErrorTableList * new_et;

    new_et = (struct PRErrorTableList *)
					PR_Malloc(sizeof(struct PRErrorTableList));
    if (!new_et)
	return errno;	/* oops */
    new_et->table = table;
    if (plugin_newtable) {
	new_et->table_rock = plugin_newtable(table, plugin_rock);
    } else {
	new_et->table_rock = 0;
    }
    new_et->next = Table_List;
    Table_List = new_et;
    return 0;
}

PR_IMPLEMENT(void)
PR_ErrorInstallPlugin(const char * const * languages,
		       PRErrorPluginLookupFn *lookup, 
		       PRErrorPluginNewtableFn *newtable,
		       struct PRErrorPluginRock *rock)
{
    struct PRErrorTableList *et;

    assert(strcmp(languages[0], "i-default") == 0);
    assert(strcmp(languages[1], "en") == 0);
    
    plugin_languages = languages;
    plugin_lookup = lookup;
    plugin_newtable = newtable;
    plugin_rock = rock;

    if (plugin_newtable) {
	for (et = Table_List; et; et = et->next) {
	    et->table_rock = plugin_newtable(et->table, plugin_rock);
	}
    }
}
