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
 *  Copyright (c) 1990 Regents of the University of Michigan.
 *  All rights reserved.
 */
/*
 *  modify.c
 */

#if 0
#ifndef lint 
static char copyright[] = "@(#) Copyright (c) 1990 Regents of the University of Michigan.\nAll rights reserved.\n";
#endif
#endif

#include "ldap-int.h"

/*
 * ldap_modify - initiate an ldap (and X.500) modify operation.  Parameters:
 *
 *	ld		LDAP descriptor
 *	dn		DN of the object to modify
 *	mods		List of modifications to make.  This is null-terminated
 *			array of struct ldapmod's, specifying the modifications
 *			to perform.
 *
 * Example:
 *	LDAPMod	*mods[] = { 
 *			{ LDAP_MOD_ADD, "cn", { "babs jensen", "babs", 0 } },
 *			{ LDAP_MOD_REPLACE, "sn", { "jensen", 0 } },
 *			0
 *		}
 *	msgid = ldap_modify( ld, dn, mods );
 */
int
LDAP_CALL
ldap_modify( LDAP *ld, const char *dn, LDAPMod **mods )
{
	int		msgid;

	LDAPDebug( LDAP_DEBUG_TRACE, "ldap_modify\n", 0, 0, 0 );

	if ( ldap_modify_ext( ld, dn, mods, NULL, NULL, &msgid )
	    == LDAP_SUCCESS ) {
		return( msgid );
	} else {
		return( -1 );	/* error is in ld handle */
	}
}

int
LDAP_CALL
ldap_modify_ext( LDAP *ld, const char *dn, LDAPMod **mods,
    LDAPControl **serverctrls, LDAPControl **clientctrls, int *msgidp )
{
	BerElement	*ber;
	int		i, rc, lderr;

	/*
	 * A modify request looks like this:
	 *	ModifyRequet ::= SEQUENCE {
	 *		object		DistinguishedName,
	 *		modifications	SEQUENCE OF SEQUENCE {
	 *			operation	ENUMERATED {
	 *				add	(0),
	 *				delete	(1),
	 *				replace	(2)
	 *			},
	 *			modification	SEQUENCE {
	 *				type	AttributeType,
	 *				values	SET OF AttributeValue
	 *			}
	 *		}
	 *	}
	 */

	LDAPDebug( LDAP_DEBUG_TRACE, "ldap_modify_ext\n", 0, 0, 0 );

	if ( !NSLDAPI_VALID_LDAP_POINTER( ld )) {
		return( LDAP_PARAM_ERROR );
	}

	if ( !NSLDAPI_VALID_NONEMPTY_LDAPMOD_ARRAY( mods )) {
		lderr = LDAP_PARAM_ERROR;
		LDAP_SET_LDERRNO( ld, lderr, NULL, NULL );
		return( lderr );
	}
	if ( dn == NULL ) {
		dn = "";
	}

	LDAP_MUTEX_LOCK( ld, LDAP_MSGID_LOCK );
	*msgidp = ++ld->ld_msgid;
	LDAP_MUTEX_UNLOCK( ld, LDAP_MSGID_LOCK );

	/* see if we should add to the cache */
	if ( ld->ld_cache_on && ld->ld_cache_modify != NULL ) {
		LDAP_MUTEX_LOCK( ld, LDAP_CACHE_LOCK );
		if ( (rc = (ld->ld_cache_modify)( ld, *msgidp, LDAP_REQ_MODIFY,
		    dn, mods )) != 0 ) {
			*msgidp = rc;
			LDAP_MUTEX_LOCK( ld, LDAP_CACHE_LOCK );
			return( LDAP_SUCCESS );
		}
		LDAP_MUTEX_UNLOCK( ld, LDAP_CACHE_LOCK );
	}

	/* create a message to send */
	if (( lderr = nsldapi_alloc_ber_with_options( ld, &ber ))
	    != LDAP_SUCCESS ) {
		return( lderr );
	}

	if ( ber_printf( ber, "{it{s{", *msgidp, LDAP_REQ_MODIFY, dn )
	    == -1 ) {
		lderr = LDAP_ENCODING_ERROR;
		LDAP_SET_LDERRNO( ld, lderr, NULL, NULL );
		ber_free( ber, 1 );
		return( lderr );
	}

	/* for each modification to be performed... */
	for ( i = 0; mods[i] != NULL; i++ ) {
		if (( mods[i]->mod_op & LDAP_MOD_BVALUES) != 0 ) {
			rc = ber_printf( ber, "{e{s[V]}}",
			    mods[i]->mod_op & ~LDAP_MOD_BVALUES,
			    mods[i]->mod_type, mods[i]->mod_bvalues );
		} else {
			rc = ber_printf( ber, "{e{s[v]}}", mods[i]->mod_op,
			    mods[i]->mod_type, mods[i]->mod_values );
		}

		if ( rc == -1 ) {
			lderr = LDAP_ENCODING_ERROR;
			LDAP_SET_LDERRNO( ld, lderr, NULL, NULL );
			ber_free( ber, 1 );
			return( lderr );
		}
	}

	if ( ber_printf( ber, "}}" ) == -1 ) {
		lderr = LDAP_ENCODING_ERROR;
		LDAP_SET_LDERRNO( ld, lderr, NULL, NULL );
		ber_free( ber, 1 );
		return( lderr );
	}

	if (( lderr = nsldapi_put_controls( ld, serverctrls, 1, ber ))
	    != LDAP_SUCCESS ) {
		ber_free( ber, 1 );
		return( lderr );
	}

	/* send the message */
	rc = nsldapi_send_initial_request( ld, *msgidp, LDAP_REQ_MODIFY,
		(char *)dn, ber );
	*msgidp = rc;
	return( rc < 0 ? LDAP_GET_LDERRNO( ld, NULL, NULL ) : LDAP_SUCCESS );
}

int
LDAP_CALL
ldap_modify_s( LDAP *ld, const char *dn, LDAPMod **mods )
{
	return( ldap_modify_ext_s( ld, dn, mods, NULL, NULL ));
}

int
LDAP_CALL
ldap_modify_ext_s( LDAP *ld, const char *dn, LDAPMod **mods,
    LDAPControl **serverctrls, LDAPControl **clientctrls )
{
	int		msgid, err;
	LDAPMessage	*res;

	if (( err = ldap_modify_ext( ld, dn, mods, serverctrls, clientctrls,
	    &msgid )) != LDAP_SUCCESS ) {
		return( err );
	}

	if ( ldap_result( ld, msgid, 1, (struct timeval *)NULL, &res ) == -1 ) {
		return( LDAP_GET_LDERRNO( ld, NULL, NULL ) );
	}

	return( ldap_result2error( ld, res, 1 ) );
}
